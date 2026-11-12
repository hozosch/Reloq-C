"""Reproduce saved timing settings before wx.App in a real NVDA process."""
import json
import os
from pathlib import Path
import traceback

import globalPluginHandler
import wx

RESULT = Path(os.environ['RELOQ_SEGMENTATION_PROBE_RESULT'])


class Player:
    def __init__(self): self.chunks = []
    def feed(self, data, size=None, onDone=None):
        if callable(size) and onDone is None: onDone = size
        self.chunks.append(bytes(data))
        if onDone: onDone()
    def idle(self): pass
    def sync(self): pass
    def stop(self): pass
    def pause(self, switch): pass
    def close(self): pass


class GlobalPlugin(globalPluginHandler.GlobalPlugin):
    def __init__(self):
        super().__init__()
        self.players = []
        wx.CallLater(1500, self.run)

    def player(self, *_args, **_kwargs):
        player = Player(); self.players.append(player); return player

    def fail(self):
        RESULT.write_text(json.dumps({'ok': False, 'error': traceback.format_exc()},
                                     ensure_ascii=False), encoding='utf8')

    def run(self):
        try:
            import config
            import nvwave
            import speech
            import synthDriverHandler
            import synthDrivers.reloqC as reloq
            self.speech = speech
            nvwave.WavePlayer = self.player
            reloq.SynthDriver._make_player = staticmethod(self.player)
            speech.cancelSpeech()
            if not synthDriverHandler.setSynth('reloqC'):
                raise RuntimeError('could not select ReLoq C')
            synth = synthDriverHandler.getSynth()
            # Arrange the same non-default saved mask as an existing profile.
            # Actual wx.CallAfter obtains the application with wx.GetApp().
            synth._acousticClockMask = reloq.ACOUSTIC_CLOCK_DEFAULT
            section = config.conf['speech'][synth.name]
            section['_acousticClockMask'] = 0
            old_get_app = wx.GetApp
            call_after_globals = wx.CallAfter.__globals__
            old_internal_get_app = call_after_globals['GetApp']
            try:
                wx.GetApp = lambda: None
                call_after_globals['GetApp'] = lambda: None
                synth.initSettings()
            finally:
                wx.GetApp = old_get_app
                call_after_globals['GetApp'] = old_internal_get_app
            if synth._acousticClockMask != 0:
                raise AssertionError('saved timing setting was not restored')
            self.speech.speak(['Die Stimme funktioniert nach dem Laden.'])
            wx.CallLater(1400, self.capture)
        except BaseException:
            self.fail()

    def capture(self):
        try:
            size = sum(len(chunk) for player in self.players for chunk in player.chunks)
            if not size: raise RuntimeError('no audio after restoring the saved setting')
            RESULT.write_text(json.dumps({'ok': True, 'pcm_bytes': size,
                                         'saved_acoustic_clock_mask': 0}), encoding='utf8')
        except BaseException:
            self.fail()
