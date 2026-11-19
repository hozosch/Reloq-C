"""Verify the Persian US-pilot experiment, including weak and final vowels."""
from __future__ import annotations

import argparse
from pathlib import Path
import re
import sys
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT / 'experiments/persian'), str(ROOT / 'nvda')]
from _reloq_eci_direct import (EciIndex, EciLanguage, EciPitch, EciText, NATIVE_INFLECTION,
                               NATIVE_PITCH, PROCESS_TAGS, SAMPLE_RATE)
from _reloq_persian_frames import FrameCapture
from continuous_driver import _PlanningVoice, _project_part, _sentence_colons_as_periods
from continuous_plan import CapturedRows, fit_us_word_pitch, indexed_us_proxy
from direct_fa_sequence import project, project_resilient
from hybrid_fa_frontend import PersianAnalysis
from nvda_driver_probe import make_language_engine, PERSIAN_PROBE, PINGLISH_PROBE, US_ENGLISH
from text_frontend import SpeechPart
import continuous_driver
import continuous_plan


def index_words(tagged, marks):
    words = tuple(re.finditer(r'`\[[^]]+\]', tagged))
    assert len(words) == len(marks) + 1
    parts, cursor = [], 0
    for word, mark in zip(words[1:], marks):
        parts.extend((tagged[cursor:word.start()], EciIndex(mark)))
        cursor = word.start()
    return (*parts, tagged[cursor:])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    voice = make_language_engine(args.library, args.library, None,
                                 Path('/unavailable-espeak'), Path('/unavailable'))
    voice.set_parameter(SAMPLE_RATE, 3)
    voice.set_parameter(NATIVE_PITCH, 65)
    voice.set_parameter(NATIVE_INFLECTION, 30)
    fa_key = voice._key(PERSIAN_PROBE.id, '1')
    en_key = voice._key(US_ENGLISH.id, '1')
    fa_engine = voice._activate(fa_key)
    fa = _PlanningVoice(voice, fa_key, fa_engine)
    en = _PlanningVoice(voice, en_key, voice._activate(en_key))
    en.raw.set_parameter(PROCESS_TAGS, 1)
    api = FrameCapture(fa._dll)

    def render(parts):
        pcm, indexes = bytearray(), []
        assert voice.speak(parts, pcm.extend, lambda mark: indexes.append((mark, len(pcm))))
        assert pcm
        return bytes(pcm), indexes

    try:
        # A single unsupported pronunciation in a sentence must not force
        # every otherwise exact US proxy through another native parse.
        # This exercises the real parser rather than a mocked pronunciation.
        latency_text='برای راه‌اندازی رایانه باید این گزینه انتخاب شود.'
        latency_tag=project((latency_text,),fa_engine._frontend,allow_fallback=False).tagged
        fast_en=_PlanningVoice(voice,en_key,en.engine)
        with patch.object(en.raw,'_phonemes_for_probe',wraps=en.raw._phonemes_for_probe) as native_probe:
            latency_proxy=indexed_us_proxy(latency_tag,fast_en,tuple(range(1,8)))
        expected_proxy=('`[.0mA.1ma.0mE] `[.0ra.0hAn.0da.1zi] `[.0ra.0ya.1nE] '
                        '`[.0ba.1yAd] `[.1in] `[.0go.0zi.1nE] `[.0En.0tE.1hab] `[.0SA.1vAd].')
        assert ''.join(part for part in latency_proxy if isinstance(part,str))==expected_proxy
        assert [part.value for part in latency_proxy if isinstance(part,EciIndex)]==list(range(1,8))
        assert native_probe.call_count<=4,('unnecessary per-word proxy probes',native_probe.call_count)

        # In every tested word, only F0 changes; the US end value reaches
        # the very last voiced frame, including an open final vowel.
        for text in ('دو.', 'دو؟', 'خانه.', 'سلام دنیا.', 'کتاب و خانه.', 'کتاب را.', 'خوب است.',
                     'صفر.', 'صفر، یک، دو.', 'این عدد صفر است.', 'قدر.'):
            tagged = project((text,), fa_engine._frontend, allow_fallback=False).tagged
            count = len(re.findall(r'`\[[^]]+\]', tagged))
            marks = tuple(range(1, count))
            target = CapturedRows.from_capture(api.capture(fa, index_words(tagged, marks), audio=False))
            donor = CapturedRows.from_capture(api.capture(en, indexed_us_proxy(tagged, en, marks), audio=False))
            before = [tuple(row.controls) for row in target.frames]
            assert fit_us_word_pitch(target, (0, len(target.pcm) // 2), donor,
                                     (0, len(donor.pcm) // 2), marks, marks, 16000, api), text
            assert any(old[1] != row.controls[1] for old, row in zip(before, target.frames)), text
            assert all(all(old[i] == row.controls[i] for i in range(62) if i != 1)
                       for old, row in zip(before, target.frames)), text
            target_end = next(row.controls[1] for row in reversed(target.frames)
                              if row.controls[2] > 0 and row.controls[1] > 0)
            donor_end = next(row.controls[1] for row in reversed(donor.frames)
                             if row.controls[2] > 0 and row.controls[1] > 0)
            assert abs(target_end - donor_end) < 3, (text, target_end, donor_end)

        for tag, expected in (('`[.1sefr]', '`[.1sEfs]'),
                              ('`[.1sAdr]', '`[.1sAds]')):
            proxy = ''.join(part for part in indexed_us_proxy(tag, en, ()) if isinstance(part, str))
            assert expected in proxy, (tag, proxy)
        assert '`[.1f' in ''.join(part for part in indexed_us_proxy('`[.1fra]', en, ())
                                 if isinstance(part, str))

        weak = project(('کتاب را.',), fa_engine._frontend, allow_fallback=False).tagged
        assert '`00 `[.0ra]' in weak
        assert '`00' in en._phonemes_for_probe(''.join(
            part for part in indexed_us_proxy(weak, en, (1,)) if isinstance(part, str)))
        assert project(('را',), fa_engine._frontend, allow_fallback=False).tagged == '`[.1ra]'
        assert '`00 `[.0Ast]' in project(('خوب است',), fa_engine._frontend, allow_fallback=False).tagged
        assert project(('است',), fa_engine._frontend, allow_fallback=False).tagged == '`[.1Ast]'
        assert project(('اسلش',), fa_engine._frontend, allow_fallback=False).tagged == '`[.0es.1lAS]'
        assert project(('یونانی',), fa_engine._frontend, allow_fallback=False).tagged == '`[.0ju.0na.1ni]'
        assert '`[.0ju.0na.1ni]' in fa_engine._frontend.analyze_character('α').parts[0].text
        assert fa_engine._frontend.analyze_character('/').parts[0].text == '`[.0es.1lAS]'
        assert fa_engine._frontend.analyze_character('\\').parts[0].text.endswith('`[.0es.1lAS]')
        # The actual NVDA labels, including ezafe and the single compound,
        # must be pronounced identically in word and character navigation.
        for symbol, label in (('-', 'خطِ تیره'), ('_', 'زیرخط')):
            written = project((label,), fa_engine._frontend, allow_fallback=False)
            named = project((EciText(symbol, character_mode=True),),
                            fa_engine._frontend, allow_fallback=False)
            assert named.tagged == written.tagged, (symbol, named.tagged, written.tagged)
            assert render((EciText(symbol, character_mode=True),))[0] == render((label,))[0], symbol
        # Character navigation uses the same English engine and voice as an
        # explicit English language event, including indexes and mixed text.
        for value in ('a', 'A', 'z', 'NVDA', 'aZ'):
            part = EciText(value, character_mode=True)
            automatic = render((EciIndex(90), part, EciIndex(91)))
            explicit = render((EciLanguage(US_ENGLISH.id, '1'), EciIndex(90), part, EciIndex(91)))
            assert automatic == explicit, value
        # Native English understands these Latin sounds itself, including
        # inside a word. Pinglish uses its own Persian phoneme analysis.
        for value in ('Straße', 'café', 'ß', 'Aßz'):
            for spelling in ((False, True) if value in ('ß', 'Aßz') else (False,)):
                part = EciText(value, character_mode=spelling)
                automatic = render((EciIndex(90), part, EciIndex(91)))
                explicit = render((EciLanguage(US_ENGLISH.id, '1'), EciIndex(90), part, EciIndex(91)))
                assert automatic == explicit, (value, spelling)
        # Existing Persian character names, unlike English running text,
        # remain available when a character is explicitly spelled.
        named_e = fa_engine._frontend.analyze_character('é')
        assert named_e.parts[0].language == 'fa'
        assert project((EciText('é', character_mode=True),), fa_engine._frontend,
                       allow_fallback=False).tagged == named_e.parts[0].text
        pinglish = make_language_engine(args.library, args.library, None,
                                        Path('/unavailable-espeak'), Path('/unavailable'),
                                        language=PINGLISH_PROBE.id)
        try:
            def ping(parts):
                pcm, marks = bytearray(), []
                assert pinglish.speak(parts, pcm.extend, marks.append)
                assert pcm
                return bytes(pcm), marks
            for original, folded in (('Straße', 'Strasse'), ('salām', 'salam'), ('šahr', 'shahr')):
                assert ping((original,))[0] == ping((folded,))[0], original
                frontend = pinglish._activate(pinglish._key(PINGLISH_PROBE.id, '1'))._frontend
                direct = project((original,), frontend, pinglish=True, allow_fallback=False)
                resilient = project_resilient((original,), frontend, pinglish=True)
                assert resilient.tagged == direct.tagged, (original, resilient.tagged, direct.tagged)
            assert ping((EciText('ß', character_mode=True),))[0] == ping((EciText('s', character_mode=True),))[0]
            assert frontend.analyze_pinglish_character('é') == frontend.analyze_character('é')
            sound, marks = ping((EciIndex(11), 'Stra', EciIndex(12), 'ße', EciIndex(13)))
            assert sound and marks == [11, 12, 13]
            assert pinglish._activate(pinglish._key(PINGLISH_PROBE.id, '1'))._frontend._worker is None
        finally:
            pinglish.close()
        mixed = EciText('Aبz', character_mode=True)
        explicit = (EciLanguage(US_ENGLISH.id, '1'), EciText('A', character_mode=True),
                    EciLanguage(PERSIAN_PROBE.id, '1'), EciText('ب', character_mode=True),
                    EciLanguage(US_ENGLISH.id, '1'), EciText('z', character_mode=True))
        assert render((mixed,)) == render(explicit)
        # A non-Persian Unicode name in the same spelled fragment still takes
        # the regular English-name route, not the Persian character path.
        foreign = fa_engine._frontend.analyze_character('Ж').parts[0]
        assert foreign.language == 'en-us'
        mixed = EciText('AЖz', character_mode=True)
        explicit = (EciLanguage(US_ENGLISH.id, '1'), EciText('A', character_mode=True),
                    EciText(foreign.text), EciText('z', character_mode=True))
        assert render((mixed,)) == render(explicit)
        for word in ('طور', 'سوی', 'طرف', 'خاطر', 'گفته', 'دیدن', 'دلیل'):
            joined = project(('به\u200c' + word,), fa_engine._frontend, allow_fallback=False)
            spaced = project(('به ' + word,), fa_engine._frontend, allow_fallback=False)
            assert joined.tagged == spaced.tagged
            assert render(('به\u200c' + word,))[0] == render(('به ' + word,))[0]
        # If a later unrelated word still needs the reference, do not lose
        # the native preposition or NVDA index ordering in the fallback path.
        unknown = 'ناموجودواژهتفکیک'
        assert fa_engine._frontend.analyze_direct(unknown) is None
        requests = []
        def reference(value):
            requests.append(value)
            assert value == unknown
            return PersianAnalysis((SpeechPart('fa', '`[.0na.0mo.1jud]'),), None, True)
        with patch.object(fa_engine._frontend, 'analyze', side_effect=reference):
            joined = project_resilient(('به', EciIndex(51), '\u200c', EciIndex(52),
                                        'طور ', EciIndex(53), unknown), fa_engine._frontend)
            spaced = project_resilient(('به طور ' + unknown,), fa_engine._frontend)
        assert joined.tagged == spaced.tagged and requests == [unknown, unknown]
        assert [part.value for part in joined.parts if isinstance(part, EciIndex)] == [51, 52, 53]
        pinglish = voice._activate(voice._key(PINGLISH_PROBE.id, '1'))
        assert project(('eslash',), pinglish._frontend, pinglish=True).tagged == '`[.0es.1lAS]'
        for text in ('ketab va khub', 'az ketab', 'joz iran', 'ketab ra', 'ya ketab', 'khub ast'):
            item = project((text,), pinglish._frontend, pinglish=True, allow_fallback=False)
            assert '`00 ' in item.tagged, (text, item.tagged)
        isolated = project(('ra',), pinglish._frontend, pinglish=True, allow_fallback=False)
        assert isolated.tagged == '`[.1ra]'
        after_english, _, _ = _project_part(pinglish, ('ra',), follows=False, preceded=True)
        assert after_english == '`00 `[.0ra]', after_english
        after_english, _, _ = _project_part(pinglish, ('ast',), follows=False, preceded=True)
        assert after_english == '`00 `[.0Ast]', after_english
        after_english, _, _ = _project_part(fa_engine, ('است',), follows=False, preceded=True)
        assert after_english == '`00 `[.0Ast]', after_english
        for text in ('NVDA ast', 'chetori ast', 'khub ast'):
            item = project((text,), pinglish._frontend, pinglish=True, allow_fallback=False)
            assert '`00 `[.0Ast]' in item.tagged, (text, item.tagged)
        assert project(('khub, ast',), pinglish._frontend, pinglish=True).tagged.endswith('`[.1Ast]')
        for text in ('ketab va khub', 'ketab ra', 'az ketab'):
            resilient = project_resilient((text,), pinglish._frontend, pinglish=True)
            assert '`00 ' in resilient.tagged, (text, resilient.tagged)
        assert _sentence_colons_as_periods('`[.1do]: `[.1se]', 'دو: سه') == '`[.1do]. `[.1se]'
        assert _sentence_colons_as_periods('`[.1do]:`[.1se]', '2:3') == '`[.1do]:`[.1se]'
        assert render(('سلام (دنیا) خوب',))[0] == render(('سلام، دنیا، خوب',))[0]
        assert render(('سلام دنیا: این یک آزمایش است؟',))[0] == render(
            ('سلام دنیا. این یک آزمایش است؟',))[0]

        # Exercise the same driver paths used by NVDA for isolated, mixed,
        # and inline-pitch speech; failure to fit must not silently pass.
        calls = []
        def observe(*args, **kwargs):
            result = fit_us_word_pitch(*args, **kwargs)
            calls.append(result)
            return result
        with patch.object(continuous_driver, 'fit_us_word_pitch', side_effect=observe), \
             patch.object(continuous_plan, 'fit_us_word_pitch', side_effect=observe):
            for parts in ((EciIndex(1), 'دو.', EciIndex(2)),
                          (EciIndex(1), 'کتاب و خانه.', EciIndex(2)),
                          (EciIndex(1), 'NVDA کتاب', EciIndex(2)),
                          (EciIndex(1), 'کتاب NVDA خانه.', EciIndex(2)),
                          (EciIndex(1), 'کتاب ', EciPitch(75), 'خانه.', EciIndex(2))):
                calls.clear()
                pcm, indexes = render(parts)
                assert calls and all(calls), (parts, calls)
                assert indexes[0] == (1, 0) and indexes[-1] == (2, len(pcm)), (parts, indexes)

            # NVDA often interrupts an English segment immediately before a
            # new Persian word. A stopped US handle must be reset before its
            # phoneme probe; every new utterance must still receive US pitch.
            expected, _ = render(('سلام دنیا.',))
            for _ in range(25):
                voice._activate(en_key)
                voice.cancel()
                assert en.raw._cancelled.is_set()
                calls.clear()
                pcm, _ = render(('سلام دنیا.',))
                assert calls and all(calls), calls
                assert pcm == expected, 'rapid navigation changed the contour'

        # Resetting a stale stop must not discard a new stop delivered
        # while the US handle is being prepared.
        saved_probe = en.raw._phonemes_for_probe
        def cancel_probe(text):
            voice.cancel()
            return saved_probe(text)
        audio = bytearray()
        with patch.object(en.raw, '_phonemes_for_probe', side_effect=cancel_probe):
            assert not voice.speak(('سلام دنیا.',), audio.extend, lambda _: None)
        assert not audio, 'a cancelled request emitted stale audio'

        # If adjacent source indexes collapse a word boundary, fit the
        # original US donor over the phrase instead of using Finnish F0.
        attempts = []
        def reject_word_marks(*args, **kwargs):
            if args[4]:
                attempts.append('words')
                return False
            result = fit_us_word_pitch(*args, **kwargs)
            attempts.append('phrase' if result else 'failed')
            return result
        with patch.object(continuous_driver, 'fit_us_word_pitch', side_effect=reject_word_marks):
            render(('سلام دنیا.',))
            assert attempts == ['words', 'phrase'], attempts

        attempts = []
        def reject_initial_donor(*args, **kwargs):
            if len(attempts) < 2:
                attempts.append('unavailable')
                return False
            result = fit_us_word_pitch(*args, **kwargs)
            attempts.append('rescue' if result else 'failed')
            return result
        with patch.object(continuous_driver, 'fit_us_word_pitch', side_effect=reject_initial_donor):
            render(('سلام دنیا.',))
            assert attempts == ['unavailable', 'unavailable', 'rescue'], attempts
    finally:
        voice.close()
    print('Passed US/Persian final-vowel pitch, weak words, word names and NVDA routing.')


if __name__ == '__main__':
    main()
