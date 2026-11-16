"""Check arbitrary Persian input, direct Pinglish, and continuous routing."""
from __future__ import annotations

import argparse
import itertools
from pathlib import Path
import random
import re
import string
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
import continuous_driver
from direct_fa_sequence import ScopedDirectPersianEngine, project, project_resilient, without_generated_stress
from nvda_driver_probe import PERSIAN_PROBE, PINGLISH_PROBE, make_language_engine
from persian_symbols import SYMBOL_TAGS, UNICODE_TAG
from text_frontend import native_sequence_accepted
from _reloq_eci_direct import EciIndex, EciLanguage, EciPitch, EciText
from _reloq_languages import US_ENGLISH


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('scope-probe', 'native-library', 'worker', 'espeak-library', 'data-parent'):
        parser.add_argument('--' + name, type=Path, required=name != 'worker')
    args = parser.parse_args()
    engine = make_language_engine(args.scope_probe, args.native_library, args.worker,
                                  args.espeak_library, args.data_parent)
    original = continuous_driver.selected_sentence
    plans = []
    def observe(*items, **options):
        plans.append(1)
        return original(*items, **options)
    continuous_driver.selected_sentence = observe

    def render(parts):
        pcm, indexes = bytearray(), []
        completed = engine.speak(parts, pcm.extend, lambda value: indexes.append((value, len(pcm))))
        assert completed
        return bytes(pcm), indexes

    try:
        frontend = engine._engines[PERSIAN_PROBE.id]._frontend
        exact = 'استفاده از NVDA هنگامِ ورود به ویندوز (به اِختیاراتِ مدیریتی نیاز دارد)'
        after = 'هنگامِ ورود به ویندوز (به اِختیاراتِ مدیریتی نیاز دارد)'
        direct = frontend.analyze_direct(after)
        assert direct and "`[.0hen.1ga.0me]" in direct.parts[0].text
        assert "`[.1'e:'G.0tja.2ra.0te]" in direct.parts[0].text
        pcm, indexes = render((EciIndex(80), exact[:24], EciIndex(81), exact[24:], EciIndex(82)))
        assert pcm and [value for value, _ in indexes] == [80, 81, 82]
        assert frontend._worker is None, 'the NVDA label should use the direct text path'
        observed, previous_prepared = [], ScopedDirectPersianEngine._speak_prepared
        def watch_status(instance, prepared, *items, **options):
            if prepared.source:
                observed.append((prepared.source, without_generated_stress(prepared).tagged))
            return previous_prepared(instance, prepared, *items, **options)
        ScopedDirectPersianEngine._speak_prepared = watch_status
        try:
            for status, expected in (('بدونِ علامت', "`[.0A.1la.0mAt]"),
                                     ('علامت‌دار', "`[.0A.1la.0mAt] `[.1dar]")):
                observed.clear()
                pcm, marks = render((EciIndex(83), EciText(exact + '  '), EciIndex(84),
                    EciText('کادرِ علامت‌زدنی  '), EciIndex(85), EciText(status + '  '), EciIndex(86)))
                assert pcm and [value for value, _ in marks] == [83, 84, 85, 86]
                expected = re.sub(r'\.[12]', '.0', expected)
                assert any(status in source and expected in tags for source, tags in observed), observed
                # The entire role and status must come from the native
                # Persian job, not from a bilingual proxy's final frames.
                direct_pcm, _ = render(('کادرِ علامت‌زدنی  ' + status + '  ',))
                assert pcm[marks[1][1]:] == direct_pcm, status
                if status == 'علامت‌دار':
                    # The original orthographic word and the two verified
                    # constituent words use the same native phrase tone.
                    assert direct_pcm == render(('کادرِ علامت‌زدنی  علامت دار  ',))[0]
                assert frontend._worker is None
        finally:
            ScopedDirectPersianEngine._speak_prepared = previous_prepared
        for text, tag in (('حالتِ بومیِ انتخاب را فعال یا غیرفعال می‌کند', '`[.0mi.1ko.0nAd]'),
                          ('ذره بین به دلیلِ بُروزِ خطا متوقف شد.', '`[.2mo.0te.1vAR.0Ref]')):
            assert frontend.analyze_direct(text), text
            job = project_resilient((text,), frontend)
            assert tag in job.tagged, text
            assert render((text,))[0]
            assert frontend._worker is None, text
        assert frontend.analyze_direct('متوقف').parts[0].text == '`[.2mo.0te.1vAR.0Ref]'
        import unicodedata
        log_label = 'نمایشِ اِتِّفاقاتِ ثبت‌شده (Log)'
        normalized = unicodedata.normalize('NFC', log_label)
        target = "`[.1'e:'t.0te.1fa.0Ra.2te]"
        for label in (log_label, normalized):
            part = frontend.analyze_direct(label.split(' (')[0])
            assert part and target in part.parts[0].text
            assert '`[.1sAbt.1So.0d\'e:\']' in part.parts[0].text
            assert render((EciIndex(92), label, EciIndex(93)))[0]
        assert frontend._worker is None, 'the normalized NVDA label should use direct text rules'
        unlisted = 'اِتِّفاقات'
        assert frontend.analyze_direct(unlisted) is None
        assert frontend.analyze(unlisted).parts == frontend.analyze(unicodedata.normalize('NFC', unlisted)).parts
        job = project_resilient(('سلام @ (🙂) دنیا',), frontend)
        assert SYMBOL_TAGS['@'] in job.tagged and '(' in job.tagged
        assert UNICODE_TAG in job.tagged and ')' in job.tagged
        # Issue 49: both examples contain words outside the direct lexicon.
        # Test the actual fallback with and without NVDA text/index splits.
        for text, expected in (('بله این حرف شما - درسته حتما', "`[.0do.1ros.0t'e:']"),
                               ('ما عالی هستیم - خخخخ', '`[.1a.0li]')):
            parts = (EciIndex(1), text[:len(text)//2], EciIndex(2),
                     text[len(text)//2:], EciIndex(3))
            plain = project_resilient((text,), frontend,
                validate=engine._engines[PERSIAN_PROBE.id]._voice._phonemes_for_probe)
            split = project_resilient(parts, frontend,
                validate=engine._engines[PERSIAN_PROBE.id]._voice._phonemes_for_probe)
            assert expected in plain.tagged and plain.tagged == split.tagged, (text, split.tagged)
            assert SYMBOL_TAGS['-'] not in split.tagged, (text, split.tagged)
            assert [item.value for item in split.parts if isinstance(item, EciIndex)] == [1, 2, 3]
            pcm, marks = render(parts)
            assert pcm and [value for value, _ in marks] == [1, 2, 3]
            # Some eSpeak versions return valid tags which spell a nearby
            # word instead of reporting an error for a whole hyphenated
            # clause. Such a result must never be requested in the first place.
            original_analyze = frontend.analyze
            def checked_analyze(value, **kwargs):
                assert '-' not in value, ('whole hyphenated reference clause', value)
                return original_analyze(value, **kwargs)
            frontend.analyze = checked_analyze
            try:
                observed = []
                previous_speak = ScopedDirectPersianEngine._speak_prepared
                def inspect_speak(instance, prepared, *args, **kwargs):
                    if prepared.source:
                        observed.append(without_generated_stress(prepared).tagged)
                    return previous_speak(instance, prepared, *args, **kwargs)
                ScopedDirectPersianEngine._speak_prepared = inspect_speak
                try:
                    # NVDA cancels the preceding utterance before a new
                    # navigation item. Its stale native stop must not make
                    # reference validation spell an otherwise valid word.
                    engine.cancel()
                    assert render(parts)[0]
                finally:
                    ScopedDirectPersianEngine._speak_prepared = previous_speak
                expected_unmarked = re.sub(r'\.[12]', '.0', expected)
                assert any(expected_unmarked in tags for tags in observed), (text, observed)
            finally:
                frontend.analyze = original_analyze
        # Mixed-language planning projects the Persian segment before it
        # starts playback, and must reset the same stale native stop.
        engine.cancel()
        persian = engine._engines[PERSIAN_PROBE.id]
        persian._cancelled.clear()
        assert persian._voice._cancelled.is_set()
        tagged, _marks, _needs_validation = continuous_driver._project_part(
            persian, ('بله این حرف شما - درسته حتما',), follows=True)
        assert "`[.0do.0ros.0t'e:']" in tagged, tagged
        direct_dash = frontend.analyze_direct('سلام-دنیا')
        assert direct_dash and '-' not in direct_dash.parts[0].text
        named_dash = project((EciText('-', character_mode=True),), frontend, allow_fallback=True)
        assert named_dash.tagged == SYMBOL_TAGS['-']
        for word, expected in (('احیا', '`[.1e.0hja]'), ('مهربد', '`[.1mehr.0bod]'),
                               ('معد', '`[.1mo.0wedd]'), ('معقر', '`[.0mo.1RA.0wwAr]')):
            recovered = project_resilient((word,), frontend,
                validate=engine._engines[PERSIAN_PROBE.id]._voice._phonemes_for_probe)
            assert recovered.tagged == expected, (word, recovered.tagged)
        # A new inflection is deliberately outside the direct admission table.
        # The native display hides its coda H, but the word must not be spelled.
        inflected = 'دلرحمترین'
        assert frontend.analyze_direct(inflected) is None
        recovered = project_resilient((inflected,), frontend,
            validate=engine._engines[PERSIAN_PROBE.id]._voice._phonemes_for_probe)
        assert recovered.tagged == '`[.2del.0rAhm.1tA.0rin]'
        worker = frontend._fallback()
        reference_calls, original_translate = [], worker.translate
        def observe_translate(text, **kwargs):
            reference_calls.append(text)
            return original_translate(text, **kwargs)
        worker.translate = observe_translate
        try:
            for _ in range(2):
                before = len(reference_calls)
                again = project_resilient((inflected,), frontend,
                    validate=engine._engines[PERSIAN_PROBE.id]._voice._phonemes_for_probe)
                assert again.tagged == recovered.tagged and len(reference_calls) > before
        finally:
            worker.translate = original_translate
        assert not hasattr(frontend, '_resilient_word_cache')
        assert render((EciIndex(90), inflected, EciIndex(91)))[0]
        cases = ('سلام @@@ دنیا', 'سلام 🙂 دنیا', 'سلام (دنیا) و خوب', 'عدد ۱۲-۱۵ است',
                 'نسخه 1.2.3', 'کتاب ۵/۳ روز', 'متن\nجدید', 'سلام\tدنیا', '!',
                 'سلام ❤️', '\u200c', '\0', 'دما ۳٫۱۴ درجه', '\u202e', '\ud800',
                 'آدرس https://example.com/path?a=1', 'email: test@example.com')
        randomizer = random.Random(20261115)
        alphabet = list('سلام دنیا کتاب123-./@#?!🙂\n\t') + ['\0', '\ud800', '\u200c', '€']
        cases += tuple(''.join(randomizer.choice(alphabet) for _ in range(randomizer.randrange(1, 16)))
                       for _ in range(48))
        for language in (PERSIAN_PROBE.id, PINGLISH_PROBE.id):
            engine.select_voice(language, '1')
            for index, text in enumerate(cases):
                split = len(text) // 2
                pcm, marks = render((EciIndex(1), text[:split], EciIndex(2), text[split:], EciIndex(3)))
                assert pcm and [value for value, _ in marks] == [1, 2, 3], (language, index, repr(text), marks)
                assert 0 <= marks[0][1] <= marks[1][1] <= marks[2][1] <= len(pcm)

        engine.select_voice(PINGLISH_PROBE.id, '1')
        pinglish = engine._engines[PINGLISH_PROBE.id]
        # Use a fresh engine so no previous reference process can hide a
        # regression in the direct grammar or in English exception routing.
        engine.close()
        engine = make_language_engine(args.scope_probe, args.native_library, args.worker,
                                      args.espeak_library, args.data_parent, language=PINGLISH_PROBE.id)
        pinglish = engine._engines[PINGLISH_PROBE.id]
        before = len(plans)
        implicit, marks = render((EciIndex(10), 'salam pho', EciIndex(11), 'ne khubtar', EciIndex(12)))
        assert len(plans) > before and pinglish._frontend._worker is None
        assert [item[0] for item in marks] == [10, 11, 12] and marks[-1][1] == len(implicit)
        explicit, _ = render(('salam ', EciLanguage(US_ENGLISH.id, '1'), 'phone',
                              EciLanguage(PINGLISH_PROBE.id, '1'), ' khubtar'))
        plain, _ = render(('salam phone khubtar',))
        assert explicit == plain
        assert pinglish._frontend._worker is None
        for word in ('chetori khane', 'khubtar salam', 'darya zaban'):
            assert render((word,))[0] and pinglish._frontend._worker is None
        plain, _ = render(('farsi',))
        combined, marks = render(('fa', EciIndex(70), '\u0304rsi'))
        assert combined == plain and [value for value, _ in marks] == [70]
        before = len(plans)
        accented, marks = render((EciIndex(71), 'salam sa', EciIndex(72), 'lām donya', EciIndex(73)))
        assert accented and len(plans) > before and [value for value, _ in marks] == [71, 72, 73]
        explicit, _ = render(('salam ', EciLanguage(US_ENGLISH.id, '1'), 'salām',
                              EciLanguage(PINGLISH_PROBE.id, '1'), ' donya'))
        unmarked, _ = render(('salam salām donya',))
        assert explicit == unmarked
        hyphen = project_resilient(('salam-e',), pinglish._frontend, pinglish=True,
                                   validate=pinglish._voice._phonemes_for_probe)
        assert '-' not in hyphen.tagged and SYMBOL_TAGS['-'] not in hyphen.tagged
        assert '`[.1ta.0vu.2merk.0ki]' not in pinglish._voice._phonemes_for_probe(hyphen.tagged)
        for word in ('salam-e', 'salam-donya', 'salam -e'):
            prepared = project_resilient((word,), pinglish._frontend, pinglish=True,
                                         validate=pinglish._voice._phonemes_for_probe)
            assert '`[.1ta.0vu.2merk.0ki]' not in pinglish._voice._phonemes_for_probe(prepared.tagged)
            assert render((word,))[0]

        engine.select_voice(PERSIAN_PROBE.id, '1')
        bracketed, _ = render(('سلام (دنیا) خوب',))
        native_comma, _ = render(('سلام، دنیا، خوب',))
        assert bracketed == native_comma
        for bracketed, comma in (('سلام (دنیا)', 'سلام (دنیا),'),
                                 ('سلام (دنیا):', 'سلام (دنیا:'),
                                 ('سلام (NVDA)', 'سلام (NVDA),'),
                                 ('سلام (NVDA):', 'سلام (NVDA:'),
                                 ('سلام (NVDA): خوب', 'سلام (NVDA: خوب'),
                                 ('NVDA (سلام)', 'NVDA (سلام),'),
                                 ('NVDA (سلام): خوب', 'NVDA (سلام: خوب')):
            actual, marks = render((EciIndex(70), bracketed, EciIndex(71)))
            expected, expected_marks = render((EciIndex(70), comma, EciIndex(71)))
            assert actual == expected and marks == expected_marks, bracketed
        for prefix in ('سلام (دنیا', 'سلام (NVDA'):
            colon = render((prefix + '):',))[0]
            comma = render((prefix + '),',))[0]
            assert colon != comma, 'the following colon must not become a comma contour'
        for opening, closing in ('()', '[]', '{}'):
            bracket = 'سلام ' + opening + 'دنیا' + closing
            assert render((bracket,))[0] == render((bracket + ',',))[0]
            assert render((bracket + ':',))[0] == render(('سلام ' + opening + 'دنیا:',))[0]
        engine.select_voice(PINGLISH_PROBE.id, '1')
        bracketed, _ = render(('salam (donya) khub',))
        native_comma, _ = render(('salam, donya, khub',))
        assert bracketed == native_comma
        assert render(('salam (donya)',))[0] == render(('salam (donya),',))[0]
        assert render(('salam (donya):',))[0] == render(('salam (donya:',))[0]
        assert render(('salam (donya):',))[0] != render(('salam (donya),',))[0]
        engine.select_voice(PERSIAN_PROBE.id, '1')
        before = len(plans)
        mixed, _ = render(('سلام (hello) دنیا',))
        # A bracketed language boundary is rendered by each native core;
        # it must never give the start of the following word to a proxy.
        assert mixed and len(plans) == before
        explicit, _ = render(('سلام (', EciLanguage(US_ENGLISH.id, '1'), 'hello) ',
                              EciLanguage(PERSIAN_PROBE.id, '1'), 'دنیا'))
        assert mixed == explicit
        # An irrelevant unmappable Persian word marker must not discard an
        # otherwise viable continuous mixed-language sentence.
        original_replay = continuous_driver.FrameReplay.replay
        original_stream = continuous_driver.FrameStream.available
        continuous_driver.FrameStream.available = staticmethod(lambda _dll: False)
        replayed = []
        def observe_replay(self, *items, **options):
            result = original_replay(self, *items, **options)
            if result:
                replayed.append(1)
            return result
        continuous_driver.FrameReplay.replay = observe_replay
        try:
            assert render(('سلام ن۱š دنیا',))[0] and replayed
        finally:
            continuous_driver.FrameReplay.replay = original_replay
            continuous_driver.FrameStream.available = staticmethod(original_stream)
        before = len(plans)
        pcm, marks = render((EciIndex(20), EciPitch(80), 'NVDA خوب است.', EciIndex(21)))
        assert len(plans) > before and marks == [(20, 0), (21, len(pcm))]
        before = len(plans)
        pcm, marks = render((EciIndex(22), 'سلام ', EciLanguage(US_ENGLISH.id, '1'),
                             EciText('NVDA', character_mode=True), EciLanguage(PERSIAN_PROBE.id, '1'),
                             'خوب', EciIndex(23)))
        assert len(plans) > before and marks == [(22, 0), (23, len(pcm))]
        pcm, marks = render((EciIndex(24), EciText('`vs40', False, '`vs40'),
                             EciText('@', character_mode=True), ' دنیا', EciIndex(25)))
        assert pcm and [item[0] for item in marks] == [24, 25]
        # Long text is broken into bounded jobs and retains all source marks.
        pcm, marks = render((EciIndex(26), 'سلام دنیا. ' * 130, EciIndex(27),
                             'سلام.', EciIndex(28)))
        assert pcm and [item[0] for item in marks] == [26, 27, 28]

        # Exercise symbols in normal text and spelling mode through the
        # actual native phoneme parser. Every expected tag must survive;
        # additional Finnish symbol names or spelled tag syntax fail this.
        count = 0
        def check_symbol_projection(local, language, text, spelling=False):
            item = EciText(text, character_mode=spelling)
            try:
                prepared = project((item,), local._frontend, pinglish=local._pinglish,
                                   allow_fallback=True)
            except (ValueError, RuntimeError, OSError, UnicodeError):
                prepared = project_resilient((item,), local._frontend, pinglish=local._pinglish,
                    validate=local._voice._phonemes_for_probe)
            tagged = prepared.tagged.replace('~', ' ')
            display = local._voice._phonemes_for_probe(tagged)
            assert native_sequence_accepted(tagged, display), (language, text, spelling, tagged, display)
        for language, word in ((PERSIAN_PROBE.id, 'سلام'), (PINGLISH_PROBE.id, 'salam')):
            engine.select_voice(language, '1')
            local = engine._engines[language]
            for char in string.punctuation + '،؛؟٪٫٬…«»€°±×÷':
                for text, spelling in ((char, False), (char + word, False),
                                       (word + char, False), (word + char + word, False),
                                       (char, True)):
                    check_symbol_projection(local, language, text, spelling)
                    count += 1
            for length in (2, 3):
                for marks in itertools.product('()-[]{}', repeat=length):
                    check_symbol_projection(local, language, word + ''.join(marks) + word)
                    count += 1
            for brackets in ('()', '[]', '{}'):
                a = word + brackets[0] + word + brackets[1] + word
                b = word + ' ' + brackets[0] + ' ' + word + ' ' + brackets[1] + ' ' + word
                assert render((a,))[0] == render((b,))[0], (language, brackets)
            one, marks = render((EciIndex(200), word, EciIndex(201), '[', EciIndex(202),
                                 word, EciIndex(203), ']', EciIndex(204), word, EciIndex(205)))
            assert one and [value for value, _ in marks] == list(range(200, 206))
        assert count == 1244

        # Punctuation can also occur in the private Persian proxy for an
        # English segment, including empty brackets at a split boundary.
        # Verify those planning inputs with the actual Finnish parser too.
        old_prepared = ScopedDirectPersianEngine._speak_prepared
        internal_brackets = []
        def check_internal(instance, prepared, *items, **options):
            if not prepared.source and any(c in re.sub(r'`\[[^]]+\]', '', prepared.tagged) for c in '[]{}'):
                tagged = prepared.tagged.replace('~', ' ')
                shown = instance._voice._phonemes_for_probe(tagged)
                assert native_sequence_accepted(tagged, shown), (tagged, shown)
                internal_brackets.append(1)
            return old_prepared(instance, prepared, *items, **options)
        ScopedDirectPersianEngine._speak_prepared = check_internal
        try:
            for language, text in ((PERSIAN_PROBE.id, 'سلام NVDA[]سلام'),
                                   (PERSIAN_PROBE.id, 'سلام NVDA{}سلام'),
                                   (PINGLISH_PROBE.id, 'salam phone[]salam')):
                engine.select_voice(language, '1')
                assert render((EciIndex(210), text, EciIndex(211)))[0]
            assert internal_brackets
        finally:
            ScopedDirectPersianEngine._speak_prepared = old_prepared
    finally:
        continuous_driver.selected_sentence = original
        engine.close()
    print(f'Passed {len(cases) * 2} arbitrary input cases, direct Pinglish, marked switches and controls.')


if __name__ == '__main__':
    main()
