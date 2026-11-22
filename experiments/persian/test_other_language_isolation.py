"""Compare representative non-Persian audio against an independent baseline."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, EciIndex, SAMPLE_RATE, ORIGINAL_ACOUSTIC_CLOCK, GENDER

TEXTS = {
    0x10000: 'The quick brown fox speaks clearly.',
    0x10001: 'A clear voice for every reader.',
    0x20000: 'Hola, buenos días a todos.',
    0x20001: 'Hola, buenos días a todos.',
    0x30000: 'Bonjour, comment allez-vous ?',
    0x30001: 'Bonjour, comment allez-vous ?',
    0x40000: 'Guten Morgen, hier ist ein Beispielsatz.',
    0x50000: 'Buongiorno, come stai?',
    0x60000: '你好，今天是星期一。',
    0x60001: '你好，今天是星期一。',
    0x70000: 'Olá, bom dia para todos.',
    0x80000: 'こんにちは、今日は月曜日です。',
    0x90000: 'Hyvää huomenta, tämä on esimerkkilause.',
    0xa0000: '안녕하세요, 오늘은 월요일입니다.',
    0x120000: 'Καλημέρα, σήμερα είναι Δευτέρα.',
    0x130000: 'Güenos días, cómo ye isto?',
}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--previous', required=True, type=Path)
    parser.add_argument('--candidate', required=True, type=Path)
    args = parser.parse_args()
    checks = 0
    for dialect, text in TEXTS.items():
        for sample in (1, 3):
            for clock, gender in ((0, 0), (1, 1)):
                renders = []
                for path in (args.previous, args.candidate):
                    voice = DirectEciEngine(path.resolve(), dialect=dialect,
                                            encoding='utf8', native_voice=True)
                    try:
                        for key, value in ((SAMPLE_RATE, sample),
                                           (ORIGINAL_ACOUSTIC_CLOCK, clock), (GENDER, gender)):
                            voice.set_parameter(key, value)
                        pcm, indices = bytearray(), []
                        assert voice.speak((EciIndex(13), text, EciIndex(14)), pcm.extend,
                                           lambda value: indices.append((value, len(pcm))))
                        assert pcm
                        renders.append((bytes(pcm), indices))
                    finally:
                        voice.close()
                assert renders[0] == renders[1], (hex(dialect), sample, clock, gender)
                checks += 1
    print(f'Passed {checks} old/new identical cases in {len(TEXTS)} other dialects.')


if __name__ == '__main__':
    main()
