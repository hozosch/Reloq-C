"""Compare mixed speech from the shared core with the accepted NVDA probe."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import EciIndex, GENDER, SAMPLE_RATE
from nvda_driver_probe import make_language_engine


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('library', 'reference', 'espeak-library', 'data-parent'):
        parser.add_argument('--' + name, type=Path, required=True)
    parser.add_argument('--reference-text', type=Path,
                        help='optional separate direct-text library for an older probe')
    args = parser.parse_args()
    engines = [make_language_engine(args.library, args.library, None,
                                    args.espeak_library, args.data_parent),
               make_language_engine(args.reference, args.reference_text or args.reference, None,
                                    args.espeak_library, args.data_parent)]
    sentences = ('NVDA خوب است.', 'NVDA شروع', 'سلام window خوب است.',
                 'نشان دادَنِ گزینه‌های خروج هنگامِ خارج شدن از NVDA',
                 'نمایشِ اِتِّفاقاتِ ثبت‌شده (Log)', 'NVDA پژوهشگر سلام',
                 'سلام-NVDA-دنیا', 'او آمد و NVDA خوب است.')
    def render(engine, text):
        split = len(text) // 2
        pcm, marks = bytearray(), []
        assert engine.speak((EciIndex(1), text[:split], EciIndex(2), text[split:], EciIndex(3)),
                            pcm.extend, lambda value: marks.append((value, len(pcm))))
        return bytes(pcm), marks
    count = 0
    try:
        for rate in (1, 3, 6):
            for gender in (0, 1):
                for engine in engines:
                    engine.set_parameter(SAMPLE_RATE, rate)
                    engine.set_parameter(GENDER, gender)
                for text in sentences:
                    first, second = (render(engine, text) for engine in engines)
                    assert first == second, (text, rate, gender, len(first[0]), len(second[0]),
                                             first[1], second[1])
                    count += 1
    finally:
        for engine in engines:
            engine.close()
    print(f'Passed {count} mixed-sentence PCM and source-index comparisons.')


if __name__ == '__main__':
    main()
