"""Regressions for sentence boundaries, Touch and production loudness."""
import argparse
from array import array
from math import sqrt
from pathlib import Path
import tempfile

from _reloq_eci_direct import (DirectEciEngine, EciIndex, GENDER,
                               NATIVE_SPEED, NATIVE_VOLUME, PROCESS_TAGS, SAMPLE_RATE)
from test_eci_german_clock_native import phonemes, render
from test_eci_german_overlay_case import lexicon


def samples(data):
    result = array('h')
    result.frombytes(data)
    return result


def rms(pcm):
    return sqrt(sum(x*x for x in pcm) / len(pcm))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--baseline', type=Path)
    args = parser.parse_args()
    library = args.library.resolve()
    english = DirectEciEngine(library, dialect=0x10000, encoding='utf8', native_voice=True)
    german = DirectEciEngine(library, dialect=0x40000, encoding='utf8', native_voice=True)
    old = (DirectEciEngine(args.baseline.resolve(), dialect=0x10000, encoding='utf8', native_voice=True)
           if args.baseline else None)
    try:
        for e in (english, german, old):
            if e:e.set_parameter(PROCESS_TAGS, 1)
        for letter in ('A', 'B', 'C', 'X', 'Z'):
            for following in ('NVDA works.', 'The next sentence starts here.', 'This works.'):
                source = f'The program uses {letter}. {following}'
                separated = source.replace(f'{letter}. ', f'{letter}.  ', 1)
                assert phonemes(english, source) == phonemes(english, separated), source
                assert render(english, [source]) == render(english, [separated]), source
        text = 'The program is written in C. NVDA works.'
        whole = render(english, [text, EciIndex(7)])
        for cut in range(len(text)+1):
            assert render(english, [text[:cut], text[cut:], EciIndex(7)]) == whole, cut
        # Issue 22: a single-letter name before a full stop must close its
        # sentence before the next one, including the reported README text.
        reported = ('The shared synthesis engine is written in C. NVDA, '
                    'Android ARM64 and Windows SAPI5 use the same core.')
        joined = reported.replace('C. NVDA', 'C NVDA')
        assert '[.1si].' in phonemes(english, reported)
        assert len(render(english, [reported])[0]) > len(render(english, [joined])[0]) + 2205
        if old:
            for text in ('A. B. C. hello', 'Dr. Smith is here.', 'I saw J. Smith.',
                         'Version 1.2. NVDA works.', 'U.S. policy is important.',
                         'The site is example.com. We read it.'):
                assert phonemes(english, text) == phonemes(old, text), text
        for text in ('Touch', 'touch', 'TOUCH', 'Das ist Touch.'):
            assert '[.1tAC]' in phonemes(german, text), (text, phonemes(german, text))
        for text in ('Tatsch', '`[.1taC]'):
            assert '[.1taC]' in phonemes(german, text), text
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / 'custom.rlex'
            lexicon(path, (('Touch', '`[.1tEst]'),))
            custom = DirectEciEngine(library, dialect=0x40000, encoding='utf8',
                                     native_voice=True, lexicon_path=path)
            try:
                custom.set_parameter(PROCESS_TAGS, 1)
                assert '[.1tEst]' in phonemes(custom, 'Touch')
                assert '[.1tAC]' in phonemes(custom, 'touch')
            finally:
                custom.close()
        if old:
            for e in (english, old):
                e.set_parameter(SAMPLE_RATE, 1)
                e.set_parameter(NATIVE_SPEED, 80)
                e.set_parameter(NATIVE_VOLUME, 100)
            phrase = 'The shared engine is written in a programming language.'
            before = rms(samples(render(old, [phrase])[0]))
            after = rms(samples(render(english, [phrase])[0]))
            assert 1.25 < after / before < 1.40, (before, after)
        for e, phrase in ((english, 'The shared engine is written in C. NVDA works.'),
                          (german, 'Der schnelle Fuchs springt über einen Zaun.')):
            for rate in range(7):
                e.set_parameter(SAMPLE_RATE, rate)
                for gender in (0, 1):
                    e.set_parameter(GENDER, gender)
                    loudness=[]
                    for level in (0, 50, 75, 100):
                        e.set_parameter(NATIVE_VOLUME, level)
                        pcm=samples(render(e, [phrase])[0])
                        assert pcm and max(abs(x) for x in pcm) < 32767, (rate, gender, level)
                        loudness.append(rms(pcm))
                    assert all(a < b for a,b in zip(loudness,loudness[1:])), (rate, gender, loudness)
    finally:
        english.close();german.close()
        if old:old.close()
    print('1.1.4 sentence boundaries, Touch, dictionary priority and volume passed')


if __name__ == '__main__':
    main()
