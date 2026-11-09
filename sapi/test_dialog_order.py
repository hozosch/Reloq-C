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
    user32.SendMessageW.argtypes = (wintypes.HWND, wintypes.UINT, wintypes.WPARAM, wintypes.LPARAM)
    user32.SendMessageW.restype = ctypes.c_ssize_t
    user32.IsWindowEnabled.argtypes = (wintypes.HWND,)
    user32.IsWindowEnabled.restype = wintypes.BOOL
    user32.IsWindow.argtypes = (wintypes.HWND,)
    user32.IsWindow.restype = wintypes.BOOL
    user32.SetDlgItemInt.argtypes = (wintypes.HWND, ctypes.c_int, wintypes.UINT, wintypes.BOOL)
    user32.GetDlgItemInt.argtypes = (wintypes.HWND, ctypes.c_int, ctypes.POINTER(wintypes.BOOL), wintypes.BOOL)
    user32.GetDlgItemInt.restype = wintypes.UINT

    # The IDs identify real controls created by dialog.c; do not use translated
    # text to locate them, since the dialog follows the Windows UI language.
    rate_id, pauses_id, mellow_id = 103, 104, 123
    overrides_id, own_rate_id, own_rate_value_id, own_pitch_id, own_volume_id, own_volume_value_id = range(126, 132)
    (rate_eci_id, rate_wpm_id, rate_wpm_value_id, pitch_eci_id,
     pitch_hz_id, pitch_value_id, pitch_hz_value_id) = range(132, 139)
    bm_getcheck, bm_click, wm_close = 0xF0, 0xF5, 0x10
    process = subprocess.Popen([str(directory / 'ReLoqSettings.exe')], cwd=directory)
    window = None
    modal = None
    saved = None

    def find_modal():
        candidates = []

        @callback
        def visit(handle, unused):
            pid = wintypes.DWORD()
            user32.GetWindowThreadProcessId(handle, ctypes.byref(pid))
            if pid.value == process.pid and user32.GetDlgItem(handle, own_rate_id):
                candidates.append(handle)
            return True

        user32.EnumWindows(visit, 0)
        return candidates[0] if candidates else None

    def open_modal():
        button = user32.GetDlgItem(window, overrides_id)
        assert button, 'Application overrides button not found'
        user32.PostMessageW(button, bm_click, 0, 0)
        end = time.monotonic() + 10
        while time.monotonic() < end:
            found = find_modal()
            if found:
                return found
            if process.poll() is not None:
                raise AssertionError(f'Settings process exited with {process.returncode}')
            time.sleep(.05)
        raise AssertionError('Application overrides dialog did not appear')

    def close_modal(handle):
        user32.PostMessageW(handle, wm_close, 0, 0)
        end = time.monotonic() + 10
        while time.monotonic() < end:
            if not user32.IsWindow(handle) and user32.IsWindowEnabled(window):
                return
            time.sleep(.05)
        raise AssertionError('Application overrides dialog did not close')

    def checked(handle, control_id):
        return user32.SendMessageW(user32.GetDlgItem(handle, control_id), bm_getcheck, 0, 0) == 1

    def set_checked(handle, control_id, enabled):
        if checked(handle, control_id) != enabled:
            user32.SendMessageW(user32.GetDlgItem(handle, control_id), bm_click, 0, 0)
        assert checked(handle, control_id) == enabled

    def number(handle, control_id):
        translated = wintypes.BOOL()
        value = user32.GetDlgItemInt(handle, control_id, ctypes.byref(translated), False)
        assert translated.value
        return value

    def select_radio(handle, selected, other):
        user32.SendMessageW(user32.GetDlgItem(handle, selected), bm_click, 0, 0)
        assert checked(handle, selected) and not checked(handle, other)
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
        modal = open_modal()
        controls = [own_rate_id, own_pitch_id, own_volume_id]
        edits = (own_rate_value_id, rate_wpm_value_id, pitch_value_id, pitch_hz_value_id, own_volume_value_id)
        saved = ([checked(modal, control) for control in controls],
                 [number(modal, control) for control in edits],
                 checked(modal, rate_wpm_id), checked(modal, pitch_hz_id))
        for control in controls:
            set_checked(modal, control, True)
        assert all(user32.GetDlgItem(modal, control) for control in
                   (*controls, *edits, rate_eci_id, rate_wpm_id, pitch_eci_id, pitch_hz_id, 1))
        select_radio(modal, rate_eci_id, rate_wpm_id)
        select_radio(modal, pitch_eci_id, pitch_hz_id)
        assert user32.IsWindowEnabled(user32.GetDlgItem(modal, own_rate_value_id))
        assert not user32.IsWindowEnabled(user32.GetDlgItem(modal, rate_wpm_value_id))
        assert user32.IsWindowEnabled(user32.GetDlgItem(modal, pitch_value_id))
        assert not user32.IsWindowEnabled(user32.GetDlgItem(modal, pitch_hz_value_id))
        assert user32.SetDlgItemInt(modal, own_rate_value_id, 87, False)
        assert user32.SetDlgItemInt(modal, pitch_value_id, 72, False)
        select_radio(modal, rate_wpm_id, rate_eci_id)
        select_radio(modal, pitch_hz_id, pitch_eci_id)
        assert not user32.IsWindowEnabled(user32.GetDlgItem(modal, own_rate_value_id))
        assert user32.IsWindowEnabled(user32.GetDlgItem(modal, rate_wpm_value_id))
        assert not user32.IsWindowEnabled(user32.GetDlgItem(modal, pitch_value_id))
        assert user32.IsWindowEnabled(user32.GetDlgItem(modal, pitch_hz_value_id))
        assert user32.SetDlgItemInt(modal, rate_wpm_value_id, 210, False)
        assert user32.SetDlgItemInt(modal, pitch_hz_value_id, 151, False)
        assert user32.SetDlgItemInt(modal, own_volume_value_id, 42, False)
        close_modal(modal)
        assert not user32.IsWindowEnabled(user32.GetDlgItem(window, 114))  # Speed boost
        modal = open_modal()
        assert all(checked(modal, control) for control in controls)
        assert checked(modal, rate_wpm_id) and checked(modal, pitch_hz_id)
        assert [number(modal, control) for control in edits] == [87, 210, 72, 151, 42]
        # Every active value and checkbox must remain in the dialog's Tab cycle.
        tab_cycle = []
        current = user32.GetDlgItem(modal, own_rate_id)
        while current not in tab_cycle:
            tab_cycle.append(current)
            current = user32.GetNextDlgTabItem(modal, current, False)
            assert current and len(tab_cycle) < 20
        for control in (*controls, rate_wpm_value_id, pitch_hz_value_id, own_volume_value_id, 1):
            assert user32.GetDlgItem(modal, control) in tab_cycle, control
        set_checked(modal, own_rate_id, False)
        set_checked(modal, own_volume_id, False)
        assert not user32.IsWindowEnabled(user32.GetDlgItem(modal, rate_wpm_value_id))
        assert not user32.IsWindowEnabled(user32.GetDlgItem(modal, own_volume_value_id))
        print('SAPI controls, keyboard order, override persistence and enable states verified')
    finally:
        if saved and process.poll() is None:
            if not modal or not user32.IsWindow(modal):
                modal = open_modal()
            set_checked(modal, own_rate_id, True)
            set_checked(modal, own_pitch_id, True)
            set_checked(modal, own_volume_id, True)
            for control, value in zip((own_rate_value_id, rate_wpm_value_id, pitch_value_id,
                                       pitch_hz_value_id, own_volume_value_id), saved[1]):
                user32.SetDlgItemInt(modal, control, value, False)
            select_radio(modal, rate_wpm_id if saved[2] else rate_eci_id,
                         rate_eci_id if saved[2] else rate_wpm_id)
            select_radio(modal, pitch_hz_id if saved[3] else pitch_eci_id,
                         pitch_eci_id if saved[3] else pitch_hz_id)
            for control, old_state in zip((own_rate_id, own_pitch_id, own_volume_id), saved[0]):
                set_checked(modal, control, old_state)
        if modal and user32.IsWindow(modal):
            close_modal(modal)
        if window:
            user32.PostMessageW(window, wm_close, 0, 0)
        try:
            process.wait(timeout=8)
        except subprocess.TimeoutExpired:
            process.kill()
            process.wait(timeout=8)


if __name__ == '__main__':
    main()
