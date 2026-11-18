"""Make isolated Persian R listening comparisons; do not install the trials.

The Japanese short-R candidate comes from the actual scoped provider rule.
For the long and final R comparisons, replay the same Persian control frames
with a bounded, local change. The final-r noise spectrum is informed by the
measured Portuguese /ar/ R, but is not an exact Portuguese rule transplant.
The following-vowel context, frame count, pitch, source timing and equalization
remain the same; each pair is also matched for overall RMS loudness.
"""
from __future__ import annotations

import argparse
import ctypes
from itertools import groupby
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT / 'experiments/persian'), str(ROOT / 'nvda')]
from _reloq_eci_direct import GENDER, NATIVE_PITCH, NATIVE_SPEED, SAMPLE_RATE, DirectEciEngine
from _reloq_persian_frames import FrameReplay, NativeFrame
from compare_fa_clarity import matched_levels, write_wav
from test_us_articulation import capture, voice


def render(engine, rows):
    pcm = bytearray()
    if not FrameReplay(engine._dll).replay(engine, rows, (), pcm.extend, lambda _i: None) or not pcm:
        raise RuntimeError('Unable to replay native R frames')
    return bytes(pcm)


def active_end(rows):
    return 1 + max(i for i, row in enumerate(rows)
                   if row.controls[2] > 1 or row.controls[7] > 5 or row.controls[8] > 5)


def groups(indices):
    for _, values in groupby(enumerate(indices), lambda p: p[1] - p[0]):
        part = [value for _, value in values]
        if len(part) >= 4:
            yield part[0], part[-1] + 1


def r_interval(rows, kind):
    end = active_end(rows)
    if kind == 'cluster':
        noise = [i for i in range(end) if rows[i].controls[8] > 20 and rows[i].controls[2] < 15]
        runs = list(groups(noise))
        if len(runs) < 2:
            raise RuntimeError('Cannot find the final F in sefr')
        stop = runs[-1][1]
        start = next(i for i in range(stop, end) if rows[i].controls[2] > 15)
    elif kind == 'vowel':
        # This is the native R contact after the initial sustained /a/ in the
        # controlled /ar/ tag, rather than an inference about arbitrary text.
        start = next(i for i in range(60, end - 10) if rows[i].controls[2] < 30)
    elif kind == 'long':
        # The two low-voicing contacts must be followed by a voiced vowel;
        # exclude the final fade at the end of the whole syllable.
        contacts = [i for i in range(50, end - 30)
                    if rows[i].controls[2] < 12 and rows[i].controls[7] > 5]
        runs = list(groups(contacts))
        if len(runs) < 2:
            raise RuntimeError('Cannot find two contacts in the long R')
        start, end = max(0, runs[0][0] - 12), min(end, runs[1][1] + 12)
    else:
        raise ValueError('unknown R context')
    if end - start < 20:
        raise RuntimeError('The controlled R interval is too short')
    return start, end


def adjust(rows, kind, mode):
    start, end = r_interval(rows, kind)
    original = bytes(rows)
    if kind == 'long':
        allowed = (9, 13, 15, 17)
    else:
        allowed = (2, 7, 8, 9, 13, 15, 17, 35, 36, 37, 38, 39, 40)
    for i in range(start, end):
        c = rows[i].controls
        t = (i - start) / max(1, end - start - 1)
        envelope = min(1.0, (i - start + 1) / 8.0, (end - i) / 15.0)
        if kind == 'long':
            # Retain the original two distinct voicing contacts and timing;
            # move their resonances toward the measured Japanese short /r/.
            for column, target in zip((9, 13, 15, 17), (300, 1500, 2550, 3050)):
                c[column] += envelope * (target - c[column])
            continue
        if mode == 'soft':
            c[2] = round(c[2] * (1.0 - .6 * envelope))
            c[7] = round(c[7] * (1.0 - .45 * envelope))
            continue
        # These native frication-band levels and R centres are measured in
        # the Brazilian Portuguese /ar/ reference. The Persian R duration
        # and neighboring vowel or F are not replaced by Portuguese audio.
        voice = 0.0 if mode == 'unvoiced' else (1.0 - min(1.0, t * 3.0))
        c[2] = round(c[2] * (1.0 - envelope * (1.0 - voice)))
        c[7] = round(c[7] * (1.0 - envelope * (1.0 - voice)))
        c[8] = round((1.0 - envelope) * c[8] + envelope * 50)
        for column, target in zip(range(35, 41), (0, 0, 65, 45, 0, 0)):
            c[column] = round((1.0 - envelope) * c[column] + envelope * target)
        if kind == 'vowel':
            for column, target in zip((9, 13, 15, 17), (450, 1500, 2200, 2900)):
                c[column] += envelope * (target - c[column])
    # Only the described controls of the captured frames may change.
    check = (NativeFrame * len(rows)).from_buffer_copy(original)
    for old, new in zip(check, rows):
        for col in allowed:
            old.controls[col] = new.controls[col]
    if bytes(check) != bytes(rows):
        raise AssertionError('R trial changed another control, timing or pitch')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--previous', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    fa, scope = voice(args.library, 0x90000)
    old, old_scope = voice(args.previous, 0x90000)
    narrator = DirectEciEngine(args.library.resolve(), dialect=0x40000, encoding='utf8', native_voice=True)
    for engine in (fa, old, narrator):
        for setting, value in ((SAMPLE_RATE, 3), (NATIVE_SPEED, 65), (NATIVE_PITCH, 65), (GENDER, 0)):
            engine.set_parameter(setting, value)
    combined, silence = bytearray(), bytes(16000)
    def say(text):
        pcm = bytearray()
        if not narrator.speak((text,), pcm.extend, lambda _i: None) or not pcm:
            raise RuntimeError('Cannot narrate R comparison')
        combined.extend(pcm); combined.extend(silence)
    def play(label, sounds):
        sounds, _ = matched_levels(sounds)
        say(label + '.')
        for index, sound in enumerate(sounds):
            say('Bisher.' if index == 0 else 'Versuch ' + str(index) + '.')
            combined.extend(sound); combined.extend(silence)
    try:
        say('Vergleich persischer R Laute. Das kurze japanische R ist im neuen Add-on. Die anderen Versuche sind nur hier zu hören.')
        for label, tag in (('Kurzes R am Anfang, Ra', '`[.1ra]'),
                           ('Kurzes R zwischen Vokalen, Ara', '`[.1ara]')):
            a, ar = capture(old, old_scope, tag, True)
            b, br = capture(fa, scope, tag, True)
            assert len(a.pcm) == len(b.pcm) and len(ar) == len(br) and a.pcm != b.pcm
            play(label, (a.pcm, b.pcm))
        _, rows = capture(fa, scope, '`[.1arra]', True)
        before = render(fa, rows)
        adjust(rows, 'long', 'tap')
        play('Langes R, Arra. Versuch eins färbt beide Kontakte japanisch',
             (before, render(fa, rows)))
        _, rows = capture(fa, scope, '`[.1ar]', True)
        before = render(fa, rows)
        adjust(rows, 'vowel', 'mixed')
        play('R nach Vokal, Ar. Versuch eins ist brasilianisch angeregt',
             (before, render(fa, rows)))
        pcm = []
        for mode in ('original', 'soft', 'unvoiced', 'mixed'):
            _, rows = capture(fa, scope, '`[.1sefr]', True)
            if mode != 'original':
                adjust(rows, 'cluster', mode)
            pcm.append(render(fa, rows))
        if len(set(pcm)) != len(pcm):
            raise RuntimeError('A final R candidate has no effect')
        play('Sefr, R nach F. Versuch eins weich auslaufend, zwei stimmloses Rauschen, drei zuerst kurz stimmhaft', pcm)
        say('Ende des Hörvergleichs.')
    finally:
        fa.close(); old.close(); narrator.close()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    write_wav(args.output, combined)
    print('Wrote controlled R listening comparisons to', args.output)


if __name__ == '__main__':
    main()
