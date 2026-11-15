"""Check direct-first analysis and native punctuation/language routing."""
import argparse
from pathlib import Path

from espeak_ng_probe import EspeakPhonemeProbe
from hybrid_fa_frontend import DirectFirstPersian
from text_frontend import SpeechPart, UnsupportedPronunciation, translate_text


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--native-library', type=Path, required=True)
    parser.add_argument('--worker', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    args = parser.parse_args()
    direct = ('سلام دنیا؟', 'او آمد.', 'ايران نقطه.', 'ﺳﻼﻡ دنیا؟',
              'من ایران را دوست دارم.', 'کتاب‌ها آمد.', 'کتاب‌ها؟', 'او و آمد.')
    native_marks = {
        'سلام،دنیا!': (SpeechPart('fa', '`[.0sA.1lam],`[.1do.0nja]!'),),
        'سلام؛ دنیا؟': (SpeechPart('fa', '`[.0sA.1lam]; `[.1do.0nja]?'),),
        'سلام  دنیا.': (SpeechPart('fa', '`[.0sA.1lam]  `[.1do.0nja].'),),
        'سلام دنیا...': (SpeechPart('fa', '`[.0sA.1lam] `[.1do.0nja]...'),),
        'او،آمد.': (SpeechPart('fa', "`[.1'u:'],`[.1'a:'.0mAd]."),),
        'Hello, NVDA?': (SpeechPart('en-us', 'Hello, NVDA?'),),
    }
    fallback = ('آیا NVDA خوب است؟', 'ده ۱۲ مهر', '؟', 'ـ')
    mixed = {
        'NVDA،سلام دنیا؟': (SpeechPart('en-us', 'NVDA,'),
                            SpeechPart('fa', '`[.0sA.1lam] `[.1do.0nja]?')),
        'سلام،NVDA!': (SpeechPart('fa', '`[.0sA.1lam],'), SpeechPart('en-us', 'NVDA!')),
    }
    with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as reference:
        expected = {text: translate_text(reference, text) for text in (*direct, *fallback)}
        pinglish = translate_text(reference, 'salam donya?', pinglish=True)
    with DirectFirstPersian(args.native_library, args.worker, args.espeak_library, args.data_parent) as frontend:
        for text in direct:
            result = frontend.analyze(text)
            assert result.parts == expected[text] and not result.used_fallback
            assert result.source_map is not None and frontend._worker is None
        for text, expected_parts in native_marks.items():
            result = frontend.analyze(text)
            assert result.parts == expected_parts and not result.used_fallback, (text, result)
            assert result.source_map is not None and frontend._worker is None
        for text in fallback:
            result = frontend.analyze(text)
            assert result.parts == expected[text] and result.used_fallback and result.source_map is None
        for text, expected_parts in mixed.items():
            result = frontend.analyze(text)
            assert result.parts == expected_parts and result.used_fallback and result.source_map is None
        for text in ('۳٫۱۴', '۱۲٬۳۴۵', 'سلام٫دنیا'):
            try:
                frontend.analyze(text)
                raise AssertionError('numeric separator was accepted as a clause comma')
            except UnsupportedPronunciation:
                pass
        assert frontend.analyze('salam donya?', pinglish=True).parts == pinglish
        frontend.cancel()
        assert frontend.analyze(fallback[0]).parts == expected[fallback[0]]
        assert frontend.analyze(direct[0]).parts == expected[direct[0]]
    try:
        frontend.analyze('سلام')
        raise AssertionError('closed frontend accepted text')
    except RuntimeError:
        pass
    print('Passed direct and fallback punctuation routing, English routing, context and cancellation/restart.')


if __name__ == '__main__':
    main()
