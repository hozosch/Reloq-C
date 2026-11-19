"""Check the German-guided Persian U's female source and isolation."""
from __future__ import annotations

import argparse
import array
import math
from pathlib import Path
from test_us_articulation import voice, capture
from _reloq_eci_direct import GENDER, HEAD_SIZE, NATIVE_SPEED, SAMPLE_RATE, SAMPLE_RATES
from _reloq_persian_frames import FrameCapture


def phone(engine, scope, tag, position, selected):
    previous = scope(int(selected))
    try:
        data = FrameCapture(engine._dll).capture(engine, (tag,), phone_segments=True)
    finally:
        assert scope(previous) == int(selected)
    assert data and len(data.syllables) > position
    start = data.syllables[position][0] / 10000.0
    end = (data.syllables[position + 1][0] / 10000.0
           if position + 1 < len(data.syllables) else data.syllables[position][1] / 10000.0)
    left, right = start + (end-start)*.30, start + (end-start)*.70
    frames = [r for r in data.frames() if left <= r.tick < right and r.controls[2] > 10]
    assert frames, (tag, start, end, 'no voiced nucleus')
    return data, frames, (left, right)


def rms(data, span, rate):
    samples = array.array('h'); samples.frombytes(data.pcm)
    left, right = (round(t * rate / 1000) for t in span)
    selected = samples[left:right]
    assert selected
    return math.sqrt(sum(s*s for s in selected) / len(selected))


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--candidate', required=True, type=Path)
    ap.add_argument('--previous', type=Path)
    args = ap.parse_args()
    fa, fs = voice(args.candidate, 0x90000)
    de, ds = voice(args.candidate, 0x40000)
    old, os = voice(args.previous, 0x90000) if args.previous else (None, None)
    checks = 0
    try:
        for gender in (0, 1):
            for head in (15, 50, 85):
                for rate in (1, 3, 6):
                    for engine in (fa, de, old):
                        if engine:
                            for key, value in ((GENDER, gender), (HEAD_SIZE, head),
                                               (SAMPLE_RATE, rate), (NATIVE_SPEED, 65)):
                                engine.set_parameter(key, value)
                    for tag, index in (('`[.1u]', 0), ('`[.1pu]', 1), ('`[.1mu]', 1),
                                       ('`[.1nu]', 1), ('`[.1ru]', 1), ('`[.1su]', 1),
                                       ('`[.1bu]', 1)):
                        actual, af, span = phone(fa, fs, tag, index, True)
                        reference, df, dspan = phone(de, ds, tag, index, False)
                        if gender:
                            av = af[len(af)//2].controls[2]
                            dav = df[len(df)//2].controls[2]
                            assert abs(av-dav) <= 2, (gender, head, rate, tag, av, dav)
                            ar, dr = rms(actual, span, SAMPLE_RATES[rate]), rms(reference, dspan, SAMPLE_RATES[rate])
                            assert .65 < ar/dr < 1.6, (gender, head, rate, tag, ar, dr)
                        if old:
                            before, bf, _ = phone(old, os, tag, index, True)
                            assert len(before.pcm) == len(actual.pcm)
                            if gender:
                                assert actual.pcm != before.pcm, (gender, head, rate, tag)
                                old_av = bf[len(bf)//2].controls[2]
                                assert af[len(af)//2].controls[2] - old_av >= 4, (tag, old_av, av)
                            else:
                                assert actual.pcm == before.pcm, (gender, head, rate, tag)
                        checks += 1
                    if old:
                        # Unrelated Persian vowels, the Finnish scope, and the
                        # German reference remain bit-identical to the previous build.
                        for tag in ('`[.1a]', '`[.1A]', '`[.1e]', '`[.1i]', '`[.1o]'):
                            assert capture(fa,fs,tag,True)[0].pcm == capture(old,os,tag,True)[0].pcm, (gender,head,rate,tag)
                        assert capture(fa,fs,'`[.1u]',False)[0].pcm == capture(old,os,'`[.1u]',False)[0].pcm
                        checks += 6
        # The female voice's formant transforms can change abruptly at some
        # head sizes. Check every setting, including the small-head voices.
        for head in range(101):
            for engine in (fa, de, old):
                if engine:
                    for key, value in ((GENDER, 1), (HEAD_SIZE, head),
                                       (SAMPLE_RATE, 3), (NATIVE_SPEED, 65)):
                        engine.set_parameter(key, value)
            current, frames, span = phone(fa, fs, '`[.1u]', 0, True)
            reference, german_frames, dspan = phone(de, ds, '`[.1u]', 0, False)
            av = frames[len(frames)//2].controls[2]
            dav = german_frames[len(german_frames)//2].controls[2]
            assert abs(av-dav) <= 2, (head, av, dav)
            ar, dr = rms(current, span, 16000), rms(reference, dspan, 16000)
            assert .65 < ar/dr < 1.6, (head, ar, dr)
            if old:
                before, old_frames, _ = phone(old, os, '`[.1u]', 0, True)
                assert len(before.pcm) == len(current.pcm)
                assert av - old_frames[len(old_frames)//2].controls[2] >= 4, head
            checks += 1
        print(f'Passed {checks} Persian U source, German reference and voice-isolation checks.')
    finally:
        fa.close(); de.close()
        if old: old.close()


if __name__ == '__main__':
    main()
