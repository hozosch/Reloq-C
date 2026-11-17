"""Render the old and automatic Korean-derived Persian contours for review."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys
from unittest.mock import patch
import wave

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, GENDER, NATIVE_INFLECTION, NATIVE_SPEED, SAMPLE_RATE
from _reloq_persian_frames import FrameCapture
from compare_korean_long import SENTENCES
from nvda_driver_probe import make_language_engine


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for option in ('library', 'espeak-library', 'data-parent', 'output'):
        parser.add_argument('--' + option, required=True, type=Path)
    parser.add_argument('--speed', type=int, default=65)
    parser.add_argument('--inflection', type=int, default=47)
    args = parser.parse_args()
    if not 0 <= args.speed <= 250 or not 0 <= args.inflection <= 100:
        parser.error('invalid speed or inflection')
    persian = make_language_engine(args.library, args.library, None, args.espeak_library, args.data_parent)
    english = DirectEciEngine(args.library, dialect=0x10000, encoding='utf8', native_voice=True)
    for voice in (persian, english):
        for parameter, value in ((SAMPLE_RATE, 3), (NATIVE_SPEED, args.speed),
                                 (NATIVE_INFLECTION, args.inflection), (GENDER, 0)):
            voice.set_parameter(parameter, value)
    original_init = FrameCapture.__init__

    def without_korean(instance, dll):
        original_init(instance, dll)
        instance.transfer_korean_pitch = None

    def render(voice, text):
        pcm = bytearray()
        if not voice.speak((text,), pcm.extend, lambda _: None) or not pcm:
            raise RuntimeError('empty comparison audio')
        return bytes(pcm)

    audio = bytearray()
    def append(value):
        audio.extend(value)
        audio.extend(bytes(12800))
    def label(text):
        append(render(english, text))

    try:
        passage = ' '.join(text for text, _ in SENTENCES)
        mixed = 'NVDA سلام دنیا. This is a test. امروز می‌خواهم درباره یک کتاب صحبت کنم.'
        label('Persian intonation comparison. Inflection is forty seven. '
              'First the previous contour, then the automatic Korean derived contour.')
        for title, text in (('Long passage.', passage), ('English and Persian switches.', mixed)):
            label(title)
            label('Previous contour.')
            with patch.object(FrameCapture, '__init__', without_korean):
                previous = render(persian, text)
            append(previous)
            label('Automatic Korean derived contour.')
            current = render(persian, text)
            append(current)
            if previous == current:
                raise RuntimeError('the Korean-derived comparison did not change')
        label('End of comparison.')
    finally:
        persian.close()
        english.close()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(args.output), 'wb') as output:
        output.setnchannels(1)
        output.setsampwidth(2)
        output.setframerate(16000)
        output.writeframes(audio)
    print(f'Wrote {args.output} ({len(audio) / 32000:.1f} seconds at 16 kHz).')


if __name__ == '__main__':
    main()
