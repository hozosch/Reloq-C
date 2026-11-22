"""Check both Persian test profiles in a running portable NVDA instance."""
import hashlib
from bisect import bisect_left
import json
import os
from pathlib import Path
import re
import time
import traceback
from types import SimpleNamespace
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
            from synthDrivers.reloqC._persian import continuous_driver, continuous_plan, direct_fa_sequence, korean_contour, nvda_driver_probe
            from synthDrivers.reloqC._persian.persian_symbols import SYMBOL_TAGS
            from synthDrivers.reloqC._reloq_eci_transport import EciTextTransport
            from synthDrivers.reloqC._reloq_persian_frames import FrameCapture
            self.speech, self.character, self.index_command = speech, CharacterModeCommand, IndexCommand
            self.native_word = reloq.NativeDictionaryWord
            self.symbol_tags = {key: re.sub(r"'([Aaeiou]):'", r"\1", value)
                                for key, value in SYMBOL_TAGS.items()}
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
            self.syllable_fits = []
            original_capture_init = FrameCapture.__init__
            self.frame_capture_class, self.original_capture_init = FrameCapture, original_capture_init
            def comparison_capture_init(instance, dll):
                original_capture_init(instance, dll)
                if instance.fit_pitch_anchors is not None:
                    original = instance.fit_pitch_anchors
                    def observe_syllables(*args):
                        result = original(*args)
                        self.syllable_fits.append(result == 0)
                        return result
                    instance.fit_pitch_anchors = observe_syllables
                def note_native_syllables(result, target, nt, donor, nd, tb, db, words,
                                          ts, nts, ds, nds, counts):
                    observed = False
                    if result == 0 and counts is not None and ts is not None and ds is not None:
                        count = tuple(counts[i] for i in range(words))
                        if any(n > 1 for n in count):
                            target_item = SimpleNamespace(syllables=tuple((ts[i].from_, ts[i].to) for i in range(nts)))
                            donor_item = SimpleNamespace(syllables=tuple((ds[i].from_, ds[i].to) for i in range(nds)))
                            target_bounds = tuple(tb[i] for i in range(words + 1))
                            donor_bounds = tuple(db[i] for i in range(words + 1))
                            tg = continuous_plan._syllable_word_edges(target_item, target_bounds, count)
                            dg = continuous_plan._syllable_word_edges(donor_item, donor_bounds, count)
                            target_ticks = [target[i].tick for i in range(nt)]
                            donor_ticks = [donor[i].tick for i in range(nd)]
                            donor_pitch = [donor[i].controls[1] for i in range(nd)]
                            for i, n in enumerate(count):
                                if n < 2:
                                    continue
                                left = tg[i] if tg is not None else continuous_plan._syllable_edges(
                                    target_item, target_bounds[i], target_bounds[i+1], n)
                                right = dg[i] if dg is not None else continuous_plan._syllable_edges(
                                    donor_item, donor_bounds[i], donor_bounds[i+1], n)
                                for t, d in zip(left, right):
                                    at = bisect_left(target_ticks, t)
                                    if at < nt and target_ticks[at] == t and abs(
                                            target[at].controls[1] - continuous_plan._interp(d, donor_ticks, donor_pitch)) < .01:
                                        observed = True
                                        break
                                if observed:
                                    break
                    self.syllable_fits.append(observed)
                    return result
                if instance.transfer_us_words is not None:
                    original_transfer = instance.transfer_us_words
                    def observe_native_syllables(target, nt, donor, nd, tb, db, words,
                                                 ts, nts, ds, nds, counts):
                        result = original_transfer(target, nt, donor, nd, tb, db, words,
                                                   ts, nts, ds, nds, counts)
                        return note_native_syllables(result, target, nt, donor, nd, tb, db, words,
                                                     ts, nts, ds, nds, counts)
                    instance.transfer_us_words = observe_native_syllables
                if instance.transfer_us_samples is not None:
                    original_samples = instance.transfer_us_samples
                    def observe_native_samples(target, nt, donor, nd, tb, db, words, rate,
                                               ts, nts, ds, nds, counts):
                        result = original_samples(target, nt, donor, nd, tb, db, words, rate,
                                                  ts, nts, ds, nds, counts)
                        if result:
                            self.syllable_fits.append(False)
                            return result
                        target_bounds = [round(tb[i] * 1000 / rate) for i in range(words + 1)]
                        donor_bounds = [round(db[i] * 1000 / rate) for i in range(words + 1)]
                        target_bounds[-1] = min(target_bounds[-1], target[nt-1].tick + 1)
                        donor_bounds[-1] = min(donor_bounds[-1], donor[nd-1].tick + 1)
                        return note_native_syllables(result, target, nt, donor, nd, target_bounds,
                                                     donor_bounds, words, ts, nts, ds, nds, counts)
                    instance.transfer_us_samples = observe_native_samples
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
            self.compact_case = False
            self.version_debug = []
            self.emoji_case = False
            self.user_controls = []
            self.continuous = []
            self.us_fits = []
            self.hz_transitions = []
            self.grouped_case = False
            self.grouped_bounds = []
            self.nav_stale_case = False
            self.nav_stale_injected = False
            old_project_part = continuous_driver._project_part
            old_prepared = direct_fa_sequence.ScopedDirectPersianEngine._speak_prepared
            old_continuous = nvda_driver_probe.try_continuous
            old_glides = nvda_driver_probe.try_persian_glides
            old_us_fit = continuous_driver.fit_us_word_pitch
            old_context_plan = continuous_driver.context_plan
            old_hz_transition = continuous_plan.match_h_to_us_z
            old_runs = nvda_driver_probe._script_runs
            def observe_runs(text):
                result = old_runs(text)
                if self.version_case or self.compact_case:
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
            def observe_hz_transition(plan, parts, en, rate):
                result = old_hz_transition(plan, parts, en, rate)
                for edge in result:
                    self.hz_transitions.append((parts[edge-1].text, parts[edge].text))
                return result
            continuous_plan.match_h_to_us_z = observe_hz_transition
            def observe_grouped_plan(parts, *args, **kwargs):
                plan = old_context_plan(parts, *args, **kwargs)
                if (self.grouped_case and len(parts) >= 2 and parts[-1].language == 'fa' and
                        parts[-1].text.rstrip().endswith('`[.0ri.1al]')):
                    syllables = plan.contexts['fa'].syllables
                    if len(syllables) >= 2:
                        self.grouped_bounds.append((plan.edges['fa'][-2], plan.edges['fa'][-1],
                                                    syllables[-2][0], syllables[-1][1], args[2]))
                return plan
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
            continuous_driver.context_plan = observe_grouped_plan
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
                if self.version_case or self.compact_case:
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
            import config
            self.spelling_config = config.conf['speech'][self.synth.name]
            self.saved_spelling_functionality = self.spelling_config['useSpellingFunctionality']
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
            # Both synthetic profiles need their own persistent clock bit;
            # toggling one must not silently change the other or Finnish.
            saved_language, saved_mask = self.synth.language, self.synth._acousticClockMask
            try:
                self.synth.language = 'fa_IR'; self.synth.originalAcousticClock = True
                if not self.synth.originalAcousticClock: raise AssertionError('Persian clock checkbox did not enable')
                self.synth.language = 'fa_Latn'; self.synth.originalAcousticClock = False
                if self.synth.originalAcousticClock: raise AssertionError('Pinglish clock checkbox did not disable')
                self.synth.language = 'fa_IR'
                if not self.synth.originalAcousticClock: raise AssertionError('Persian clock was lost on switch')
                self.synth.language = 'fa_Latn'; self.synth.originalAcousticClock = True
                if not self.synth.originalAcousticClock: raise AssertionError('Pinglish clock checkbox did not enable')
                self.synth.language = 'fa_IR'; self.synth.originalAcousticClock = False
                self.synth.language = 'fa_Latn'
                if not self.synth.originalAcousticClock: raise AssertionError('Pinglish clock was lost on switch')
            finally:
                self.synth._acousticClockMask = saved_mask
                self.synth.language = saved_language
            self.cases = [('fa_IR', 'سلام دنیا؟', False),
                          ('fa_Latn', 'salam donya?', False),
                          ('fa_Latn', 'salaam donyaa?', False),
                          ('fa_IR', '`vs30 سلام دنیا', 'tags_slow'),
                          ('fa_IR', '`vs180 سلام دنیا', 'tags_fast'),
                          ('fa_IR', 'سلام دنیا', 'tags_plain'),
                          ('fa_IR', '`vbhz220 سلام دنیا', 'tags_pitch'),
                          ('fa_IR', '`[.1sAd] سلام', 'tags_phonemes'),
                          ('fa_Latn', '`vs30 salam donya', 'tags_slow'),
                          ('fa_Latn', '`vs180 salam donya', 'tags_fast'),
                          *((locale, tag, 'tags_phoneme_own')
                            for locale in ('fa_IR', 'fa_Latn')
                            for tag in ('`[.1qa]', '`[.1Ra]', '`[.1xa]', '`[.1Ga]',
                                        '`[.1?a]', '`[.1wa]', '`[.1h?a]', '`[.1hwa]',
                                        '`[.1solh]', '`[.1solhw]',
                                        "`[.1'A:'k.0si]", '`[.1Ak.0si]')),
                          ('fa_IR', 'او آمد.', False),
                          ('fa_Latn', 'u aamad.', False),
                          ('fa_IR', 'بدبخت', False),
                          ('fa_IR', 'خطِ تیره', False),
                          ('fa_IR', 'زیرخط', False),
                          ('fa_IR', '-', True),
                          ('fa_IR', '_', True),
                          *(( 'fa_IR', text, 'issue49_character_navigation') for text in ('-', '_', '/', '\\')),
                          ('fa_IR', '/', 'issue50_slash_some'),
                          ('fa_IR', '\\', 'issue50_slash_most'),
                          ('fa_IR', 'سلام / دنیا', 'issue50_slash_some'),
                          ('fa_IR', 'سلام \\ دنیا', 'issue50_slash_most'),
                          *(( locale, text, 'issue49_character_navigation') for text in ('a', 'z')
                            for locale in ('fa_IR', 'en_US')),
                          *((locale, text, True) for text in ('a', 'A', 'z', 'NVDA')
                            for locale in ('fa_IR', 'en_US')),
                          *(( 'fa_IR', 'به' + separator + word, 'issue49_preposition')
                            for word in ('طور', 'سوی', 'طرف', 'خاطر', 'گفته', 'دیدن', 'دلیل')
                            for separator in ('\u200c', ' ')),
                          *(( 'fa_IR', text, 'issue49_final_accent') for text in (
                              'حجازی', 'بردباری', 'سلامتی', 'کثیریان', 'عباسی',
                              'منچستریونایتد', 'سایبری', 'گزینه زمستانی ما او بود.')),
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
                          ('fa_IR', 'پخش کردنِ صدا هنگامِ آغازِ کار و خروج از NVDA', 'pipeline_user_latency'),
                          ('fa_IR', 'بررسیِ خودکارِ به‌روزرسانیها برای NVDA', 'pipeline_user_latency'),
                          ('fa_IR', 'پخش کردنِ صدا هنگامِ آغازِ کار و خروج از NVDAبررسیِ خودکارِ به‌روزرسانیها برای NVDA', 'pipeline_user_latency'),
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
                          *(( 'fa_IR', 'heading    level 3  ۱,۹۰۰,۰۰۰ ریال', mode)
                            for mode in ('issue49_grouped_driver', 'issue49_grouped_none',
                                         'issue49_grouped_some')),
                          ('fa_IR', 'heading level 3 1,900,000 ریال', 'issue49_grouped_driver'),
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
                          ('fa_Latn', 'salam Straße donya', False),
                          ('fa_Latn', 'salam Strasse donya', False),
                          ('fa_IR', 'Straße', False),
                          ('en_US', 'Straße', False),
                          ('fa_Latn', 'fa\u0304rsi', False),
                          ('fa_Latn', 'farsi', False),
                          ('fa_Latn', 'salam-e', False),
                          ('fa_Latn', 'salam-e', 'pipeline'),
                          ('fa_Latn', 'salam-e', 'pipeline_most'),
                          ('fa_IR', 'استفاده از NVDA هنگامِ ورود به ویندوز (به اِختیاراتِ مدیریتی نیاز دارد)', 'pipeline'),
                          ('fa_IR', 'استفاده از تنظیماتِ ذخیره‌شده‌ی فعلی هنگامِ ورود به ویندوز و دیگر صفحاتِ امن (به اِختیاراتِ مدیریتی نیاز دارد)', 'pipeline'),
                          ('fa_IR', 'استفاده از تنظیماتِ ذخیره‌شده‌ی فعلی هنگامِ ورود به ویندوز و دیگر صفحاتِ امن (به اِختیاراتِ مدیریتی نیاز دارد)', 'pipeline'),
                          ('fa_IR', 'استفاده از تنظیماتِ ذخیره‌شده‌ی فعلی هنگامِ ورود به ویندوز و دیگر صفحاتِ امن (به اِختیاراتِ مدیریتی نیاز دارد)', 'pipeline'),
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
                          *((locale, 'Ж', spelling) for locale in ('fa_IR', 'fa_Latn')
                            for spelling in (False, True)),
                          ('en_US', 'cyrillic capital letter zhe', False),
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
            # The record symbol directly adjacent to a Persian headline was
            # reported as making the whole line silent in Dev-26. Exercise
            # NVDA's actual speech pipeline with symbols set to NONE, also
            # when its text/index boundaries divide the emoji from the word.
            self.cases.extend((('fa_IR', '⏺پزشکیان: روحانیون به کمک دولت بیان', 'issue49_record_line'),
                               ('fa_IR', '⏺️پزشکیان: روحانیون به کمک دولت بیان', 'issue49_record_line'),
                               ('fa_IR', '⏺پزشکیان: روحانیون به کمک دولت بیان', 'issue49_record_split'),
                               ('fa_IR', 'پزشکیان: روحانیون به کمک دولت بیان', 'issue49_record_line')))
            self.cases.extend((('fa_IR', 'روح zero', 'issue49_h_z'),
                               ('fa_IR', 'روح zero', 'issue49_h_z_language'),
                               ('fa_IR', 'کتاب و خانه', 'issue49_weak_and'),
                               ('fa_IR', 'کتاب را', 'issue49_weak_object'),
                               ('fa_IR', 'NVDA را', 'issue49_weak_english_host'),
                               ('fa_IR', 'را', 'issue49_weak_isolated'),
                               ('fa_IR', 'خوب است', 'issue49_weak_copula'),
                               ('fa_IR', 'است', 'issue49_weak_copula_isolated'),
                               ('fa_IR', 'NVDA است', 'issue49_weak_copula_english_host'),
                               ('fa_Latn', 'khub ast', 'issue49_weak_copula'),
                               ('fa_Latn', 'ast', 'issue49_weak_copula_isolated'),
                               ('fa_Latn', 'NVDA ast', 'issue49_weak_copula_english_host'),
                               ('fa_Latn', 'zooon ast', 'issue49_weak_copula'),
                               ('fa_Latn', 'zooon raa', 'issue49_weak_object'),
                               ('fa_Latn', 'ketaab va zooon', 'issue49_weak_and'),
                               ('fa_IR', 'پَر', 'issue49_own_vowel'),
                               ('fa_IR', 'پِر', 'issue49_own_vowel'),
                               ('fa_IR', 'پُر', 'issue49_own_vowel'),
                               ('fa_IR', 'راه‌اندازی', 'issue49_nominal_compound'),
                               ('fa_IR', 'راه‌اندازیِ', 'issue49_nominal_compound'),
                               ('fa_IR', '96', 'issue50_number'),
                               ('fa_IR', '۹۶', 'issue50_number'),
                               ('fa_Latn', '96', 'issue50_number'),
                               ('fa_IR', '66', 'issue50_contour_66'),
                               ('fa_IR', '۶۶', 'issue50_contour_66'),
                               ('fa_Latn', '66', 'issue50_contour_66'),
                               ('fa_IR', 'پرانتز باز', 'issue50_contour_parantez'),
                               ('fa_Latn', 'parantez baaz', 'issue50_contour_parantez'),
                               ('fa_Latn', 'paraantez baaz', 'issue50_contour_parantez'),
                               ('fa_IR', '196', 'issue50_number_group'),
                               *(( 'fa_IR', value, 'issue50_hundred_short')
                                 for value in ('100', '101', '123', '167', '۱۹۹', '۱۲۰', '۱۷۱')),
                               ('fa_IR', '96: سلام', 'issue50_colon_number'),
                               ('fa_IR', 'سلام: 96', 'issue50_colon_number'),
                               ('fa_IR', 'NVDA: / دنیا', 'issue50_colon_mixed'),
                               ('fa_IR', 'NVDA. / دنیا', 'issue50_colon_mixed'),
                               ('fa_IR', 'NVDA: / دنیا', 'issue50_colon_mixed_none'),
                               ('fa_IR', 'NVDA. / دنیا', 'issue50_colon_mixed_none'),
                               ('fa_IR', 'افزونه‌های', 'issue50_plural'),
                               ('fa_IR', 'برنامه‌های', 'issue50_plural'),
                               ('fa_IR', 'کانال‌ها', 'issue50_l_h'),
                               ('fa_IR', 'پاتیل‌ها', 'issue50_l_h'),
                               ('fa_IR', 'تیره', 'issue50_r_after_i_vowel'),
                               ('fa_IR', 'ایران', 'issue50_r_after_i_vowel'),
                               ('fa_IR', 'سیری', 'issue50_r_after_i_vowel'),
                               ('fa_IR', 'زیرخط', 'issue50_r_after_i_x_speed38'),
                               ('fa_Latn', 'zirkhat', 'issue50_r_after_i_x_speed38'),
                               ('fa_IR', 'بختیاری‌زاده', 'issue49_native_new_compound'),
                               ('fa_IR', 'بختیارزاده', 'issue49_native_new_compound'),
                               ('fa_IR', 'فوتبال', 'issue49_native_new_compound'),
                               ('fa_IR', 'فوتبالیست', 'issue49_native_new_compound'),
                               ('fa_IR', 'پژوهشگران', 'issue49_native_new_compound'),
                               ('fa_IR', 'پژوهشگری', 'issue49_native_new_compound'),
                               ('fa_IR', 'پژوهشگرها', 'issue49_native_new_compound'),
                               ('fa_IR', 'پژوهشگر‌ها', 'issue49_native_new_compound'),
                               ('fa_IR', 'کارگرها', 'issue49_native_new_compound'),
                               ('fa_IR', 'عکس', 'issue49_native_new_compound'),
                               ('fa_IR', 'عکسی', 'issue49_native_new_compound'),
                               ('fa_Latn', 'aks', 'issue49_native_new_compound'),
                               ('fa_Latn', 'aksi', 'issue49_native_new_compound'),
                               ('fa_Latn', 'bar', 'issue49_native_new_compound'),
                               ('fa_Latn', 'baar', 'issue49_native_new_compound'),
                               ('fa_Latn', 'man', 'issue49_native_new_compound'),
                               ('fa_Latn', 'maan', 'issue49_native_new_compound'),
                               ('fa_Latn', 'salam', 'issue49_native_new_compound'),
                               ('fa_Latn', 'salaam', 'issue49_native_new_compound'),
                               ('fa_Latn', 'iran', 'issue49_native_new_compound'),
                               ('fa_Latn', 'iraan', 'issue49_native_new_compound'),
                               ('fa_IR', 'عمومی', 'issue49_native_new_compound'),
                               ('fa_Latn', 'omumi', 'issue49_native_new_compound'),
                               ('fa_IR', 'خصوصی', 'issue49_native_new_compound'),
                               ('fa_Latn', 'khosusi', 'issue49_native_new_compound'),
                               ('fa_IR', 'تنظیمات عمومی', 'issue49_native_new_compound'),
                               ('fa_Latn', 'tanzimaat omumi', 'issue49_native_new_compound'),
                               ('fa_IR', 'کوچک', 'issue49_native_new_compound'),
                               ('fa_IR', 'کوچکتر', 'issue49_native_new_compound'),
                               ('fa_IR', 'اصغر', 'issue49_native_new_compound'),
                               ('fa_IR', 'اکبر', 'issue49_native_new_compound'),
                               ('fa_IR', 'اخگر', 'issue49_native_new_compound'),
                               ('fa_IR', 'احمد', 'issue49_native_new_compound'),
                               ('fa_IR', 'کودک', 'issue49_native_new_compound'),
                               ('fa_IR', 'دو کودک', 'issue49_native_new_compound'),
                               ('fa_IR', 'مادر کودک', 'issue49_native_new_compound'),
                               ('fa_IR', 'معرفی', 'issue49_native_new_compound'),
                               ('fa_IR', 'خواجه‌ئیان', 'issue49_native_new_compound'),
                               ('fa_IR', 'سلام دنیا؟', 'issue50_question_some'),
                               ('fa_IR', 'سلام دنیا?', 'issue50_question_ascii_some'),
                               ('fa_IR', 'سلام دنیا؟', 'issue50_question_split'),
                               ('fa_IR', 'سلام دنیا؟', 'issue50_question_none'),
                               ('fa_IR', 'سلام دنیا?', 'issue50_question_ascii_none'),
                               ('fa_IR', 'سلام دنیا؟', 'issue50_question_most'),
                               ('fa_IR', 'سلام دنیا?', 'issue50_question_ascii_most'),
                               ('fa_IR', 'سلام:دنیا', 'issue50_compact_colon_most'),
                               ('fa_IR', 'سلام:دنیا', 'issue50_compact_colon_split_most'),
                               ('fa_IR', 'سلام دونقطه. دنیا', 'issue50_compact_colon_reference_none'),
                               ('fa_IR', 'من کتاب', 'issue49_weak_pronoun'),
                               ('fa_IR', 'اسلش', 'issue49_weak_eslash'),
                               ('fa_Latn', 'eslash', 'issue49_weak_eslash'),
                               ('fa_IR', 'جز از ایران', 'issue49_weak_except'),
                               ('fa_Latn', 'joz iraan', 'issue49_weak_except'),
                               ('fa_Latn', 'ketaab raa', 'issue49_weak_object'),
                               ('fa_IR', 'سلام دنیا', 'issue49_weak_us_stale'),
                               ('fa_Latn', 'salaam donyaa', 'issue49_weak_us_stale'),
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
            self.cases.extend((locale, char, mode) for locale in ('fa_IR', 'fa_Latn')
                              for char, mode in (('⚭', 'unicode_language_flow'),
                                                 (' ', 'unicode_language_flow_reference'),
                                                 ('⚭', 'unicode_language_spelled'),
                                                 ('🙂', 'unicode_language_emoji'),
                                                 ('⚭', 'unicode_language_single'),
                                                 ('⚭', 'unicode_language_single_reference')))
            self.cases.append(('fa_IR', 'راه‌اندازی. راه‌اندازی مجدد. سیستم راه‌اندازی شد.', 'comparison_rahandazi'))
            self.cases.append(('fa_IR', '۹۶. شماره ۹۶. ۹۶: سلام دنیا.', 'comparison_96'))
            self.cases.extend(('fa_IR', 'سلام دنیا خوب.', 'comparison_' + kind + '_pitch')
                              for kind in ('finnish', 'plain'))
            self.cases.extend((locale, char, mode)
                              for locale in ('de_DE', 'en_US', 'fa_IR', 'fa_Latn')
                              for char in ('⚭', '⊖', '≪', 'Ж', 'α', '⚽', '🙂', '\x7f', '\u00a0', '\u200b',
                                           '\u202e', '✓')
                              for mode in ('unicode_spelling_on', 'unicode_spelling_off'))
            self.cases.extend((locale, char, mode)
                              for locale in ('de_DE', 'en_US', 'fa_IR', 'fa_Latn')
                              for char in ('⚭', '≪', 'Ж', 'α', '🙂', '«', '»', '€', '×', '→', '\u00a0', '\u200b',
                                           '\u202e', '✓',
                                           '@', '+', '?', ':', 'a', '1', ' ', '\t', '\n', '\r', '\x7f')
                              for mode in ('unicode_single_on', 'unicode_single_off', 'unicode_single_direct',
                                           'unicode_single_reference'))
            self.cases.extend((locale, text, mode) for locale, original, reference in (
                ('fa_IR', 'سلام ⚭ دنیا', 'سلام   دنیا'),
                ('fa_Latn', 'salam ⚭ donya', 'salam   donya'))
                for text, mode in ((original, 'unicode_flow'), (reference, 'unicode_flow_reference')))
            self.cases.extend((locale, left + char + right, 'unicode_control_flow')
                              for locale, left, right in (('fa_IR', 'سلام ', ' دنیا'),
                                                          ('fa_Latn', 'salam ', ' donya'))
                              for char in ('\x7f', '\x01', '\x1c'))
            self.next()
        except BaseException:
            self.fail()

    def next(self):
        try:
            if self.index == len(self.cases):
                if self.results[0]['pcm_sha256'] == self.results[1]['pcm_sha256']:
                    raise AssertionError('single-a Pinglish lost its short vowel')
                matching = next(row for row in self.results if row['locale']=='fa_Latn'
                                and row['text']=='salaam donyaa?' and row['spelling'] is False)
                if self.results[0]['pcm_sha256'] != matching['pcm_sha256']:
                    raise AssertionError('explicit aa Pinglish differs from Persian script')
                for written, roman in (('عمومی', 'omumi'), ('خصوصی', 'khosusi'),
                                       ('تنظیمات عمومی', 'tanzimaat omumi')):
                    rows = [row for row in self.results if row['spelling']=='issue49_native_new_compound'
                            and row['text'] in (written,roman)]
                    if len(rows)!=2 or rows[0]['pcm_sha256']!=rows[1]['pcm_sha256']:
                        raise AssertionError('Persian and Pinglish timing differs: '+ascii((written,roman)))
                for locale in ('fa_IR', 'fa_Latn'):
                    for requested, prior in (('`[.1qa]', '`[.1Ra]'), ('`[.1xa]', '`[.1Ga]'),
                                             ('`[.1?a]', '`[.1wa]'), ('`[.1h?a]', '`[.1hwa]'),
                                             ('`[.1solh]', '`[.1solhw]')):
                        def item(text):
                            return next(row for row in self.results if row['locale']==locale and
                                        row['text']==text and row['spelling']=='tags_phoneme_own')
                        if item(requested)['pcm_sha256'] != item(prior)['pcm_sha256']:
                            raise AssertionError('Persian user phoneme changed the selected sound: '+locale+' '+requested)
                    if item("`[.1'A:'k.0si]")['pcm_sha256'] == item('`[.1Ak.0si]')['pcm_sha256']:
                        raise AssertionError('Explicit long and short vowel trials sounded identical: '+locale)
                unicode_reference = next(row for row in self.results
                                         if row['locale'] == 'en_US' and
                                         row['text'] == 'cyrillic capital letter zhe')
                for locale in ('fa_IR', 'fa_Latn'):
                    for spelling in (False, True):
                        named = next(row for row in self.results if row['locale'] == locale and
                                     row['text'] == 'Ж' and row['spelling'] is spelling)
                        if named['pcm_sha256'] != unicode_reference['pcm_sha256']:
                            raise AssertionError('Unicode name was not routed to English: ' + locale)
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
                        pinglish_results = []
                        for mode in ('issue49_version_some', 'issue49_version_split'):
                            pinglish = next(row for row in self.results if row['locale'] == 'fa_Latn' and
                                            row['text'] == text and row['spelling'] == mode)
                            if pinglish['pcm_sha256'] == en_version['pcm_sha256']:
                                raise AssertionError('Pinglish incorrectly used the English version reading: ' + mode)
                            pinglish_results.append(pinglish['pcm_sha256'])
                        if pinglish_results[0] != pinglish_results[1]:
                            raise AssertionError('NVDA indexes changed the Pinglish version: ' + text)
                plain = next(row for row in self.results if row['spelling'] == 'tags_plain')
                pitch = next(row for row in self.results if row['spelling'] == 'tags_pitch')
                if plain['pcm_sha256'] == pitch['pcm_sha256']:
                    raise AssertionError('NVDA did not apply the Persian ECI pitch tag')
                script_glide = next(row for row in self.results if row['text'] == 'او آمد.')
                roman_glide = next(row for row in self.results if row['text'] == 'u aamad.')
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
                    raise AssertionError('accented Pinglish differs from its Persian spelling')
                sharp = next(row for row in self.results
                             if row['text'] == 'salam Straße donya' and row['spelling'] is False)
                folded = next(row for row in self.results
                              if row['text'] == 'salam Strasse donya' and row['spelling'] is False)
                if sharp['pcm_sha256'] != folded['pcm_sha256']:
                    raise AssertionError('Pinglish sharp s was not analyzed as Persian ss')
                english_word = [row for row in self.results if row['text'] == 'Straße'
                                and row['spelling'] is False and row['locale'] in ('fa_IR', 'en_US')]
                if len(english_word) != 2 or english_word[0]['pcm_sha256'] != english_word[1]['pcm_sha256']:
                    raise AssertionError('English sharp s did not reach the native English analysis')
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
                colon_mixed = next(row for row in self.results if row['text'] == 'NVDA: / دنیا' and
                                   row['spelling'] == 'issue50_colon_mixed')
                period_mixed = next(row for row in self.results if row['text'] == 'NVDA. / دنیا' and
                                    row['spelling'] == 'issue50_colon_mixed')
                if colon_mixed['pcm_sha256'] != period_mixed['pcm_sha256']:
                    raise AssertionError('NVDA lost the sentence boundary before a spoken slash')
                colon_none = next(row for row in self.results if row['text'] == 'NVDA: / دنیا' and
                                  row['spelling'] == 'issue50_colon_mixed_none')
                period_none = next(row for row in self.results if row['text'] == 'NVDA. / دنیا' and
                                   row['spelling'] == 'issue50_colon_mixed_none')
                if colon_none['pcm_sha256'] != period_none['pcm_sha256']:
                    raise AssertionError('NVDA lost the sentence boundary at symbol level NONE')
                for level in ('some', 'none', 'most'):
                    arabic = next(row for row in self.results if row['spelling'] == 'issue50_question_' + level)
                    ascii_mark = next(row for row in self.results if row['spelling'] == 'issue50_question_ascii_' + level)
                    if arabic['pcm_sha256'] != ascii_mark['pcm_sha256']:
                        raise AssertionError('NVDA lost Persian question intonation at ' + level)
                split = next(row for row in self.results if row['spelling'] == 'issue50_question_split')
                ordinary = next(row for row in self.results if row['spelling'] == 'issue50_question_some')
                if split['pcm_sha256'] != ordinary['pcm_sha256']:
                    raise AssertionError('NVDA index changed the Persian question contour')
                compact = next(row for row in self.results if row['spelling'] == 'issue50_compact_colon_most')
                compact_split = next(row for row in self.results if row['spelling'] == 'issue50_compact_colon_split_most')
                reference = next(row for row in self.results if row['spelling'] == 'issue50_compact_colon_reference_none')
                if compact['pcm_sha256'] != reference['pcm_sha256']:
                    raise AssertionError('NVDA lost the sentence contour after an announced colon')
                if compact_split['pcm_sha256'] != compact['pcm_sha256']:
                    raise AssertionError('NVDA index changed the announced-colon contour')
                def sound(locale, text, mode):
                    return next(row['pcm_sha256'] for row in self.results
                                if row['locale'] == locale and row['text'] == text and row['spelling'] == mode)
                for locale in ('fa_IR', 'fa_Latn'):
                    reference = sound(locale, ' ', 'unicode_language_flow_reference')
                    if sound(locale, '⚭', 'unicode_language_flow') != reference:
                        raise AssertionError('language switch turned running text into a character name: ' + locale)
                    if sound(locale, '⚭', 'unicode_language_spelled') == reference:
                        raise AssertionError('language switch lost explicit character spelling: ' + locale)
                    if sound(locale, '🙂', 'unicode_language_emoji') == reference:
                        raise AssertionError('language switch lost an emoji: ' + locale)
                    if sound(locale, '⚭', 'unicode_language_single') != sound(locale, '⚭', 'unicode_language_single_reference'):
                        raise AssertionError('language switch lost the isolated character name: ' + locale)
                self.spelling_config['useSpellingFunctionality'] = self.saved_spelling_functionality
                for locale in ('de_DE', 'en_US', 'fa_IR', 'fa_Latn'):
                    for char in ('⚭', '⊖', '≪', 'Ж', 'α', '⚽', '🙂', '\x7f', '\u00a0', '\u200b',
                                 '\u202e', '✓'):
                        if sound(locale, char, 'unicode_spelling_on') != sound(locale, char, 'unicode_spelling_off'):
                            raise AssertionError('NVDA lost the Unicode name while spelling: ' + ascii((locale, char)))
                    for char in ('⚭', '≪', 'Ж', 'α', '🙂', '«', '»', '€', '×', '→', '\u00a0', '\u200b',
                                 '\u202e', '✓',
                                 '@', '+', '?', ':', 'a', '1', ' ', '\t', '\n', '\r', '\x7f'):
                        reference = sound(locale, char, 'unicode_single_reference')
                        for mode in ('unicode_single_on', 'unicode_single_off', 'unicode_single_direct'):
                            if sound(locale, char, mode) != reference:
                                raise AssertionError('NVDA lost an isolated Unicode name: ' + ascii((locale, char, mode)))
                import speechDictHandler
                for locale, original, reference in (('fa_IR', 'سلام ⚭ دنیا', 'سلام   دنیا'),
                                                    ('fa_Latn', 'salam ⚭ donya', 'salam   donya')):
                    if (self.symbols.processSpeechSymbols(locale, '⚭', self.symbols.SymbolLevel.NONE).strip() == '⚭'
                            and speechDictHandler.processText('⚭') == '⚭'
                            and sound(locale, original, 'unicode_flow') != sound(locale, reference, 'unicode_flow_reference')):
                        raise AssertionError('unmapped Unicode name was spoken in running text: ' + locale)
                    for char in ('\x7f', '\x01', '\x1c'):
                        if (self.symbols.processSpeechSymbols(locale, char, self.symbols.SymbolLevel.NONE) == char
                                and speechDictHandler.processText(char) == char
                                and sound(locale, original.replace('⚭', char), 'unicode_control_flow') !=
                                    sound(locale, reference, 'unicode_flow_reference')):
                            raise AssertionError('control name was spoken in running text: ' + ascii((locale, char)))
                for symbol, label in (('-', 'خطِ تیره'), ('_', 'زیرخط')):
                    if sound('fa_IR', symbol, True) != sound('fa_IR', label, False):
                        raise AssertionError('NVDA symbol differs from its written Persian name: ' + symbol)
                for text in ('a', 'A', 'z', 'NVDA'):
                    if sound('fa_IR', text, True) != sound('en_US', text, True):
                        raise AssertionError('NVDA English character does not use the US voice: ' + text)
                for text in ('a', 'z'):
                    if sound('fa_IR', text, 'issue49_character_navigation') != sound(
                            'en_US', text, 'issue49_character_navigation'):
                        raise AssertionError('NVDA navigation did not use the US letter voice: ' + text)
                for word in ('طور', 'سوی', 'طرف', 'خاطر', 'گفته', 'دیدن', 'دلیل'):
                    if sound('fa_IR', 'به\u200c' + word, 'issue49_preposition') != sound(
                            'fa_IR', 'به ' + word, 'issue49_preposition'):
                        raise AssertionError('NVDA pronounced a joined preposition differently: ' + word)
                self.frame_capture_class.__init__ = self.original_capture_init
                self.done_signal.unregister(self.speech_done)
                RESULT.write_text(json.dumps({'ok': True, 'cases': self.results,
                    'first_symbol_preparations': self.symbol_preparations[:4],
                    'first_native_preparations': self.native_preparations[:4]}, ensure_ascii=True), encoding='utf8')
                return
            locale, text, spelling = self.cases[self.index]
            self.speech.cancelSpeech()
            if getattr(self, 'saved_rate_for_r_test', None) is not None:
                old_rate, old_boost = self.saved_rate_for_r_test
                self.synth.rateBoost = old_boost
                self.synth.rate = old_rate
                self.saved_rate_for_r_test = None
            self.synth.language = locale
            if spelling in ('unicode_spelling_on', 'unicode_spelling_off'):
                self.spelling_config['useSpellingFunctionality'] = spelling == 'unicode_spelling_on'
            if spelling in ('unicode_single_on', 'unicode_single_off'):
                self.spelling_config['useSpellingFunctionality'] = spelling == 'unicode_single_on'
            self.nav_stale_case = spelling == 'issue49_weak_us_stale'
            self.nav_stale_injected = False
            self.before_us_fits = len(self.us_fits)
            self.before_syllable_fits = len(self.syllable_fits)
            self.sample_case = isinstance(spelling, str) and spelling.startswith('comparison_')
            self.sample_finnish = spelling == 'comparison_finnish_pitch'
            self.reference_calibration = spelling == 'comparison_old_calibration'
            if self.sample_case:
                self.synth.sampleRate = '3'  # 16 kHz, like the first listening experiment
                self.synth.rateBoost = False
                self.synth.rate = 23  # the driver's unboosted ECI speed 65
                self.synth.pitch = 68 if self.reference_calibration else 65
                self.synth.inflection = 37 if self.reference_calibration else 30
            if spelling == 'issue50_r_after_i_x_speed38':
                self.saved_rate_for_r_test = (self.synth.rate, self.synth.rateBoost)
                self.synth.rateBoost = False
                self.synth.rate = 38
            self.synth.voiceTags = isinstance(spelling, str) and spelling.startswith('tags_')
            if self.synth.language != locale:
                raise AssertionError('language selection did not stick: ' + locale)
            self.before = {id(player): len(player.chunks) for player in self.players}
            self.before_projected = len(self.projected)
            self.grouped_case = isinstance(spelling, str) and spelling.startswith('issue49_grouped_')
            self.before_grouped = len(self.grouped_bounds)
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
            elif spelling in ('issue49_hamza_spelling', 'issue49_original_spelling', 'issue49_character_navigation'):
                sequence = list(self.speech.getSpellingSpeech(text, locale=locale, endsUtterance=False))
            elif spelling in ('issue49_preposition', 'issue49_final_accent'):
                sequence = [text]
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
            elif isinstance(spelling, str) and spelling.startswith('unicode_language_'):
                marker = ([self.character(True), self.native_word(text), self.character(False)]
                          if spelling in ('unicode_language_spelled', 'unicode_language_single_reference') else [text])
                if spelling in ('unicode_language_single', 'unicode_language_single_reference'):
                    sequence = [self.language_command('en_US'), *marker]
                else:
                    before, after = ('سلام ', ' دنیا') if locale == 'fa_IR' else ('salam ', ' donya')
                    sequence = [before, self.index_command(19001), self.language_command('en_US'), *marker,
                                self.index_command(19002), self.language_command(locale), after]
            elif spelling in ('unicode_spelling_on', 'unicode_spelling_off'):
                sequence = list(self.speech.getSpellingSpeech(text, locale=locale, endsUtterance=False))
            elif spelling in ('unicode_single_on', 'unicode_single_off'):
                sequence = [text, self.index_command(19000 + self.index)]
            elif spelling == 'unicode_single_direct':
                sequence = [text, self.index_command(19000 + self.index)]
            elif spelling == 'unicode_single_reference':
                sequence = [self.character(True), self.native_word(text), self.character(False),
                            self.index_command(19000 + self.index)]
            elif spelling in ('unicode_flow', 'unicode_flow_reference', 'unicode_control_flow'):
                sequence = [text]
            elif spelling == 'issue49_korean_spelling':
                sequence = [self.character(True), text, self.character(False)]
            elif spelling in ('issue49_h_z', 'issue49_h_z_language'):
                sequence = ([text] if spelling == 'issue49_h_z' else
                            ['روح ', self.language_command('en_US'), 'zero',
                             self.language_command('fa_IR')])
            elif spelling == 'english_exception':
                sequence = ['salam ', self.language_command('en_US'), 'phone',
                            self.language_command('fa_Latn'), ' donya']
            elif spelling == 'accent_exception':
                sequence = ['salam salam donya']
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
            elif spelling == 'issue49_record_split':
                sequence = [text[:1], self.index_command(14000 + self.index), text[1:]]
            elif spelling in ('issue49_record_line',):
                sequence = [text]
            elif spelling == 'issue50_question_split':
                sequence = [text[:-1], self.index_command(12000 + self.index), text[-1:]]
            elif spelling == 'issue50_compact_colon_split_most':
                position = text.index(':')
                sequence = [text[:position], self.index_command(13000 + self.index), text[position:]]
            elif spelling in ('checkbox_unchecked', 'checkbox_checked', 'checkbox_checked_most'):
                status = 'بدونِ علامت' if spelling == 'checkbox_unchecked' else 'علامت‌دار'
                # NVDA speaks an object's name, role and state as separate
                # sequence items; its real preprocessing adds the separators.
                sequence = [text, 'کادرِ علامت‌زدنی', status]
            elif isinstance(spelling, str) and spelling.startswith('tags_'):
                sequence = [text]
            elif self.sample_case or (isinstance(spelling, str) and spelling.startswith(('issue49_weak_', 'issue49_grouped_', 'issue50_'))) or spelling in ('pipeline', 'pipeline_most', 'pipeline_stop', 'pipeline_log', 'pipeline_symbols', 'pipeline_mixed_symbols', 'pipeline_user_latency', 'pipeline_mixed_latency', 'pipeline_bracket', 'pipeline_dash', 'issue49_hyphen', 'issue49_unknown', 'issue49_quotes', 'issue49_numeric', 'issue49_version_some', 'issue49_emoji_some', 'issue49_heading', 'issue49_korean', 'issue49_korean_word', 'issue49_persian_unicode', 'issue49_persian_unicode_none', 'issue49_persian_unicode_all', 'issue49_korean_english', 'issue49_hamza_line', 'issue49_hamza_line_none', 'issue49_own_vowel', 'issue49_nominal_compound', 'issue49_native_new_compound'):
                sequence = [text]
            else:
                sequence = ([self.character(True), text, self.character(False)] if spelling else [text])
            self.profile_case = (spelling in ('pipeline_user_latency', 'pipeline_mixed_latency', 'driver_user_latency') or
                                 spelling == 'pipeline' and text.startswith('استفاده از تنظیماتِ ذخیره‌شده‌ی'))
            self.stage = {}
            self.started = time.perf_counter()
            self.pipeline = spelling in ('pipeline', 'pipeline_most', 'pipeline_stop', 'pipeline_log', 'pipeline_symbols', 'pipeline_mixed_symbols', 'pipeline_user_latency', 'pipeline_mixed_latency', 'pipeline_bracket', 'pipeline_dash', 'issue49_hyphen', 'issue49_unknown', 'checkbox_unchecked', 'checkbox_checked', 'checkbox_checked_most', 'decimal_pipeline', 'decimal_indexed_pipeline', 'decimal_grouped_pipeline', 'decimal_signed_pipeline')
            self.pipeline = self.pipeline or self.sample_case or (isinstance(spelling, str) and spelling.startswith('tags_'))
            self.pipeline = self.pipeline or (isinstance(spelling, str) and spelling.startswith('issue49_weak_'))
            self.pipeline = self.pipeline or spelling in ('issue49_grouped_none', 'issue49_grouped_some')
            self.pipeline = self.pipeline or (isinstance(spelling, str) and spelling.startswith('issue50_'))
            self.pipeline = self.pipeline or spelling in ('issue49_record_line', 'issue49_record_split')
            self.pipeline = self.pipeline or spelling in ('issue49_h_z', 'issue49_h_z_language')
            self.pipeline = self.pipeline or spelling in ('issue49_quotes', 'issue49_numeric', 'issue49_own_vowel', 'issue49_nominal_compound', 'issue49_native_new_compound')
            self.pipeline = self.pipeline or spelling in ('issue49_preposition', 'issue49_final_accent',
                                                          'issue49_character_navigation',
                                                          'unicode_spelling_on', 'unicode_spelling_off',
                                                          'unicode_single_on', 'unicode_single_off', 'unicode_single_reference',
                                                          'unicode_flow', 'unicode_flow_reference', 'unicode_control_flow')
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
            self.compact_case = isinstance(spelling, str) and spelling.startswith('issue50_compact_colon_')
            self.before_version_debug = len(self.version_debug)
            self.before_emoji = len(self.emoji_submissions)
            self.before_korean = len(self.korean_calls)
            self.before_continuous = len(self.continuous)
            self.before_hz = len(self.hz_transitions)
            if self.pipeline:
                self.speech.speak([self.index_command(self.index + 1), *sequence],
                                  symbolLevel=(self.symbols.SymbolLevel.MOST if isinstance(spelling, str) and spelling.startswith('issue50_') and spelling.endswith('_most') else
                                               self.symbols.SymbolLevel.SOME if self.sample_case or spelling == 'issue49_grouped_some' or (isinstance(spelling, str) and spelling.startswith('issue50_') and not spelling.endswith('_none')) or spelling in (
                                               'issue49_quotes', 'issue49_preposition', 'issue49_final_accent', 'issue49_character_navigation', 'issue49_version_some', 'issue49_version_split',
                                               'issue49_emoji_some', 'issue49_emoji_split',
                                               'issue49_heading', 'issue49_heading_split',
                                               'issue49_korean', 'issue49_korean_word',
                                               'issue49_korean_pitch', 'issue49_korean_mid_pitch', 'issue49_korean_spelling',
                                               'issue49_persian_unicode', 'issue49_korean_english', 'issue49_hamza_line', 'issue49_own_vowel', 'issue49_nominal_compound',
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
            if self.grouped_case:
                tags = ' '.join(self.projected[self.before_projected:])
                if '`[.0ri.1al]' not in tags:
                    raise AssertionError('NVDA lost the Persian currency word: ' + ascii(tags))
                boundaries = self.grouped_bounds[self.before_grouped:]
                if spelling == 'issue49_grouped_driver' and not boundaries:
                    raise AssertionError('NVDA did not exercise the mixed sentence plan: ' + ascii(text))
                for first, last, syllable_start, syllable_end, rate in boundaries:
                    if not first * 10000000 <= syllable_start * rate <= syllable_end * rate <= last * 10000000:
                        raise AssertionError('NVDA cut a Persian word after a grouped number: ' + ascii(boundaries))
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
            if spelling in ('issue49_h_z', 'issue49_h_z_language') and not any(
                    self.hz_transitions[self.before_hz:]):
                raise AssertionError('NVDA did not apply the H-to-Z noise transition')
            if spelling in ('issue49_record_line', 'issue49_record_split'):
                tags = ' '.join(self.projected[self.before_projected:])
                if '`[.0pAzS.0ki.1an]' not in tags or '`[.0bA.1jan]' not in tags:
                    raise AssertionError('NVDA lost part of the reported Persian line: ' + ascii(tags))
                if text.startswith('⏺') and '`[.0dok.1me.0je]' not in tags:
                    raise AssertionError('NVDA lost the CLDR record-button name: ' + ascii(tags))
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
                            'issue49_weak_copula': '`00 `[.0Ast]',
                            'issue49_weak_copula_english_host': '`00 `[.0Ast]',
                            'issue49_weak_copula_isolated': '`[.1Ast]',
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
            if spelling == 'issue49_own_vowel':
                expected = {'پَر': '`[.1pAr]', 'پِر': '`[.1per]', 'پُر': '`[.1por]'}[text]
                tags = ' '.join(self.projected[self.before_projected:])
                if expected not in tags:
                    raise AssertionError('NVDA lost an explicitly marked Persian vowel: ' + ascii((text, tags)))
            if spelling == 'issue49_character_navigation' and text in ('-', '_'):
                tags = ' '.join(self.projected[self.before_projected:])
                if self.symbol_tags[text] not in tags:
                    raise AssertionError('NVDA navigation lost the written symbol name: ' + ascii((text, tags)))
            if ((spelling == 'issue49_character_navigation' and text in ('/', '\\')) or
                    spelling in ('issue50_slash_some', 'issue50_slash_most')):
                tags = ' '.join(self.projected[self.before_projected:])
                if self.symbol_tags['/'] not in tags:
                    raise AssertionError('NVDA slash label lost its short e or final stress: ' + ascii((text, tags)))
                if '\\' in text and '`[.1bAk]' not in tags:
                    raise AssertionError('NVDA backslash label lost its first word: ' + ascii((text, tags)))
            if spelling == 'issue49_final_accent':
                expected = {'حجازی': '`[.0he.0Ja.1zi]', 'بردباری': '`[.0bord.0ba.1ri]',
                            'سلامتی': '`[.0sA.0la.0mA.1ti]', 'کثیریان': '`[.0kA.0si.0ri.1jan]',
                            'عباسی': '`[.0Ab.0ba.1si]',
                            'منچستریونایتد': '`[.0mAn.0Ces.0te.0rju.0naj.1ted]',
                            'سایبری': '`[.0saj.0be.1ri]',
                            'گزینه زمستانی ما او بود.': '`[.0ze.0mes.0ta.1ni]'}[text]
                tags = ' '.join(self.projected[self.before_projected:])
                if expected not in tags:
                    raise AssertionError('NVDA lost a reported final word accent: ' + ascii((text, tags)))
            if spelling == 'issue49_nominal_compound':
                expected = ('`[.0ra.0hAn.0da.1zi.0e]' if text.endswith('ِ') else
                            '`[.0ra.0hAn.0da.1zi]')
                tags = ' '.join(self.projected[self.before_projected:])
                if expected not in tags:
                    raise AssertionError('NVDA lost nominal compound accent: ' + ascii((text, tags)))
                if not any(self.syllable_fits[self.before_syllable_fits:]):
                    raise AssertionError('NVDA missed observed syllable timing for ' + ascii(text))
            if spelling in ('issue50_number', 'issue50_number_group'):
                tags = ' '.join(self.projected[self.before_projected:])
                if '`[.2nA.0vA.0do.1SeS]' not in tags:
                    raise AssertionError('NVDA lost Persian 96 pronunciation: ' + ascii((text, tags)))
                if not any(self.syllable_fits[self.before_syllable_fits:]):
                    raise AssertionError('NVDA missed the number syllable timing: ' + ascii(text))
            if spelling in ('issue50_contour_66', 'issue50_contour_parantez'):
                tags = ' '.join(self.projected[self.before_projected:])
                if spelling == 'issue50_contour_66':
                    expected = '`[.0SAs.0to.1SeS]'
                else:
                    expected = ('`[.0pA.0rAn.1tez] `[.1baz]' if text == 'parantez baaz'
                                else '`[.0pA.0ran.1tez] `[.1baz]')
                if expected not in tags or not any(self.syllable_fits[self.before_syllable_fits:]):
                    raise AssertionError('NVDA lost the reported contour or syllable alignment: ' + ascii((text, tags)))
            if spelling in ('issue50_compact_colon_most', 'issue50_compact_colon_split_most',
                            'issue50_compact_colon_reference_none'):
                tags = ' '.join(self.projected[self.before_projected:])
                if '`[.0do.0noR.1te]' not in tags:
                    raise AssertionError('NVDA colon name lost its connected numeral vowel: ' + ascii((text, tags)))
            if spelling == 'issue50_hundred_short':
                tags = ' '.join(self.projected[self.before_projected:])
                if not re.search(r'\.[012]sA', tags) or "s'A:'" in tags:
                    raise AssertionError('NVDA elongated the hundred-group vowel: ' + ascii((text, tags)))
            if spelling == 'issue50_plural':
                expected = ('`[.0Af.0zu.0ne.1ha.0je]' if text == 'افزونه‌های' else
                            "`[.0bAr.0na.0me.1ha.0je]")
                tags = ' '.join(self.projected[self.before_projected:])
                if expected not in tags:
                    raise AssertionError('NVDA lost plural/ezafe stress: ' + ascii((text, tags)))
            if spelling == 'issue50_l_h':
                expected = {'کانال‌ها': '`[.0ka.0nal.1ha]',
                            'پاتیل‌ها': '`[.0pa.0til.1ha]'}[text]
                tags = ' '.join(self.projected[self.before_projected:])
                if expected not in tags or not any(self.us_fits[self.before_us_fits:]):
                    raise AssertionError('NVDA lost the plural h or US contour: ' + ascii((text, tags)))
            if spelling == 'issue50_r_after_i_vowel':
                expected = {'تیره': "`[.0ti.1re]", 'ایران': "`[.0i.1ran]",
                            'سیری': '`[.0si.1ri]'}[text]
                tags = ' '.join(self.projected[self.before_projected:])
                if expected not in tags or not any(self.us_fits[self.before_us_fits:]):
                    raise AssertionError('NVDA lost the medial r or US contour: ' + ascii((text, tags)))
            if spelling == 'issue50_r_after_i_x_speed38':
                tags = ' '.join(self.projected[self.before_projected:])
                if self.synth.rate != 38 or '`[.1zir.0GAt]' not in tags or not any(self.us_fits[self.before_us_fits:]):
                    raise AssertionError('NVDA lost zirkhat or its US contour at speed 38: ' + ascii(tags))
            if spelling == 'issue49_native_new_compound':
                expected={'بختیاری‌زاده':"`[.0bAG.0ti.0ja.0ri.0za.1de]",
                          'بختیارزاده':"`[.0bAG.0ti.0jar.0za.1de]",
                          'فوتبال':"`[.0fut.1bal]", 'فوتبالیست':"`[.0fut.0ba.1list]",
                          'پژوهشگران':'`[.0pA.0Zu.0heS.0gA.1ran]',
                          'پژوهشگری':'`[.0pA.0Zu.0heS.0gA.1ri]',
                          'پژوهشگرها':'`[.0pA.0Zu.0heS.0gAr.1ha]',
                          'پژوهشگر‌ها':'`[.0pA.0Zu.0heS.0gAr.1ha]',
                          'کارگرها':'`[.0kar.0gAr.1ha]',
                          'کوچک':"`[.0ku.1CAk]", 'کوچکتر':"`[.0ku.0CAk.1tAr]",
                          'اصغر':'`[.0As.1RAr]', 'اکبر':'`[.0Ak.1bAr]',
                          'اخگر':'`[.0AG.1gAr]', 'احمد':'`[.0Ah.1mAd]',
                          'عکس':'`[.1Aks]', 'عکسی':'`[.1Ak.0si]',
                          'aks':'`[.1Aks]', 'aksi':'`[.1Ak.0si]',
                          'bar':'`[.1bAr]', 'baar':'`[.1bar]',
                          'man':'`[.1mAn]', 'maan':'`[.1man]',
                          'salam':'`[.0sA.1lAm]', 'salaam':'`[.0sA.1lam]',
                          'iran':'`[.0i.1rAn]', 'iraan':"`[.0i.1ran]",
                          'عمومی':'`[.0o.0mu.1mi]', 'omumi':'`[.0o.0mu.1mi]',
                          'خصوصی':'`[.0Go.0su.1si]', 'khosusi':'`[.0Go.0su.1si]',
                          'تنظیمات عمومی':'`[.0tAn.0zi.1mat] `[.0o.0mu.1mi]',
                          'tanzimaat omumi':'`[.0tAn.0zi.1mat] `[.0o.0mu.1mi]',
                          'کودک':"`[.0ku.1dAk]",
                          'دو کودک':"`[.1do] `[.0ku.1dAk]",
                          'مادر کودک':"`[.0ma.1dAr] `[.0ku.1dAk]",
                          'معرفی':'`[.0mo.0wAr.0re.1fi]',
                          'خواجه‌ئیان':"`[.1Ga.0Je.0i.1an]"}[text]
                tags=' '.join(self.projected[self.before_projected:])
                if expected not in tags or not any(self.us_fits[self.before_us_fits:]):
                    raise AssertionError('NVDA lost native word rules or US contour: '+ascii((text,tags)))
            if spelling in ('tags_slow', 'tags_fast', 'tags_pitch', 'tags_phonemes'):
                expected = text.split(' ', 1)[0]
                if expected not in self.user_controls[self.before_user_controls:]:
                    raise AssertionError('NVDA lost the ECI tag: ' + expected)
            if spelling == 'tags_phoneme_own' and not any(self.us_fits[self.before_us_fits:]):
                raise AssertionError('NVDA explicit phonetics lost the chosen Persian contour: '+ascii(text))
            times = [value for player in self.players
                     for value in player.times[self.before.get(id(player), 0):]]
            if not times:
                raise RuntimeError('missing NVDA audio timing for ' + ascii(text))
            if spelling == 'pipeline_dash' and len(self.continuous) != self.before_continuous:
                raise AssertionError('NVDA lost the separated language boundary around a hyphen: ' + ascii(text))
            if spelling in ('issue49_hyphen', 'issue49_unknown'):
                tags = ' '.join(self.projected[self.before_projected:])
                expected = ("`[.0do.0ros.1te]" if 'درسته' in text else '`[.0a.1li]')
                if expected not in tags:
                    details = self.projected_sources[self.before_projected_sources:]
                    raise AssertionError('NVDA spelled a Persian word: ' + ascii(details))
            if spelling in ('issue49_heading', 'issue49_heading_split'):
                tags = ' '.join(self.projected[self.before_projected:])
                if not tags or ']`[' in tags:
                    raise AssertionError('NVDA joined Persian number and vowel tags: ' + ascii(tags))
            if self.version_case and locale == 'fa_Latn':
                tags = ' '.join(self.projected[self.before_projected:])
                if ']`[' in tags or any(value not in tags for value in ('jek', 'pAnJ', 'sefr')):
                    raise AssertionError('NVDA lost Pinglish version numbers or joined tags: ' + ascii(tags))
            if spelling is True and text in 'ًٌٍَُِّْٰ':
                expected = {'ً':'`[.1An]', 'ٌ':'`[.1on]', 'ٍ':'`[.1en]',
                            'َ':'`[.1A]', 'ُ':'`[.1o]', 'ِ':'`[.1e]',
                            'ّ':'`[.0tAS.1did]', 'ْ':'`[.0sa.1ken]',
                            'ٰ':"`[.0A.1lef] `[.0mAR.0su.1re]"}[text]
                tags = ' '.join(self.projected[self.before_projected:])
                if expected not in tags:
                    raise AssertionError('NVDA lost the Persian character name: ' + ascii(tags))
            if ((spelling is True and text in 'ٔأˈˌə̇αé⚽/\\') or
                    spelling in ('issue49_hamza_line', 'issue49_hamza_line_none')):
                expected = {'ٔ': "`[.0hAm.1ze.0je] `[.0ba.1la]",
                            'أ': "`[.0A.1lef] `[.0hAm.1ze]"}.get(text, self.symbol_tags.get(text))
                tags = ' '.join(self.projected[self.before_projected:])
                if expected not in tags:
                    raise AssertionError('NVDA lost the reported character name: ' + ascii((text, tags)))
            if (locale in ('fa_IR', 'fa_Latn') and text == '\x7f' and
                    isinstance(spelling, str) and spelling.startswith('unicode_')):
                tags = ' '.join(self.projected[self.before_projected:])
                if '`[.1hAzf]' not in tags:
                    raise AssertionError('NVDA did not use the Persian delete name: ' + ascii(tags))
            if (locale in ('fa_IR', 'fa_Latn') and text in ('\u202e', '✓') and
                    isinstance(spelling, str) and spelling.startswith('unicode_')):
                expected = {'\u202e': '`[.0baz.0ne.0vi.1si.0je]', '✓': '`[.1tik]'}[text]
                tags = ' '.join(self.projected[self.before_projected:])
                if expected not in tags:
                    raise AssertionError('NVDA lost a Persian symbol name: ' + ascii((text, tags)))
            # The same ASCII plus sign is also exercised for other voices
            # and for the independent Unicode cases below. This particular
            # expectation belongs to the original Persian alāve cases only.
            if locale == 'fa_IR' and (text in ('به علاوه', 'بعلاوه') or
                                      text == '+' and spelling is True):
                tags = ' '.join(self.projected[self.before_projected:])
                expected = {"بعلاوه": "`[.0be.0A.0la.1ve]",
                            "به علاوه": "`[.0be] `[.0A.0la.1ve]",
                            "+": "`[.0be] `[.0A.0la.1ve]"}[text]
                if expected not in tags:
                    raise AssertionError('NVDA lost the corrected alāve reading: ' + ascii(tags))
            if spelling in ('pipeline', 'pipeline_most', 'pipeline_stop', 'pipeline_log', 'pipeline_symbols', 'pipeline_mixed_symbols', 'pipeline_user_latency', 'checkbox_unchecked', 'checkbox_checked', 'checkbox_checked_most'):
                tags = ' '.join(self.projected[self.before_projected:])
                if spelling in ('pipeline', 'pipeline_most') and locale == 'fa_IR' and ('`[.0hen.1ga.0me]' not in tags or
                        "`[.1eG.0tja.2ra.0te]" not in tags):
                    raise AssertionError('NVDA preprocessing lost the vocalized Persian words: ' + ascii(tags))
                if spelling == 'pipeline_stop' and '`[.0mo.0te.0vAR.1Ref]' not in tags:
                    raise AssertionError('NVDA spelled the Persian word for stopped: ' + ascii(tags))
                if spelling == 'pipeline_log' and ("`[.1et.0te.1fa.0Ra.2te]" not in tags or
                        "`[.0sAbt.0So.1de]" not in tags):
                    raise AssertionError('NVDA spelled part of the normalized log label: ' + ascii(tags))
                if 'تنظیماتِ ذخیره‌شده‌ی' in text and (
                        '`[.0zA.0Gi.0re.0So.1de.0je]' not in tags or
                        '`[.0mo.0di.0ri.0jA.1ti]' not in tags):
                    raise AssertionError('NVDA lost reported sentence morphology or stress: ' + ascii(tags))
                if spelling == 'pipeline_user_latency' and 'نشان دادَنِ' in text and any(tag not in tags for tag in (
                        '`[.0ne.1San]', '`[.1da.0dA.2ne]', '`[.0hen.1ga.0me]', '`[.0Ga.1reJ]')):
                    raise AssertionError('NVDA spelled part of the reported latency example: ' + ascii(tags))
                if spelling == 'pipeline_user_latency' and 'پخش کردنِ' in text and any(tag not in tags for tag in (
                        '`[.1pAGS]', '`[.0kAr.1dA.0ne]', '`[.0hen.1ga.0me]')):
                    raise AssertionError('NVDA spelled part of the playback label: ' + ascii(tags))
                if spelling == 'pipeline_user_latency' and 'بررسیِ' in text and any(tag not in tags for tag in (
                        '`[.0bAr.0rA.1si.0e]', '`[.0God.1ka.0re]')):
                    raise AssertionError('NVDA spelled part of the update label: ' + ascii(tags))
                if spelling == 'pipeline_symbols':
                    expected_words = (('`[.0sA.1lAm]', '`[.0don.1jA]') if locale == 'fa_Latn'
                                      else ('`[.0sA.1lam]', '`[.0do.1nja]'))
                    if any(word not in tags for word in expected_words) or '`[.1ta.0vu.2merk.0ki]' in tags:
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
            if self.compact_case:
                result['compact_debug'] = self.version_debug[self.before_version_debug:]
                result['projection'] = self.projected_sources[self.before_projected_sources:]
            self.results.append(result)
            self.index += 1
            self.next()
        except BaseException:
            self.fail()
