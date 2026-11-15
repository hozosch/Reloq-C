"""Check direct-first phrase analysis and isolated whole-text fallback."""
import argparse
from pathlib import Path

from espeak_ng_probe import EspeakPhonemeProbe
from hybrid_fa_frontend import DirectFirstPersian
from text_frontend import translate_text


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--native-library', type=Path, required=True)
    parser.add_argument('--worker', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    args = parser.parse_args()
    direct = ('سلام دنیا؟', 'او آمد.', 'ايران نقطه.', 'ﺳﻼﻡ دنیا؟')
    fallback = ('من ایران را دوست دارم.', 'آیا NVDA خوب است؟', 'ده ۱۲ مهر')
    with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as reference:
        expected = {text: translate_text(reference, text) for text in (*direct, *fallback)}
        pinglish = translate_text(reference, 'salam donya?', pinglish=True)
    with DirectFirstPersian(args.native_library, args.worker, args.espeak_library, args.data_parent) as frontend:
        for text in direct:
            result = frontend.analyze(text)
            assert result.parts == expected[text] and not result.used_fallback
            assert result.source_map is not None and frontend._worker is None
        for text in fallback:
            result = frontend.analyze(text)
            assert result.parts == expected[text] and result.used_fallback and result.source_map is None
        assert frontend.analyze('salam donya?', pinglish=True).parts == pinglish
        frontend.cancel()
        assert frontend.analyze(fallback[0]).parts == expected[fallback[0]]
        assert frontend.analyze(direct[0]).parts == expected[direct[0]]
    try:
        frontend.analyze('سلام')
        raise AssertionError('closed frontend accepted text')
    except RuntimeError:
        pass
    print('Passed direct-first analysis, whole-phrase fallback, Latin routing and cancellation/restart.')


if __name__ == '__main__':
    main()
