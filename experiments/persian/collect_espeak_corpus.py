"""Regenerate the reference corpus using an installed eSpeak NG executable."""
from __future__ import annotations

import argparse
import json
from pathlib import Path
import subprocess

HERE = Path(__file__).resolve().parent


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--espeak', default='espeak-ng')
    parser.add_argument('--cases', type=Path, default=HERE / 'reference_espeak_1_51.jsonl')
    parser.add_argument('--output', required=True, type=Path)
    args = parser.parse_args()
    version = subprocess.check_output([args.espeak, '--version'], text=True).strip()
    cases = [json.loads(line) for line in args.cases.read_text(encoding='utf8').splitlines()]
    lines = []
    for case in cases:
        voice, text = case['voice'], case['text']
        phonemes = subprocess.check_output([args.espeak, '-v', voice, '-q', '-x', text], text=True).strip()
        ipa = subprocess.check_output([args.espeak, '-v', voice, '-q', '--ipa=3', text], text=True).strip()
        lines.append(json.dumps({'voice': voice, 'text': text, 'phonemes': phonemes, 'ipa': ipa}, ensure_ascii=False))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text('\n'.join(lines) + '\n', encoding='utf8')
    print(f'Wrote {len(lines)} references to {args.output}; {version}')


if __name__ == '__main__':
    main()
