"""Listen to Persian consonant-final R experiments based on Portuguese /ar/.

This is an offline frame comparison, not an installed pronunciation rule.
The preceding /f/, timing, pitch and controls outside the R interval remain
unchanged. The donor's frication contour and formants come from the actual
Brazilian final R; two voice envelopes are compared after /f/.
"""
from __future__ import annotations

import argparse
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT / 'experiments/persian'), str(ROOT / 'nvda')]
from _reloq_eci_direct import DirectEciEngine, GENDER, NATIVE_PITCH, NATIVE_SPEED, SAMPLE_RATE
from _reloq_persian_frames import NativeFrame
from compare_fa_clarity import matched_levels, write_wav
from compare_r_context import active_end, r_interval, render
from test_us_articulation import capture, voice


def trial(rows, donor, mode):
    start, end = r_interval(rows, 'cluster')
    donor_start = next(i for i, row in enumerate(donor)
                       if row.controls[8] >= 20 and row.controls[37] >= 20)
    donor_end = active_end(donor)
    assert donor_end - donor_start >= 8 and end - start >= 8
    original = bytes(rows)
    allowed = (2, 7, 8, 9, 13, 15, 17, 35, 36, 37, 38, 39, 40)
    for index in range(start, end):
        t = (index - start) / max(1, end - start - 1)
        source = donor[donor_start + round(t * (donor_end - donor_start - 1))].controls
        c = rows[index].controls
        onset = min(1.0, (index - start + 1) / 7.0)
        offset = min(1.0, (end - index) / 14.0)
        mix = onset * offset
        if mode == 'noise':
            c[2] *= 1.0 - onset
            c[7] *= 1.0 - onset
        elif mode == 'voiced_then_noise':
            voice = max(0.0, 1.0 - (index - start + 1) / 24.0)
            c[2] = c[2] * (1.0 - onset) + source[2] * onset * voice
            c[7] = c[7] * (1.0 - onset) + source[7] * onset * voice
        else:
            raise ValueError(mode)
        for column in (8, 9, 13, 15, 17, 35, 36, 37, 38, 39, 40):
            c[column] = (1.0 - mix) * c[column] + mix * source[column]
        # The native frame adapter accepts integer-valued source levels.
        for column in (2, 7, 8, 35, 36, 37, 38, 39, 40):
            c[column] = round(c[column])
    # No neighboring sound, pitch, frame clock or unrelated control changes.
    check = (NativeFrame * len(rows)).from_buffer_copy(original)
    for before, after in zip(check, rows):
        for column in allowed:
            before.controls[column] = after.controls[column]
    if bytes(check) != bytes(rows):
        raise AssertionError('A candidate changed a neighboring sound or frame timing')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    fa, fa_scope = voice(args.library, 0x90000)
    pt, pt_scope = voice(args.library, 0x70000)
    narrator = DirectEciEngine(args.library.resolve(), dialect=0x40000, encoding='utf8', native_voice=True)
    for engine in (fa, pt, narrator):
        for key, value in ((SAMPLE_RATE, 3), (NATIVE_SPEED, 65), (NATIVE_PITCH, 65), (GENDER, 0)):
            engine.set_parameter(key, value)
    audio, silence = bytearray(), bytes(16000)
    def say(text):
        chunk = bytearray()
        if not narrator.speak((text,), chunk.extend, lambda _i: None) or not chunk:
            raise RuntimeError('Cannot narrate the R comparison')
        audio.extend(chunk); audio.extend(silence)
    try:
        _, donor = capture(pt, pt_scope, '`[.1ar]', False)
        variants = []
        for mode in ('current', 'noise', 'voiced_then_noise'):
            _, rows = capture(fa, fa_scope, '`[.1sefr]', True)
            if mode != 'current':
                trial(rows, donor, mode)
            variants.append(render(fa, rows))
        if len(set(variants)) != len(variants):
            raise AssertionError('A comparison does not change the R')
        variants, _ = matched_levels(variants)
        say('Sefr, R nach F. Im Add-on bleibt hier der bisherige Laut. Jetzt folgen zwei neue Hörversuche.')
        for label, sound in zip(('Bisher', 'Versuch eins, Rauschen',
                                 'Versuch zwei, kurz stimmhaft, dann Rauschen'), variants):
            say(label + '.')
            audio.extend(sound); audio.extend(silence)
        say('Ende des Vergleichs.')
    finally:
        fa.close(); pt.close(); narrator.close()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    write_wav(args.output, audio)
    print('Wrote consonant-final R listening comparison:', args.output)


if __name__ == '__main__':
    main()
