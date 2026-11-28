"""Compare /s/, closure and stop timing through the public ECI interface."""
import argparse
from array import array
from pathlib import Path

from _reloq_eci_direct import DirectEciEngine, EciIndex, NATIVE_WPM, PROCESS_TAGS, SAMPLE_RATE
from test_eci_german_clock_native import phonemes, render


def s_and_release(pcm):
    samples = array('h')
    samples.frombytes(pcm)
    size = 11  # approximately one millisecond at 11025 Hz
    powers = [sum(int(x) * int(x) for x in samples[i:i + size]) / size
              for i in range(0, min(len(samples), 3300) - size, size)]
    threshold = max(100.0, max(powers[:30]) * 0.0004)
    end = next((i for i in range(10, len(powers) - 4)
                if all(power < threshold for power in powers[i:i + 5])), None)
    assert end is not None, 'no closure after the initial s'
    release = next((i for i in range(end + 5, len(powers) - 2)
                    if all(power >= threshold for power in powers[i:i + 3])), None)
    assert release is not None, 'no stop release after the closure'
    return end, release


def make_engine(path, dialect):
    e = DirectEciEngine(path.resolve(), dialect=dialect, encoding='utf8', native_voice=True)
    e.set_parameter(SAMPLE_RATE, 1)
    e.set_parameter(PROCESS_TAGS, 1)
    return e


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--library', type=Path, required=True)
    p.add_argument('--reference', type=Path, required=True)
    p.add_argument('--dictionary', type=Path)
    args = p.parse_args()
    changed = ('ski', 'scout', 'school', 'sky', 'skew', 'screen', 'stop', 'spill')
    unchanged = ('see', 'say', 'ask', 'display', 'rescue', 'good dog',
                 '`[.1skhWt]', '`[.1trAnsh.2krIpt]')
    checks = 0
    for dialect in (0x10000, 0x10001):
        current, before = make_engine(args.library, dialect), make_engine(args.reference, dialect)
        try:
            for rate in (135, 200, 320):
                for engine in (current, before):
                    engine.set_parameter(NATIVE_WPM, rate)
                for word in changed:
                    assert phonemes(current, word) == phonemes(before, word), (dialect, word)
                    audio = render(current, [word + '.'])[0]
                    old = render(before, [word + '.'])[0]
                    end, release = s_and_release(audio)
                    old_end, old_release = s_and_release(old)
                    assert abs(end-old_end)<=2, (dialect, rate, word, 's', end, old_end)
                    assert release-end <= old_release-old_end+2, \
                        (dialect, rate, word, 'closure', end, release, old_end, old_release)
                    if rate == 135:
                        assert release-end <= old_release-old_end-3, \
                            (dialect, rate, word, 'shorter closure', end, release, old_end, old_release)
                    checks += 1
                for word in unchanged:
                    assert render(current, [word]) == render(before, [word]), (dialect, rate, word)
                    checks += 1
                parts = ['Please ', EciIndex(1), 'transcribe ', EciIndex(2),
                         'the scout, ', EciIndex(3), 'stop and spill.', EciIndex(4)]
                new_audio, new_marks = render(current, parts)
                old_audio, old_marks = render(before, parts)
                assert len(new_audio) == len(old_audio) and new_marks == old_marks
                assert new_marks[-1][1] == len(new_audio)
                checks += 1
        finally:
            current.close()
            before.close()
    if args.dictionary:
        current, before = make_engine(args.library, 0x10000), make_engine(args.reference, 0x10000)
        try:
            for engine in (current, before):
                engine.set_dictionary_path(args.dictionary.resolve())
            assert render(current, ['transcript']) == render(before, ['transcript'])
            new=render(current, ['transcribe'])[0]
            old=render(before, ['transcribe'])[0]
            assert len(new)==len(old) and new!=old
            checks += 2
        finally:
            current.close()
            before.close()
    for dialect, text in ((0x40000, 'Du wachst, wenn ihr wachst.'),
                          (0x120000, 'Καλημέρα σας!')):
        current, before = make_engine(args.library, dialect), make_engine(args.reference, dialect)
        try:
            assert render(current, [text]) == render(before, [text]), hex(dialect)
            checks += 1
        finally:
            current.close()
            before.close()
    print(f'English s-stop timing: {checks} s, closure, index and isolation checks')


if __name__ == '__main__':
    main()
