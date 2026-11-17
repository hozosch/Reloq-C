"""Compare dev131 and word-aligned Korean pitch on the same Persian voice."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys
import wave

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, GENDER, NATIVE_INFLECTION, NATIVE_PITCH, NATIVE_SPEED, SAMPLE_RATE
from nvda_driver_probe import make_language_engine


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for option in ('library', 'espeak-library', 'data-parent', 'output'):
        parser.add_argument('--' + option, required=True, type=Path)
    args = parser.parse_args()
    persian = make_language_engine(args.library, args.library, None, args.espeak_library, args.data_parent)
    english = DirectEciEngine(args.library, dialect=0x10000, encoding='utf8', native_voice=True)
    for voice in (persian, english):
        for parameter, value in ((SAMPLE_RATE, 3), (NATIVE_SPEED, 65), (NATIVE_PITCH, 65),
                                 (NATIVE_INFLECTION, 47), (GENDER, 0)):
            voice.set_parameter(parameter, value)
    api = persian._persian_korean_contour.api
    aligned = api.transfer_korean_aligned
    if aligned is None:
        raise RuntimeError('word-aligned Korean transfer is missing')
    audio = bytearray()

    def append(voice, text, *, pause=6400):
        pcm = bytearray()
        if not voice.speak((text,), pcm.extend, lambda _: None) or not pcm:
            raise RuntimeError('empty comparison audio')
        audio.extend(pcm)
        audio.extend(bytes(pause))
        return bytes(pcm)

    try:
        append(english, 'Persian intonation comparison. For each example: the previous version, then the new version.')
        cases = (
            ('Individual words.', ('سلام', 'کتاب', 'یک', 'دو', 'خوب')),
            ('A longer sentence.', ('امروز می‌خواهم درباره یک کتاب صحبت کنم.',)),
            ('A question.', ('شما هم کتاب خواندن را دوست دارید؟',)),
            ('English and Persian together.', ('NVDA سلام دنیا. This is a test. امروز کتاب می‌خوانم.',)),
        )
        for label, texts in cases:
            append(english, label)
            for title, implementation in (('Previous version.', None), ('New version.', aligned)):
                append(english, title)
                api.transfer_korean_aligned = implementation
                for text in texts:
                    append(persian, text)
        append(english, 'End of comparison.')
    finally:
        api.transfer_korean_aligned = aligned
        persian.close()
        english.close()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(args.output), 'wb') as output:
        output.setnchannels(1)
        output.setsampwidth(2)
        output.setframerate(16000)
        output.writeframes(audio)
    print('Wrote', args.output, 'seconds', round(len(audio) / 32000, 1))


if __name__ == '__main__':
    main()
