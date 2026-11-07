"""Check the real SAPI settings dialog's keyboard and visual control order."""
import argparse
import ctypes
from ctypes import wintypes
from pathlib import Path
import subprocess
import time


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--directory', type=Path, required=True)
    directory = parser.parse_args().directory.resolve()
    user32 = ctypes.WinDLL('user32', use_last_error=True)
    callback = ctypes.WINFUNCTYPE(wintypes.BOOL, wintypes.HWND, wintypes.LPARAM)
    user32.EnumWindows.argtypes = (callback, wintypes.LPARAM)
    user32.GetWindowThreadProcessId.argtypes = (wintypes.HWND, ctypes.POINTER(wintypes.DWORD))
    user32.GetDlgItem.argtypes = (wintypes.HWND, ctypes.c_int)
    user32.GetDlgItem.restype = wintypes.HWND
    user32.GetNextDlgTabItem.argtypes = (wintypes.HWND, wintypes.HWND, wintypes.BOOL)
    user32.GetNextDlgTabItem.restype = wintypes.HWND
    user32.GetWindowRect.argtypes = (wintypes.HWND, ctypes.POINTER(wintypes.RECT))
    user32.GetWindowRect.restype = wintypes.BOOL
    user32.PostMessageW.argtypes = (wintypes.HWND, wintypes.UINT, wintypes.WPARAM, wintypes.LPARAM)

    # The IDs identify real controls created by dialog.c; do not use translated
    # text to locate them, since the dialog follows the Windows UI language.
    rate_id, pauses_id, mellow_id = 103, 104, 123
    process = subprocess.Popen([str(directory / 'ReLoqSettings.exe')], cwd=directory)
    window = None
    try:
        end = time.monotonic() + 20
        while time.monotonic() < end and window is None:
            candidates = []

            @callback
            def visit(handle, unused):
                pid = wintypes.DWORD()
                user32.GetWindowThreadProcessId(handle, ctypes.byref(pid))
                if (pid.value == process.pid and user32.GetDlgItem(handle, rate_id)
                        and user32.GetDlgItem(handle, mellow_id)):
                    candidates.append(handle)
                return True

            user32.EnumWindows(visit, 0)
            if candidates:
                window = candidates[0]
                break
            if process.poll() is not None:
                raise AssertionError(f'Settings process exited with {process.returncode}')
            time.sleep(.1)
        assert window, 'Settings dialog did not appear'
        rate, mellow, pauses = (user32.GetDlgItem(window, control)
                                for control in (rate_id, mellow_id, pauses_id))
        assert rate and mellow and pauses, 'Expected controls not found'
        assert user32.GetNextDlgTabItem(window, rate, False) == mellow
        assert user32.GetNextDlgTabItem(window, mellow, False) == pauses
        rectangles = [wintypes.RECT() for _ in range(3)]
        for control, rect in zip((rate, mellow, pauses), rectangles):
            assert user32.GetWindowRect(control, ctypes.byref(rect))
        assert rectangles[0].bottom <= rectangles[1].top
        assert rectangles[1].bottom <= rectangles[2].top
        print('SAPI sample rate, mellow mode and pause controls are ordered for Tab and on screen')
    finally:
        if window:
            user32.PostMessageW(window, 0x0010, 0, 0)  # WM_CLOSE
        try:
            process.wait(timeout=8)
        except subprocess.TimeoutExpired:
            process.kill()
            process.wait(timeout=8)


if __name__ == '__main__':
    main()
