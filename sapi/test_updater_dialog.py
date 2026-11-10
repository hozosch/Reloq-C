"""Exercise the real updater controls and per-user persistence on Windows."""
import argparse
import ctypes
from ctypes import wintypes
import subprocess
import time
import winreg
from pathlib import Path

parser=argparse.ArgumentParser();parser.add_argument('--directory',type=Path,required=True);args=parser.parse_args()
exe=args.directory.resolve()/'ReLoqUpdate.exe';assert exe.is_file(),exe
key_path=r'Software\ReLoq C\SAPI5'
with winreg.CreateKey(winreg.HKEY_CURRENT_USER,key_path) as key:
    try:old,old_type=winreg.QueryValueEx(key,'AutomaticUpdates')
    except FileNotFoundError:old=None;old_type=None
    winreg.SetValueEx(key,'AutomaticUpdates',0,winreg.REG_DWORD,1)

user32=ctypes.WinDLL('user32',use_last_error=True)
user32.GetDlgItem.argtypes=[wintypes.HWND,ctypes.c_int];user32.GetDlgItem.restype=wintypes.HWND
user32.GetWindowThreadProcessId.argtypes=[wintypes.HWND,ctypes.POINTER(wintypes.DWORD)]
user32.SendMessageW.argtypes=[wintypes.HWND,wintypes.UINT,wintypes.WPARAM,wintypes.LPARAM]
user32.SendMessageW.restype=wintypes.LPARAM
user32.PostMessageW.argtypes=user32.SendMessageW.argtypes;user32.PostMessageW.restype=wintypes.BOOL
callback=ctypes.WINFUNCTYPE(wintypes.BOOL,wintypes.HWND,wintypes.LPARAM)
proc=subprocess.Popen([str(exe)])
try:
    end=time.monotonic()+15;window=None
    while time.monotonic()<end and not window:
        found=[]
        @callback
        def visit(handle,unused):
            pid=wintypes.DWORD();user32.GetWindowThreadProcessId(handle,ctypes.byref(pid))
            if pid.value==proc.pid and user32.GetDlgItem(handle,300):found.append(handle)
            return True
        user32.EnumWindows(visit,0)
        if found:window=found[0]
        elif proc.poll() is not None:raise AssertionError('Updater settings exited early')
        else:time.sleep(.05)
    assert window,'Updater settings did not open'
    checkbox=user32.GetDlgItem(window,300);check_button=user32.GetDlgItem(window,301)
    close_button=user32.GetDlgItem(window,1)
    assert checkbox and check_button and close_button
    assert user32.SendMessageW(checkbox,0x00F0,0,0)==1
    user32.SendMessageW(checkbox,0x00F5,0,0)
    assert user32.SendMessageW(checkbox,0x00F0,0,0)==0
    with winreg.OpenKey(winreg.HKEY_CURRENT_USER,key_path) as key:
        assert winreg.QueryValueEx(key,'AutomaticUpdates')[0]==0
    user32.PostMessageW(window,0x0010,0,0)
    assert proc.wait(timeout=10)==0
    print('SAPI updater: native settings dialog, checkbox and per-user persistence passed')
finally:
    if proc.poll() is None:proc.terminate();proc.wait(timeout=10)
    with winreg.CreateKey(winreg.HKEY_CURRENT_USER,key_path) as key:
        if old is None:
            try:winreg.DeleteValue(key,'AutomaticUpdates')
            except FileNotFoundError:pass
        else:winreg.SetValueEx(key,'AutomaticUpdates',0,old_type,old)
