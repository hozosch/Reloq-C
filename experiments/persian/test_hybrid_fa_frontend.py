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
              'من ایران را دوست دارم.', 'کتاب‌ها آمد.', 'کتاب‌ها؟', 'او و آمد.',
              'ده ۱۲ مهر')
    native_marks = {
        'سلام،دنیا!': (SpeechPart('fa', '`[.0sA.1lam],`[.1do.0nja]!'),),
        'سلام؛ دنیا؟': (SpeechPart('fa', '`[.0sA.1lam]; `[.1do.0nja]?'),),
        'سلام  دنیا.': (SpeechPart('fa', '`[.0sA.1lam]  `[.1do.0nja].'),),
        'سلام دنیا...': (SpeechPart('fa', '`[.0sA.1lam] `[.1do.0nja]...'),),
        'او،آمد.': (SpeechPart('fa', "`[.1'u:'],`[.1'a:'.0mAd]."),),
        'Hello, NVDA?': (SpeechPart('en-us', 'Hello, NVDA?'),),
        '۳٫۱۴': (SpeechPart('fa', "`[.2se] `[.0noR.2t'e:'] `[.0jek] `[.1Ca.0har]"),),
    }
    fallback = ('آیا NVDA خوب است؟', '؟', 'ـ')
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
        for first, second in (('۱۲٬۳۴۵', '12,345'), ('۱٬۲۳۴٫۵', '1,234.5')):
            one, two = frontend.analyze(first), frontend.analyze(second)
            assert one.parts == two.parts and not one.used_fallback and not two.used_fallback
        for text, number in (('NVDA ۳٫۱۴', '۳٫۱۴'),
                             ('آیا NVDA ۱٬۲۳۴٫۵ خوب است؟', '۱٬۲۳۴٫۵'),
                             ('سلام ۱۲، NVDA ۳٫۱۴؟', '۳٫۱۴'), ('NVDA −۳', '−۳')):
            result = frontend.analyze(text)
            assert result.used_fallback and result.source_map is None, text
            assert any(part.language == 'en-us' and 'NVDA' in part.text for part in result.parts), text
            direct_number = frontend.analyze(number).parts[0].text
            assert any(direct_number in part.text for part in result.parts if part.language == 'fa'), text
        for text in ('سلام٫دنیا', '3,14', '۳٫', '۱۲٬۳۴', '۱۲-۳'):
            try:
                frontend.analyze(text)
                raise AssertionError('numeric separator was accepted as a clause comma')
            except UnsupportedPronunciation:
                pass
        for number in ('۳٫۱۴', '۱٬۲۳۴٫۵'):
            result = frontend.analyze('salam ' + number + '?', pinglish=True)
            expected_number = frontend.analyze(number).parts[0].text
            assert result.used_fallback and any(expected_number in p.text for p in result.parts)
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
