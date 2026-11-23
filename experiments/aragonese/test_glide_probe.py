"""Check private AU formant controls against earlier probes.

No production provider is edited. Environment controls are process-wide; all
calls here are serial. Context scanning is validated by sound comparisons.
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

PREFIX = 'RELOQ_ARAGON_PROBE_'


def set_controls(**values):
    for suffix, value in values.items():
        os.environ[PREFIX + suffix] = str(value)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline', type=Path, required=True)
    parser.add_argument('--previous', type=Path, required=True)
    parser.add_argument('--glide', type=Path, required=True)
    args = parser.parse_args()
    engines = [DirectEciEngine(path.resolve(), dialect=0x20000, encoding='utf8', native_voice=True)
               for path in (args.baseline,args.previous,args.glide)]
    base, old, candidate = engines
    saved = {key: value for key,value in os.environ.items() if key.startswith(PREFIX)}
    checks = 0
    medial = ('auca', 'pauto', 'auto')
    final = ('cudiau', 'costau', 'cudiau y casa')
    controls = ('puent', 'bueno', 'euforia', 'cuna', 'hola', 'casa', 'a hueso')
    try:
        for key in saved: del os.environ[key]
        for engine in engines: engine.set_parameter(PROCESS_TAGS,1)
        for rate in (1,3,6):
            for speed in (40,80):
                for gender in (0,1):
                    for engine in engines:
                        engine.set_parameter(SAMPLE_RATE,rate)
                        engine.set_parameter(NATIVE_SPEED,speed)
                        engine.set_parameter(GENDER,gender)
                    set_controls(SH_PERCENT=65,AU_PERCENT=40,GLIDE_SCOPE=0)
                    for word in (*medial,*final,*controls,'`[.1Sa.0da]','`[.1Ca.0da]','`[.1Ta.0da]'):
                        assert render(candidate,[word])[0] == render(old,[word])[0], (rate,speed,gender,word,'old reference')
                        checks += 1
                    set_controls(GLIDE_SCOPE=1, AU_MEDIAL_F1=40, AU_MEDIAL_F2=40,
                                 AU_FINAL_F1=0, AU_FINAL_F2=0,
                                 AU_MEDIAL_DURATION=100, AU_FINAL_DURATION=100)
                    for word in medial:
                        audio = render(candidate,[word])[0]
                        assert audio != render(base,[word])[0], (rate,speed,gender,word,'medial must change')
                        assert audio == render(old,[word])[0], (rate,speed,gender,word,'medial previous setting')
                        checks += 1
                    for word in (*final,*controls):
                        assert render(candidate,[word])[0] == render(base,[word])[0], (rate,speed,gender,word,'final/control must not change')
                        checks += 1
                    set_controls(AU_MEDIAL_F1=0, AU_MEDIAL_F2=0, AU_FINAL_F1=40, AU_FINAL_F2=40)
                    for word in final:
                        audio = render(candidate,[word])[0]
                        assert audio != render(base,[word])[0], (rate,speed,gender,word,'final must change')
                        assert audio == render(old,[word])[0], (rate,speed,gender,word,'final previous setting')
                        checks += 1
                    for word in (*medial,*controls):
                        assert render(candidate,[word])[0] == render(base,[word])[0], (rate,speed,gender,word,'medial/control must not change')
                        checks += 1
                    set_controls(AU_MEDIAL_F1=0, AU_MEDIAL_F2=40, AU_FINAL_F1=0, AU_FINAL_F2=65)
                    for word in (*medial,*final):
                        assert candidate._phonemes_for_probe(word) == base._phonemes_for_probe(word), (rate,speed,gender,word,'phones')
                        normal = render(candidate,[word])[0]
                        set_controls(AU_MEDIAL_F1=20, AU_FINAL_F1=20, AU_FINAL_F2=80)
                        alternate = render(candidate,[word])[0]
                        assert len(alternate) == len(normal) and alternate != normal, (rate,speed,gender,word,'formant control')
                        set_controls(AU_MEDIAL_F1=0, AU_FINAL_F1=0, AU_FINAL_F2=65)
                        checks += 1
                    for word in controls:
                        assert render(candidate,[word])[0] == render(base,[word])[0], (rate,speed,gender,word,'other glide')
                        checks += 1
    finally:
        for engine in engines: engine.close()
        for key in list(os.environ):
            if key.startswith(PREFIX): del os.environ[key]
        os.environ.update(saved)
    print(f'Passed {checks} AU position, formant, reference and control comparisons')


if __name__ == '__main__':
    main()
