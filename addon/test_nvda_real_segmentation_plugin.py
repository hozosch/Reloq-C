"""Temporary probe inside real NVDA, including its speech manager and dictionaries."""
import hashlib
import json
import os
from pathlib import Path
import re
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
            import speechDictHandler
            from speechDictHandler.types import SpeechDictEntry
            import synthDriverHandler
            import synthDrivers.reloqC as reloq
            self.speech = speech
            nvwave.WavePlayer = self.player
            reloq.SynthDriver._make_player = staticmethod(self.player)
            speech.cancelSpeech()
            if not synthDriverHandler.setSynth('reloqC'): raise RuntimeError('could not select ReLoq C')
            self.synth = synthDriverHandler.getSynth()
            temporary = next(d for d in speechDictHandler.listAvailableSpeechDictDefinitions()
                             if getattr(d.source, 'value', d.source) == 'temp')
            temporary.dictionary.append(SpeechDictEntry('reloqtest', 'swapfile'))
            source = RESULT.parent / 'nvda-import-fixture.dic'
            source.write_bytes(('# Real NVDA speech dictionary\r\n'
                'probenull\tErsatz\t0\t0\r\n'
                'probeganz\tTermin 2035\t1\t2\r\n'
                '^probe([0-9]+)$\tZahl \\1\t0\t1\r\n').encode('utf-8-sig'))
            manager = self.synth.imported_dictionary_manager()
            if manager.process('legacyprobe', 0x40000, '1') != 'Sonderwort':
                raise AssertionError('previous NVDA source was not migrated correctly')
            if self.synth.dictionary_manager().sources:
                raise AssertionError('previous NVDA source is still treated as native ECI')
            legacy = RESULT.parent / 'legacy-nvda-dictionary.dic'
            if not legacy.is_file() or '\t0\t2' not in legacy.read_text(encoding='utf-8-sig'):
                raise AssertionError('source NVDA dictionary was altered')
            _copy, count, created = manager.import_file(source, 0x40000, 'all')
            if not created or count != 3: raise AssertionError('NVDA dictionary fixture not imported')
            if manager.process('probenull', 0x40000, '1') != 'Ersatz':
                raise AssertionError('NVDA option fields leaked into replacement')
            if manager.process('probeganz', 0x40000, '1') != 'Termin 2035':
                raise AssertionError('literal replacement digits were changed')
            if manager.process('probe57', 0x40000, '1') != 'Zahl 57':
                raise AssertionError('NVDA regular-expression replacement was changed')
            self.synth.language = 'de_DE'
            self.synth.voiceTags = False
            original = self.synth._transport.submit
            def submit(parts):
                self.records.append([{'kind':type(item).__name__, 'text':getattr(item,'text',None),
                                      'original':getattr(item,'original',None),
                                      'guarded':getattr(item,'segmentation_guarded',False)} for item in parts])
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
            self.cases += [(word,False,text) for text in ('DICT:plain', 'DICT:compound')
                           for word in (False,True)]
            self.cases += [(word,False,text) for text in ('IMPORT:plain', 'IMPORT:digits', 'IMPORT:regex', 'IMPORT:legacy')
                           for word in (False,True)]
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
                        if (row['word'] or row['camel']) and row['input'].startswith('ROLE:'):
                            label = 'Fenster' if row['input'] == 'ROLE:window' else 'Dokument'
                            if not re.search(r'YouTube\s+' + label, combined):
                                raise AssertionError('NVDA lost the object boundary: ' + repr(row))
                        if row['input'] == 'DICT:compound' and row['word']:
                            parts = [p for group in row['submissions'] for p in group]
                            if not any('swapfile' in (p['text'] or '') and p['guarded'] for p in parts):
                                raise AssertionError('NVDA dictionary replacement was not protected: ' + repr(row))
                            if not any('framebuffer' in (p['text'] or '') and not p['guarded'] for p in parts):
                                raise AssertionError('unrelated word was protected too: ' + repr(row))
                        if row['input'].startswith('IMPORT:'):
                            expected = {'IMPORT:plain':'Ersatz', 'IMPORT:digits':'Termin 2035',
                                        'IMPORT:regex':'Zahl 57', 'IMPORT:legacy':'Sonderwort'}[row['input']]
                            if expected not in combined or '\t0\t' in combined or '\t1\t' in combined:
                                raise AssertionError('NVDA import changed a replacement or included options: ' + repr(row))
                    plain = [row for row in self.results if row['input'] == 'DICT:plain']
                    if len(plain) != 2 or plain[0]['pcm_sha256'] != plain[1]['pcm_sha256']:
                        raise AssertionError('personal dictionary changed unrelated NVDA speech: ' + repr(plain))
                    compounds = [row for row in self.results if row['input'] == 'DICT:compound']
                    if len(compounds) != 2 or compounds[0]['pcm_sha256'] == compounds[1]['pcm_sha256']:
                        raise AssertionError('unrelated English compound was not segmented: ' + repr(compounds))
                RESULT.write_text(json.dumps({'ok':True,'cases':self.results}, ensure_ascii=False),encoding='utf8')
                return
            word,camel,text = self.cases[self.case]
            self.speech.cancelSpeech()
            self.synth._wordSegmentation = word
            self.synth._camelCase = camel
            self.synth.language = 'en_US' if text.startswith('DICT:') else 'de_DE'
            self.records.clear()
            self.before = [(p,len(p.chunks)) for p in self.players]
            if text.startswith('IMPORT:'):
                original = {'IMPORT:plain':'probenull', 'IMPORT:digits':'probeganz',
                            'IMPORT:regex':'probe57', 'IMPORT:legacy':'legacyprobe'}[text]
                self.speech.speak([original])
            elif text.startswith('DICT:'):
                self.speech.speak(['reloqtest ' + ('hello' if text == 'DICT:plain' else 'framebuffer')])
            elif text.startswith('ROLE:'):
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
