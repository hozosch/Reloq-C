#!/usr/bin/env python3
"""Produce exactly the replaceable binaries for the existing NVDA driver."""
from pathlib import Path
import argparse
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'addon'))
from build_synth import merged_german_lexicon, lexicon_rows, assert_x64_pe


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--dll', type=Path, default=ROOT/'eci'/'eci.dll')
    ap.add_argument('--output', type=Path, default=ROOT/'dist'/'replacement-files')
    args = ap.parse_args()
    data = args.dll.read_bytes()
    assert_x64_pe(data)
    entries = merged_german_lexicon()
    count = len(lexicon_rows(entries))
    if count != 16225:
        raise SystemExit(f'Unexpected dictionary entry count: {count} (expected 16225)')
    args.output.mkdir(parents=True, exist_ok=True)
    (args.output/'eci.dll').write_bytes(data)
    (args.output/'german_apple_observed.rlex').write_bytes(entries)
    print(f'OK: {args.output} -- eci.dll ({len(data)} bytes), German lexicon ({count} entries)')

if __name__ == '__main__':
    main()
