"""Check both Persian test profiles in a running portable NVDA instance."""
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
                                      'cases': self.results}, ensure_ascii=True), encoding='utf8')

    def player(self, *_args, **_kwargs):
        player = Player()
        self.players.append(player)
        return player

    def setup(self):
        try:
            import nvwave
            import speech
            from speech.commands import CharacterModeCommand, IndexCommand
            import synthDriverHandler
            import synthDrivers.reloqC as reloq
            self.speech, self.character, self.index_command = speech, CharacterModeCommand, IndexCommand
            nvwave.WavePlayer = self.player
            reloq.SynthDriver._make_player = staticmethod(self.player)
            speech.cancelSpeech()
            if not synthDriverHandler.setSynth('reloqC'):
                raise RuntimeError('could not select ReLoq C')
            self.synth = synthDriverHandler.getSynth()
            for locale in ('fa_IR', 'fa_Latn'):
                if locale not in self.synth.availableLanguages:
                    raise AssertionError('Persian test profile is absent: ' + locale)
            self.cases = [('fa_IR', 'سلام دنیا؟', False),
                          ('fa_Latn', 'salam donya?', False),
                          ('fa_Latn', 'salam khub?', False),
                          ('fa_Latn', 'salam khubtar?', False),
                          ('fa_Latn', 'w', True),
                          ('fa_IR', 'ق', True),
                          ('de_DE', 'Guten Tag.', False)]
            self.next()
        except BaseException:
            self.fail()

    def next(self):
        try:
            if self.index == len(self.cases):
                if self.results[0]['pcm_sha256'] != self.results[1]['pcm_sha256']:
                    raise AssertionError('confirmed script and Pinglish words differ')
                RESULT.write_text(json.dumps({'ok': True, 'cases': self.results}, ensure_ascii=True), encoding='utf8')
                return
            locale, text, spelling = self.cases[self.index]
            self.speech.cancelSpeech()
            self.synth.language = locale
            if self.synth.language != locale:
                raise AssertionError('language selection did not stick: ' + locale)
            self.before = {id(player): len(player.chunks) for player in self.players}
            sequence = ([self.character(True), text, self.character(False)] if spelling else [text])
            self.synth.speak([self.index_command(self.index + 1), *sequence])
            wx.CallLater(1600, self.capture)
        except BaseException:
            self.fail()

    def capture(self):
        try:
            locale, text, spelling = self.cases[self.index]
            audio = b''.join(chunk for player in self.players
                             for chunk in player.chunks[self.before.get(id(player), 0):])
            if not audio:
                raise RuntimeError('no actual NVDA audio for ' + ascii(text))
            self.results.append({'locale': locale, 'text': text, 'spelling': spelling,
                                 'pcm_bytes': len(audio), 'pcm_sha256': hashlib.sha256(audio).hexdigest()})
            self.index += 1
            self.next()
        except BaseException:
            self.fail()
