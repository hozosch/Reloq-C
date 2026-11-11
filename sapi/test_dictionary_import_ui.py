"""Exercise the real SAPI personal-file dialogs with an isolated user profile."""
import argparse
import ctypes
from ctypes import wintypes
import os
from pathlib import Path
import tempfile
import threading
import time
import winreg


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--directory',type=Path,required=True)
    args=parser.parse_args()
    user=ctypes.WinDLL('user32',use_last_error=True)
    advapi=ctypes.WinDLL('advapi32',use_last_error=True)
    enum=ctypes.WINFUNCTYPE(wintypes.BOOL,wintypes.HWND,wintypes.LPARAM)
    user.EnumWindows.argtypes=(enum,wintypes.LPARAM)
    user.EnumChildWindows.argtypes=(wintypes.HWND,enum,wintypes.LPARAM)
    user.GetWindowThreadProcessId.argtypes=(wintypes.HWND,ctypes.POINTER(wintypes.DWORD))
    user.GetDlgItem.argtypes=(wintypes.HWND,ctypes.c_int);user.GetDlgItem.restype=wintypes.HWND
    user.SendMessageW.argtypes=(wintypes.HWND,wintypes.UINT,wintypes.WPARAM,wintypes.LPARAM)
    user.SendMessageW.restype=ctypes.c_ssize_t
    user.PostMessageW.argtypes=(wintypes.HWND,wintypes.UINT,wintypes.WPARAM,wintypes.LPARAM)
    user.IsWindow.argtypes=(wintypes.HWND,);user.IsWindow.restype=wintypes.BOOL
    user.IsWindowVisible.argtypes=(wintypes.HWND,);user.IsWindowVisible.restype=wintypes.BOOL
    user.GetClassNameW.argtypes=(wintypes.HWND,wintypes.LPWSTR,ctypes.c_int)
    advapi.RegOverridePredefKey.argtypes=(wintypes.HKEY,wintypes.HKEY);advapi.RegOverridePredefKey.restype=wintypes.LONG
    hkcu=wintypes.HKEY(ctypes.c_int32(0x80000001).value)
    key_name=f'Software\\ReLoq-C-ImportTest-{os.getpid()}'
    key=winreg.CreateKeyEx(winreg.HKEY_CURRENT_USER,key_name,0,winreg.KEY_ALL_ACCESS)
    assert advapi.RegOverridePredefKey(hkcu,wintypes.HKEY(int(key)))==0
    old_appdata=os.environ.get('LOCALAPPDATA')
    root=tempfile.TemporaryDirectory(prefix='reloq-sapi-import-')
    os.environ['LOCALAPPDATA']=root.name
    thread=None;main_window=None;own=None;choice=None
    def send_text(handle,text):
        value=ctypes.create_unicode_buffer(text)
        user.SendMessageW(handle,0x000C,0,ctypes.cast(value,ctypes.c_void_p).value)
    def windows():
        result=[]
        @enum
        def visit(handle,_):
            pid=wintypes.DWORD();user.GetWindowThreadProcessId(handle,ctypes.byref(pid))
            if pid.value==os.getpid() and user.IsWindowVisible(handle):result.append(handle)
            return True
        user.EnumWindows(visit,0);return result
    def wait_for(predicate):
        deadline=time.monotonic()+15
        while time.monotonic()<deadline:
            for handle in windows():
                if predicate(handle):return handle
            if thread is not None and not thread.is_alive():raise AssertionError('SAPI settings closed early')
            time.sleep(.05)
        raise AssertionError('Expected dialog did not appear')
    def click(parent,ident):
        button=user.GetDlgItem(parent,ident);assert button,ident
        user.PostMessageW(button,0xF5,0,0)
    try:
        first=Path(root.name)/'first.dic';second=Path(root.name)/'second.dic'
        first.write_bytes(b'shared\tfirst\n');second.write_bytes(b'shared\tsecond\n')
        with winreg.CreateKey(winreg.HKEY_CURRENT_USER,r'Software\ReLoq C\SAPI5') as config:
            winreg.SetValueEx(config,'DictionaryFiles',0,winreg.REG_MULTI_SZ,
                [f'00010000\t0\t{first}',f'00010000\t0\t{second}'])
        module=ctypes.WinDLL(str((args.directory/'ReLoqSapi.dll').resolve()),use_last_error=True)
        show=module.ReLoqSettings;show.argtypes=(wintypes.HWND,ctypes.c_int);show.restype=ctypes.c_long
        result=[];thread=threading.Thread(target=lambda:result.append(show(None,0x10000)),daemon=True);thread.start()
        main_window=wait_for(lambda w: bool(user.GetDlgItem(w,155)))
        click(main_window,155)
        own=wait_for(lambda w: bool(user.GetDlgItem(w,600)))
        listing=user.GetDlgItem(own,600)
        assert user.SendMessageW(listing,0x18B,0,0)==2  # LB_GETCOUNT
        user.SendMessageW(listing,0x186,1,0)  # LB_SETCURSEL
        user.SendMessageW(own,0x111,600|(1<<16),listing)  # LBN_SELCHANGE
        click(own,603)  # Move the later file earlier.
        time.sleep(.1)
        click(own,604)  # Move it back down.
        time.sleep(.1)
        click(own,602)  # Remove the selected second file.
        time.sleep(.1)
        assert user.SendMessageW(listing,0x18B,0,0)==1
        click(own,1)
        deadline=time.monotonic()+15
        while user.IsWindow(own) and time.monotonic()<deadline:time.sleep(.05)
        assert not user.IsWindow(own),'dictionary dialog did not save'
        with winreg.OpenKey(winreg.HKEY_CURRENT_USER,r'Software\ReLoq C\SAPI5') as config:
            saved,_=winreg.QueryValueEx(config,'DictionaryFiles')
            assert saved==[f'00010000\t0\t{first}'],saved
        click(main_window,155)
        own=wait_for(lambda w: bool(user.GetDlgItem(w,600)))
        click(own,601)
        picker=wait_for(lambda w:w not in (main_window,own) and bool(user.GetDlgItem(w,1)))
        edits=[]
        @enum
        def child(handle,_):
            name=ctypes.create_unicode_buffer(64);user.GetClassNameW(handle,name,len(name))
            if name.value=='Edit' and user.IsWindowVisible(handle):edits.append(handle)
            return True
        user.EnumChildWindows(picker,child,0)
        assert edits,'file name field not found'
        send_text(edits[-1],str(second));click(picker,1)
        choice=wait_for(lambda w:bool(user.GetDlgItem(w,605)) and bool(user.GetDlgItem(w,606)))
        click(choice,1)
        deadline=time.monotonic()+15
        while user.IsWindow(choice) and time.monotonic()<deadline:time.sleep(.05)
        assert not user.IsWindow(choice),'file type dialog did not close'
        assert user.SendMessageW(user.GetDlgItem(own,600),0x18B,0,0)==2
        click(own,1)
        deadline=time.monotonic()+15
        while user.IsWindow(own) and time.monotonic()<deadline:time.sleep(.05)
        assert not user.IsWindow(own),'new file did not save'
        with winreg.OpenKey(winreg.HKEY_CURRENT_USER,r'Software\ReLoq C\SAPI5') as config:
            saved,_=winreg.QueryValueEx(config,'DictionaryFiles')
            assert len(saved)==2,saved
            for entry,expected in zip(saved,(first,second)):
                language,volume,path=entry.split('\t',2)
                assert language=='00010000' and volume=='0' and os.path.samefile(path,expected),saved
        print('SAPI own dictionary dialogs: list, priority, removal, file selection and language/volume passed')
    finally:
        for handle in (choice,own,main_window):
            if handle and user.IsWindow(handle):user.PostMessageW(handle,0x10,0,0)
        if thread:thread.join(10)
        advapi.RegOverridePredefKey(hkcu,None);key.Close()
        advapi.RegDeleteTreeW.argtypes=(wintypes.HKEY,wintypes.LPCWSTR)
        advapi.RegDeleteTreeW(hkcu,key_name)
        if old_appdata is None:os.environ.pop('LOCALAPPDATA',None)
        else:os.environ['LOCALAPPDATA']=old_appdata
        root.cleanup()


if __name__=='__main__':main()
