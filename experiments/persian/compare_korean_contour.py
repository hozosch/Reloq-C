"""Compare current Persian F0 with a Korean donor, without changing segments.

This is a listening experiment, not an implementation of Persian prosody.
The donor's F0 is stretched to the Persian utterance, and its median is
matched to the original. Durations, lexical stress and all non-F0 controls
are kept exactly the same. Both languages are synthesized by ReLoq-C.
"""
from __future__ import annotations

import argparse
from bisect import bisect_left
from pathlib import Path
from statistics import median
import wave

from hybrid_fa_frontend import DirectFirstPersian
from test_us_articulation import capture, voice
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE
from _reloq_persian_frames import FrameReplay

CASES = (
    ('سلام دنیا.', '안녕 세계.'),
    ('صحبت می‌کنم.', '말합니다.'),
    ('این یک آزمایش است.', '이것은 시험입니다.'),
    ('این یک آزمایش است؟', '이것은 시험입니까?'),
)


def replay(engine, scope, frames):
    pcm = bytearray()
    previous = scope(1)
    try:
        assert FrameReplay(engine._dll).replay(engine, frames, (), pcm.extend, lambda _: None)
    finally:
        assert scope(previous) == 1
    return bytes(pcm)


def transplant(fa, fa_scope, ko, ko_scope, tag, korean):
    source, target = capture(fa, fa_scope, tag, True)
    donor, rows = capture(ko, ko_scope, korean, False)
    original = [(r.tick, r.controls[1]) for r in target if r.controls[2] > 10 and r.controls[1] > 0]
    voiced = [(r.tick, r.controls[1]) for r in rows if r.controls[2] > 10 and r.controls[1] > 0]
    if len(original) < 10 or len(voiced) < 10:
        raise ValueError('A donor or target has too little voiced speech')
    assert replay(fa, fa_scope, target) == source.pcm
    ticks, f0 = zip(*voiced)
    start, end = original[0][0], original[-1][0]
    if end <= start or ticks[-1] <= ticks[0]:
        raise ValueError('A pitch contour has no duration')

    def mapped(tick):
        position = ticks[0] + max(0.0, min(1.0, (tick - start) / (end - start))) * (ticks[-1] - ticks[0])
        i = bisect_left(ticks, position)
        if i == 0:
            return f0[0]
        if i == len(ticks):
            return f0[-1]
        fraction = (position - ticks[i - 1]) / (ticks[i] - ticks[i - 1])
        return f0[i - 1] + fraction * (f0[i] - f0[i - 1])

    target_median = median(value for _, value in original)
    scale = target_median / median(mapped(tick) for tick, _ in original)
    for frame in target:
        if frame.controls[1] > 0:
            frame.controls[1] = mapped(frame.tick) * scale
    candidate = replay(fa, fa_scope, target)
    assert len(candidate) == len(source.pcm) and candidate != source.pcm
    return source.pcm, candidate, donor.pcm


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('candidate', 'espeak-library', 'data-parent', 'output'):
        parser.add_argument('--' + name, required=True, type=Path)
    parser.add_argument('--labels', choices=('de', 'en'), default='de')
    args = parser.parse_args()
    frontend = DirectFirstPersian(args.candidate, None, args.espeak_library, args.data_parent)
    fa, fa_scope = voice(args.candidate, 0x90000)
    ko, ko_scope = voice(args.candidate, 0xa0000)
    labels, label_scope = voice(args.candidate, 0x40000 if args.labels == 'de' else 0x10000)
    for engine in (fa, ko, labels):
        for parameter, value in ((SAMPLE_RATE, 3), (NATIVE_SPEED, 65), (GENDER, 0)):
            engine.set_parameter(parameter, value)
    audio = bytearray()

    def append(pcm):
        assert pcm
        audio.extend(pcm)
        audio.extend(bytes(9600))

    def say(text):
        sample, _ = capture(labels, label_scope, text, False)
        append(sample.pcm)

    try:
        if args.labels == 'de':
            say('Vergleich der persischen Melodie. Zuerst die bisherige Version, '
                'danach eine koreanische Tonhöhenkurve, auf dieselbe persische '
                'Sprache übertragen. Laute, Betonungspositionen und Timing bleiben gleich. '
                'Dies ist nur eine Hörprobe.')
        else:
            say('Persian intonation comparison. First, the current version. '
                'Then, a Korean pitch contour transferred to the same Persian speech. '
                'Sounds, stress positions, and timing stay the same. '
                'This is a listening experiment only.')
        for number, (text, korean) in enumerate(CASES, 1):
            analysis = frontend.analyze_direct(text)
            if analysis is None or len(analysis.parts) != 1 or analysis.parts[0].language != 'fa':
                raise ValueError('No directly covered Persian sentence: ' + text)
            before, after, donor = transplant(fa, fa_scope, ko, ko_scope, analysis.parts[0].text, korean)
            say((f'Beispiel {number}. Bisher.' if args.labels == 'de' else f'Example {number}. Current.'))
            append(before)
            say('Koreanische Melodie.' if args.labels == 'de' else 'Korean pitch contour.')
            append(after)
            print(number, text, korean, len(before), len(donor), flush=True)
        say('Ende des Vergleichs.' if args.labels == 'de' else 'End of comparison.')
    finally:
        frontend.close()
        fa.close()
        ko.close()
        labels.close()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(args.output), 'wb') as out:
        out.setnchannels(1)
        out.setsampwidth(2)
        out.setframerate(16000)
        out.writeframes(audio)
    print('Wrote', args.output, 'seconds', round(len(audio) / 32000, 1))


if __name__ == '__main__':
    main()
