"""Check F0 transitions using the real ECI renderer and source indices."""
import argparse
import ctypes
from pathlib import Path

from _reloq_eci_direct import (DirectEciEngine, EciIndex, EciPitch,
                               NATIVE_PITCH, PITCH_GLIDE_MS, PROCESS_TAGS, SAMPLE_RATE)
from test_eci_german_clock_native import render
from test_eci_inline_queue import TEXTS


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--baseline', type=Path)
    args = parser.parse_args()
    checks = 0
    for dialect, pieces in {**TEXTS, 0x130000: ('A ', 'voz', ' ye clara.')}.items():
        engine = DirectEciEngine(args.library.resolve(), dialect=dialect,
                                 encoding='utf8', native_voice=True)
        previous = (DirectEciEngine(args.baseline.resolve(), dialect=dialect,
                                    encoding='utf8', native_voice=True) if args.baseline else None)
        try:
            engine.set_parameter(PROCESS_TAGS, 1)
            assert engine._dll.eciGetParam(engine._handle, PITCH_GLIDE_MS) == 0
            prefix, word, suffix = pieces
            plain = prefix + word + suffix
            commands = [prefix, EciIndex(71), '`vbhz200 ' + word, EciIndex(72), suffix]
            ordinary = render(engine, [plain])
            normal = render(engine, commands)
            if previous:
                previous.set_parameter(PROCESS_TAGS, 1)
                assert ordinary == render(previous, [plain]), (hex(dialect), 'default speech changed')
                assert normal == render(previous, commands), (hex(dialect), 'default pitch changed')
                checks += 2
            engine.set_parameter(PITCH_GLIDE_MS, 80)
            assert ordinary == render(engine, [plain]), (hex(dialect), 'ordinary speech changed')
            glide = render(engine, commands)
            assert len(glide[0]) == len(normal[0]) and glide[1] == normal[1], (hex(dialect), 'timing changed')
            assert [n for n, _ in glide[1]] == [71, 72]
            assert glide[0] != normal[0], (hex(dialect), 'pitch change was not smoothed')
            engine.set_parameter(PITCH_GLIDE_MS, 0)
            assert render(engine, commands) == normal, (hex(dialect), 'restore')
            checks += 5
            if dialect in (0x60000, 0x60001, 0x80000, 0xa0000):
                # CJK providers use their native word volume scheduling for
                # Hertz markers. Ordinary VB and volume must retain their own
                # timing and acoustic volume in the same sentence.
                leading = ['`vbhz150 ' + plain]
                initial = render(engine, leading)
                vb = [prefix, EciIndex(81), '`vb90 ' + word, EciIndex(82), suffix]
                plain_vb = render(engine, vb)
                pitch = [prefix, EciIndex(83), EciPitch(90), word, EciIndex(84), suffix]
                plain_pitch = render(engine, pitch)
                volume = [prefix, '`vv40 ' + word, suffix]
                plain_volume = render(engine, volume)
                base_pitch = engine._dll.eciGetParam(engine._handle, NATIVE_PITCH)
                engine.set_parameter(PITCH_GLIDE_MS, 80)
                assert render(engine, leading) == initial, (hex(dialect), 'initial Hertz')
                eased_vb = render(engine, vb)
                assert len(eased_vb[0]) == len(plain_vb[0]) and eased_vb[1] == plain_vb[1], (hex(dialect), 'VB timing')
                assert eased_vb[0] != plain_vb[0], (hex(dialect), 'VB transition')
                eased_pitch = render(engine, pitch)
                assert len(eased_pitch[0]) == len(plain_pitch[0]) and eased_pitch[1] == plain_pitch[1], (hex(dialect), 'pitch mark timing')
                if dialect == 0x80000:
                    assert eased_pitch == plain_pitch, (hex(dialect), 'Japanese pitch mark fallback')
                else:
                    assert eased_pitch[0] != plain_pitch[0], (hex(dialect), 'pitch mark transition')
                assert render(engine, [f'`vb{base_pitch} ' + prefix, '`vv40 ' + word, suffix]) == plain_volume, (hex(dialect), 'volume changed')
                checks += 7
        finally:
            engine.close()
            if previous:
                previous.close()

    for dialect in (0x10000, 0x40000):
        engine = DirectEciEngine(args.library.resolve(), dialect=dialect,
                                 encoding='utf8', native_voice=True)
        try:
            engine.set_parameter(PROCESS_TAGS, 1)
            engine.set_parameter(SAMPLE_RATE, 6)
            phrase = ['`vf0 `vbhz120 `[.1a] ', EciIndex(1),
                      '`vbhz180 `[.1a] ', EciIndex(2), '`vbhz120 `[.1a]']
            outputs = []
            for duration in (0, 20, 40, 80):
                engine.set_parameter(PITCH_GLIDE_MS, duration)
                audio, marks = render(engine, phrase)
                assert [mark for mark, _ in marks] == [1, 2]
                outputs.append((audio, marks))
            assert len({len(audio) for audio, _ in outputs}) == 1
            assert len({audio for audio, _ in outputs}) == 4, (hex(dialect), 'durations not distinct')
            assert all(marks == outputs[0][1] for _, marks in outputs), (hex(dialect), 'indices moved')
            # First text has not yet encountered a new target.
            first_boundary = outputs[0][1][0][1]
            assert all(audio[:first_boundary] == outputs[0][0][:first_boundary]
                       for audio, _ in outputs), (hex(dialect), 'pre-transition audio changed')
            initial = ['`vf0 `vbhz150 `[.1a]']
            engine.set_parameter(PITCH_GLIDE_MS, 0)
            first = render(engine, initial)
            engine.set_parameter(PITCH_GLIDE_MS, 80)
            assert render(engine, initial) == first, (hex(dialect), 'first pitch should not glide')
            # NVDA's explicit ECI pitch marks use the same transition path.
            pitch_parts = ['`vf0 `[.1a] ', EciIndex(7), EciPitch(90), '`[.1a]']
            glide = render(engine, pitch_parts)
            engine.set_parameter(PITCH_GLIDE_MS, 0)
            direct = render(engine, pitch_parts)
            assert len(glide[0]) == len(direct[0]) and glide[1] == direct[1]
            assert glide[0] != direct[0], (hex(dialect), 'ECI pitch mark not smoothed')
            checks += 7
            for invalid in (-1, 501):
                assert engine._dll.eciSetParam(engine._handle, PITCH_GLIDE_MS, invalid) == -1
                checks += 1
            engine.set_parameter(PITCH_GLIDE_MS, 50)
            engine._dll.eciReset.argtypes = (ctypes.c_void_p,)
            engine._dll.eciReset.restype = ctypes.c_int
            assert engine._dll.eciReset(engine._handle)
            assert engine._dll.eciGetParam(engine._handle, PITCH_GLIDE_MS) == 0
            checks += 1
        finally:
            engine.close()
    print(f'F0 glide: {checks} multilingual PCM, compatibility, timing and ECI checks passed')


if __name__ == '__main__':
    main()
