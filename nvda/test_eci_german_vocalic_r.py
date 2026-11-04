"""Check the German vocalic-r to /ts/ connection through the ECI API.

Inspect the natural, unindexed utterance, indices, models and neighboring
contexts. Measurements from the original ECI include both voiced energy and
the relative 1.4/1.6-kHz band energy. This signal check does not replace a
listening comparison of the vocalic-r colour. PCM remains in memory.
"""
import argparse
import array
import math
from pathlib import Path

from _reloq_eci_direct import (DirectEciEngine, EciIndex, GENDER, HEAD_SIZE,
    HIGH_RATE_PROFILE, NATIVE_SPEED, NATIVE_VOLUME, SAMPLE_RATE)


def capture(engine, parts):
    pcm = bytearray()
    indices = []
    assert engine.speak(parts, pcm.extend, lambda value: indices.append((value, len(pcm))))
    assert pcm and indices and indices[-1][1] == len(pcm)
    samples = array.array('h', pcm)
    assert any(abs(value) > 8 for value in samples)
    return bytes(pcm), indices


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--baseline', type=Path)
    args = parser.parse_args()
    paths = [args.library] + ([args.baseline] if args.baseline else [])
    engines = [DirectEciEngine(path.resolve(), dialect=0x40000,
                encoding='utf8', experimental_generic=True) for path in paths]
    checks = 0
    try:
        for engine in engines:
            for parameter, value in ((NATIVE_SPEED, 75), (NATIVE_VOLUME, 100),
                                     (HEAD_SIZE, 50), (HIGH_RATE_PROFILE, 3)):
                engine.set_parameter(parameter, value)
        # 20-ms window from the unindexed natural utterance. At 11.025 kHz
        # the old renderer's corresponding RMS was about 266, while the
        # original's was about 2771. It does not measure the colour on its own.
        engines[0].set_parameter(SAMPLE_RATE, 1)
        engines[0].set_parameter(GENDER, 0)
        natural, _ = capture(engines[0], ['Der Zahn.', EciIndex(1)])
        samples = array.array('h', natural)
        start = round(.115 * 11025)
        window = samples[start:start+220]
        rms = math.sqrt(sum(value*value for value in window)/len(window))
        assert rms > 500, ('vocalic r ended too early before word-initial /ts/', rms)
        checks += 1
        # In the measured original, all six 20-ms windows starting at
        # 90..115 ms have more energy at 1.6 than at 1.4 kHz. Allow two
        # mismatches for frame/pitch variation; actual colour needs listening.
        def band(window, hz):
            re=sum(value*math.cos(2*math.pi*hz*i/11025) for i,value in enumerate(window))
            im=sum(value*math.sin(2*math.pi*hz*i/11025) for i,value in enumerate(window))
            return math.hypot(re,im)
        matches=0
        for ms in range(90,116,5):
            start=round(ms*11025/1000);window=samples[start:start+220]
            matches += band(window,1600)>band(window,1400)
        assert matches>=4, ('vocalic r spectral profile', matches)
        checks += 1
        for rate in (0, 1, 3, 2, 4, 5, 6):
            for gender in (0, 1):
                for engine in engines:
                    engine.set_parameter(SAMPLE_RATE, rate)
                    engine.set_parameter(GENDER, gender)
                for left, right, affected in (
                    ('Der ', 'Zahn.', True), ('Der ', 'Zug.', True),
                    ('Vor ', 'Zahn.', True), ('Der ', 'Mann.', False),
                    ('Der, ', 'Zahn.', False), ('', 'Zehn.', False),
                    ('', 'Zwei.', False), ('', 'Katze.', False),
                    ('', 'Herz.', False), ('', 'kurz.', False)):
                    parts = [left, EciIndex(1), right, EciIndex(2)]
                    current = capture(engines[0], parts)
                    if len(engines) > 1:
                        before = capture(engines[1], parts)
                        assert len(current[0]) == len(before[0]) and current[1] == before[1], (
                            rate, gender, left+right, 'duration/index changed')
                        assert (current[0] != before[0]) == affected, (
                            rate, gender, left+right, 'unexpected source change')
                    checks += 1
    finally:
        for engine in engines:
            engine.close()
    print(f'German vocalic r: {checks} energy, spectral, context, model, rate and index checks; listening still needed')


if __name__ == '__main__':
    main()
