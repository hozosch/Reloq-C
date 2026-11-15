"""Compare the isolated eSpeak API probe to the matching eSpeak executable."""
from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import re
import subprocess

from espeak_ng_probe import EspeakPhonemeProbe

ROOT = Path(__file__).resolve().parents[2]


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--data-parent', required=True, type=Path)
    parser.add_argument('--binary', required=True, type=Path)
    args = parser.parse_args()
    corpus = [json.loads(line) for line in
              (ROOT / 'experiments/persian/reference_espeak_1_51.jsonl').read_text(encoding='utf8').splitlines()]
    env = {**os.environ, 'ESPEAK_DATA_PATH': str(args.data_parent.resolve())}
    count = 0
    with EspeakPhonemeProbe(args.library, args.data_parent) as probe:
        version = probe.version
        banner = subprocess.check_output([str(args.binary.resolve()), '--version'], env=env, text=True)
        match = re.search(r'\b\d+\.\d+(?:\.\d+)?\b', banner)
        assert match and match.group(0) == version, ('library and executable versions differ', version, banner)
        for row in corpus:
            expected = subprocess.check_output(
                [str(args.binary.resolve()), '-v', row['voice'], '-q', '-x', row['text']],
                env=env, text=True).split()
            clauses = probe.translate(row['text'], voice=row['voice'])
            actual = ' '.join(c.phonemes for c in clauses).split()
            assert actual == expected, ('API differs from same-version CLI', row, actual, expected)
            count += 1
        text = 'سلام دنیا. ایران نقطه؟'
        separated = probe.translate(text, separator='|')
        assert len(separated) == 2
        assert "'A" in separated[0].phonemes and "'i:" in separated[1].phonemes
        assert separated[0].terminator != separated[1].terminator
        lookahead = probe.translate('سلام، دنیا؟')
        assert len(lookahead) == 2 and "sal'Am" in lookahead[0].phonemes and "d'onjA" in lookahead[1].phonemes
        assert probe.translate(' ') and not probe.translate(' ')[0].phonemes
        assert '(en)' in probe.translate('salam', voice='fa')[0].phonemes
        assert '(en)' not in probe.translate('salam', voice='fa-latn')[0].phonemes
        assert probe.translate('') == ()
        for invalid in ('NUL\0text',):
            try:
                probe.translate(invalid)
            except ValueError:
                pass
            else:
                raise AssertionError('NUL input not rejected')
        try:
            EspeakPhonemeProbe(args.library, args.data_parent)
        except RuntimeError:
            pass
        else:
            raise AssertionError('concurrent eSpeak instance allowed')
        count += 9
    try:
        probe.translate('سلام')
    except RuntimeError:
        pass
    else:
        raise AssertionError('closed probe still usable')
    print(f'Passed {count + 1} eSpeak API, voice, clause and UTF-8 checks; version {version}.')


if __name__ == '__main__':
    main()
