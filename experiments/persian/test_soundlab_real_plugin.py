"""Exercise the actual sound-lab add-on and wx dialog inside running NVDA."""
import json
import os
from pathlib import Path
import time
import traceback

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
            assert self.dialog.text.GetName() == 'Testtext'
            assert self.dialog.controls['frequencies'][1].GetName() == 'Frequenz F2: Änderung in Hz'
            assert self.dialog.readout.GetName() == 'Gemessene Resonanzen der letzten Probe'
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
            RESULT.write_text(json.dumps({'ok': True, 'menu': True, 'labeled_controls': True,
                                         'persian_original_and_f2': True}), encoding='utf8')
        except Exception:
            self.fail()
