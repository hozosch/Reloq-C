"""Check both Persian test profiles in a running portable NVDA instance."""
import hashlib
import json
import os
from pathlib import Path
import time
import traceback

import globalPluginHandler
import wx

RESULT = Path(os.environ['RELOQ_SEGMENTATION_PROBE_RESULT'])


class Player:
    def __init__(self): self.chunks, self.times = [], []
    def feed(self, data, size=None, onDone=None):
        if callable(size) and onDone is None: onDone = size
        self.times.append(time.perf_counter())
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
        self.players, self.results, self.index, self.audio = [], [], 0, {}
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
            from speech.commands import CharacterModeCommand, IndexCommand, LangChangeCommand
            import synthDriverHandler
            import synthDrivers.reloqC as reloq
            import characterProcessing
            from synthDrivers.reloqC._persian import continuous_driver, direct_fa_sequence
            self.speech, self.character, self.index_command = speech, CharacterModeCommand, IndexCommand
            self.symbols = characterProcessing
            self.projected = []
            old_project_part = continuous_driver._project_part
            old_prepared = direct_fa_sequence.ScopedDirectPersianEngine._speak_prepared
            def observe_project(*args, **kwargs):
                result = old_project_part(*args, **kwargs)
                self.projected.append(result[0])
                return result
            def observe_prepared(instance, prepared, *args, **kwargs):
                if prepared.source:
                    self.projected.append(prepared.tagged)
                return old_prepared(instance, prepared, *args, **kwargs)
            continuous_driver._project_part = observe_project
            direct_fa_sequence.ScopedDirectPersianEngine._speak_prepared = observe_prepared
            self.language_command = LangChangeCommand
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
                          ('fa_IR', 'او آمد.', False),
                          ('fa_Latn', 'u amad.', False),
                          ('fa_IR', 'بدبخت', False),
                          ('fa_IR', 'تعجب', False),
                          ('fa_IR', '۱۰۰', False),
                          ('fa_IR', 'متوقف', False),
                          ('fa_IR', 'ذره بین به دلیلِ بُروزِ خطا متوقف شد.', 'pipeline_stop'),
                          ('fa_IR', 'حالتِ بومیِ انتخاب را فعال یا غیرفعال می‌کند', False),
                          ('fa_IR', 'ذره بین به دلیلِ بُروزِ خطا متوقف شد.', False),
                          ('fa_IR', 'توجه', False),
                          ('fa_IR', 'امتحان امکان آمپر', False),
                          ('fa_IR', 'کتابم کتابت کتابش کتابها', False),
                          ('fa_IR', 'سوال', False),
                          ('fa_Latn', 'badbakht soal?', False),
                          ('fa_Latn', 'salam khub?', False),
                          ('fa_Latn', 'salam khubtar?', False),
                          ('fa_Latn', 'salam phone donya', False),
                          ('fa_Latn', 'salam phone donya', 'english_exception'),
                          ('fa_IR', 'سلام (دنیا) 🙂', False),
                          ('fa_IR', 'عدد ۱۲-۱۵ است', False),
                          ('fa_IR', 'سلام (دنیا) خوب', False),
                          ('fa_IR', 'سلام، دنیا، خوب', False),
                          ('fa_Latn', 'salam (donya) khub', False),
                          ('fa_Latn', 'salam, donya, khub', False),
                          ('fa_IR', 'احیا مهربد معد معقر', False),
                          ('fa_IR', 'سلام (hello) دنیا', False),
                          ('fa_Latn', 'salam salām donya', False),
                          ('fa_Latn', 'salam salām donya', 'accent_exception'),
                          ('fa_Latn', 'fa\u0304rsi', False),
                          ('fa_Latn', 'farsi', False),
                          ('fa_Latn', 'salam-e', False),
                          ('fa_Latn', 'salam-e', 'pipeline'),
                          ('fa_Latn', 'salam-e', 'pipeline_most'),
                          ('fa_IR', 'استفاده از NVDA هنگامِ ورود به ویندوز (به اِختیاراتِ مدیریتی نیاز دارد)', 'pipeline'),
                          ('fa_IR', 'استفاده از NVDA هنگامِ ورود به ویندوز (به اِختیاراتِ مدیریتی نیاز دارد)', 'pipeline_most'),
                          ('fa_IR', 'استفاده از NVDA هنگامِ ورود به ویندوز (به اِختیاراتِ مدیریتی نیاز دارد)', 'checkbox_unchecked'),
                          ('fa_IR', 'استفاده از NVDA هنگامِ ورود به ویندوز (به اِختیاراتِ مدیریتی نیاز دارد)', 'checkbox_checked'),
                          ('fa_IR', 'استفاده از NVDA هنگامِ ورود به ویندوز (به اِختیاراتِ مدیریتی نیاز دارد)', 'checkbox_checked_most'),
                          ('fa_IR', 'نمایشِ اِتِّفاقاتِ ثبت‌شده (Log)', 'pipeline_log'),
                          ('fa_Latn', 'w', True),
                          ('fa_Latn', 'NVDA', True),
                          ('fa_IR', 'ق', True),
                          ('fa_IR', 'ا', True),
                          ('fa_IR', '؟', True),
                          ('fa_IR', '🙂', True),
                          ('fa_Latn', '5?', True),
                          ('fa_IR', 'سلام ؟ دنیا', 'mixedspelling'),
                          ('fa_IR', 'NVDA ', 'english'),
                          ('fa_IR', 'خوب', 'persian'),
                          ('fa_IR', 'NVDA خوب', 'mixed'),
                          ('fa_IR', 'NVDA خوب', False),
                          ('en_US', 'NVDA خوب', 'mixed'),
                          ('fa_IR', 'تنظیمات را باز کنید', False),
                          ('en_US', 'Open settings', False),
                          ('de_DE', 'Guten Tag.', False)]
            self.next()
        except BaseException:
            self.fail()

    def next(self):
        try:
            if self.index == len(self.cases):
                if self.results[0]['pcm_sha256'] != self.results[1]['pcm_sha256']:
                    raise AssertionError('confirmed script and Pinglish words differ')
                script_glide = next(row for row in self.results if row['text'] == 'او آمد.')
                roman_glide = next(row for row in self.results if row['text'] == 'u amad.')
                if script_glide['pcm_sha256'] != roman_glide['pcm_sha256']:
                    raise AssertionError('confirmed Persian and Pinglish vowel glides differ')
                mixed = next(row for row in self.results if row['spelling'] == 'mixed')
                plain = next(row for row in self.results
                             if row['text'] == 'NVDA خوب' and row['spelling'] is False)
                if mixed['pcm_sha256'] != plain['pcm_sha256']:
                    raise AssertionError('unmarked mixed text differs from the explicit switch')
                english_home = next(row for row in self.results
                                    if row['locale'] == 'en_US' and row['spelling'] == 'mixed')
                if mixed['pcm_sha256'] != english_home['pcm_sha256']:
                    raise AssertionError('English home language changed the mixed sentence')
                if self.audio['mixed'] == self.audio['english'] + self.audio['persian']:
                    raise AssertionError('mixed sentence was rendered as two disconnected sessions')
                automatic = next(row for row in self.results
                                 if row['text'] == 'salam phone donya' and row['spelling'] is False)
                explicit = next(row for row in self.results if row['spelling'] == 'english_exception')
                if automatic['pcm_sha256'] != explicit['pcm_sha256']:
                    raise AssertionError('Pinglish English exception differs from explicit routing')
                automatic = next(row for row in self.results
                                 if row['text'] == 'salam salām donya' and row['spelling'] is False)
                explicit = next(row for row in self.results if row['spelling'] == 'accent_exception')
                if automatic['pcm_sha256'] != explicit['pcm_sha256']:
                    raise AssertionError('accented Pinglish differs from explicit English routing')
                combined = next(row for row in self.results if row['text'] == 'fa\u0304rsi')
                ordinary = next(row for row in self.results if row['text'] == 'farsi')
                if combined['pcm_sha256'] != ordinary['pcm_sha256']:
                    raise AssertionError('combining Latin mark changed the Pinglish word')
                for locale, brackets, commas in (
                        ('fa_IR', 'سلام (دنیا) خوب', 'سلام، دنیا، خوب'),
                        ('fa_Latn', 'salam (donya) khub', 'salam, donya, khub')):
                    first = next(row for row in self.results if row['locale'] == locale and row['text'] == brackets)
                    second = next(row for row in self.results if row['locale'] == locale and row['text'] == commas)
                    if first['pcm_sha256'] != second['pcm_sha256']:
                        raise AssertionError('native bracket melody differs for ' + locale)
                RESULT.write_text(json.dumps({'ok': True, 'cases': self.results}, ensure_ascii=True), encoding='utf8')
                return
            locale, text, spelling = self.cases[self.index]
            self.speech.cancelSpeech()
            self.synth.language = locale
            if self.synth.language != locale:
                raise AssertionError('language selection did not stick: ' + locale)
            self.before = {id(player): len(player.chunks) for player in self.players}
            self.before_projected = len(self.projected)
            if spelling == 'mixed':
                sequence = [self.language_command('en_US'), 'NVDA ',
                            self.language_command('fa_IR'), 'خوب']
            elif spelling == 'english':
                sequence = [self.language_command('en_US'), text]
            elif spelling == 'persian':
                sequence = [text]
            elif spelling == 'mixedspelling':
                sequence = ['سلام ', self.character(True), '؟', self.character(False), ' دنیا']
            elif spelling == 'english_exception':
                sequence = ['salam ', self.language_command('en_US'), 'phone',
                            self.language_command('fa_Latn'), ' donya']
            elif spelling == 'accent_exception':
                sequence = ['salam ', self.language_command('en_US'), 'salām',
                            self.language_command('fa_Latn'), ' donya']
            elif spelling in ('checkbox_unchecked', 'checkbox_checked', 'checkbox_checked_most'):
                status = 'بدونِ علامت' if spelling == 'checkbox_unchecked' else 'علامت‌دار'
                # NVDA speaks an object's name, role and state as separate
                # sequence items; its real preprocessing adds the separators.
                sequence = [text, 'کادرِ علامت‌زدنی', status]
            elif spelling in ('pipeline', 'pipeline_most', 'pipeline_stop', 'pipeline_log'):
                sequence = [text]
            else:
                sequence = ([self.character(True), text, self.character(False)] if spelling else [text])
            self.started = time.perf_counter()
            if spelling in ('pipeline', 'pipeline_most', 'pipeline_stop', 'pipeline_log', 'checkbox_unchecked', 'checkbox_checked', 'checkbox_checked_most'):
                self.speech.speak([self.index_command(self.index + 1), *sequence],
                                  symbolLevel=(self.symbols.SymbolLevel.MOST if spelling in ('pipeline_most', 'checkbox_checked_most')
                                               else self.symbols.SymbolLevel.NONE))
            else:
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
            times = [value for player in self.players
                     for value in player.times[self.before.get(id(player), 0):]]
            if not times:
                raise RuntimeError('missing NVDA audio timing for ' + ascii(text))
            if spelling in ('pipeline', 'pipeline_most', 'pipeline_stop', 'pipeline_log', 'checkbox_unchecked', 'checkbox_checked', 'checkbox_checked_most'):
                tags = ' '.join(self.projected[self.before_projected:])
                if spelling in ('pipeline', 'pipeline_most') and locale == 'fa_IR' and ('`[.0hen.1ga.0me]' not in tags or
                        "`[.1'e:'G.0tja.2ra.0te]" not in tags):
                    raise AssertionError('NVDA preprocessing lost the vocalized Persian words: ' + ascii(tags))
                if spelling == 'pipeline_stop' and '`[.2mo.0te.1vAR.0Ref]' not in tags:
                    raise AssertionError('NVDA spelled the Persian word for stopped: ' + ascii(tags))
                if spelling == 'pipeline_log' and ("`[.1'e:'t.0te.1fa.0Ra.2te]" not in tags or
                        "`[.1sAbt] `[.1So.0d'e:']" not in tags):
                    raise AssertionError('NVDA spelled part of the normalized log label: ' + ascii(tags))
                if spelling in ('checkbox_unchecked', 'checkbox_checked', 'checkbox_checked_most'):
                    expected = ("`[.0'A:'.1la.0mAt]" if spelling == 'checkbox_unchecked' else
                                "`[.0'A:'.1la.0mAt] `[.1dar]")
                    if expected not in tags:
                        raise AssertionError('NVDA checkbox state was not spoken as a Persian word: ' + ascii(tags))
                if locale == 'fa_Latn' and ('`[.1ta.0vu.2merk.0ki]' in tags or '-`[' in tags):
                    raise AssertionError('Finnish hyphen name leaked into Pinglish')
            if spelling in ('english', 'persian') or spelling == 'mixed' and locale == 'fa_IR':
                self.audio[spelling] = audio
            self.results.append({'locale': locale, 'text': text, 'spelling': spelling,
                                 'pcm_bytes': len(audio), 'pcm_sha256': hashlib.sha256(audio).hexdigest(),
                                 'first_audio_ms': round((min(times) - self.started) * 1000, 1)})
            self.index += 1
            self.next()
        except BaseException:
            self.fail()
