"""Compare a continuous Persian passage with sentence-aligned Korean F0.

This is a hearing experiment, not a Persian intonation model. The reference
and candidate use the identical Persian synthesis frames and stress markers.
Only F0 is replaced, by the corresponding Korean-core sentence pitch curve.
The donor curve is interpolated between voiced frames, stretched separately
in each sentence, and median-normalized over the whole passage.
"""
from __future__ import annotations

import argparse
from bisect import bisect_left
from pathlib import Path
from statistics import median
import wave

from direct_fa_sequence import EciIndex, project_resilient
from hybrid_fa_frontend import DirectFirstPersian
from test_us_articulation import capture, voice
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE
from _reloq_persian_frames import FrameCapture, FrameReplay

SENTENCES = (
    ('سلام دوستان.', '안녕하세요, 여러분.'),
    ('امروز می‌خواهم درباره یک کتاب صحبت کنم.', '오늘은 책 한 권에 대해 이야기하고 싶습니다.'),
    ('این کتاب درباره دو دوست است.', '이 책은 두 친구에 관한 이야기입니다.'),
    ('آن‌ها هر روز با هم به مدرسه می‌روند.', '그들은 매일 함께 학교에 갑니다.'),
    ('در راه به مردم سلام می‌کنند و درباره زندگی صحبت می‌کنند.',
     '길에서 사람들에게 인사하고 삶에 대해 이야기합니다.'),
    ('وقتی به خانه برمی‌گردند، کتاب می‌خوانند و به موسیقی گوش می‌دهند.',
     '집에 돌아오면 책을 읽고 음악을 듣습니다.'),
    ('شما هم کتاب خواندن را دوست دارید؟', '여러분도 책을 읽는 것을 좋아합니까?'),
)


def indexed_capture(engine, scope, parts, selected):
    previous = scope(int(selected))
    try:
        result = FrameCapture(engine._dll).capture(engine, parts)
        assert result and result.pcm
        return result
    finally:
        assert scope(previous) == int(selected)


def replay(engine, scope, frames):
    pcm = bytearray()
    previous = scope(1)
    try:
        assert FrameReplay(engine._dll).replay(engine, frames, (), pcm.extend, lambda _: None)
    finally:
        assert scope(previous) == 1
    return bytes(pcm)


def transfer(target, donors):
    fa_frames = target.frames()
    assert len(target.indexes) == len(donors) == len(SENTENCES)
    # At 16 kHz each frame tick is one millisecond and the indexes are sample
    # offsets. The Korean sentences are captured independently: Korean's ECI
    # index callbacks may be emitted together at the end of an utterance.
    fa_edges = [at / 16 for _, at in target.indexes] + [len(target.pcm) / 32]
    if any(a >= b for a, b in zip(fa_edges, fa_edges[1:])):
        raise ValueError('Sentence boundary indexes are not strictly ordered')

    original, proposed = [], []
    for number, (fa_start, fa_end, donor) in enumerate(
            zip(fa_edges, fa_edges[1:], donors), 1):
        a = [r for r in fa_frames if fa_start <= r.tick < fa_end]
        b = donor.frames()
        fa_voiced = [(r.tick, r.controls[1]) for r in a if r.controls[2] > 10 and r.controls[1] > 0]
        ko_voiced = [(r.tick, r.controls[1]) for r in b if r.controls[2] > 10 and r.controls[1] > 0]
        if len(fa_voiced) < 10 or len(ko_voiced) < 10:
            raise ValueError('Insufficient voiced material in sentence ' + str(number))
        a_start, a_end = fa_voiced[0][0], fa_voiced[-1][0]
        ticks, pitches = zip(*ko_voiced)
        if a_end <= a_start or ticks[-1] <= ticks[0]:
            raise ValueError('A sentence has no pitch duration')

        def mapped(tick):
            fraction = max(0.0, min(1.0, (tick - a_start) / (a_end - a_start)))
            position = ticks[0] + fraction * (ticks[-1] - ticks[0])
            i = bisect_left(ticks, position)
            if i == 0:
                return pitches[0]
            if i == len(ticks):
                return pitches[-1]
            part = (position - ticks[i - 1]) / (ticks[i] - ticks[i - 1])
            return pitches[i - 1] + part * (pitches[i] - pitches[i - 1])

        for row in a:
            if row.controls[1] > 0:
                value = mapped(row.tick)
                proposed.append((row, value))
                if row.controls[2] > 10:
                    original.append((row.controls[1], value))
        print('sentence', number, 'Persian', len(fa_voiced), 'Korean', len(ko_voiced),
              'duration ms', round(fa_end - fa_start), flush=True)
    scale = median(a for a, _ in original) / median(b for _, b in original)
    for row, value in proposed:
        row.controls[1] = value * scale
    return fa_frames


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('candidate', 'espeak-library', 'data-parent', 'output'):
        parser.add_argument('--' + name, required=True, type=Path)
    parser.add_argument('--labels', choices=('de', 'en'), default='de')
    parser.add_argument('--speed', type=int, default=65)
    args = parser.parse_args()
    if not 0 <= args.speed <= 250:
        parser.error('--speed must be between 0 and 250')
    frontend = DirectFirstPersian(args.candidate, None, args.espeak_library, args.data_parent)
    fa, fa_scope = voice(args.candidate, 0x90000)
    ko, ko_scope = voice(args.candidate, 0xa0000)
    labels, label_scope = voice(args.candidate, 0x40000 if args.labels == 'de' else 0x10000)
    for engine in (fa, ko, labels):
        for parameter, value in ((SAMPLE_RATE, 3), (NATIVE_SPEED, args.speed), (GENDER, 0)):
            engine.set_parameter(parameter, value)
    audio = bytearray()

    def append(pcm):
        assert pcm
        audio.extend(pcm)
        audio.extend(bytes(12800))

    def say(text):
        result, _ = capture(labels, label_scope, text, False)
        append(result.pcm)

    try:
        fa_parts = []
        for index, (persian, korean) in enumerate(SENTENCES, 1):
            fa_parts.extend((EciIndex(index), persian + ' '))
        job = project_resilient(fa_parts, frontend)
        assert [p.value for p in job.parts if isinstance(p, EciIndex)] == list(range(1, len(SENTENCES) + 1))
        target = indexed_capture(fa, fa_scope, job.parts, True)
        donors = [indexed_capture(ko, ko_scope, (korean,), False) for _, korean in SENTENCES]
        original_frames = target.frames()
        assert replay(fa, fa_scope, original_frames) == target.pcm
        frames = transfer(target, donors)
        for before, after in zip(original_frames, frames):
            assert before.tick == after.tick and all(before.controls[i] == after.controls[i]
                                                     for i in range(62) if i != 1)
        candidate = replay(fa, fa_scope, frames)
        assert len(candidate) == len(target.pcm) and candidate != target.pcm
        if args.labels == 'de':
            say('Längere Hörprobe. Zuerst die bisherige persische Melodie, '
                'danach eine koreanische Tonhöhenkurve, satzweise auf denselben Text übertragen. '
                'Wörter, Betonungspositionen und Lautlängen bleiben gleich. Nur eine Hörprobe.')
            say('Bisherige Melodie.')
            append(target.pcm)
            say('Koreanische Melodie.')
            append(candidate)
            say('Ende des Vergleichs.')
        else:
            say('A longer listening comparison. First, the current Persian intonation, '
                'then Korean pitch contours transferred sentence by sentence to the same Persian text. '
                'Words, stress positions, and sound durations stay the same. Listening experiment only.')
            say('Current intonation.')
            append(target.pcm)
            say('Korean pitch contours.')
            append(candidate)
            say('End of comparison.')
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
