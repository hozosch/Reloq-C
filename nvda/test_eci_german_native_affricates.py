"""Word-initial /ts/ and /tsv/ in German's shared native path.

Compare with the same native rules before the male AF correction. Speech,
public index positions and energy measurements stay in memory.
"""
import argparse
from pathlib import Path

from _reloq_eci_direct import (DirectEciEngine, EciIndex, SAMPLE_RATE,
    SAMPLE_RATES, NATIVE_SPEED, GENDER, HEAD_SIZE, PROCESS_TAGS)


def capture(engine, text):
    pcm = bytearray()
    indices = []
    assert engine.speak([text, EciIndex(17)], pcm.extend,
                        lambda n: indices.append((n, len(pcm))))
    assert pcm and indices == [(17, len(pcm))]
    return bytes(pcm), indices


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--library', type=Path, required=True)
    p.add_argument('--baseline', type=Path, required=True)
    a = p.parse_args()
    engines = [DirectEciEngine(path.resolve(), dialect=0x40000, encoding='utf8',
                               experimental_generic=True)
               for path in (a.baseline, a.library)]
    onset = ('Zehn.', 'Zwei.', 'Zahn.', 'Zimmer.', 'zwanzig.', 'Der Zahn.',
             'Das ist Zeit.', 'dazwischen.')
    controls = ('Katze.', 'setzen.', 'Mütze.', 'kurz.', 'Herz.',
                'Susi sieht sieben Segel.', 'Das war richtig.')
    checks = 0
    try:
        for rate in SAMPLE_RATES:
            for gender, head in ((0, 50), (1, 29), (1, 50)):
                for engine in engines:
                    for param, value in ((SAMPLE_RATE, rate), (GENDER, gender),
                                         (HEAD_SIZE, head), (NATIVE_SPEED, 75),
                                         (PROCESS_TAGS, 1)):
                        engine.set_parameter(param, value)
                for text in onset + controls:
                    old, new = (capture(engine, text) for engine in engines)
                    assert len(old[0]) == len(new[0]) and old[1] == new[1], (rate, gender, text, 'timing')
                    if gender or text in controls:
                        assert old == new, (rate, gender, text, 'unrelated or female PCM')
                    else:
                        assert old != new, (rate, gender, text, 'male correction missing')
                    checks += 1
                # A word-scoped gender change must choose the model at that
                # word, including when the request began with the other model.
                for tag in ('vg0', 'vg1'):
                    text = 'Bitte `' + tag + ' Zehn.'
                    old, new = (capture(engine, text) for engine in engines)
                    assert len(old[0]) == len(new[0]) and old[1] == new[1]
                    assert (old == new) == (tag == 'vg1'), (rate, gender, tag)
                    checks += 1
    finally:
        for engine in engines:
            engine.close()
    print(f'German native Z: {checks} PCM, gender, inline-tag, index and timing comparisons across seven rates')


if __name__ == '__main__':
    main()
