"""Compare slow VWPM durations with original ECI; exercise other languages.

The reference counts come from the original Eloquence_64 v25 host DLL under
Windows x86. Traditional Chinese and Greek are checked independently.
"""
import argparse
import json
from pathlib import Path

from test_eci_inline_queue import Engine

ROOT = Path(__file__).resolve().parents[1]
REFERENCE = ROOT / 'docs' / 'validation' / 'eci-original-vwpm-low.json'
VALUES = (5, 8, 10, 12, 16, 20, 30, 50, 150, 200)
ADDITIONAL = {
    0x60001: '你好。',
    0x120000: 'Γεια σου.',
}


def samples(library, language, text, value):
    e = Engine(library, language)
    try:
        e.voice(6, 50)
        return len(e.speak(f'`vwpm{value} {text}')) // 2
    finally:
        e.close()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    args = parser.parse_args()
    library = args.library.resolve()
    reference = json.loads(REFERENCE.read_text(encoding='utf8'))
    checks = 0
    for case in reference['cases']:
        if case['wpm'] not in VALUES:
            continue
        actual = samples(library, case['language'], case['text'], case['wpm'])
        # Original and ReLoq quantize 11.025 kHz in slightly different frame
        # blocks. Differences larger than 12 ms indicate a timing regression.
        delta = abs(actual - case['samples'])
        assert delta <= 133, (hex(case['language']), case['wpm'], actual,
                              case['samples'], delta)
        checks += 1
    assert checks == 130, checks
    original_checks = checks
    for language, text in ADDITIONAL.items():
        counts = [samples(library, language, text, value) for value in VALUES]
        assert all(a > b for a, b in zip(counts, counts[1:])), (hex(language), counts)
        checks += len(counts)
    # The original has 16-bit overflow at 1--4 WPM (including a completely
    # silent Canadian French case). These values use the 5-WPM native timing
    # with pitch-preserving streaming expansion, so check monotone timing and
    # the actual scale in every language, not the reference's broken samples.
    cases = [(c['language'], c['text'], c['samples']) for c in reference['cases']
             if c['wpm'] == 5]
    cases += [(language, text, samples(library, language, text, 5))
              for language, text in ADDITIONAL.items()]
    slow_checks = 0
    for language, text, baseline in cases:
        counts = [samples(library, language, text, value) for value in (1, 2, 3, 4)]
        assert all(a > b for a, b in zip(counts, counts[1:] + [baseline])), (hex(language), counts)
        for value, actual in zip((1, 2, 3, 4), counts):
            expected = baseline * 5 / value
            assert abs(actual - expected) <= max(220, expected * .015), (hex(language), value, actual, expected)
            slow_checks += 1
    transition_checks = 0
    for language, text in ((0x40000, 'Hallo.'), (0x60000, '你好。'),
                           (0x60001, '你好。'), (0x80000, 'こんにちは。'),
                           (0xa0000, '안녕하세요.'), (0x120000, 'Γεια.')):
        e = Engine(library, language)
        try:
            e.voice(6, 50)
            first = len(e.speak(f'`vwpm1 {text}'))
            second = len(e.speak(f'`vwpm5 {text}'))
            for a, b in ((1, 5), (5, 1)):
                mixed = len(e.speak(f'`vwpm{a} {text} `vwpm{b} {text}'))
                assert abs(mixed - first - second) <= max(1000, (first + second) * .02), (hex(language), a, b, mixed, first, second)
                transition_checks += 1
            assert e.speak(f'`vwpm1 `vs50 {text}') == e.speak(f'`vs50 {text}'), hex(language)
            transition_checks += 1
        finally:
            e.close()
    e = Engine(library, 0x60000)
    try:
        e.voice(6, 50)
        # Chinese's embedded English bank must receive the current WPM too.
        counts = [len(e.speak(f'`vwpm{value} Hello world.'))
                  for value in (1, 5, 20, 50, 150)]
        assert all(a > b for a, b in zip(counts, counts[1:])), counts
        transition_checks += len(counts)
    finally:
        e.close()
    print(f'VWPM: {original_checks} original timing checks and '
          f'{checks - original_checks} additional-language checks passed; '
          f'{slow_checks} very-slow and {transition_checks} transition checks passed')


if __name__ == '__main__':
    main()
