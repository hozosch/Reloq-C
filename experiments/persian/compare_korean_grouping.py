"""Compare Korean phrase grouping without fitting a reference pitch curve.

The translated Korean source, the previous word-by-word donor and the new
phrase-grouped donor all use Korean inflection 37 and the same Persian frame
capture. A final section recreates the earlier translated-Korean sample at 30.
The optional checks verify both live runtime and a previous raw PCM capture.
"""
from __future__ import annotations

import argparse
from pathlib import Path
import sys
import wave

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import GENDER, NATIVE_INFLECTION, NATIVE_PITCH, NATIVE_SPEED, SAMPLE_RATE
from compare_korean_long import SENTENCES, indexed_capture, replay, transfer
from compare_korean_sources import mapped_pitch, original_clip, without_boundary_silence
from direct_fa_sequence import EciIndex, project_resilient
from hybrid_fa_frontend import DirectFirstPersian
from korean_contour import _TAG, _SYLLABLE, _punctuation, proxy
from nvda_driver_probe import make_language_engine
from test_us_articulation import voice


def old_proxy(tagged):
    """The dev135 donor: preserve one Korean pseudo-word per Persian tag."""
    matches = tuple(_TAG.finditer(tagged))
    if not matches:
        return None
    parts = []
    for index, match in enumerate(matches):
        if index:
            gap = tagged[matches[index - 1].end():match.start()]
            parts.append(_punctuation(gap) + ' ')
        parts.append('가' * max(1, len(_SYLLABLE.findall(match.group()))))
    parts.append(_punctuation(tagged[matches[-1].end():]) or '.')
    return ''.join(parts)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('candidate', 'espeak-library', 'data-parent', 'output'):
        parser.add_argument('--' + name, required=True, type=Path)
    parser.add_argument('--original', type=Path, help='verify the earlier English listening sample')
    parser.add_argument('--old-pcm', type=Path, help='verify the earlier runtime PCM for this passage')
    parser.add_argument('--check-runtime', action='store_true', help='verify the new section against live runtime PCM')
    args = parser.parse_args()
    frontend = DirectFirstPersian(args.candidate, None, args.espeak_library, args.data_parent)
    fa, fa_scope = voice(args.candidate, 0x90000)
    ko, ko_scope = voice(args.candidate, 0xa0000)
    en, _ = voice(args.candidate, 0x10000)
    for engine in (fa, ko, en):
        for parameter, value in ((SAMPLE_RATE, 3), (NATIVE_SPEED, 65), (NATIVE_PITCH, 65), (GENDER, 0)):
            engine.set_parameter(parameter, value)
    recording = bytearray()

    def pause(seconds):
        recording.extend(bytes(round(16000 * seconds) * 2))

    def say(text):
        pcm = bytearray()
        if not en.speak((text,), pcm.extend, lambda _: None) or not pcm:
            raise RuntimeError('English label synthesis failed')
        recording.extend(pcm)
        pause(.45)

    def play(pcm):
        recording.extend(pcm)
        pause(.9)

    try:
        source = tuple(part for i, (text, _) in enumerate(SENTENCES, 1)
                       for part in (EciIndex(i), text + ' '))
        job = project_resilient(source, frontend)
        target = indexed_capture(fa, fa_scope, job.parts, True)
        tagged = [project_resilient((text,), frontend).tagged for text, _ in SENTENCES]
        donor_text = {'translated': [text for _, text in SENTENCES],
                      'previous': [old_proxy(text) for text in tagged],
                      'grouped': [proxy(text) for text in tagged]}
        ko.set_parameter(NATIVE_INFLECTION, 37)
        donors = {name: [indexed_capture(ko, ko_scope, (text,), False) for text in values]
                  for name, values in donor_text.items()}
        variants = {}
        for name in ('translated', 'previous', 'grouped'):
            values, _ = mapped_pitch(target, donors[name])
            frames, unchanged = target.frames(), target.frames()
            for index, value in values:
                frames[index].controls[1] = value
            if any(a.tick != b.tick or any(a.controls[i] != b.controls[i] for i in range(62) if i != 1)
                   for a, b in zip(frames, unchanged)):
                raise AssertionError('a non-pitch frame control changed')
            variants[name] = replay(fa, fa_scope, frames)
            if len(variants[name]) != len(target.pcm):
                raise AssertionError('Persian timing changed')
        ko.set_parameter(NATIVE_INFLECTION, 30)
        original_donors = [indexed_capture(ko, ko_scope, (text,), False)
                           for text in donor_text['translated']]
        original = replay(fa, fa_scope, transfer(target, original_donors))
        if args.original and without_boundary_silence(original) != without_boundary_silence(original_clip(args.original)):
            raise AssertionError('the earlier Korean section was not reproduced')
        if args.old_pcm and variants['previous'] != args.old_pcm.read_bytes():
            raise AssertionError('the previous donor does not match the previous runtime PCM')
        if args.check_runtime:
            engine = make_language_engine(args.candidate, args.candidate, None, args.espeak_library, args.data_parent)
            try:
                for parameter, value in ((SAMPLE_RATE, 3), (NATIVE_SPEED, 65), (NATIVE_PITCH, 65),
                                         (NATIVE_INFLECTION, 30), (GENDER, 0)):
                    engine.set_parameter(parameter, value)
                actual = bytearray()
                if not engine.speak((' '.join(text for text, _ in SENTENCES),), actual.extend, lambda _: None):
                    raise RuntimeError('runtime synthesis failed')
                if bytes(actual) != variants['grouped']:
                    raise AssertionError('the grouped donor does not match the live runtime PCM')
            finally:
                engine.close()
        say('A controlled listening comparison. Every section uses the same Persian words, sounds and timing.')
        say('First: the translated Korean sentences at inflection thirty seven.')
        play(variants['translated'])
        say('Second: the previous word by word Korean donor at inflection thirty seven.')
        play(variants['previous'])
        say('Third: the new Korean phrase grouping at inflection thirty seven.')
        play(variants['grouped'])
        say('Fourth: the original listening experiment, translated Korean at inflection thirty.')
        play(original)
        say('End of comparison.')
    finally:
        frontend.close()
        fa.close()
        ko.close()
        en.close()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(args.output), 'wb') as output:
        output.setnchannels(1)
        output.setsampwidth(2)
        output.setframerate(16000)
        output.writeframes(recording)
    print('Wrote', args.output, 'seconds', round(len(recording) / 32000, 1))


if __name__ == '__main__':
    main()
