"""Generate the eSpeak NG reference clips for the Persian listening probe."""
from __future__ import annotations

import argparse
from pathlib import Path
import subprocess

WORDS = {
    'fa_iran': ('fa', 'ایران'),
    'fa_stop_sequence': ('fa', 'پا تا کار'),
    'fa_ta': ('fa', 'تا'),
    'fa_noqte': ('fa', 'نقطه'),
    'fa_qalam': ('fa', 'قلم'),
    'ar_qalam': ('ar', 'قَلَم'),
    'ar_noqta': ('ar', 'نُقْطَة'),
    'fa_statement': ('fa', 'سلام دنیا.'),
    'fa_question': ('fa', 'سلام دنیا؟'),
}

KLATT_WORDS = {
    'fa_klatt_iran': ('fa+klatt', 'ایران'),
    'fa_klatt2_iran': ('fa+klatt2', 'ایران'),
    'sv_jag': ('sv', 'jag'),
    'sv_klatt_jag': ('sv+klatt', 'jag'),
    'fa_klatt_statement': ('fa+klatt', 'سلام دنیا.'),
    'fa_klatt_qalam': ('fa+klatt', 'قلم'),
}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--espeak', default='espeak-ng', help='eSpeak NG executable')
    parser.add_argument('--output', required=True, type=Path)
    parser.add_argument('--include-klatt', action='store_true',
                        help='also capture Klatt and Swedish comparison voices')
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    version = subprocess.check_output([args.espeak, '--version']).decode('utf8', errors='replace')
    (args.output / 'version.txt').write_text(version, encoding='utf8')
    output = []
    words = WORDS | (KLATT_WORDS if args.include_klatt else {})
    for name, (language, text) in words.items():
        phones = subprocess.check_output([args.espeak, '-v', language, '-q', '--ipa=3', text])
        output.append(f'{name}: {text}\n{phones.decode("utf8", errors="replace")}')
        subprocess.run([args.espeak, '-v', language, '-w', str(args.output / (name + '.wav')),
                        text], check=True)
    (args.output / 'phonemes.txt').write_text(''.join(output), encoding='utf8')
    print(f'Wrote {len(words)} eSpeak NG references to {args.output}')


if __name__ == '__main__':
    main()
