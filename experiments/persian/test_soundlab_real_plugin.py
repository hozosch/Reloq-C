"""Exercise the actual sound-lab add-on and wx dialog inside running NVDA."""
import json
import os
from pathlib import Path
import time
import traceback
import ctypes

import globalPluginHandler
import wx

RESULT = Path(os.environ['RELOQ_SEGMENTATION_PROBE_RESULT'])


class Player:
    def __init__(self, *args, **kwargs): self.chunks = []
    def feed(self, data): self.chunks.append(bytes(data))
    def idle(self): pass
    def close(self): pass


class GlobalPlugin(globalPluginHandler.GlobalPlugin):
    def __init__(self):
        super().__init__()
        wx.CallLater(1500, self.setup)

    def fail(self):
        RESULT.write_text(json.dumps({'ok': False, 'error': traceback.format_exc()}), encoding='utf8')

    def setup(self):
        try:
            import gui
            import globalPlugins.reloqSoundLab as soundlab
            active = [p for p in globalPluginHandler.runningPlugins if isinstance(p, soundlab.GlobalPlugin)]
            assert active, 'sound lab did not register as an NVDA global plugin'
            assert active[0].item is not None, 'sound lab Tools menu missing'
            self.players = []
            def make_player(*args, **kwargs):
                player = Player(*args, **kwargs); self.players.append(player); return player
            soundlab.nvwave.WavePlayer = make_player
            self.dialog = soundlab.SoundLabDialog(gui.mainFrame)
            self.errors = []
            self.dialog.error = self.errors.append
            # Read the names NVDA gets from the actual Windows accessibility
            # objects. wx.GetName() alone does not establish the MSAA label.
            from NVDAObjects.IAccessible import getNVDAObjectFromEvent
            self.dialog.Show()
            self.dialog.Raise()
            wx.YieldIfNeeded()
            book = self.dialog.pages['base'][0].GetParent()
            self.accessible_names = []
            groups = [
                [('Testtext', self.dialog.text), ('Sprache', self.dialog.language),
                 ('Stimme', self.dialog.gender), ('Abtastrate', self.dialog.rate),
                 ('Kopfgröße', self.dialog.head), ('Geschwindigkeit', self.dialog.speed),
                 ('Tonhöhe', self.dialog.pitch),
                 ('Beginn des Abschnitts in Millisekunden', self.dialog.start),
                 ('Ende in Millisekunden; 0 bedeutet bis zum Ende', self.dialog.end),
                 ('Abschnitte', self.dialog.voiced)],
                [('Frequenz F%s: Änderung in Hz' % i, field)
                 for i, field in enumerate(self.dialog.controls['frequencies'], 1)],
                [('Bandbreite B%s: Änderung in Hz' % i, field)
                 for i, field in enumerate(self.dialog.controls['bandwidths'], 1)],
                [('Rauschbandbreite B%s: Änderung in Hz' % i, field)
                 for i, field in enumerate(self.dialog.controls['noise_widths'], 1)],
            ]
            send = ctypes.windll.user32.SendMessageW
            send.argtypes = (ctypes.c_void_p, ctypes.c_uint, ctypes.c_size_t, ctypes.c_ssize_t)
            send.restype = ctypes.c_void_p
            def inspect(label, field):
                hwnd = int(field.GetHandle())
                handles = [hwnd]
                if isinstance(field, wx.SpinCtrl):
                    buddy = send(hwnd, 0x046A, 0, 0)  # UDM_GETBUDDY
                    if buddy: handles.insert(0, int(buddy))
                names = []
                for handle in handles:
                    obj = getNVDAObjectFromEvent(handle, -4, 0)  # OBJID_CLIENT
                    if obj is not None: names.append(obj.name)
                self.accessible_names.append((label, names))
                assert label in names, (label, names)
            for index, fields in enumerate(groups):
                book.SetSelection(index)
                wx.YieldIfNeeded()
                for label, field in fields: inspect(label, field)
            inspect('Gemessene Resonanzen der letzten Probe', self.dialog.readout)
            book.SetSelection(0)
            initial = self.dialog.values(); self.dialog.set_values(initial)
            assert self.dialog.values() == initial
            self.dialog.listen(False)
            self.phase = 0; self.deadline = time.monotonic() + 60
            wx.CallLater(100, self.check)
        except Exception:
            self.fail()

    def check(self):
        try:
            if self.dialog._busy:
                assert time.monotonic() < self.deadline, 'sound lab playback timed out'
                wx.CallLater(100, self.check); return
            assert not self.errors, self.errors
            assert self.players and self.players[-1].chunks, 'sound lab did not play audio'
            assert 'F1 ' in self.dialog.readout.GetValue(), 'measured formants missing'
            if self.phase == 0:
                self.before = b''.join(self.players[-1].chunks)
                self.dialog.controls['frequencies'][1].SetValue(250)
                self.phase = 1
                self.dialog.listen(True)
                wx.CallLater(100, self.check); return
            after = b''.join(self.players[-1].chunks)
            assert self.before != after and len(self.before) == len(after), 'F2 adjustment failed'
            assert self.dialog.buttons[0].IsEnabled(), 'controls did not re-enable'
            self.dialog._closed = True
            self.dialog.Destroy()
            RESULT.write_text(json.dumps({'ok': True, 'menu': True, 'labeled_controls': self.accessible_names,
                                         'persian_original_and_f2': True}), encoding='utf8')
        except Exception:
            self.fail()
