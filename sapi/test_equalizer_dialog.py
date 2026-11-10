"""Exercise the real SAPI equalizer dialogs with ordinary Win32 controls."""
import argparse
import ctypes
from ctypes import wintypes
import os
from pathlib import Path
import threading
import time


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--directory',type=Path,required=True)
    directory=parser.parse_args().directory.resolve()
    u=ctypes.WinDLL('user32',use_last_error=True)
    cb=ctypes.WINFUNCTYPE(wintypes.BOOL,wintypes.HWND,wintypes.LPARAM)
    u.EnumWindows.argtypes=(cb,wintypes.LPARAM)
    u.GetWindowThreadProcessId.argtypes=(wintypes.HWND,ctypes.POINTER(wintypes.DWORD))
    u.GetDlgItem.argtypes=(wintypes.HWND,ctypes.c_int);u.GetDlgItem.restype=wintypes.HWND
    u.GetNextDlgTabItem.argtypes=(wintypes.HWND,wintypes.HWND,wintypes.BOOL);u.GetNextDlgTabItem.restype=wintypes.HWND
    u.IsWindow.argtypes=(wintypes.HWND,);u.IsWindow.restype=wintypes.BOOL
    u.IsWindowEnabled.argtypes=(wintypes.HWND,);u.IsWindowEnabled.restype=wintypes.BOOL
    u.SendMessageW.argtypes=(wintypes.HWND,wintypes.UINT,wintypes.WPARAM,wintypes.LPARAM);u.SendMessageW.restype=ctypes.c_ssize_t
    u.PostMessageW.argtypes=(wintypes.HWND,wintypes.UINT,wintypes.WPARAM,wintypes.LPARAM)
    u.SetDlgItemInt.argtypes=(wintypes.HWND,ctypes.c_int,wintypes.UINT,wintypes.BOOL)
    u.GetDlgItemInt.argtypes=(wintypes.HWND,ctypes.c_int,ctypes.POINTER(wintypes.BOOL),wintypes.BOOL);u.GetDlgItemInt.restype=wintypes.UINT
    module=ctypes.WinDLL(str(directory/'ReLoqSapi.dll'),use_last_error=True)
    show=module.ReLoqSettings;show.argtypes=(wintypes.HWND,ctypes.c_int);show.restype=ctypes.c_long
    result=[];worker=threading.Thread(target=lambda:result.append(show(None,-1)),daemon=True);worker.start()
    EQ_DIALOG,EQ_ENABLED,EQ_PRESET,EQ_LOW,EQ_MID,EQ_HIGH,EQ_ADVANCED=range(141,148)
    FILTER_SELECT,FILTER_ENABLED,FILTER_FREQ,FILTER_GAIN,FILTER_WIDTH=range(148,153)
    BM_CLICK,BM_GETCHECK,WM_CLOSE,WM_COMMAND=0xF5,0xF0,0x10,0x111
    CB_SETCURSEL,CB_GETCURSEL=0x14E,0x147
    def find(control):
        found=[]
        @cb
        def visit(h,unused):
            pid=wintypes.DWORD();u.GetWindowThreadProcessId(h,ctypes.byref(pid))
            if pid.value==os.getpid() and u.GetDlgItem(h,control):found.append(h)
            return True
        u.EnumWindows(visit,0)
        return found[0] if found else None
    def wait(control):
        deadline=time.monotonic()+10
        while time.monotonic()<deadline:
            h=find(control)
            if h:
                needed=((EQ_LOW,EQ_MID,EQ_HIGH) if control==EQ_ENABLED else
                        (FILTER_FREQ,FILTER_GAIN,FILTER_WIDTH) if control==FILTER_SELECT else ())
                ready=bool(u.GetDlgItem(h,1))
                for id in needed:
                    ok=wintypes.BOOL();u.GetDlgItemInt(h,id,ctypes.byref(ok),True)
                    ready=ready and bool(ok.value)
                if ready:return h
            if not worker.is_alive():raise AssertionError(f'dialog exited: {result}')
            time.sleep(.05)
        raise AssertionError(f'dialog control {control} not found')
    def click(h,id):
        widget=u.GetDlgItem(h,id);assert widget,id;u.SendMessageW(widget,BM_CLICK,0,0)
    def checked(h,id):return u.SendMessageW(u.GetDlgItem(h,id),BM_GETCHECK,0,0)==1
    def setcheck(h,id,enable):
        if checked(h,id)!=enable:click(h,id)
        assert checked(h,id)==enable
    def number(h,id,signed=False):
        ok=wintypes.BOOL();v=u.GetDlgItemInt(h,id,ctypes.byref(ok),signed);assert ok.value,id
        return ctypes.c_int32(v).value if signed else v
    def setnumber(h,id,value,signed=False):
        assert u.SetDlgItemInt(h,id,ctypes.c_uint(value).value,signed)
    def select(h,id,index):
        widget=u.GetDlgItem(h,id);assert widget
        assert u.SendMessageW(widget,CB_SETCURSEL,index,0)>=0
        u.SendMessageW(h,WM_COMMAND,id|(1<<16),widget)
    def selection(h,id):return u.SendMessageW(u.GetDlgItem(h,id),CB_GETCURSEL,0,0)
    def close(h):
        u.PostMessageW(h,WM_CLOSE,0,0);deadline=time.monotonic()+10
        while time.monotonic()<deadline and u.IsWindow(h):time.sleep(.05)
        assert not u.IsWindow(h)
    def tab_contains(h,first,ids):
        seen=[];current=u.GetDlgItem(h,first)
        while current not in seen:
            assert current and len(seen)<30
            seen.append(current);current=u.GetNextDlgTabItem(h,current,False)
        for id in ids:assert u.GetDlgItem(h,id) in seen,id
    window=eq=advanced=None
    original_eq=original_filter=None
    try:
        window=wait(EQ_DIALOG);u.PostMessageW(u.GetDlgItem(window,EQ_DIALOG),BM_CLICK,0,0)
        eq=wait(EQ_ENABLED)
        original_eq=(checked(eq,EQ_ENABLED),selection(eq,EQ_PRESET))
        setcheck(eq,EQ_ENABLED,True);select(eq,EQ_PRESET,4)
        original_custom=[number(eq,id,True) for id in (EQ_LOW,EQ_MID,EQ_HIGH)]
        select(eq,EQ_PRESET,1)
        assert [number(eq,id,True) for id in (EQ_LOW,EQ_MID,EQ_HIGH)]==[4,0,-1]
        setnumber(eq,EQ_LOW,-3,True)
        assert selection(eq,EQ_PRESET)==4
        select(eq,EQ_PRESET,2);select(eq,EQ_PRESET,4)
        assert [number(eq,id,True) for id in (EQ_LOW,EQ_MID,EQ_HIGH)]==[-3,0,-1]
        tab_contains(eq,EQ_ENABLED,(EQ_ENABLED,EQ_PRESET,EQ_LOW,EQ_MID,EQ_HIGH,EQ_ADVANCED,1))
        u.PostMessageW(u.GetDlgItem(eq,EQ_ADVANCED),BM_CLICK,0,0);advanced=wait(FILTER_SELECT)
        original_filter=(checked(advanced,FILTER_ENABLED),number(advanced,FILTER_FREQ),
                         number(advanced,FILTER_GAIN,True),number(advanced,FILTER_WIDTH))
        setcheck(advanced,FILTER_ENABLED,True);setnumber(advanced,FILTER_FREQ,777)
        setnumber(advanced,FILTER_GAIN,-4,True);setnumber(advanced,FILTER_WIDTH,50)
        select(advanced,FILTER_SELECT,1);select(advanced,FILTER_SELECT,0)
        assert checked(advanced,FILTER_ENABLED)
        assert (number(advanced,FILTER_FREQ),number(advanced,FILTER_GAIN,True),number(advanced,FILTER_WIDTH))==(777,-4,50)
        tab_contains(advanced,FILTER_SELECT,(FILTER_SELECT,FILTER_ENABLED,FILTER_FREQ,FILTER_GAIN,FILTER_WIDTH,1))
        setcheck(advanced,FILTER_ENABLED,original_filter[0]);setnumber(advanced,FILTER_FREQ,original_filter[1])
        setnumber(advanced,FILTER_GAIN,original_filter[2],True);setnumber(advanced,FILTER_WIDTH,original_filter[3])
        close(advanced);advanced=None
        select(eq,EQ_PRESET,4)
        for id,value in zip((EQ_LOW,EQ_MID,EQ_HIGH),original_custom):setnumber(eq,id,value,True)
        select(eq,EQ_PRESET,original_eq[1]);setcheck(eq,EQ_ENABLED,original_eq[0])
        print('SAPI equalizer: keyboard controls, presets, signed gains and five selectable filters passed')
    finally:
        if advanced and u.IsWindow(advanced):close(advanced)
        if eq and u.IsWindow(eq):close(eq)
        if window and u.IsWindow(window):close(window)
        worker.join(timeout=8)
        assert not worker.is_alive() and result and result[0]>=0,f'dialog failed: {result}'


if __name__=='__main__':main()
