"""Isolate why translated Korean and the live syllable proxy sound different.

All four sections replay one identical Persian frame capture. They compare
translated Korean at inflection 30 with the neutral syllable proxy at the same
inflection, translated Korean at 37, and the earlier proxy at 37. The first
three share one passage-wide pitch scale; the fourth uses unscaled Korean
pitch. The fifth checks the calibrated runtime. This is a diagnostic, not a
Persian model.
"""
from __future__ import annotations

import argparse
import array
from bisect import bisect_left
import math
from pathlib import Path
from statistics import median
import sys
import wave

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import GENDER, NATIVE_INFLECTION, NATIVE_PITCH, NATIVE_SPEED, SAMPLE_RATE
from compare_korean_long import SENTENCES, indexed_capture, replay, transfer
from direct_fa_sequence import EciIndex, project_resilient
from hybrid_fa_frontend import DirectFirstPersian
from korean_contour import korean_inflection, korean_pitch, proxy
from nvda_driver_probe import make_language_engine
from test_us_articulation import voice


def mapped_pitch(target, donors):
    rows = target.frames()
    edges = [at / 16 for _, at in target.indexes] + [len(target.pcm) / 32]
    if len(edges) != len(donors) + 1 or any(a >= b for a, b in zip(edges, edges[1:])):
        raise ValueError('invalid sentence marks')
    mapped, voiced = [], []
    for start, end, donor in zip(edges, edges[1:], donors):
        selected = [(i, row) for i, row in enumerate(rows) if start <= row.tick < end]
        target_voice = [row for _, row in selected if row.controls[2] > 10 and row.controls[1] > 0]
        donor_voice = [row for row in donor.frames() if row.controls[2] > 10 and row.controls[1] > 0]
        if len(target_voice) < 10 or len(donor_voice) < 10:
            raise ValueError('insufficient voiced material')
        first, last = target_voice[0].tick, target_voice[-1].tick
        ticks = [row.tick for row in donor_voice]
        pitch = [row.controls[1] for row in donor_voice]
        if first >= last or ticks[0] >= ticks[-1]:
            raise ValueError('invalid voiced duration')
        for index, row in selected:
            if row.controls[1] <= 0:
                continue
            fraction = max(0.0, min(1.0, (row.tick - first) / (last - first)))
            at = ticks[0] + fraction * (ticks[-1] - ticks[0])
            i = bisect_left(ticks, at)
            if i == 0:
                value = pitch[0]
            elif i == len(ticks):
                value = pitch[-1]
            else:
                f = (at - ticks[i - 1]) / (ticks[i] - ticks[i - 1])
                value = pitch[i - 1] + f * (pitch[i] - pitch[i - 1])
            mapped.append((index, value))
            if row.controls[2] > 10:
                voiced.append((row.controls[1], value))
    return mapped, voiced


def original_clip(path):
    """Read the Korean section from the published English long comparison."""
    with wave.open(str(path), 'rb') as source:
        if (source.getnchannels(), source.getsampwidth(), source.getframerate()) != (1, 2, 16000):
            raise ValueError('unexpected original audio format')
        samples = array.array('h')
        samples.frombytes(source.readframes(source.getnframes()))
    gaps, start = [], None
    for i, value in enumerate(samples):
        if value == 0 and start is None:
            start = i
        elif value != 0 and start is not None:
            if i - start >= 5000:
                gaps.append((start, i))
            start = None
    if len(gaps) != 5:
        raise ValueError('unexpected original comparison layout')
    return samples[gaps[3][1]:gaps[4][0]].tobytes()


def without_boundary_silence(pcm):
    values = array.array('h')
    values.frombytes(pcm)
    start, end = 0, len(values)
    while start < end and values[start] == 0:
        start += 1
    while end > start and values[end - 1] == 0:
        end -= 1
    return values[start:end].tobytes()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('candidate', 'espeak-library', 'data-parent', 'output'):
        parser.add_argument('--' + name, required=True, type=Path)
    parser.add_argument('--original', type=Path, help='optionally verify the earlier English listening sample')
    parser.add_argument('--check-runtime', action='store_true', help='verify the last section against live runtime PCM')
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
        donor_text = [(korean, proxy(project_resilient((persian,), frontend).tagged))
                      for persian, korean in SENTENCES]
        donors = {}
        for inflection in (30, 37):
            ko.set_parameter(NATIVE_INFLECTION, inflection)
            for column, name in enumerate(('translated', 'neutral')):
                donors[name, inflection] = [indexed_capture(ko, ko_scope, (row[column],), False)
                                           for row in donor_text]
        _, reference_voiced = mapped_pitch(target, donors['translated', 30])
        scale = median(original for original, _ in reference_voiced) / median(value for _, value in reference_voiced)
        reference = replay(fa, fa_scope, transfer(target, donors['translated', 30]))
        if args.original and without_boundary_silence(reference) != without_boundary_silence(original_clip(args.original)):
            raise AssertionError('the original Korean listening section was not reproduced')
        variants = {}
        for name, inflection, factor in (('neutral', 30, scale), ('translated', 37, scale), ('neutral', 37, 1.0)):
            values, _ = mapped_pitch(target, donors[name, inflection])
            frames, unchanged = target.frames(), target.frames()
            for index, value in values:
                frames[index].controls[1] = value * factor
            if any(a.tick != b.tick or any(a.controls[i] != b.controls[i] for i in range(62) if i != 1)
                   for a, b in zip(frames, unchanged)):
                raise AssertionError('a non-pitch frame control changed')
            pcm = replay(fa, fa_scope, frames)
            if len(pcm) != len(target.pcm):
                raise AssertionError('Persian duration changed')
            variants[name, inflection] = pcm
        ko.set_parameter(NATIVE_PITCH, korean_pitch(65))
        ko.set_parameter(NATIVE_INFLECTION, korean_inflection(30))
        calibrated = [indexed_capture(ko, ko_scope, (row[1],), False) for row in donor_text]
        values, _ = mapped_pitch(target, calibrated)
        frames = target.frames()
        for index, value in values:
            frames[index].controls[1] = value
        current = replay(fa, fa_scope, frames)
        if args.check_runtime:
            engine = make_language_engine(args.candidate, args.candidate, None, args.espeak_library, args.data_parent)
            try:
                for parameter, value in ((SAMPLE_RATE, 3), (NATIVE_SPEED, 65), (NATIVE_PITCH, 65),
                                         (NATIVE_INFLECTION, 30), (GENDER, 0)):
                    engine.set_parameter(parameter, value)
                actual = bytearray()
                if not engine.speak((' '.join(text for text, _ in SENTENCES),), actual.extend, lambda _: None):
                    raise RuntimeError('runtime synthesis failed')
                if bytes(actual) != current:
                    raise AssertionError('the current runtime does not match its comparison section')
            finally:
                engine.close()
        say('Controlled comparison: the same Persian words, sounds, voice and timing in every section. '
            'Only the Korean source, inflection and overall pitch level change.')
        say('One: the original listening experiment. Korean translations, inflection thirty.')
        play(reference)
        say('Two: neutral Korean syllables, inflection thirty.')
        play(variants['neutral', 30])
        say('Three: the original Korean translations, inflection thirty seven.')
        play(variants['translated', 37])
        say('Four: earlier neutral Korean syllables, inflection thirty seven, without pitch normalization.')
        play(variants['neutral', 37])
        say('Five: the current calibrated Persian settings, pitch sixty five, inflection thirty.')
        play(current)
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
    print('Passage-wide scale:', round(scale, 6), 'semitones:', round(12 * math.log2(scale), 3))
    print('Wrote', args.output, 'seconds', round(len(recording) / 32000, 1))


if __name__ == '__main__':
    main()
