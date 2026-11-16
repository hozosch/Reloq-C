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
        self.done_for_case = -1
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
            from synthDrivers.reloqC._persian import continuous_driver, direct_fa_sequence, nvda_driver_probe
            from synthDrivers.reloqC._reloq_eci_transport import EciTextTransport
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
            # Keep these measurements in the test plugin, not in the voice.
            # They separate NVDA preprocessing, queueing and synthesis while
            # using the same real driver and per-request text processing.
            self.profile_case = False
            self.stage = {}
            def trace(method, name):
                def observed(*args, **kwargs):
                    if self.profile_case:
                        self.stage.setdefault(name + '_begin', time.perf_counter())
                    try:
                        return method(*args, **kwargs)
                    finally:
                        if self.profile_case:
                            self.stage.setdefault(name + '_end', time.perf_counter())
                return observed
            reloq.SynthDriver._protect_before_speech = trace(reloq.SynthDriver._protect_before_speech, 'protect')
            reloq.SynthDriver.speak = trace(reloq.SynthDriver.speak, 'driver')
            EciTextTransport._run_job = trace(EciTextTransport._run_job, 'job')
            nvda_driver_probe.PersianProbeLanguageEngine.speak = trace(
                nvda_driver_probe.PersianProbeLanguageEngine.speak, 'engine')
            self.language_command = LangChangeCommand
            nvwave.WavePlayer = self.player
            reloq.SynthDriver._make_player = staticmethod(self.player)
            speech.cancelSpeech()
            if not synthDriverHandler.setSynth('reloqC'):
                raise RuntimeError('could not select ReLoq C')
            self.synth = synthDriverHandler.getSynth()
            self.done_signal = synthDriverHandler.synthDoneSpeaking
            self.done_signal.register(self.speech_done)
            from speech import speech as speech_module
            self.speech_manager = getattr(speech_module, '_manager', None)
            for locale in ('fa_IR', 'fa_Latn'):
                if locale not in self.synth.availableLanguages:
                    raise AssertionError('Persian test profile is absent: ' + locale)
            self.cases = [('fa_IR', 'سلام دنیا؟', False),
                          ('fa_Latn', 'salam donya?', False),
                          ('fa_IR', 'او آمد.', False),
                          ('fa_Latn', 'u amad.', False),
                          ('fa_IR', 'بدبخت', False),
                          ('fa_IR', 'خطِ تیره', False),
                          ('fa_IR', 'زیرخط', False),
                          ('fa_IR', 'تعجب', False),
                          ('fa_IR', '۱۰۰', False),
                          ('fa_IR', 'متوقف', False),
                          ('fa_IR', 'علامت', False),
                          ('fa_IR', 'نشان دادَنِ گزینه‌های خروج هنگامِ خارج شدن از NVDA', 'pipeline_user_latency'),
                          ('fa_IR', 'نشان دادَنِ گزینه‌های خروج هنگامِ خارج شدن از NVDA', 'pipeline_user_latency'),
                          ('fa_IR', 'اکنون نشان دادَنِ گزینه‌های خروج هنگامِ خارج شدن از NVDA', 'pipeline_user_latency'),
                          ('fa_IR', 'نشان دادَنِ گزینه‌های خروج هنگامِ خارج شدن از NVDA', 'driver_user_latency'),
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
                          ('fa_IR', 'سلام[دنیا]سلام', 'pipeline_symbols'),
                          ('fa_IR', 'سلام-دنیا', 'pipeline_symbols'),
                          ('fa_Latn', 'salam(donya)salam', 'pipeline_symbols'),
                          ('fa_IR', 'سلام NVDA[]سلام', 'pipeline_mixed_symbols'),
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
                          ('fa_IR', 'version 3.5 is available.', 'decimal_split'),
                          ('en_US', 'version 3.5 is available.', 'decimal_split'),
                          ('fa_IR', 'version 3.5 is available.', 'decimal_pipeline'),
                          ('en_US', 'version 3.5 is available.', 'decimal_pipeline'),
                          ('fa_IR', 'version 3.5 is available.', 'decimal_indexed_pipeline'),
                          ('fa_IR', 'version 1,234.5 is available.', 'decimal_grouped_pipeline'),
                          ('en_US', 'version 1,234.5 is available.', 'decimal_grouped_pipeline'),
                          ('fa_IR', 'version -3.5 is available.', 'decimal_signed_pipeline'),
                          ('en_US', 'version -3.5 is available.', 'decimal_signed_pipeline'),
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
                for mode in ('decimal_split', 'decimal_pipeline', 'decimal_grouped_pipeline', 'decimal_signed_pipeline'):
                    persian = next(row for row in self.results if row['locale'] == 'fa_IR' and row['spelling'] == mode)
                    english = next(row for row in self.results if row['locale'] == 'en_US' and row['spelling'] == mode)
                    if persian['pcm_sha256'] != english['pcm_sha256']:
                        raise AssertionError('English decimal changed language in the ' + mode + ' case')
                ordinary = next(row for row in self.results if row['locale'] == 'fa_IR' and row['spelling'] == 'decimal_pipeline')
                indexed = next(row for row in self.results if row['locale'] == 'fa_IR' and row['spelling'] == 'decimal_indexed_pipeline')
                if ordinary['pcm_sha256'] != indexed['pcm_sha256']:
                    raise AssertionError('NVDA indexes changed the English decimal')
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
                self.done_signal.unregister(self.speech_done)
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
            elif spelling in ('persian', 'driver_user_latency'):
                sequence = [text]
            elif spelling == 'mixedspelling':
                sequence = ['سلام ', self.character(True), '؟', self.character(False), ' دنیا']
            elif spelling == 'english_exception':
                sequence = ['salam ', self.language_command('en_US'), 'phone',
                            self.language_command('fa_Latn'), ' donya']
            elif spelling == 'accent_exception':
                sequence = ['salam ', self.language_command('en_US'), 'salām',
                            self.language_command('fa_Latn'), ' donya']
            elif spelling == 'decimal_split':
                sequence = ['version ', self.index_command(1000 + self.index), '3',
                            self.index_command(2000 + self.index), '.', '5 is available.']
            elif spelling == 'decimal_pipeline':
                sequence = ['version ', '3.5 is available.']
            elif spelling == 'decimal_indexed_pipeline':
                sequence = ['version ', '3', self.index_command(3000 + self.index), '.',
                            self.index_command(4000 + self.index), '5 is available.']
            elif spelling == 'decimal_grouped_pipeline':
                sequence = ['version ', '1,234.5 is available.']
            elif spelling == 'decimal_signed_pipeline':
                sequence = ['version ', '-3.5 is available.']
            elif spelling in ('checkbox_unchecked', 'checkbox_checked', 'checkbox_checked_most'):
                status = 'بدونِ علامت' if spelling == 'checkbox_unchecked' else 'علامت‌دار'
                # NVDA speaks an object's name, role and state as separate
                # sequence items; its real preprocessing adds the separators.
                sequence = [text, 'کادرِ علامت‌زدنی', status]
            elif spelling in ('pipeline', 'pipeline_most', 'pipeline_stop', 'pipeline_log', 'pipeline_symbols', 'pipeline_mixed_symbols', 'pipeline_user_latency'):
                sequence = [text]
            else:
                sequence = ([self.character(True), text, self.character(False)] if spelling else [text])
            self.profile_case = spelling in ('pipeline_user_latency', 'driver_user_latency')
            self.stage = {}
            self.started = time.perf_counter()
            self.pipeline = spelling in ('pipeline', 'pipeline_most', 'pipeline_stop', 'pipeline_log', 'pipeline_symbols', 'pipeline_mixed_symbols', 'pipeline_user_latency', 'checkbox_unchecked', 'checkbox_checked', 'checkbox_checked_most', 'decimal_pipeline', 'decimal_indexed_pipeline', 'decimal_grouped_pipeline', 'decimal_signed_pipeline')
            if self.pipeline:
                self.speech.speak([self.index_command(self.index + 1), *sequence],
                                  symbolLevel=(self.symbols.SymbolLevel.MOST if spelling in ('pipeline_most', 'checkbox_checked_most')
                                               else self.symbols.SymbolLevel.NONE))
            else:
                self.synth.speak([self.index_command(self.index + 1), *sequence])
            # Prefer the real completion signal, retaining the original
            # 1.6-second bound if an NVDA version does not expose it.
            wx.CallLater(35, self.wait_for_completion, self.index, time.monotonic() + 1.6)
        except BaseException:
            self.fail()

    def speech_done(self, synth=None, **_kwargs):
        if synth is self.synth:
            self.done_for_case = self.index

    def wait_for_completion(self, index, deadline):
        try:
            if index != self.index:
                return
            idle = not self.pipeline
            if self.pipeline and self.speech_manager is not None:
                idle = (self.speech_manager._hasNoMoreSpeech() and
                        not self.speech_manager._synthStillSpeaking())
            if (self.done_for_case == index and idle) or time.monotonic() >= deadline:
                self.capture()
            else:
                wx.CallLater(35, self.wait_for_completion, index, deadline)
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
            if spelling in ('pipeline', 'pipeline_most', 'pipeline_stop', 'pipeline_log', 'pipeline_symbols', 'pipeline_mixed_symbols', 'pipeline_user_latency', 'checkbox_unchecked', 'checkbox_checked', 'checkbox_checked_most'):
                tags = ' '.join(self.projected[self.before_projected:])
                if spelling in ('pipeline', 'pipeline_most') and locale == 'fa_IR' and ('`[.0hen.1ga.0me]' not in tags or
                        "`[.1'e:'G.0tja.2ra.0te]" not in tags):
                    raise AssertionError('NVDA preprocessing lost the vocalized Persian words: ' + ascii(tags))
                if spelling == 'pipeline_stop' and '`[.2mo.0te.1vAR.0Ref]' not in tags:
                    raise AssertionError('NVDA spelled the Persian word for stopped: ' + ascii(tags))
                if spelling == 'pipeline_log' and ("`[.1'e:'t.0te.1fa.0Ra.2te]" not in tags or
                        "`[.1sAbt.1So.0d'e:']" not in tags):
                    raise AssertionError('NVDA spelled part of the normalized log label: ' + ascii(tags))
                if spelling == 'pipeline_user_latency' and any(tag not in tags for tag in (
                        '`[.1ne.0San]', '`[.1da.0dA.2ne]', '`[.0hen.1ga.0me]', '`[.1Ga.0reJ]')):
                    raise AssertionError('NVDA spelled part of the reported latency example: ' + ascii(tags))
                if spelling == 'pipeline_symbols' and ('`[.0sA.1lam]' not in tags or
                        '`[.1do.0nja]' not in tags or '`[.1ta.0vu.2merk.0ki]' in tags):
                    raise AssertionError('NVDA punctuation damaged a Persian word: ' + ascii(tags))
                if spelling == 'pipeline_mixed_symbols' and ('`[.0sA.1lam]' not in tags or
                        '`[.1ta.0vu.2merk.0ki]' in tags):
                    raise AssertionError('NVDA mixed punctuation damaged Persian speech: ' + ascii(tags))
                if spelling in ('checkbox_unchecked', 'checkbox_checked', 'checkbox_checked_most'):
                    expected = ("`[.0A.1la.0mAt]" if spelling == 'checkbox_unchecked' else
                                "`[.0A.1la.0mAt] `[.1dar]")
                    if expected not in tags:
                        raise AssertionError('NVDA checkbox state was not spoken as a Persian word: ' + ascii(tags))
                if locale == 'fa_Latn' and ('`[.1ta.0vu.2merk.0ki]' in tags or '-`[' in tags):
                    raise AssertionError('Finnish hyphen name leaked into Pinglish')
            if spelling in ('english', 'persian') or spelling == 'mixed' and locale == 'fa_IR':
                self.audio[spelling] = audio
            result = {'locale': locale, 'text': text, 'spelling': spelling,
                      'pcm_bytes': len(audio), 'pcm_sha256': hashlib.sha256(audio).hexdigest(),
                      'first_audio_ms': round((min(times) - self.started) * 1000, 1)}
            if self.profile_case:
                result['stage_ms'] = {name: round((value - self.started) * 1000, 1)
                                      for name, value in self.stage.items()}
            self.results.append(result)
            self.index += 1
            self.next()
        except BaseException:
            self.fail()
