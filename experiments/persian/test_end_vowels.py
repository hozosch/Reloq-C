"""Check explicit long end-vowel comparisons and the preferred Iran timing."""
from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, GENDER, NATIVE_SPEED, PROCESS_TAGS, SAMPLE_RATE
from listening import render
from salam_samples import (IRAN_ESPEAK, IRAN_PREFERRED, NO_TE_LONG, NO_TE_SHORT,
                           SALAM, TA_LONG, TA_SHORT)
from test_selected_probe import KEYS, set_probe
from test_vowel_timing import capture


def nucleus(engine: DirectEciEngine, text: str, phone: int) -> int:
    _, rows = capture(engine, text, 1)
    matches = [row[5] for row in rows if row[0] == phone]
    assert len(matches) == 1, ('expected one vowel', text, rows)
    return matches[0]


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--previous', required=True, type=Path)
    parser.add_argument('--probe', required=True, type=Path)
    args = parser.parse_args()
    corpus = [json.loads(line) for line in
              (ROOT / 'experiments/persian/reference_espeak_1_51.jsonl').read_text(encoding='utf8').splitlines()]
    by_text = {row['text']: row['phonemes'] for row in corpus if row['voice'] == 'fa'}
    assert by_text['نقطه'].endswith('e:') and by_text['تا'].endswith('A:')
    assert by_text['ایران'].startswith("'i:")
    previous = DirectEciEngine(args.previous.resolve(), dialect=0x90000, encoding='utf8', native_voice=True)
    probe = DirectEciEngine(args.probe.resolve(), dialect=0x90000, encoding='utf8', native_voice=True)
    count = 0
    try:
        for e in (previous, probe):
            e.set_parameter(PROCESS_TAGS, 1)
            e.set_parameter(SAMPLE_RATE, 3)
        for gender in (0, 1):
            for speed in (40, 50, 98, 200):
                for e in (previous, probe):
                    e.set_parameter(GENDER, gender)
                    e.set_parameter(NATIVE_SPEED, speed)
                for text in (IRAN_ESPEAK, IRAN_PREFERRED, NO_TE_LONG, NO_TE_SHORT, TA_LONG, TA_SHORT):
                    assert text in probe._phonemes_for_probe(text), ('invalid tag', text)
                for short, long, phone in ((IRAN_ESPEAK, IRAN_PREFERRED, 34),
                                           (NO_TE_SHORT, NO_TE_LONG, 29),
                                           (TA_SHORT, TA_LONG, 34)):
                    assert nucleus(probe, long, phone) > nucleus(probe, short, phone), (
                        'explicit long vowel not longer', gender, speed, short, long)
                    count += 1
                # With S and the A-centre experiment disabled, the accepted
                # Salam, internal E and earlier Iran remain bit-identical.
                set_probe(a=1, q=5, x=1, h=5, l=4, rise=2, vowel=1)
                for text in (SALAM, '`[.1sefr]', IRAN_PREFERRED):
                    assert render(previous, text) == render(probe, text), ('prior sample changed', gender, speed, text)
                    count += 1
        print(f'Passed {count} Iran and final-vowel comparison checks.')
    finally:
        previous.close()
        probe.close()
        for key in (*KEYS, 'RELOQ_FA_TRACE_NUC'):
            os.environ.pop(key, None)


if __name__ == '__main__':
    main()
