"""Generate checked Persian letter-name tags from the pinned eSpeak data."""
from __future__ import annotations

import argparse
from pathlib import Path

from compile_direct_lexicon import EXPECTED_DICT, digest
from espeak_ng_probe import EspeakPhonemeProbe
from espeak_worker import EspeakWorker
from reference_letter_names import LETTERS, reference_tag
from test_selected_probe import engine, set_confirmed_persian_probe
from text_frontend import native_tag_accepted

HERE = Path(__file__).resolve().parent


def generate(worker, native) -> str:
    output = [
        '/* Generated from eSpeak NG Persian character names; GPL-3.0-or-later.\n',
        ' * See reference-espeak/persian and compile_direct_letters.py. */\n',
    ]
    for char in LETTERS:
        tag = reference_tag(worker, char)
        if not native_tag_accepted(tag, native._phonemes_for_probe(tag)):
            raise RuntimeError(f'native parser changed the name of U+{ord(char):04X}')
        output.append(f'    {{0x{ord(char):04x}u, "{tag}"}},\n')
    return ''.join(output)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--worker', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    parser.add_argument('--probe', type=Path, required=True)
    parser.add_argument('--output', type=Path, default=HERE / 'direct_fa_letters.inc')
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    if digest(args.data_parent / 'espeak-ng-data/fa_dict') != EXPECTED_DICT:
        raise RuntimeError('unexpected eSpeak Persian dictionary')
    with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as api:
        if api.version != '1.53.0':
            raise RuntimeError('letter names require eSpeak NG 1.53.0')
    set_confirmed_persian_probe()
    native = engine(args.probe, 0x90000)
    try:
        with EspeakWorker(args.worker, args.espeak_library, args.data_parent) as worker:
            value = generate(worker, native)
    finally:
        native.close()
    if args.check:
        if args.output.read_text(encoding='utf8') != value:
            raise RuntimeError('generated Persian letter names differ')
    else:
        args.output.write_text(value, encoding='utf8', newline='\n')
    print(f'Checked {len(LETTERS)} direct Persian letter names.')


if __name__ == '__main__':
    main()
