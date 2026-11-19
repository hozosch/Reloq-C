"""Exercise sentence-at-a-time Persian speech and preserved NVDA marks."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
import continuous_driver
from _reloq_eci_direct import EciIndex, EciPitch, EciText, NATIVE_SPEED, SAMPLE_RATE
from nvda_driver_probe import PERSIAN_PROBE, make_language_engine


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    args = parser.parse_args()
    home = (PERSIAN_PROBE.id, '1')
    text = 'سلام دنیا. این یک آزمایش است. شما کتاب دارید؟'
    jobs = continuous_driver.sentence_jobs((text,), home, hard_only=True)
    assert len(jobs) == 3, jobs
    for uninterrupted in ('عدد 1.5.0 خوب است', 'ساعت 12:30 است', 'سلام، دوست من؛ خوب هستی',
                          '`[.1sA.0lAm] دنیا. بخش بعدی', 'سلام!دنیا'):
        assert not continuous_driver.sentence_jobs((uninterrupted,), home, hard_only=True), uninterrupted
    assert not continuous_driver.sentence_jobs((EciPitch(75), text), home, hard_only=True)
    assert not continuous_driver.sentence_jobs((EciText(text, character_mode=True),), home, hard_only=True)
    version = continuous_driver.sentence_jobs(('نسخه 1.5.0 آمد. بعد می‌رویم.',), home, hard_only=True)
    assert len(version) == 2, version
    assert len(continuous_driver.sentence_jobs(('سلام: دنیا',), home, hard_only=True)) == 2

    engine = make_language_engine(args.library, args.library, None, Path('/unavailable'), Path('/unavailable'))
    engine.set_parameter(SAMPLE_RATE, 3)
    engine.set_parameter(NATIVE_SPEED, 82)
    def render(parts):
        pcm, marks = bytearray(), []
        assert engine.speak(parts, pcm.extend, lambda value: marks.append((value, len(pcm))))
        return bytes(pcm), marks
    original = continuous_driver.project
    events = []
    def observe(parts, *args, **kwargs):
        events.append('project')
        return original(parts, *args, **kwargs)
    try:
        expected = b''.join(render(job)[0] for job in jobs)
        continuous_driver.project = observe
        pcm, marks = bytearray(), []
        def audio(chunk):
            if chunk and not pcm:
                events.append('first audio')
            pcm.extend(chunk)
        assert engine.speak((text,), audio, lambda value: marks.append((value, len(pcm))))
        assert bytes(pcm) == expected and not marks
        assert events.count('project') == 3 and events.index('first audio') < events.index('project', 1), events
        continuous_driver.project = original
        first, second, third = 'سلام دنیا. ', 'این یک آزمایش است. ', 'شما کتاب دارید؟'
        pcm, marks = render((EciIndex(11), first, EciIndex(12), second, EciIndex(13), third, EciIndex(14)))
        assert pcm and [mark for mark, _ in marks] == [11, 12, 13, 14], marks
        assert marks[0][1] == 0 and marks[-1][1] == len(pcm), marks
        assert all(a[1] <= b[1] for a, b in zip(marks, marks[1:])), marks
    finally:
        continuous_driver.project = original
        engine.close()
    print('Passed Persian sentence boundaries, early first audio and source marks.')


if __name__ == '__main__':
    main()
