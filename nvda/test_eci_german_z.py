"""German affricates: real PCM, end indices and unchanged timing, in RAM."""
from pathlib import Path
import argparse
import array
import math
from _reloq_eci_direct import (DirectEciEngine, EciIndex, SAMPLE_RATE,
    SAMPLE_RATES, NATIVE_SPEED, GENDER, HEAD_SIZE, HIGH_RATE_PROFILE)


def capture(engine, text):
    data = bytearray()
    indices = []
    assert engine.speak([text, EciIndex(23)], data.extend,
                        lambda n: indices.append((n, len(data))))
    assert data and indices == [(23, len(data))]
    samples = array.array('h', data)
    assert any(abs(n) > 8 for n in samples), (text, 'silent')
    return (bytes(data), indices, max(map(abs, samples)),
            sum(n in (-32768, 32767) for n in samples))


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--library', type=Path, required=True)
    p.add_argument('--baseline', type=Path, required=True)
    a = p.parse_args()
    cases = ('Zehn.', 'Zwei.', 'Zahl.', 'zu.', 'Zunge.', 'Zitronen.',
             'zwanzig.', 'Zwerge.', 'Zimmer.', 'ziehen.',
             'Ausführen Zehn.', 'Ausführen Zwei.', 'gut Zwei.', 'Platz Zehn.')
    controls = ('Hallo Welt.', 'Susi sieht sieben Segel.', 'Farbe und Papier.',
                'Katze.', 'setzen.', 'Mütze.', 'Hitze.', 'kurz.', 'Platz.')
    engines = [DirectEciEngine(f.resolve(), dialect=0x40000,
                              encoding='utf8', native_voice=True)
               for f in (a.baseline, a.library)]
    checks = 0
    level_checks = 0
    female_checks = 0
    try:
        for rate in SAMPLE_RATES:
            for female, head in ((0, 50), (1, 29), (1, 50)):
                for e in engines:
                    for parameter, value in ((SAMPLE_RATE, rate), (GENDER, female),
                            (HEAD_SIZE, head), (NATIVE_SPEED, 75),
                            (HIGH_RATE_PROFILE, 0)):
                        e.set_parameter(parameter, value)
                for text in cases:
                    before, after = (capture(e, text) for e in engines)
                    assert len(before[0]) == len(after[0]) and before[1] == after[1], (rate, female, text, 'timing')
                    # The accepted female/head-50 baseline already reaches
                    # the PCM rails at 8 kHz. Require exact identity below,
                    # and forbid any additional peak/rail regression here.
                    assert after[2] <= max(32759, before[2]), (rate, female, head, text, 'peak regression')
                    assert after[3] <= before[3], (rate, female, head, text, 'new rail samples')
                    checks += 1
                    if female:
                        assert before == after, (rate, head, text, 'accepted female Z changed')
                        female_checks += 1
                    elif rate == 1 and text in cases[:10]:
                        # At the unchanged 11-kHz clock, the initial /ts/
                        # isolates the level adjustment from the new colour.
                        n = int(SAMPLE_RATES[rate]*.025)
                        old = array.array('h', before[0])[:n]
                        new = array.array('h', after[0])[:n]
                        delta = 10*math.log10(sum(x*x for x in new)/sum(x*x for x in old))
                        assert -2.1 < delta < -1.9, (female, text, 'Z level', delta)
                        level_checks += 1
                for text in controls:
                    assert capture(engines[0], text) == capture(engines[1], text), (rate, female, text, 'unrelated speech changed')
                    checks += 1
    finally:
        for e in engines:
            e.close()
    print(f'German Z: {checks} PCM/index/timing/peak/control comparisons, {level_checks} male onset-level checks and {female_checks} unchanged female cases, all seven rates; no audio files')


if __name__ == '__main__':
    main()
