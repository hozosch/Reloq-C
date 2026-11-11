"""Temporary probe inside real NVDA, including its speech manager and dictionaries."""
import hashlib
import json
import os
from pathlib import Path
import threading
import traceback

import globalPluginHandler
import wx

RESULT = Path(os.environ['RELOQ_SEGMENTATION_PROBE_RESULT'])
DIAGNOSE = os.environ.get('RELOQ_SEGMENTATION_DIAGNOSE') == '1'


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
        self.results = []
        self.case = 0
        self.players = []
        self.records = []
        wx.CallLater(1500, self.setup)

    def fail(self):
        RESULT.write_text(json.dumps({'ok':False, 'error':traceback.format_exc(),
                                      'cases':self.results}, ensure_ascii=False), encoding='utf8')

    def player(self, *_args, **_kwargs):
        item = Player();self.players.append(item);return item

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
            if not synthDriverHandler.setSynth('reloqC'): raise RuntimeError('could not select ReLoq C')
            self.synth = synthDriverHandler.getSynth()
            self.synth.language = 'de_DE'
            self.synth.voiceTags = False
            original = self.synth._transport.submit
            def submit(parts):
                self.records.append([{'kind':type(item).__name__, 'text':getattr(item,'text',None),
                                      'original':getattr(item,'original',None)} for item in parts])
                return original(parts)
            self.synth._transport.submit = submit
            self.synth.configure_dictionaries([], True, lambda error, details: wx.CallAfter(self.ready, error))
        except BaseException:
            self.fail()

    def ready(self, error):
        try:
            if error: raise RuntimeError(str(error))
            if self.synth._wordseg_keys is None: raise RuntimeError('dictionary keys not ready')
            if 'YouTube' not in self.synth._wordseg_keys[0x40000]:
                raise RuntimeError('German community dictionary fixture missing')
            self.cases = [(word,camel,text) for text in ('YouTube Fenster.', 'YouTube Dokument.',
                'Vor YouTube Fenster. Danach weiter.', 'ROLE:window', 'ROLE:document')
                for word,camel in ((False,False),(True,False),(False,True),(True,True))]
            self.next()
        except BaseException:
            self.fail()

    def next(self):
        try:
            if self.case == len(self.cases):
                if not DIAGNOSE:
                    for row in self.results:
                        combined = ''.join(part.get('text') or '' for group in row['submissions'] for part in group)
                        if 'YouTubeFenster' in combined or 'YouTubeDokument' in combined:
                            raise AssertionError('NVDA joined adjacent words: ' + repr(row))
                RESULT.write_text(json.dumps({'ok':True,'cases':self.results}, ensure_ascii=False),encoding='utf8')
                return
            word,camel,text = self.cases[self.case]
            self.speech.cancelSpeech()
            self.synth._wordSegmentation = word
            self.synth._camelCase = camel
            self.records.clear()
            self.before = [(p,len(p.chunks)) for p in self.players]
            if text.startswith('ROLE:'):
                import controlTypes
                role = controlTypes.Role.WINDOW if text == 'ROLE:window' else controlTypes.Role.DOCUMENT
                label = 'Fenster' if text == 'ROLE:window' else 'Dokument'
                sequence = self.speech.getPropertiesSpeech(name='YouTube', role=role, roleText=label)
                self.speech.speak(sequence)
            else:
                self.speech.speak([text])
            wx.CallLater(1200,self.capture)
        except BaseException:
            self.fail()

    def capture(self):
        try:
            word,camel,text = self.cases[self.case]
            starts = {id(player):count for player,count in self.before}
            audio = b''.join(chunk for p in self.players for chunk in p.chunks[starts.get(id(p),0):])
            if not self.records or not audio:
                raise RuntimeError('real NVDA did not produce native speech: ' + repr(self.records))
            self.results.append({'word':word,'camel':camel,'input':text,'submissions':self.records[:],
                                 'pcm_bytes':len(audio),'pcm_sha256':hashlib.sha256(audio).hexdigest()})
            self.case += 1
            self.next()
        except BaseException:
            self.fail()
