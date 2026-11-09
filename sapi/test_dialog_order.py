"""Check the real SAPI settings dialog's keyboard and visual control order."""
import argparse
import ctypes
from ctypes import wintypes
import os
from pathlib import Path
import threading
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
    user32.GetDlgItemTextW.argtypes = (wintypes.HWND, ctypes.c_int, wintypes.LPWSTR, ctypes.c_int)
    user32.GetDlgItemTextW.restype = wintypes.UINT

    # The IDs identify real controls created by dialog.c; do not use translated
    # text to locate them, since the dialog follows the Windows UI language.
    rate_id, pauses_id, mellow_id = 103, 104, 123
    overrides_id, own_rate_id, own_rate_value_id, own_pitch_id, own_volume_id, own_volume_value_id = range(126, 132)
    (rate_eci_id, rate_wpm_id, rate_wpm_value_id, pitch_eci_id,
     pitch_hz_id, pitch_value_id, pitch_hz_value_id) = range(132, 139)
    bm_getcheck, bm_click, wm_close = 0xF0, 0xF5, 0x10
    # Create the real settings dialog in this process. Win32 intentionally
    # does not expose edit-control text reliably across process boundaries.
    module = ctypes.WinDLL(str(directory / 'ReLoqSapi.dll'), use_last_error=True)
    show_settings = module.ReLoqSettings
    show_settings.argtypes = (wintypes.HWND, ctypes.c_int)
    show_settings.restype = ctypes.c_long
    result = []
    thread = threading.Thread(target=lambda: result.append(show_settings(None, -1)), daemon=True)
    thread.start()
    own_pid = os.getpid()
    window = None
    modal = None
    saved = None

    def find_modal():
        candidates = []

        @callback
        def visit(handle, unused):
            pid = wintypes.DWORD()
            user32.GetWindowThreadProcessId(handle, ctypes.byref(pid))
            if pid.value == own_pid and user32.GetDlgItem(handle, 1):
                # WM_INITDIALOG creates the controls before populating their
                # numbers. Wait for all values, not merely the first button.
                ready = True
                for control_id in (own_rate_value_id, rate_wpm_value_id, pitch_value_id,
                                   pitch_hz_value_id, own_volume_value_id):
                    translated = wintypes.BOOL()
                    user32.GetDlgItemInt(handle, control_id, ctypes.byref(translated), False)
                    ready = ready and bool(translated.value)
                if ready:
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
            if not thread.is_alive():
                raise AssertionError(f'Settings dialog exited with {result}')
            time.sleep(.05)
        state = []

        @callback
        def inspect(handle, unused):
            pid = wintypes.DWORD()
            user32.GetWindowThreadProcessId(handle, ctypes.byref(pid))
            if pid.value == own_pid and user32.GetDlgItem(handle, own_rate_id):
                for control_id in (1, own_rate_value_id, rate_wpm_value_id, pitch_value_id,
                                   pitch_hz_value_id, own_volume_value_id):
                    value = ctypes.create_unicode_buffer(128)
                    user32.GetDlgItemTextW(handle, control_id, value, len(value))
                    state.append((control_id, bool(user32.GetDlgItem(handle, control_id)), value.value))
            return True

        user32.EnumWindows(inspect, 0)
        raise AssertionError(f'Application overrides dialog did not initialize: {state}')

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
                if (pid.value == own_pid and user32.GetDlgItem(handle, rate_id)
                        and user32.GetDlgItem(handle, mellow_id) and user32.GetDlgItem(handle, 1)):
                    pitch_ready = wintypes.BOOL()
                    hz_ready = wintypes.BOOL()
                    user32.GetDlgItemInt(handle, 105, ctypes.byref(pitch_ready), False)
                    user32.GetDlgItemInt(handle, 125, ctypes.byref(hz_ready), False)
                    if pitch_ready.value and hz_ready.value:
                        candidates.append(handle)
                return True

            user32.EnumWindows(visit, 0)
            if candidates:
                window = candidates[0]
                break
            if not thread.is_alive():
                raise AssertionError(f'Settings dialog exited with {result}')
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
        main_pitch, main_hertz = number(window, 105), number(window, 125)
        modal = open_modal()
        assert number(modal, pitch_value_id) == main_pitch
        assert number(modal, pitch_hz_value_id) == main_hertz
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
        if saved and thread.is_alive():
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
        thread.join(timeout=8)
        assert not thread.is_alive(), 'Settings dialog did not close'
        assert result and result[0] >= 0, f'Settings dialog failed: {result}'


if __name__ == '__main__':
    main()
