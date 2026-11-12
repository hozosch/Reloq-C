"""Real-NVDA regression for protected acronym and dictionary word boundaries."""
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
        self.players, self.results, self.case = [], [], 0
        wx.CallLater(1500, self.setup)

    def fail(self):
        if hasattr(self, 'builtin_original'):
            self.builtin[:] = self.builtin_original
        RESULT.write_text(json.dumps({'ok': False, 'error': traceback.format_exc(),
                                      'cases': self.results}, ensure_ascii=False), encoding='utf8')

    def player(self, *_args, **_kwargs):
        item = Player(); self.players.append(item); return item

    def setup(self):
        try:
            import characterProcessing
            import config
            from config.featureFlag import FeatureFlag
            from config.featureFlagEnums import BoolFlag
            import nvwave
            import speech
            import speechDictHandler
            import synthDriverHandler
            import synthDrivers.reloqC as reloq
            self.speech, self.config, self.symbols = speech, config, characterProcessing
            self.FeatureFlag, self.BoolFlag = FeatureFlag, BoolFlag
            self.builtin = next(d.dictionary for d in speechDictHandler.listAvailableSpeechDictDefinitions()
                                if getattr(d.source, 'value', d.source) == 'builtin')
            self.builtin_original = list(self.builtin)
            if not self.builtin_original:
                raise RuntimeError('NVDA builtin dictionary fixture is empty')
            nvwave.WavePlayer = self.player
            reloq.SynthDriver._make_player = staticmethod(self.player)
            speech.cancelSpeech()
            if not synthDriverHandler.setSynth('reloqC'):
                raise RuntimeError('could not select ReLoq C')
            self.synth = synthDriverHandler.getSynth()
            own_native = RESULT.parent / 'boundary-native-DEU.dic'
            own_native.write_bytes(b'TestEintrag\tHaus\n')
            source = reloq.DictionarySource(str(own_native.resolve()), 0x40000, 0)
            self.synth.configure_dictionaries([source], False,
                lambda error, details: wx.CallAfter(self.ready, error))
        except BaseException:
            self.fail()

    def ready(self, error):
        try:
            if error: raise RuntimeError(str(error))
            if not self.synth._wordseg_keys or 'TestEintrag' not in self.synth._wordseg_keys[0x40000]:
                raise RuntimeError('own native dictionary key was not loaded')
            self.cases = []
            for locale, levels in (('en_US', (0, 100, 300)), ('en_GB', (0, 100, 300)),
                                   ('de_DE', (0, 300))):
                for normalize in (False, True):
                    for builtin in (False, True):
                        for word, camel in ((False, False), (True, True)):
                            for level in levels:
                                for reference in (False, True):
                                    self.cases.append((locale, normalize, builtin, word, camel, level, reference))
            self.next()
        except BaseException:
            self.fail()

    def next(self):
        try:
            if self.case == len(self.cases):
                for row in self.results:
                    if row['reference']:
                        continue
                    matching = next(other for other in self.results if other['reference'] and
                        all(other[key] == row[key] for key in ('locale', 'normalize', 'builtin', 'word', 'camel', 'level')))
                    if row['pcm_sha256'] != matching['pcm_sha256']:
                        raise AssertionError('Protected text changed punctuation or pronunciation: ' + repr(row))
                self.builtin[:] = self.builtin_original
                RESULT.write_text(json.dumps({'ok': True, 'cases': self.results}, ensure_ascii=False), encoding='utf8')
                return
            locale, normalize, builtin, word, camel, level, reference = self.cases[self.case]
            self.speech.cancelSpeech()
            self.builtin[:] = self.builtin_original if builtin else []
            flag = self.BoolFlag.ENABLED if normalize else self.BoolFlag.DISABLED
            self.config.conf['speech']['unicodeNormalization'] = self.FeatureFlag(flag, self.BoolFlag.ENABLED)
            if bool(self.config.conf['speech']['unicodeNormalization']) != normalize:
                raise RuntimeError('NVDA normalization mode did not change')
            self.synth.language = locale
            self.synth._wordSegmentation, self.synth._camelCase = word, camel
            self.synth.voiceTags = False
            self.before = [(p, len(p.chunks)) for p in self.players]
            if locale == 'de_DE':
                text = 'Haus. Danach' if reference else 'TestEintrag. Danach'
            else:
                text = 'packs and P D Effs. Next, U R Els!' if reference else 'PACs and PDFs. Next, URLs!'
            self.speech.speak([text], symbolLevel=self.symbols.SymbolLevel(level))
            wx.CallLater(1200, self.capture)
        except BaseException:
            self.fail()

    def capture(self):
        try:
            locale, normalize, builtin, word, camel, level, reference = self.cases[self.case]
            starts = {id(player): count for player, count in self.before}
            audio = b''.join(chunk for player in self.players for chunk in player.chunks[starts.get(id(player), 0):])
            if not audio:
                raise RuntimeError('real NVDA produced no speech for ' + repr(self.cases[self.case]))
            self.results.append({'locale': locale, 'normalize': normalize, 'builtin': builtin,
                                 'word': word, 'camel': camel, 'level': level, 'reference': reference,
                                 'pcm_bytes': len(audio), 'pcm_sha256': hashlib.sha256(audio).hexdigest()})
            self.case += 1
            self.next()
        except BaseException:
            self.fail()
