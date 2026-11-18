"""Check both Persian test profiles in a running portable NVDA instance."""
import hashlib
import json
import os
from pathlib import Path
import re
import time
import traceback
import wave

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
            from speech.commands import CharacterModeCommand, IndexCommand, LangChangeCommand, PitchCommand
            import synthDriverHandler
            import synthDrivers.reloqC as reloq
            import characterProcessing
            from synthDrivers.reloqC._persian import continuous_driver, direct_fa_sequence, korean_contour, nvda_driver_probe
            from synthDrivers.reloqC._persian.persian_symbols import SYMBOL_TAGS
            from synthDrivers.reloqC._reloq_eci_transport import EciTextTransport
            from synthDrivers.reloqC._reloq_persian_frames import FrameCapture
            self.speech, self.character, self.index_command = speech, CharacterModeCommand, IndexCommand
            self.symbol_tags = SYMBOL_TAGS
            # Reproduce dev136's donor mapping inside the same real NVDA
            # driver to compare the user's old and new slider settings.
            self.reference_calibration = False
            calibrated_pitch = korean_contour.korean_pitch
            calibrated_inflection = korean_contour.korean_inflection
            def comparison_pitch(value):
                return value if self.reference_calibration else calibrated_pitch(value)
            def comparison_inflection(value):
                if self.reference_calibration:
                    return ((value * 37 + 15) // 30 if value <= 30 else
                            37 + ((value - 30) * 63 + 35) // 70)
                return calibrated_inflection(value)
            korean_contour.korean_pitch = comparison_pitch
            korean_contour.korean_inflection = comparison_inflection
            self.sample_finnish = False
            original_capture_init = FrameCapture.__init__
            self.frame_capture_class, self.original_capture_init = FrameCapture, original_capture_init
            def comparison_capture_init(instance, dll):
                original_capture_init(instance, dll)
                if self.sample_finnish:
                    instance.transfer_korean_pitch = None
            FrameCapture.__init__ = comparison_capture_init
            self.pitch_command = PitchCommand
            self.symbols = characterProcessing
            self.projected = []
            self.projected_sources = []
            self.emoji_submissions = []
            self.korean_calls = []
            self.version_case = False
            self.version_debug = []
            self.emoji_case = False
            self.user_controls = []
            self.continuous = []
            self.us_fits = []
            self.nav_stale_case = False
            self.nav_stale_injected = False
            old_project_part = continuous_driver._project_part
            old_prepared = direct_fa_sequence.ScopedDirectPersianEngine._speak_prepared
            old_continuous = nvda_driver_probe.try_continuous
            old_glides = nvda_driver_probe.try_persian_glides
            old_us_fit = continuous_driver.fit_us_word_pitch
            old_runs = nvda_driver_probe._script_runs
            def observe_runs(text):
                result = old_runs(text)
                if self.version_case:
                    self.version_debug.append(('runs', text, result))
                return result
            nvda_driver_probe._script_runs = observe_runs
            old_korean = korean_contour.KoreanContour.apply
            def observe_korean(instance, rows, count, tagged, *args, **kwargs):
                result = old_korean(instance, rows, count, tagged, *args, **kwargs)
                if result:
                    self.korean_calls.append(tagged)
                return result
            korean_contour.KoreanContour.apply = observe_korean
            def observe_project(*args, **kwargs):
                result = old_project_part(*args, **kwargs)
                self.projected.append(result[0])
                return result
            def observe_prepared(instance, prepared, *args, **kwargs):
                self.user_controls.extend(part.tag_control for part in prepared.parts
                                          if getattr(part, 'tag_control', None))
                if prepared.source:
                    self.projected.append(prepared.tagged)
                    self.projected_sources.append((prepared.source, prepared.tagged))
                return old_prepared(instance, prepared, *args, **kwargs)
            def observe_continuous(*args, **kwargs):
                result = old_continuous(*args, **kwargs)
                if result is not None:
                    self.continuous.append(result)
                return result
            def observe_us_fit(*args, **kwargs):
                result = old_us_fit(*args, **kwargs)
                self.us_fits.append(result)
                return result
            def inject_stale_us_stop(owner, *args, **kwargs):
                if self.nav_stale_case and not self.nav_stale_injected:
                    # The previous job was stopped on the English handle;
                    # the next request itself is not cancelled.
                    english = owner._activate(owner._key(nvda_driver_probe.US_ENGLISH.id,
                                                         owner._home[1]))
                    english.cancel()
                    self.nav_stale_injected = True
                return old_glides(owner, *args, **kwargs)
            continuous_driver._project_part = observe_project
            continuous_driver.fit_us_word_pitch = observe_us_fit
            direct_fa_sequence.ScopedDirectPersianEngine._speak_prepared = observe_prepared
            nvda_driver_probe.try_continuous = observe_continuous
            nvda_driver_probe.try_persian_glides = inject_stale_us_stop
            # Keep these measurements in the test plugin, not in the voice.
            # They separate NVDA preprocessing, queueing and synthesis while
            # using the same real driver and per-request text processing.
            self.profile_case = False
            self.stage = {}
            self.symbol_preparations = []
            self.native_preparations = []
            old_select = nvda_driver_probe.PersianProbeLanguageEngine.select_voice
            def observe_select(instance, language, voice):
                start = time.perf_counter()
                try:
                    return old_select(instance, language, voice)
                finally:
                    if language in (nvda_driver_probe.PERSIAN_PROBE.id, nvda_driver_probe.PINGLISH_PROBE.id):
                        self.native_preparations.append({
                            'language': language,
                            'milliseconds': round((time.perf_counter() - start) * 1000, 1),
                            'english_ready': bool(getattr(instance, '_persian_english_prepared', False))})
            nvda_driver_probe.PersianProbeLanguageEngine.select_voice = observe_select
            original_prepare = reloq.prepare_persian_symbols
            def observe_prepare(locale):
                start = time.perf_counter()
                result = original_prepare(locale)
                if locale in ('fa_IR', 'fa_Latn'):
                    self.symbol_preparations.append({'locale': locale,
                        'milliseconds': round((time.perf_counter() - start) * 1000, 1)})
                return result
            reloq.prepare_persian_symbols = observe_prepare
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
            reloq.SynthDriver._protect_tags = trace(reloq.SynthDriver._protect_tags, 'tags')
            reloq.protect_joiners = trace(reloq.protect_joiners, 'joiners')
            original_decimals = reloq.protect_decimals
            def observe_decimals(driver, sequence, **kwargs):
                result = original_decimals(driver, sequence, **kwargs)
                if self.version_case:
                    self.version_debug.append(('decimals', repr(sequence), repr(result)))
                return result
            reloq.protect_decimals = trace(observe_decimals, 'decimals')
            characterProcessing.processSpeechSymbols = trace(characterProcessing.processSpeechSymbols, 'symbols')
            original_synth_speak = reloq.SynthDriver.speak
            def observe_synth_speech(instance, sequence):
                if self.emoji_case:
                    self.emoji_submissions.append(tuple(item for item in sequence if isinstance(item, str)))
                if self.version_case:
                    self.version_debug.append(('synth', repr(sequence)))
                return original_synth_speak(instance, sequence)
            reloq.SynthDriver.speak = trace(observe_synth_speech, 'driver')
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
            import speechDictHandler
            builtin = next(d for d in speechDictHandler.listAvailableSpeechDictDefinitions()
                           if getattr(d.source, 'value', d.source) == 'builtin')
            if not builtin.enabled or not builtin.dictionary:
                raise RuntimeError('NVDA builtin dictionary must be enabled for the version tests')
            builtin_version = 'v1.5.0'
            for entry in builtin.dictionary:
                builtin_version = entry.sub(builtin_version)
            if builtin_version != 'v 1.5.0':
                raise RuntimeError('NVDA builtin dictionary did not split the version fixture')
            self.done_signal = synthDriverHandler.synthDoneSpeaking
            self.done_signal.register(self.speech_done)
            from speech import speech as speech_module
            self.speech_manager = getattr(speech_module, '_manager', None)
            for locale in ('fa_IR', 'fa_Latn'):
                if locale not in self.synth.availableLanguages:
                    raise AssertionError('Persian test profile is absent: ' + locale)
            self.cases = [('fa_IR', 'سلام دنیا؟', False),
                          ('fa_Latn', 'salam donya?', False),
                          ('fa_IR', '`vs30 سلام دنیا', 'tags_slow'),
                          ('fa_IR', '`vs180 سلام دنیا', 'tags_fast'),
                          ('fa_IR', 'سلام دنیا', 'tags_plain'),
                          ('fa_IR', '`vbhz220 سلام دنیا', 'tags_pitch'),
                          ('fa_IR', '`[.1sAd] سلام', 'tags_phonemes'),
                          ('fa_Latn', '`vs30 salam donya', 'tags_slow'),
                          ('fa_Latn', '`vs180 salam donya', 'tags_fast'),
                          ('fa_IR', 'او آمد.', False),
                          ('fa_Latn', 'u amad.', False),
                          ('fa_IR', 'بدبخت', False),
                          ('fa_IR', 'خطِ تیره', False),
                          ('fa_IR', 'زیرخط', False),
                          ('fa_IR', 'تعجب', False),
                          ('fa_IR', '۱۰۰', False),
                          ('fa_IR', 'متوقف', False),
                          ('fa_IR', 'علامت', False),
                          ('fa_IR', 'به علاوه', False),
                          ('fa_IR', 'بعلاوه', False),
                          ('fa_IR', '+', True),
                          ('fa_IR', '/', True),
                          ('fa_IR', '\\', True),
                          ('fa_IR', 'نشان دادَنِ گزینه‌های خروج هنگامِ خارج شدن از NVDA', 'pipeline_user_latency'),
                          ('fa_IR', 'نشان دادَنِ گزینه‌های خروج هنگامِ خارج شدن از NVDA', 'pipeline_user_latency'),
                          ('fa_IR', 'اکنون نشان دادَنِ گزینه‌های خروج هنگامِ خارج شدن از NVDA', 'pipeline_user_latency'),
                          ('fa_IR', 'NVDA نشان دادَنِ گزینه‌های خروج هنگامِ خارج شدن از', 'pipeline_mixed_latency'),
                          ('fa_IR', 'NVDA سلام دنیا', 'pipeline_mixed_latency'),
                          ('fa_IR', 'NVDA خوب', 'pipeline_mixed_latency'),
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
                          ('fa_IR', 'سلام «دنیا» خوب', 'issue49_quotes'),
                          ('fa_IR', 'سلام (دنیا) خوب', 'issue49_quotes'),
                          ('fa_IR', '4 of 14', 'issue49_numeric'),
                          ('en_US', '4 of 14', 'issue49_numeric'),
                          *((locale, text, mode) for text in (
                              'Google Chrome, version 157.0.8103.79 (AMD64)',
                              'BlindRadio, version 0.2.100',
                              'v1.5.0', 'NVDA1.5.0')
                            for locale, mode in (('fa_IR', 'issue49_version_some'),
                                                 ('en_US', 'issue49_version_some'),
                                                 ('fa_IR', 'issue49_version_split'))),
                          ('fa_Latn', 'v1.5.0', 'issue49_version_some'),
                          ('fa_Latn', 'v1.5.0', 'issue49_version_split'),
                          ('fa_Latn', 'NVDA1.5.0', 'issue49_version_some'),
                          ('fa_Latn', 'NVDA1.5.0', 'issue49_version_split'),
                          *(( 'fa_IR', emoji, mode) for emoji in ('⚽️', '⚡️', '❤️')
                            for mode in ('issue49_emoji_some', 'issue49_emoji_split')),
                          *(('fa_IR', f'سرنوشتارِ سطحِ {level}ِ بعدی‌ای نیست', 'issue49_heading')
                            for level in range(1, 7)),
                          ('fa_IR', 'سرنوشتارِ سطحِ 2ِ قبلی‌ای نیست', 'issue49_heading_split'),
                          ('fa_IR', 'سلام دنیا. این یک آزمایش است؟', 'issue49_korean'),
                          ('fa_IR', 'NVDA سلام دنیا', 'issue49_korean'),
                          ('fa_IR', 'Hello. سلام دنیا', 'issue49_korean'),
                          ('fa_IR', 'Hello، سلام دنیا', 'issue49_korean'),
                          ('fa_IR', 'سلام NVDA دنیا', 'issue49_korean'),
                          ('fa_IR', 'سلام', 'issue49_korean_word'),
                          ('fa_IR', 'یک', 'issue49_korean_word'),
                          ('fa_Latn', 'salam', 'issue49_korean_word'),
                          ('fa_IR', 'سلام', 'issue49_korean_pitch'),
                          ('fa_IR', 'سلام دنیا', 'issue49_korean_mid_pitch'),
                          ('fa_IR', 'سلام', 'issue49_korean_spelling'),
                          ('fa_IR', 'می‌خواهم درسته', 'issue49_persian_unicode'),
                          ('fa_IR', 'می‌خواهم درسته', 'issue49_persian_unicode_none'),
                          ('fa_IR', 'می‌خواهم درسته', 'issue49_persian_unicode_all'),
                          ('fa_IR', 'This is an English test.', 'issue49_korean_english'),
                          ('fa_Latn', 'salam (donya) khub', False),
                          ('fa_Latn', 'salam, donya, khub', False),
                          ('fa_IR', 'سلام (دنیا)', 'pipeline_bracket'),
                          ('fa_IR', 'سلام (دنیا),', 'pipeline_bracket'),
                          ('fa_IR', 'سلام (NVDA): خوب', 'pipeline_bracket'),
                          ('fa_IR', 'سلام (NVDA: خوب', 'pipeline_bracket'),
                          ('fa_IR', 'سلام (دنیا):', 'pipeline_bracket'),
                          ('fa_IR', 'سلام (دنیا:', 'pipeline_bracket'),
                          ('fa_Latn', 'salam (donya)', 'pipeline_bracket'),
                          ('fa_Latn', 'salam (donya),', 'pipeline_bracket'),
                          ('fa_IR', 'احیا مهربد معد معقر', False),
                          ('fa_IR', 'سلام (hello) دنیا', False),
                          ('fa_IR', 'NVDA-خوب', False),
                          ('fa_IR', 'سلام-NVDA-خوب', False),
                          ('fa_Latn', 'salam-phone-khub', False),
                          ('fa_IR', 'NVDA-خوب', 'pipeline_dash'),
                          ('fa_IR', 'سلام-NVDA-خوب', 'pipeline_dash'),
                          ('fa_Latn', 'salam-phone-khub', 'pipeline_dash'),
                          ('fa_IR', 'سلام–NVDA—خوب', 'pipeline_dash'),
                          ('fa_Latn', 'salam–phone–khub', 'pipeline_dash'),
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
                          ('fa_IR', 'بله این حرف شما - درسته حتما', 'issue49_hyphen'),
                          ('fa_IR', 'ما عالی هستیم - خخخخ', 'issue49_hyphen'),
                          ('fa_IR', 'درسته', 'issue49_unknown'),
                          ('fa_Latn', 'salam(donya)salam', 'pipeline_symbols'),
                          ('fa_IR', 'سلام NVDA[]سلام', 'pipeline_mixed_symbols'),
                          ('fa_Latn', 'w', True),
                          ('fa_Latn', 'NVDA', True),
                          ('fa_IR', 'ق', True),
                          ('fa_IR', 'ا', True),
                          ('fa_IR', '؟', True),
                          *(( 'fa_IR', mark, True) for mark in 'ًٌٍَُِّْٰ'),
                          *((locale, mark, True) for locale in ('fa_IR', 'fa_Latn')
                            for mark in 'ٔأˈˌə̇αé⚽'),
                          ('fa_IR', 'ٔ', 'issue49_hamza_line'),
                          ('fa_IR', 'ٔ', 'issue49_hamza_line_none'),
                          ('fa_IR', 'ٔ', 'issue49_hamza_spelling'),
                          *(( 'fa_IR', mark, mode) for mark in 'ًٌٍَُِّْٰ'
                            for mode in ('issue49_original_line', 'issue49_original_spelling')),
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
            # Check contextual function words through NVDA's actual pipeline,
            # including an English host and a word read in isolation.
            self.cases.extend((('fa_IR', 'کتاب و خانه', 'issue49_weak_and'),
                               ('fa_IR', 'کتاب را', 'issue49_weak_object'),
                               ('fa_IR', 'NVDA را', 'issue49_weak_english_host'),
                               ('fa_IR', 'را', 'issue49_weak_isolated'),
                               ('fa_IR', 'من کتاب', 'issue49_weak_pronoun'),
                               ('fa_IR', 'اسلش', 'issue49_weak_eslash'),
                               ('fa_Latn', 'eslash', 'issue49_weak_eslash'),
                               ('fa_IR', 'جز از ایران', 'issue49_weak_except'),
                               ('fa_Latn', 'joz iran', 'issue49_weak_except'),
                               ('fa_Latn', 'ketab ra', 'issue49_weak_object'),
                               ('fa_IR', 'سلام دنیا', 'issue49_weak_us_stale'),
                               ('fa_Latn', 'salam donya', 'issue49_weak_us_stale'),
                               ('fa_IR', 'یونانی', 'issue49_weak_greek_adjective'),
                               ('fa_IR', 'ژاپنی', 'issue49_weak_japanese_adjective')))
            # Capture Finnish-contour samples through NVDA's preprocessing,
            # speech manager, driver and packaged Windows DLL.
            passage = ('سلام دوستان. امروز می‌خواهم درباره یک کتاب صحبت کنم. '
                       'این کتاب درباره دو دوست است. آن‌ها هر روز با هم به مدرسه می‌روند. '
                       'در راه به مردم سلام می‌کنند و درباره زندگی صحبت می‌کنند. '
                       'وقتی به خانه برمی‌گردند، کتاب می‌خوانند و به موسیقی گوش می‌دهند. '
                       'شما هم کتاب خواندن را دوست دارید؟')
            samples = (('word', 'سلام'), ('passage', passage),
                       ('switch', 'سلام NVDA دنیا. Hello. سلام دنیا. این یک آزمایش است.'))
            self.cases.extend(('fa_IR', text, 'comparison_finnish_' + name) for name, text in samples)
            self.cases.append(('fa_IR', passage, 'comparison_finnish_dense'))
            self.cases.extend((('fa_IR', 'سلام دنیا: این یک آزمایش است؟', 'comparison_colon'),
                               ('fa_IR', 'سلام دنیا. این یک آزمایش است؟', 'comparison_period')))
            self.cases.extend(('fa_IR', 'سلام دنیا خوب.', 'comparison_' + kind + '_pitch')
                              for kind in ('finnish', 'plain'))
            self.next()
        except BaseException:
            self.fail()

    def next(self):
        try:
            if self.index == len(self.cases):
                if self.results[0]['pcm_sha256'] != self.results[1]['pcm_sha256']:
                    raise AssertionError('confirmed script and Pinglish words differ')
                for mark in 'ًٌٍَُِّْٰ':
                    line = next(row for row in self.results if row['text'] == mark and
                                row['spelling'] == 'issue49_original_line')
                    spelled = next(row for row in self.results if row['text'] == mark and
                                   row['spelling'] == 'issue49_original_spelling')
                    if line['pcm_sha256'] != spelled['pcm_sha256']:
                        raise AssertionError('NVDA line and character navigation differ: ' + ascii(mark))
                line = next(row for row in self.results if row['spelling'] == 'issue49_hamza_line')
                spelled = next(row for row in self.results if row['spelling'] == 'issue49_hamza_spelling')
                if line['pcm_sha256'] != spelled['pcm_sha256']:
                    raise AssertionError('NVDA line and spelling name differ for combining hamza')
                for locale in ('fa_IR', 'fa_Latn'):
                    slow = next(row for row in self.results if row['locale'] == locale and row['spelling'] == 'tags_slow')
                    fast = next(row for row in self.results if row['locale'] == locale and row['spelling'] == 'tags_fast')
                    if slow['pcm_bytes'] <= fast['pcm_bytes'] * 1.5:
                        raise AssertionError('NVDA did not apply Persian ECI speed tags: ' + locale)
                quoted = next(row for row in self.results if row['text'] == 'سلام «دنیا» خوب')
                bracketed = next(row for row in self.results if row['text'] == 'سلام (دنیا) خوب' and
                                 row['spelling'] == 'issue49_quotes')
                if quoted['pcm_sha256'] != bracketed['pcm_sha256']:
                    raise AssertionError('angle quotes were spoken at the SOME punctuation level')
                fa_number = next(row for row in self.results if row['locale'] == 'fa_IR' and
                                 row['spelling'] == 'issue49_numeric')
                en_number = next(row for row in self.results if row['locale'] == 'en_US' and
                                 row['spelling'] == 'issue49_numeric')
                if fa_number['pcm_sha256'] != en_number['pcm_sha256']:
                    raise AssertionError('English number phrase was split between languages')
                for text in ('Google Chrome, version 157.0.8103.79 (AMD64)',
                             'BlindRadio, version 0.2.100', 'v1.5.0', 'NVDA1.5.0'):
                    fa_version = next(row for row in self.results if row['locale'] == 'fa_IR' and
                                      row['text'] == text and row['spelling'] == 'issue49_version_some')
                    en_version = next(row for row in self.results if row['locale'] == 'en_US' and
                                      row['text'] == text and row['spelling'] == 'issue49_version_some')
                    split_version = next(row for row in self.results if row['locale'] == 'fa_IR' and
                                         row['text'] == text and row['spelling'] == 'issue49_version_split')
                    if fa_version['pcm_sha256'] != en_version['pcm_sha256']:
                        raise AssertionError('English version number changed language: ' + text)
                    if fa_version['pcm_sha256'] != split_version['pcm_sha256']:
                        raise AssertionError('NVDA indexes changed the English version: ' + text)
                    if text in ('v1.5.0', 'NVDA1.5.0'):
                        for mode in ('issue49_version_some', 'issue49_version_split'):
                            pinglish = next(row for row in self.results if row['locale'] == 'fa_Latn' and
                                            row['text'] == text and row['spelling'] == mode)
                            if pinglish['pcm_sha256'] != en_version['pcm_sha256']:
                                raise AssertionError('Pinglish changed the English version: ' + mode)
                plain = next(row for row in self.results if row['spelling'] == 'tags_plain')
                pitch = next(row for row in self.results if row['spelling'] == 'tags_pitch')
                if plain['pcm_sha256'] == pitch['pcm_sha256']:
                    raise AssertionError('NVDA did not apply the Persian ECI pitch tag')
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
                if not {'fa_IR', 'fa_Latn'}.issubset(
                        {item['locale'] for item in self.symbol_preparations}):
                    raise AssertionError('Persian symbol data was not prepared on voice selection')
                for locale, brackets, commas in (
                        ('fa_IR', 'سلام (دنیا) خوب', 'سلام، دنیا، خوب'),
                        ('fa_Latn', 'salam (donya) khub', 'salam, donya, khub')):
                    first = next(row for row in self.results if row['locale'] == locale and row['text'] == brackets)
                    second = next(row for row in self.results if row['locale'] == locale and row['text'] == commas)
                    if first['pcm_sha256'] != second['pcm_sha256']:
                        raise AssertionError('native bracket melody differs for ' + locale)
                for locale, brackets, comma in (
                        ('fa_IR', 'سلام (دنیا)', 'سلام (دنیا),'),
                        ('fa_IR', 'سلام (NVDA): خوب', 'سلام (NVDA: خوب'),
                        ('fa_IR', 'سلام (دنیا):', 'سلام (دنیا:'),
                        ('fa_Latn', 'salam (donya)', 'salam (donya),')):
                    first = next(row for row in self.results if row['locale'] == locale and
                                 row['text'] == brackets and row['spelling'] == 'pipeline_bracket')
                    second = next(row for row in self.results if row['locale'] == locale and
                                  row['text'] == comma and row['spelling'] == 'pipeline_bracket')
                    if first['pcm_sha256'] != second['pcm_sha256']:
                        raise AssertionError('NVDA lost the continuing bracket tone for ' + locale)
                fa_colon = next(row for row in self.results if row['text'] == 'سلام (دنیا):' and
                                row['spelling'] == 'pipeline_bracket')
                fa_comma = next(row for row in self.results if row['text'] == 'سلام (دنیا),' and
                                row['spelling'] == 'pipeline_bracket')
                if fa_colon['pcm_sha256'] == fa_comma['pcm_sha256']:
                    raise AssertionError('NVDA replaced the colon contour with a comma contour')
                pitch = next(row for row in self.results if row['spelling'] == 'comparison_finnish_pitch')
                plain_pitch = next(row for row in self.results if row['spelling'] == 'comparison_plain_pitch')
                if pitch['pcm_sha256'] == plain_pitch['pcm_sha256']:
                    raise AssertionError('NVDA pitch command did not change the Finnish melody')
                plain = next(row for row in self.results if row['spelling'] == 'comparison_finnish_passage')
                dense = next(row for row in self.results if row['spelling'] == 'comparison_finnish_dense')
                if plain['pcm_sha256'] != dense['pcm_sha256']:
                    raise AssertionError('NVDA word indexes changed the Finnish sentence melody')
                colon = next(row for row in self.results if row['spelling'] == 'comparison_colon')
                period = next(row for row in self.results if row['spelling'] == 'comparison_period')
                if colon['pcm_sha256'] != period['pcm_sha256']:
                    raise AssertionError('NVDA colon melody differs from the full-stop melody')
                self.frame_capture_class.__init__ = self.original_capture_init
                self.done_signal.unregister(self.speech_done)
                RESULT.write_text(json.dumps({'ok': True, 'cases': self.results,
                    'first_symbol_preparations': self.symbol_preparations[:4],
                    'first_native_preparations': self.native_preparations[:4]}, ensure_ascii=True), encoding='utf8')
                return
            locale, text, spelling = self.cases[self.index]
            self.speech.cancelSpeech()
            self.synth.language = locale
            self.nav_stale_case = spelling == 'issue49_weak_us_stale'
            self.nav_stale_injected = False
            self.before_us_fits = len(self.us_fits)
            self.sample_case = isinstance(spelling, str) and spelling.startswith('comparison_')
            self.sample_finnish = spelling == 'comparison_finnish_pitch'
            self.reference_calibration = spelling == 'comparison_old_calibration'
            if self.sample_case:
                self.synth.sampleRate = '3'  # 16 kHz, like the first listening experiment
                self.synth.rateBoost = False
                self.synth.rate = 23  # the driver's unboosted ECI speed 65
                self.synth.pitch = 68 if self.reference_calibration else 65
                self.synth.inflection = 37 if self.reference_calibration else 30
            self.synth.voiceTags = isinstance(spelling, str) and spelling.startswith('tags_')
            if self.synth.language != locale:
                raise AssertionError('language selection did not stick: ' + locale)
            self.before = {id(player): len(player.chunks) for player in self.players}
            self.before_projected = len(self.projected)
            self.before_projected_sources = len(self.projected_sources)
            self.before_user_controls = len(self.user_controls)
            if spelling == 'mixed':
                sequence = [self.language_command('en_US'), 'NVDA ',
                            self.language_command('fa_IR'), 'خوب']
            elif spelling == 'english':
                sequence = [self.language_command('en_US'), text]
            elif spelling in ('persian', 'driver_user_latency'):
                sequence = [text]
            elif spelling == 'mixedspelling':
                sequence = ['سلام ', self.character(True), '؟', self.character(False), ' دنیا']
            elif spelling in ('issue49_hamza_spelling', 'issue49_original_spelling'):
                sequence = list(self.speech.getSpellingSpeech(text, locale=locale, endsUtterance=False))
            elif spelling == 'issue49_original_line':
                sequence = [text]
            elif spelling == 'issue49_korean_pitch':
                sequence = [self.pitch_command(multiplier=1.2), text, self.pitch_command()]
            elif spelling == 'issue49_korean_mid_pitch':
                sequence = ['سلام ', self.pitch_command(multiplier=1.2), 'دنیا', self.pitch_command()]
            elif spelling == 'comparison_finnish_pitch':
                sequence = ['سلام ', self.pitch_command(multiplier=1.2), 'دنیا ', self.pitch_command(), 'خوب.']
            elif spelling == 'comparison_finnish_dense':
                sequence = [part for i, word in enumerate(text.split(), 60000)
                            for part in (word + ' ', self.index_command(i))]
            elif spelling == 'issue49_korean_spelling':
                sequence = [self.character(True), text, self.character(False)]
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
            elif spelling == 'issue49_version_split':
                number = re.search(r'[0-9]+(?:\.[0-9]+)+', text)
                if number is None:
                    raise AssertionError('missing issue 49 version number')
                value = number.group()
                sequence = [text[:number.start()] + value[:1], self.index_command(7000 + self.index),
                            value[1:3], self.index_command(8000 + self.index), value[3:] + text[number.end():]]
            elif spelling == 'issue49_emoji_split':
                sequence = [text[0], self.index_command(9000 + self.index), text[1:]]
            elif spelling == 'issue49_heading_split':
                number = re.search('[0-9]', text)
                if number is None:
                    raise AssertionError('missing heading level')
                at = number.end()
                sequence = [text[:at], self.index_command(10000 + self.index), text[at:]]
            elif spelling in ('checkbox_unchecked', 'checkbox_checked', 'checkbox_checked_most'):
                status = 'بدونِ علامت' if spelling == 'checkbox_unchecked' else 'علامت‌دار'
                # NVDA speaks an object's name, role and state as separate
                # sequence items; its real preprocessing adds the separators.
                sequence = [text, 'کادرِ علامت‌زدنی', status]
            elif isinstance(spelling, str) and spelling.startswith('tags_'):
                sequence = [text]
            elif self.sample_case or (isinstance(spelling, str) and spelling.startswith('issue49_weak_')) or spelling in ('pipeline', 'pipeline_most', 'pipeline_stop', 'pipeline_log', 'pipeline_symbols', 'pipeline_mixed_symbols', 'pipeline_user_latency', 'pipeline_mixed_latency', 'pipeline_bracket', 'pipeline_dash', 'issue49_hyphen', 'issue49_unknown', 'issue49_quotes', 'issue49_numeric', 'issue49_version_some', 'issue49_emoji_some', 'issue49_heading', 'issue49_korean', 'issue49_korean_word', 'issue49_persian_unicode', 'issue49_persian_unicode_none', 'issue49_persian_unicode_all', 'issue49_korean_english', 'issue49_hamza_line', 'issue49_hamza_line_none'):
                sequence = [text]
            else:
                sequence = ([self.character(True), text, self.character(False)] if spelling else [text])
            self.profile_case = spelling in ('pipeline_user_latency', 'pipeline_mixed_latency', 'driver_user_latency')
            self.stage = {}
            self.started = time.perf_counter()
            self.pipeline = spelling in ('pipeline', 'pipeline_most', 'pipeline_stop', 'pipeline_log', 'pipeline_symbols', 'pipeline_mixed_symbols', 'pipeline_user_latency', 'pipeline_mixed_latency', 'pipeline_bracket', 'pipeline_dash', 'issue49_hyphen', 'issue49_unknown', 'checkbox_unchecked', 'checkbox_checked', 'checkbox_checked_most', 'decimal_pipeline', 'decimal_indexed_pipeline', 'decimal_grouped_pipeline', 'decimal_signed_pipeline')
            self.pipeline = self.pipeline or self.sample_case or (isinstance(spelling, str) and spelling.startswith('tags_'))
            self.pipeline = self.pipeline or (isinstance(spelling, str) and spelling.startswith('issue49_weak_'))
            self.pipeline = self.pipeline or spelling in ('issue49_quotes', 'issue49_numeric')
            self.pipeline = self.pipeline or spelling in ('issue49_version_some', 'issue49_version_split',
                                                           'issue49_emoji_some', 'issue49_emoji_split',
                                                           'issue49_heading', 'issue49_heading_split',
                                                           'issue49_korean', 'issue49_korean_word',
                                                           'issue49_korean_pitch', 'issue49_korean_mid_pitch', 'issue49_korean_spelling',
                                                           'issue49_persian_unicode', 'issue49_persian_unicode_none',
                                                           'issue49_persian_unicode_all', 'issue49_korean_english',
                                                           'issue49_hamza_line', 'issue49_hamza_line_none',
                                                           'issue49_hamza_spelling', 'issue49_original_line',
                                                           'issue49_original_spelling')
            self.emoji_case = spelling in ('issue49_emoji_some', 'issue49_emoji_split')
            self.version_case = spelling in ('issue49_version_some', 'issue49_version_split')
            self.before_version_debug = len(self.version_debug)
            self.before_emoji = len(self.emoji_submissions)
            self.before_korean = len(self.korean_calls)
            self.before_continuous = len(self.continuous)
            if self.pipeline:
                self.speech.speak([self.index_command(self.index + 1), *sequence],
                                  symbolLevel=(self.symbols.SymbolLevel.SOME if self.sample_case or spelling in (
                                               'issue49_quotes', 'issue49_version_some', 'issue49_version_split',
                                               'issue49_emoji_some', 'issue49_emoji_split',
                                               'issue49_heading', 'issue49_heading_split',
                                               'issue49_korean', 'issue49_korean_word',
                                               'issue49_korean_pitch', 'issue49_korean_mid_pitch', 'issue49_korean_spelling',
                                               'issue49_persian_unicode', 'issue49_korean_english', 'issue49_hamza_line',
                                               'issue49_hamza_spelling', 'issue49_original_line',
                                               'issue49_original_spelling') else
                                               self.symbols.SymbolLevel.ALL if spelling == 'issue49_persian_unicode_all' else
                                               self.symbols.SymbolLevel.MOST if spelling in ('pipeline_most', 'checkbox_checked_most')
                                               else self.symbols.SymbolLevel.NONE))
            else:
                self.synth.speak([self.index_command(self.index + 1), *sequence])
            # Prefer the real completion signal, retaining the original
            # 1.6-second bound if an NVDA version does not expose it.
            wx.CallLater(35, self.wait_for_completion, self.index,
                         time.monotonic() + (12 if self.sample_case else 1.6))
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
            completed = self.done_for_case == index and idle
            if self.sample_case and not completed and time.monotonic() >= deadline:
                raise TimeoutError('NVDA comparison did not finish: ' + self.cases[index][2])
            if completed or time.monotonic() >= deadline:
                self.capture()
            else:
                wx.CallLater(35, self.wait_for_completion, index, deadline)
        except BaseException:
            self.fail()

    def capture(self):
        try:
            locale, text, spelling = self.cases[self.index]
            if self.emoji_case:
                submitted = self.emoji_submissions[self.before_emoji:]
                if not submitted or any('\ufe0f' in part for sequence in submitted for part in sequence):
                    raise AssertionError('emoji presentation selector leaked to the synthesizer: ' + ascii(text))
            if spelling in ('issue49_korean', 'issue49_korean_word', 'issue49_korean_pitch', 'issue49_korean_mid_pitch',
                            'issue49_korean_spelling') and len(self.korean_calls) != self.before_korean:
                raise AssertionError('NVDA unexpectedly applied Korean-derived Persian pitch: ' + ascii(text))
            if spelling in ('issue49_korean_word', 'issue49_korean_pitch', 'issue49_korean_spelling') or (
                    isinstance(spelling, str) and spelling.startswith('issue49_persian_unicode')):
                tags = ' '.join(self.projected[self.before_projected:])
                if not tags or '`[.0ju.1ni.0kod]' in tags:
                    raise AssertionError('ordinary Persian text produced a Unicode announcement: ' + ascii(tags))
            if spelling == 'issue49_korean_english' and len(self.korean_calls) != self.before_korean:
                raise AssertionError('Korean-derived pitch changed English-only speech')
            audio = b''.join(chunk for player in self.players
                             for chunk in player.chunks[self.before.get(id(player), 0):])
            if not audio:
                raise RuntimeError('no actual NVDA audio for ' + ascii(text))
            if self.sample_case:
                if len(self.korean_calls) != self.before_korean:
                    raise AssertionError('Finnish sample unexpectedly used a Korean donor')
                with wave.open(str(RESULT.parent / (spelling + '.wav')), 'wb') as output:
                    output.setnchannels(1)
                    output.setsampwidth(2)
                    output.setframerate(16000)
                    output.writeframes(audio)
            if isinstance(spelling, str) and spelling.startswith('issue49_weak_'):
                tags = ' '.join(self.projected[self.before_projected:])
                expected = {'issue49_weak_and': '`[.0vA]',
                            'issue49_weak_object': '`[.0ra]',
                            'issue49_weak_english_host': '`[.0ra]',
                            'issue49_weak_isolated': '`[.1ra]',
                            'issue49_weak_pronoun': '`[.1mAn]',
                            'issue49_weak_eslash': '`[.0es.1lAS]',
                            'issue49_weak_except': '`00 `[.0Joz]',
                            'issue49_weak_us_stale': '`[.0sA.1lam]',
                            'issue49_weak_greek_adjective': '`[.0ju.0na.1ni]',
                            'issue49_weak_japanese_adjective': '`[.0Za.0po.1ni]'}[spelling]
                if expected not in tags:
                    raise AssertionError('NVDA lost contextual word stress: ' + ascii((text, tags)))
                if spelling == 'issue49_weak_us_stale' and (
                        not self.nav_stale_injected or not any(self.us_fits[self.before_us_fits:])):
                    raise AssertionError('NVDA lost US pitch after cancelling the previous English job')
            if spelling in ('tags_slow', 'tags_fast', 'tags_pitch', 'tags_phonemes'):
                expected = text.split(' ', 1)[0]
                if expected not in self.user_controls[self.before_user_controls:]:
                    raise AssertionError('NVDA lost the ECI tag: ' + expected)
            times = [value for player in self.players
                     for value in player.times[self.before.get(id(player), 0):]]
            if not times:
                raise RuntimeError('missing NVDA audio timing for ' + ascii(text))
            if spelling == 'pipeline_dash' and len(self.continuous) != self.before_continuous:
                raise AssertionError('NVDA lost the separated language boundary around a hyphen: ' + ascii(text))
            if spelling in ('issue49_hyphen', 'issue49_unknown'):
                tags = ' '.join(self.projected[self.before_projected:])
                expected = ("`[.0do.0ros.1t'e:']" if 'درسته' in text else '`[.0a.1li]')
                if expected not in tags:
                    details = self.projected_sources[self.before_projected_sources:]
                    raise AssertionError('NVDA spelled a Persian word: ' + ascii(details))
            if spelling in ('issue49_heading', 'issue49_heading_split'):
                tags = ' '.join(self.projected[self.before_projected:])
                if not tags or ']`[' in tags:
                    raise AssertionError('NVDA joined Persian number and vowel tags: ' + ascii(tags))
            if spelling is True and text in 'ًٌٍَُِّْٰ':
                expected = {'ً':'`[.1An]', 'ٌ':'`[.1on]', 'ٍ':'`[.1en]',
                            'َ':"`[.1'A:']", 'ُ':"`[.1'o:']", 'ِ':"`[.1'e:']",
                            'ّ':'`[.0tAS.1did]', 'ْ':'`[.0sa.1ken]',
                            'ٰ':"`[.0A.1lef] `[.0mAR.0s'u:'.1r'e:']"}[text]
                tags = ' '.join(self.projected[self.before_projected:])
                if expected not in tags:
                    raise AssertionError('NVDA lost the Persian character name: ' + ascii(tags))
            if ((spelling is True and text in 'ٔأˈˌə̇αé⚽/\\') or
                    spelling in ('issue49_hamza_line', 'issue49_hamza_line_none')):
                expected = {'ٔ': "`[.0hAm.1z'e:'.0j'e:'] `[.0ba.1la]",
                            'أ': "`[.0A.1lef] `[.0hAm.1z'e:']"}.get(text, self.symbol_tags.get(text))
                tags = ' '.join(self.projected[self.before_projected:])
                if expected not in tags:
                    raise AssertionError('NVDA lost the reported character name: ' + ascii((text, tags)))
            if text in ('به علاوه', 'بعلاوه', '+'):
                tags = ' '.join(self.projected[self.before_projected:])
                expected = ("`[.0be.0A.0la.1v'e:']" if text == 'بعلاوه' else
                            "`[.0b'e:'] `[.0A.0la.1v'e:']")
                if expected not in tags:
                    raise AssertionError('NVDA lost the corrected alāve reading: ' + ascii(tags))
            if spelling in ('pipeline', 'pipeline_most', 'pipeline_stop', 'pipeline_log', 'pipeline_symbols', 'pipeline_mixed_symbols', 'pipeline_user_latency', 'checkbox_unchecked', 'checkbox_checked', 'checkbox_checked_most'):
                tags = ' '.join(self.projected[self.before_projected:])
                if spelling in ('pipeline', 'pipeline_most') and locale == 'fa_IR' and ('`[.0hen.1ga.0me]' not in tags or
                        "`[.1'e:'G.0tja.2ra.0te]" not in tags):
                    raise AssertionError('NVDA preprocessing lost the vocalized Persian words: ' + ascii(tags))
                if spelling == 'pipeline_stop' and '`[.0mo.0te.0vAR.1Ref]' not in tags:
                    raise AssertionError('NVDA spelled the Persian word for stopped: ' + ascii(tags))
                if spelling == 'pipeline_log' and ("`[.1'e:'t.0te.1fa.0Ra.2te]" not in tags or
                        "`[.1sAbt.1So.0d'e:']" not in tags):
                    raise AssertionError('NVDA spelled part of the normalized log label: ' + ascii(tags))
                if spelling == 'pipeline_user_latency' and any(tag not in tags for tag in (
                        '`[.0ne.1San]', '`[.1da.0dA.2ne]', '`[.0hen.1ga.0me]', '`[.0Ga.1reJ]')):
                    raise AssertionError('NVDA spelled part of the reported latency example: ' + ascii(tags))
                if spelling == 'pipeline_symbols' and ('`[.0sA.1lam]' not in tags or
                        '`[.0do.1nja]' not in tags or '`[.1ta.0vu.2merk.0ki]' in tags):
                    raise AssertionError('NVDA punctuation damaged a Persian word: ' + ascii(tags))
                if spelling == 'pipeline_mixed_symbols' and ('`[.0sA.1lam]' not in tags or
                        '`[.1ta.0vu.2merk.0ki]' in tags):
                    raise AssertionError('NVDA mixed punctuation damaged Persian speech: ' + ascii(tags))
                if spelling in ('checkbox_unchecked', 'checkbox_checked', 'checkbox_checked_most'):
                    expected = ("`[.0A.0la.1mAt]" if spelling == 'checkbox_unchecked' else
                                "`[.0A.0la.0mAt] `[.1dar]")
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
            if spelling == 'pipeline_bracket':
                result['projection'] = ' '.join(self.projected[self.before_projected:])
            if self.version_case:
                result['version_debug'] = self.version_debug[self.before_version_debug:]
            self.results.append(result)
            self.index += 1
            self.next()
        except BaseException:
            self.fail()
