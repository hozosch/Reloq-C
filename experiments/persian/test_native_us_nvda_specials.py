"""Keep NVDA audio and cursor indexes unchanged for punctuation and symbols."""
from __future__ import annotations
import argparse
from pathlib import Path
import random
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import EciIndex, NATIVE_SPEED, SAMPLE_RATE
from nvda_driver_probe import make_language_engine, PERSIAN_PROBE, PINGLISH_PROBE


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    path = parser.parse_args().library.resolve()
    absent = Path('/unavailable')
    rng = random.Random(1497)
    cases = (
        (PERSIAN_PROBE, ('💻 —', '۱۴۰۲ ؟ :', 'می‌مرد : دنیا', 'سلام (دنیا)', 'فوری، دنیا',
                         'سلام🙂 دنیا', 'کتاب - خانه', 'فوری.'),
         ('سلام', 'دنیا', 'کتاب', 'را', 'است', 'من', 'خانه', 'او', 'آمد', 'برای', 'این',
          'دو', 'فوری', 'ظهور', 'ناگهانی', '۱۲', '۱۴۰۲', '💻', '—', '؟', ':', '(', ')', '،')),
        (PINGLISH_PROBE, ('12 (', 'soal ? 💻', 'salaam (donyaa)', 'salaam - donyaa', 'salaam donyaa.'),
         ('salaam', 'donyaa', 'ketaab', 'raa', 'ast', 'man', 'khoone', 'u', 'aamad', 'in',
          'soal', 'gozine', 'khoob', 'fori', '12', '💻', ':', '?', '(', ')')),
    )
    checks = 0
    for profile, explicit, words in cases:
        owners = [make_language_engine(path, path, None, absent, absent, language=profile.id,
                                       native_sentences=enabled) for enabled in (False, True)]
        try:
            for owner in owners:
                owner.set_parameter(SAMPLE_RATE, 3)
                owner.set_parameter(NATIVE_SPEED, 82)
            random_cases = tuple(' '.join(rng.choice(words) for _ in range(rng.randint(1, 3)))
                                 for _ in range(70))
            for text in (*explicit, *random_cases):
                at = len(text) // 2
                parts = (EciIndex(10), text[:at], EciIndex(11), text[at:], EciIndex(12))
                results = []
                for owner in owners:
                    pcm, marks = bytearray(), []
                    status = owner.speak(parts, pcm.extend, lambda number: marks.append((number, len(pcm))))
                    results.append((status, bytes(pcm), tuple(marks)))
                assert results[0] == results[1], (profile.locale, text, results[0][2], results[1][2])
                checks += 1
        finally:
            for owner in owners:
                owner.close()
    print(f'Passed {checks} native NVDA symbol and punctuation audio/index comparisons.')


if __name__ == '__main__':
    main()
