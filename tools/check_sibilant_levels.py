"""Check the intended native S-source changes against the previous build."""
from pathlib import Path
import argparse
import hashlib
import json
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, SAMPLE_RATE

TEXTS = {0x10001: 'Sister says seventy six.', 0x20001: 'Susana usa seis vasos.',
         0x30000: 'Ceci est aussi un son.', 0x30001: 'Ceci est aussi un son.',
         0x50000: 'Sassi, pasta, stazione.', 0x70000: 'Seis sapatos são seus.',
         0x90000: 'Susi saa sisua.', 0x60000: 'si4 si4 si4.', 0xa0000: '사시사사 쌍.'}


def capture(engine, text):
    digest = hashlib.sha256()
    samples = 0
    def pcm(data):
        nonlocal samples
        digest.update(data)
        samples += len(data) // 2
    assert engine.speak([text], pcm, None)
    return {'samples': samples, 'sha256': digest.hexdigest()}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--baseline', type=Path, required=True)
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    rows = []
    for language, text in TEXTS.items():
        engines = [DirectEciEngine(path.resolve(), dialect=language, encoding='utf8',
                                   native_voice=True) for path in (args.baseline, args.library)]
        try:
            changed = False
            for rate in range(7):
                for engine in engines:
                    engine.set_parameter(SAMPLE_RATE, rate)
                before, after = [capture(engine, text) for engine in engines]
                assert before['samples'] == after['samples']
                changed |= before != after
                assert capture(engines[0], 'mama.') == capture(engines[1], 'mama.'), (hex(language), rate)
                rows.append({'language': hex(language), 'rate': rate, 'text': text,
                             'before': before, 'after': after})
            assert changed, f'S level is unchanged for {language:#x}'
        finally:
            for engine in engines:
                engine.close()
    report = {'languages': 9, 's_comparisons': len(rows), 'unchanged_vowel_controls': len(rows),
              'no_audio_files': True, 'records': rows}
    if args.output:
        args.output.write_text(json.dumps(report, ensure_ascii=False, indent=2) + '\n')
    print('Native S balance: nine languages changed; 63 durations and vowel-only PCM controls preserved')


if __name__ == '__main__':
    main()
