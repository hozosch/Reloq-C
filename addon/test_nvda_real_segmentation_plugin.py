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
AMIR_WORDS = ('available', 'landmark', 'selection', 'Paperback', 'organically',
              'expression', 'Jackson', 'developmental', 'suggestion',
              'Jayson', 'Peterson', 'Tilton', 'Coleman', 'Petersen', 'Wiggins')
ACRONYM_REFERENCES = {'CDs': 'C Dees', 'DVDs': 'D V Dees', 'ROMs': 'roms',
                      'IDs': 'Eye Dees', 'PDFs': 'P D Effs', 'URLs': 'U R Els',
                      'PACs': 'packs', 'FAQs': 'facks', 'SMSs': 'S M Esses',
                      'BlindRadio': 'Blind Radio', 'BlindRSS': 'Blind RSS'}
ACRONYM_SENTENCE = ('She was also outspent on TV advertising, with a flood of ads '
                    'from left-leaning PACs and super PACs pushing support for Colom.')


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
        if hasattr(self, 'builtin_original'):
            self.builtin[:] = self.builtin_original
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
            import speechViewer
            self.viewer_records = []
            speechViewer.isActive = True
            speechViewer.appendSpeechSequence = lambda seq: self.viewer_records.append(
                ''.join(item for item in seq if isinstance(item, str)))
            self.speech = speech
            self.builtin = next(d.dictionary for d in speechDictHandler.listAvailableSpeechDictDefinitions()
                                if getattr(d.source, 'value', d.source) == 'builtin')
            self.builtin_original = list(self.builtin)
            if not self.builtin_original:
                raise RuntimeError('NVDA builtin dictionary fixture is empty')
            nvwave.WavePlayer = self.player
            reloq.SynthDriver._make_player = staticmethod(self.player)
            speech.cancelSpeech()
            if not synthDriverHandler.setSynth('reloqC'): raise RuntimeError('could not select ReLoq C')
            self.synth = synthDriverHandler.getSynth()
            temporary = next(d for d in speechDictHandler.listAvailableSpeechDictDefinitions()
                             if getattr(d.source, 'value', d.source) == 'temp')
            temporary.dictionary.append(SpeechDictEntry('reloqtest', 'swapfile'))
            temporary.dictionary.append(SpeechDictEntry('ABCs', 'Ersatz'))
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
            # These checks use the real SpeechDictEntry implementation from
            # NVDA rather than the lightweight host-test substitute. They
            # provide the reference behavior for other platform adapters.
            scoped = RESULT.parent / 'nvda-scoped-reference.dic'
            scoped_data = ('# Scoped NVDA dictionary\r\n'
                'literalprobe\t\\\\1\t1\t0\r\n'
                '(?P<stem>regprobe)([0-9]+)\t\\g<stem>-\\2\t1\t1\r\n'
                'strasse\tWeg\t0\t2\r\n'
                'teilprobe\tTeil\t1\t3\r\n'
                'anfangprobe\tAnfang\t1\t4\r\n'
                'endeprobe\tEnde\t1\t5\r\n'
                'wild?card\tPlatzhalter\t1\t6\r\n'
                '\\#zeichen\t\\#Ersatz\t1\t0\r\n').encode('utf-8-sig')
            scoped.write_bytes(scoped_data)
            _, scoped_count, created = manager.import_file(scoped, 0x10000, '2')
            if not created or scoped_count != 8:
                raise AssertionError('scoped NVDA dictionary was not imported')
            reference_cases = (
                ('literalprobe', r'\\1'),
                ('regprobe42', 'regprobe-42'),
                ('Straße', 'Weg'),
                ('teilprobe', 'teilprobe'),
                ('xteilprobe', 'xTeil'),
                ('anfangprobe', 'anfangprobe'),
                ('anfangprobex', 'Anfangx'),
                ('endeprobe', 'endeprobe'),
                ('xendeprobe', 'xEnde'),
                ('wildXcard', 'Platzhalter'),
                ('#zeichen', '#Ersatz'),
            )
            for text, expected in reference_cases:
                actual = manager.process(text, 0x10000, '2')
                if actual != expected:
                    raise AssertionError(f'NVDA reference {text!r}: {actual!r} != {expected!r}')
            for language, voice in ((0x10000, '1'), (0x10001, '2'), (0x40000, '2')):
                if manager.process('literalprobe', language, voice) != 'literalprobe':
                    raise AssertionError('NVDA dictionary escaped its language/voice scope')
            if scoped.read_bytes() != scoped_data:
                raise AssertionError('NVDA dictionary source was modified')
            self.synth.language = 'de_DE'
            self.synth.voiceTags = False
            original = self.synth._transport.submit
            def submit(parts):
                self.records.append([{'kind':type(item).__name__, 'text':getattr(item,'text',None),
                                      'original':getattr(item,'original',None),
                                      'guarded':getattr(item,'segmentation_guarded',False)} for item in parts])
                return original(parts)
            self.synth._transport.submit = submit
            own_native = RESULT.parent / 'own-native-DEU.dic'
            own_native.write_bytes(b'TestEintrag\tHaus\n')
            source = reloq.DictionarySource(str(own_native.resolve()), 0x40000, 0)
            self.synth.configure_dictionaries([source], True, lambda error, details: wx.CallAfter(self.ready, error))
        except BaseException:
            self.fail()

    def ready(self, error):
        try:
            if error: raise RuntimeError(str(error))
            if self.synth._wordseg_keys is None: raise RuntimeError('dictionary keys not ready')
            if 'YouTube' not in self.synth._wordseg_keys[0x40000]:
                raise RuntimeError('German community dictionary fixture missing')
            if 'TestEintrag' not in self.synth._wordseg_keys[0x40000]:
                raise RuntimeError('Own native dictionary fixture missing')
            english_keys = self.synth._wordseg_keys[0x10000]
            # Community dictionaries are updated independently. Do not assume
            # they lack real words: if a word has a dictionary entry, that
            # entry must take priority over automatic segmentation.
            self.ordinary_compound = next((word for word in ('framefile', 'swapbuffer')
                                           if word not in english_keys), None)
            if self.ordinary_compound is None:
                raise RuntimeError('all independent compound test words are in the dictionary')
            self.community_words = [word for word in ('swapfile', 'framebuffer') if word in english_keys]
            self.cases = [(word,camel,text) for text in ('YouTube Fenster.', 'YouTube Dokument.',
                'Vor YouTube Fenster. Danach weiter.', 'ROLE:window', 'ROLE:document')
                for word,camel in ((False,False),(True,False),(False,True),(True,True))]
            self.cases += [(word,False,text) for text in ('DICT:plain', 'DICT:compound')
                           for word in (False,True)]
            self.cases += [(word,False,'COMMUNITY:' + text) for text in self.community_words
                           for word in (False,True)]
            self.cases += [(word,False,dialect + ':' + text)
                           for dialect in ('ENUS', 'ENGB')
                           for text in ('fileswap',)
                           for word in (False,True)]
            self.cases += [(word,False,text) for text in ('IMPORT:plain', 'IMPORT:digits', 'IMPORT:regex', 'IMPORT:legacy')
                           for word in (False,True)]
            self.cases += [(word,camel,'ACR-' + lang + ':' + text)
                           for lang, texts in (('EN', ('CDs', 'DVDs', 'ROMs', 'IDs', 'PDFs', 'URLs',
                                                      'PACs', 'FAQs', 'SMSs', 'MP3', 'BlindRadio', 'BlindRSS')),
                                               ('ENGB', ('CDs', 'DVDs', 'ROMs', 'IDs', 'PDFs', 'URLs',
                                                        'PACs', 'FAQs', 'SMSs', 'MP3', 'BlindRadio', 'BlindRSS')),
                                               ('DE', ('CDs', 'BlindRadio')))
                           for text in texts
                           for word,camel in ((False,False),(True,False),(False,True),(True,True))]
            self.cases += [(word,camel,'ACRREF-' + lang + ':' + text)
                           for lang in ('EN', 'ENGB') for text in ACRONYM_REFERENCES
                           for word,camel in ((False,False),(True,False),(False,True),(True,True))
                           if camel or text not in ('BlindRadio', 'BlindRSS')]
            self.cases += [(word,camel,'ACRPHRASE-' + lang + ':' + mode)
                           for lang in ('EN', 'ENGB') for mode in ('actual', 'reference')
                           for word,camel in ((False,False),(True,False),(False,True),(True,True))]
            self.cases += [(word,camel,'AMIR-' + lang + ':sequence')
                           for lang in ('EN', 'ENGB')
                           for word,camel in ((False,False),(True,False),(False,True),(True,True))]
            self.cases += [(word,camel,'PERSONAL:ABCs')
                           for word,camel in ((False,False),(True,False),(False,True),(True,True))]
            self.cases += [(word,camel,'NATIVE:' + source) for source in ('TestEintrag', 'reference')
                           for word,camel in ((False,False),(True,False),(False,True),(True,True))]
            # The reporter had removed builtin.dic. Empty only this isolated
            # NVDA process's in-memory dictionary, leaving its file intact.
            self.cases += [(word,camel,'NOBUILTIN-' + lang + ':' + text)
                           for lang in ('EN', 'ENGB')
                           for text in ACRONYM_REFERENCES
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
                        if (row['word'] or row['camel']) and row['input'].startswith('ROLE:'):
                            label = 'Fenster' if row['input'] == 'ROLE:window' else 'Dokument'
                            if not re.search(r'YouTube\s+' + label, combined):
                                raise AssertionError('NVDA lost the object boundary: ' + repr(row))
                        if row['input'] == 'DICT:compound' and row['word']:
                            parts = [p for group in row['submissions'] for p in group]
                            if not any('swapfile' in (p['text'] or '') and p['guarded'] for p in parts):
                                raise AssertionError('NVDA dictionary replacement was not protected: ' + repr(row))
                            if not any(self.ordinary_compound in (p['text'] or '') and not p['guarded'] for p in parts):
                                raise AssertionError('unrelated word was protected too: ' + repr(row))
                        if row['input'].startswith('IMPORT:'):
                            expected = {'IMPORT:plain':'Ersatz', 'IMPORT:digits':'Termin 2035',
                                        'IMPORT:regex':'Zahl 57', 'IMPORT:legacy':'Sonderwort'}[row['input']]
                            if expected not in combined or '\t0\t' in combined or '\t1\t' in combined:
                                raise AssertionError('NVDA import changed a replacement or included options: ' + repr(row))
                        if row['input'].startswith('ACR-'):
                            original = row['input'].split(':', 1)[1]
                            active = (original in ('CDs', 'DVDs', 'ROMs', 'IDs', 'PDFs', 'URLs',
                                                   'PACs', 'FAQs', 'SMSs', 'MP3') or row['camel'] or
                                      (row['word'] and row['input'].startswith(('ACR-EN:', 'ACR-ENGB:'))))
                            if active and original not in combined:
                                raise AssertionError('NVDA builtin dictionary split a protected acronym or case word: ' + repr(row))
                            if not any(original in text for text in row['viewer']):
                                raise AssertionError('NVDA speech viewer lost the original word: ' + repr(row))
                        if row['input'] == 'PERSONAL:ABCs' and 'Ersatz' not in combined:
                            raise AssertionError('Personal NVDA dictionary did not take precedence: ' + repr(row))
                        if row['input'] == 'PERSONAL:ABCs' and not any('ABCs' in text for text in row['viewer']):
                            raise AssertionError('NVDA speech viewer lost the personal-dictionary text: ' + repr(row))
                        if row['input'] == 'NATIVE:TestEintrag' and 'TestEintrag' not in combined:
                            raise AssertionError('Own native dictionary key was split: ' + repr(row))
                        if row['input'] == 'NATIVE:TestEintrag' and not any('TestEintrag' in text for text in row['viewer']):
                            raise AssertionError('NVDA speech viewer lost the native-dictionary text: ' + repr(row))
                        if row['input'].startswith('YouTube ') and 'YouTube' not in combined:
                            raise AssertionError('Native dictionary key was split: ' + repr(row))
                        if row['input'].startswith('AMIR-'):
                            position = 0
                            for item in AMIR_WORDS:
                                match = re.search(r'(?<!\w)' + re.escape(item) + r'(?!\w)', combined[position:])
                                if not match:
                                    raise AssertionError('NVDA lost or joined an Amir word: ' + repr(row))
                                position += match.end()
                    for language in ('EN', 'ENGB'):
                        amir_rows = [row for row in self.results if row['input'] == 'AMIR-' + language + ':sequence']
                        if len(amir_rows) != 4 or len({row['pcm_sha256'] for row in amir_rows}) != 1:
                            raise AssertionError('An Amir word changed with segmentation: ' + repr(amir_rows))
                        for word, camel in ((False,False),(True,False),(False,True),(True,True)):
                            for original in ACRONYM_REFERENCES:
                                if not camel and original in ('BlindRadio', 'BlindRSS'):
                                    continue
                                spoken = next(row for row in self.results if row['input'] == 'ACR-' + language + ':' + original
                                              and (row['word'],row['camel']) == (word,camel))
                                reference = next(row for row in self.results if row['input'] == 'ACRREF-' + language + ':' + original
                                                 and (row['word'],row['camel']) == (word,camel))
                                if spoken['pcm_sha256'] != reference['pcm_sha256']:
                                    raise AssertionError('Amir pronunciation differed from its reference: ' + repr(spoken))
                            for original in ACRONYM_REFERENCES:
                                if not camel and original in ('BlindRadio', 'BlindRSS'):
                                    continue
                                normal = next(row for row in self.results if row['input'] == 'ACR-' + language + ':' + original
                                              and (row['word'],row['camel']) == (word,camel))
                                no_builtin = next(row for row in self.results if row['input'] == 'NOBUILTIN-' + language + ':' + original
                                                  and (row['word'],row['camel']) == (word,camel))
                                if normal['pcm_sha256'] != no_builtin['pcm_sha256']:
                                    raise AssertionError('builtin dictionary changed protected speech: ' + repr(no_builtin))
                            sentence = next(row for row in self.results if row['input'] == 'ACRPHRASE-' + language + ':actual'
                                            and (row['word'],row['camel']) == (word,camel))
                            reference = next(row for row in self.results if row['input'] == 'ACRPHRASE-' + language + ':reference'
                                             and (row['word'],row['camel']) == (word,camel))
                            if sentence['pcm_sha256'] != reference['pcm_sha256']:
                                raise AssertionError('PACs sentence differed from packs: ' + repr(sentence))
                    for word, camel in ((False,False),(True,False),(False,True),(True,True)):
                        native = next(row for row in self.results if row['input'] == 'NATIVE:TestEintrag'
                                      and (row['word'],row['camel']) == (word,camel))
                        reference = next(row for row in self.results if row['input'] == 'NATIVE:reference'
                                         and (row['word'],row['camel']) == (word,camel))
                        if native['pcm_sha256'] != reference['pcm_sha256']:
                            raise AssertionError('Own native dictionary replacement was not applied: ' + repr(native))
                    plain = [row for row in self.results if row['input'] == 'DICT:plain']
                    if len(plain) != 2 or plain[0]['pcm_sha256'] != plain[1]['pcm_sha256']:
                        raise AssertionError('personal dictionary changed unrelated NVDA speech: ' + repr(plain))
                    compounds = [row for row in self.results if row['input'] == 'DICT:compound']
                    if len(compounds) != 2 or compounds[0]['pcm_sha256'] == compounds[1]['pcm_sha256']:
                        raise AssertionError('unrelated English compound was not segmented: ' + repr(compounds))
                    for word in self.community_words:
                        rows = [row for row in self.results if row['input'] == 'COMMUNITY:' + word]
                        if len(rows) != 2 or rows[0]['pcm_sha256'] != rows[1]['pcm_sha256']:
                            raise AssertionError('community dictionary was overridden: ' + repr(rows))
                    for dialect in ('ENUS', 'ENGB'):
                        for text in ('fileswap',):
                            rows = [row for row in self.results if row['input'] == dialect + ':' + text]
                            if len(rows) != 2 or rows[0]['pcm_sha256'] == rows[1]['pcm_sha256']:
                                raise AssertionError('English compound was not segmented in NVDA: ' + repr(rows))
                self.builtin[:] = self.builtin_original
                RESULT.write_text(json.dumps({'ok':True,'cases':self.results}, ensure_ascii=False),encoding='utf8')
                return
            word,camel,text = self.cases[self.case]
            self.speech.cancelSpeech()
            if text.startswith('NOBUILTIN-') and self.builtin:
                self.builtin.clear()
            self.synth._wordSegmentation = word
            self.synth._camelCase = camel
            self.synth.language = ('en_GB' if text.startswith(('ENGB:', 'ACR-ENGB:', 'ACRREF-ENGB:', 'ACRPHRASE-ENGB:', 'AMIR-ENGB:', 'NOBUILTIN-ENGB:')) else
                                   'en_US' if text.startswith(('ENUS:', 'DICT:', 'COMMUNITY:', 'ACR-EN:', 'ACRREF-EN:', 'ACRPHRASE-EN:', 'AMIR-EN:', 'NOBUILTIN-EN:')) else 'de_DE')
            self.records.clear()
            self.viewer_records.clear()
            self.before = [(p,len(p.chunks)) for p in self.players]
            if text.startswith('IMPORT:'):
                original = {'IMPORT:plain':'probenull', 'IMPORT:digits':'probeganz',
                            'IMPORT:regex':'probe57', 'IMPORT:legacy':'legacyprobe'}[text]
                self.speech.speak([original])
            elif text.startswith('DICT:'):
                self.speech.speak(['reloqtest ' + ('hello' if text == 'DICT:plain' else self.ordinary_compound)])
            elif text.startswith('COMMUNITY:'):
                self.speech.speak([text.split(':', 1)[1]])
            elif text.startswith(('ENUS:', 'ENGB:')):
                self.speech.speak([text.split(':', 1)[1]])
            elif text.startswith('ACR-'):
                self.speech.speak([text.split(':', 1)[1]])
            elif text.startswith('ACRREF-'):
                self.speech.speak([ACRONYM_REFERENCES[text.split(':', 1)[1]]])
            elif text.startswith('ACRPHRASE-'):
                mode = text.split(':', 1)[1]
                self.speech.speak([ACRONYM_SENTENCE if mode == 'actual' else ACRONYM_SENTENCE.replace('PACs', 'packs')])
            elif text.startswith('AMIR-'):
                self.speech.speak(list(AMIR_WORDS))
            elif text.startswith('NOBUILTIN-'):
                self.speech.speak([text.split(':', 1)[1]])
            elif text.startswith('PERSONAL:'):
                self.speech.speak([text.split(':', 1)[1]])
            elif text.startswith('NATIVE:'):
                self.speech.speak(['Haus' if text == 'NATIVE:reference' else text.split(':', 1)[1]])
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
                                 'viewer':self.viewer_records[:],
                                 'pcm_bytes':len(audio),'pcm_sha256':hashlib.sha256(audio).hexdigest()})
            self.case += 1
            self.next()
        except BaseException:
            self.fail()
