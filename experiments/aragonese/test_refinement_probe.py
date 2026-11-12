"""Check variable, private /sh/ and AU listening probes against references.

The environment controls are present only in the explicitly supplied
experimental library. Tests are serial because the controls are process-wide.
"""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, GENDER, NATIVE_SPEED, PROCESS_TAGS, SAMPLE_RATE
from test_eci_german_clock_native import render

SH = 'RELOQ_ARAGON_PROBE_SH_PERCENT'
AU = 'RELOQ_ARAGON_PROBE_AU_PERCENT'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline', type=Path, required=True)
    parser.add_argument('--previous-sh', type=Path, required=True)
    parser.add_argument('--previous-au', type=Path, required=True)
    parser.add_argument('--refined', type=Path, required=True)
    args = parser.parse_args()
    paths = (args.baseline, args.previous_sh, args.previous_au, args.refined)
    engines = [DirectEciEngine(path.resolve(), dialect=0x20000, encoding='utf8', native_voice=True)
               for path in paths]
    base, previous_sh, previous_au, candidate = engines
    saved = {key: os.environ.get(key) for key in (SH, AU)}
    checks = 0
    try:
        for engine in engines:
            engine.set_parameter(PROCESS_TAGS, 1)
        for rate in (1, 3, 6):
            for speed in (40, 80):
                for gender in (0, 1):
                    for engine in engines:
                        engine.set_parameter(SAMPLE_RATE, rate)
                        engine.set_parameter(NATIVE_SPEED, speed)
                        engine.set_parameter(GENDER, gender)
                    os.environ[AU] = '0'
                    for text in ('`[.1Sa.0da]', '`[.1Swe.0la]', '`[.0a.1Si.0na]', '`[.1peS]'):
                        durations = []
                        for percent in (80, 65, 50):
                            os.environ[SH] = str(percent)
                            assert text in candidate._phonemes_for_probe(text), (rate, speed, gender, text)
                            audio = render(candidate, [text])[0]
                            assert audio, (rate, speed, gender, text)
                            if percent == 80:
                                assert audio == render(previous_sh, [text])[0], (rate, speed, gender, text)
                            durations.append(len(audio))
                            checks += 1
                        assert durations[0] > durations[1] > durations[2], (rate, speed, gender, text, durations)
                    # A change to S must not change the Castilian interdental,
                    # the tested t-sh affricates, or ordinary Spanish examples.
                    for percent in (80, 65, 50):
                        os.environ[SH] = str(percent)
                        for text in ('`[.1Ta.0da]', '`[.1Ca.0da]', '`[.1a.0Ca]', '`[.1aC]', 'hola', 'casa'):
                            assert render(candidate, [text])[0] == render(base, [text])[0], (rate, speed, gender, text)
                            checks += 1
                    os.environ[SH] = '80'
                    for word in ('auca', 'pauto', 'cudiau', 'auto', 'auca y pauto'):
                        phones = base._phonemes_for_probe(word)
                        assert 'aw' in phones, (rate, speed, gender, word, phones)
                        before = render(base, [word])[0]
                        alternatives = []
                        for percent in (0, 40, 65, 100):
                            os.environ[AU] = str(percent)
                            assert candidate._phonemes_for_probe(word) == phones, (rate, speed, gender, word)
                            audio = render(candidate, [word])[0]
                            assert len(audio) == len(before), (rate, speed, gender, word)
                            if percent == 0:
                                assert audio == before, (rate, speed, gender, word)
                            elif percent == 100:
                                assert audio == render(previous_au, [word])[0], (rate, speed, gender, word)
                            alternatives.append(audio)
                            checks += 1
                        assert len(set(alternatives)) == len(alternatives), (rate, speed, gender, word)
                    for percent in (0, 40, 65, 100):
                        os.environ[AU] = str(percent)
                        for word in ('puent', 'hola', 'casa', 'mocet', 'cuna'):
                            assert render(candidate, [word])[0] == render(base, [word])[0], (rate, speed, gender, word)
                            checks += 1
    finally:
        for engine in engines:
            engine.close()
        for key, value in saved.items():
            if value is None:
                os.environ.pop(key, None)
            else:
                os.environ[key] = value
    print(f'Passed {checks} isolated rate, speed, voice and unchanged-reference comparisons')


if __name__ == '__main__':
    main()
