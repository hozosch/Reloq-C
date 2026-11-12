"""Check the experimental Aragonese voice in an actual NVDA process."""
import hashlib
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
        self.players, self.results, self.index = [], [], 0
        wx.CallLater(1500, self.setup)

    def fail(self):
        RESULT.write_text(json.dumps({'ok': False, 'error': traceback.format_exc(),
                                      'cases': self.results}, ensure_ascii=False), encoding='utf8')

    def player(self, *_args, **_kwargs):
        player = Player(); self.players.append(player); return player

    def setup(self):
        try:
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
            self.synth = synthDriverHandler.getSynth()
            if 'an_ES' not in self.synth.availableLanguages:
                raise AssertionError('Aragonese is absent from the language list')
            self.cases = [('an_ES', 'Esta ye una preba de voz.'),
                          ('an_ES', 'Auca y pauto. Sinyal y cabanyera.'),
                          ('es_ES', 'Esta es una prueba de voz.'),
                          ('an_ES', 'L’aragonés ye una luenga de Aragón.')]
            self.next()
        except BaseException:
            self.fail()

    def next(self):
        try:
            if self.index == len(self.cases):
                RESULT.write_text(json.dumps({'ok': True, 'cases': self.results},
                                              ensure_ascii=False), encoding='utf8')
                return
            locale, text = self.cases[self.index]
            self.speech.cancelSpeech()
            self.synth.language = locale
            if self.synth.language != locale:
                raise AssertionError('language selection did not stick: ' + locale)
            self.before = {id(p): len(p.chunks) for p in self.players}
            self.speech.speak([text])
            wx.CallLater(1400, self.capture)
        except BaseException:
            self.fail()

    def capture(self):
        try:
            locale, text = self.cases[self.index]
            audio = b''.join(chunk for p in self.players
                             for chunk in p.chunks[self.before.get(id(p), 0):])
            if not audio: raise RuntimeError('no real NVDA audio for ' + text)
            self.results.append({'locale': locale, 'text': text, 'pcm_bytes': len(audio),
                                 'pcm_sha256': hashlib.sha256(audio).hexdigest()})
            self.index += 1
            self.next()
        except BaseException:
            self.fail()
