"""Check arbitrary Persian input, direct Pinglish, and continuous routing."""
from __future__ import annotations

import argparse
from pathlib import Path
import random
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
import continuous_driver
from direct_fa_sequence import project_resilient
from nvda_driver_probe import PERSIAN_PROBE, PINGLISH_PROBE, make_language_engine
from persian_symbols import SYMBOL_TAGS, UNICODE_TAG
from _reloq_eci_direct import EciIndex, EciLanguage, EciPitch, EciText
from _reloq_languages import US_ENGLISH


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('scope-probe', 'native-library', 'worker', 'espeak-library', 'data-parent'):
        parser.add_argument('--' + name, type=Path, required=True)
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
        job = project_resilient(('سلام @ (🙂) دنیا',), frontend)
        assert SYMBOL_TAGS['@'] in job.tagged and SYMBOL_TAGS['('] in job.tagged
        assert UNICODE_TAG in job.tagged and SYMBOL_TAGS[')'] in job.tagged
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

        engine.select_voice(PERSIAN_PROBE.id, '1')
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
    finally:
        continuous_driver.selected_sentence = original
        engine.close()
    print(f'Passed {len(cases) * 2} arbitrary input cases, direct Pinglish, marked switches and controls.')


if __name__ == '__main__':
    main()
