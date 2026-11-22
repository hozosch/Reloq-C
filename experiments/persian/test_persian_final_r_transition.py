"""Exercise narrow final-R resonance smoothing at mixed-language joins."""
from __future__ import annotations

import argparse
import ctypes
from pathlib import Path
import subprocess
import sys
import tempfile
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
import continuous_plan
from _reloq_eci_direct import EciIndex, GENDER, NATIVE_SPEED, SAMPLE_RATE
from _reloq_persian_frames import NativeFrame
from nvda_driver_probe import make_language_engine
from test_persian_r_after_i_contexts import capture
from test_persian_final_r_connected import actual_sentence, r_frames
from test_us_articulation import voice


def unit(library, name):
    fn = getattr(ctypes.CDLL(str(library)), name)
    fn.argtypes = (ctypes.POINTER(NativeFrame), ctypes.c_size_t, ctypes.c_size_t, ctypes.c_size_t)
    fn.restype = ctypes.c_int
    rows = (NativeFrame * 30)()
    for i, row in enumerate(rows):
        row.controls[0], row.controls[2], row.controls[7] = 1, 25, 20
        for col, value in ((9, 450), (13, 1500), (15, 2200), (17, 2900 if i < 5 else 3600)):
            row.controls[col] = value
    old = (NativeFrame * 30).from_buffer_copy(bytes(rows))
    assert fn(rows, 30, 5, 30) == 1
    assert abs(rows[5].controls[17] - rows[4].controls[17]) < 40
    assert rows[14].controls[17] == old[14].controls[17]
    for i in range(30):
        for col in range(62):
            if 5 <= i < 14 and col == 17:
                continue
            assert rows[i].controls[col] == old[i].controls[col], (i, col)
    for reason in ('small_jump', 'silence', 'short'):
        rows = (NativeFrame * 30).from_buffer_copy(bytes(old))
        if reason == 'small_jump': rows[5].controls[17] = 3000
        elif reason == 'silence': rows[5].controls[2] = rows[5].controls[7] = 0
        else:
            for i in range(7, 30): rows[i].controls[2] = rows[i].controls[7] = 0
        before = bytes(rows)
        assert fn(rows, 30, 5, 30) == 0, reason
        assert bytes(rows) == before
    assert fn(rows, 30, 0, 30) == -1


def connected_release(library, previous):
    current, scope = voice(library, 0x90000)
    prior, old_scope = voice(previous, 0x90000) if previous else (None, None)
    checks = 0
    try:
        for rate in (1, 3, 6):
            for gender in (0, 1):
                for speed in (40, 82, 100, 170):
                    for engine in (current, prior):
                        if engine:
                            for key, value in ((SAMPLE_RATE,rate),(GENDER,gender),(NATIVE_SPEED,speed)):
                                engine.set_parameter(key,value)
                    # No short /æ/ here: the older reference predates the
                    # user's separately approved short-æ duration change.
                    tag = '`[.0zo.1hur] `[.0nu.0ge.1hi.0ni]'
                    result = capture(current, scope, tag)
                    start, end = (round(x/10000) for x in result.syllables[3])
                    rows = result.frames()
                    assert any(start <= row.tick <= end and row.controls[8] >= 40 for row in rows)
                    if (rate,gender,speed) == (3,0,82):
                        by_tick = {row.tick: row.controls for row in rows}
                        assert by_tick[end][17] == 2900
                        assert by_tick[end+1][17] == 2940
                        assert by_tick[end+10][17] == 3300
                        assert by_tick[end+18][17] == 3600
                    if prior:
                        old = capture(prior, old_scope, tag)
                        assert old.syllables == result.syllables
                        before = old.frames()
                        assert len(before) == len(rows)
                        for row, reference in zip(rows,before):
                            assert row.tick == reference.tick
                            for col in range(62):
                                if row.tick > end and col in (15, 17, 19):
                                    continue
                                assert row.controls[col] == reference.controls[col], (
                                    rate,gender,speed,row.tick,col)
                        if (rate,gender,speed) == (3,0,82):
                            # Dev30 already limited F4. F3 and F5 must also
                            # change in the actual R release, not in the R's
                            # steady portion or in a synthetic-only fixture.
                            for column in (15, 19):
                                changed = [row.tick for row, reference in zip(rows,before)
                                           if row.controls[column] != reference.controls[column]]
                                assert changed and min(changed) == end+1 and max(changed) < end+20, (column,changed)
                                interval = [row.controls[column] for row in rows
                                            if end <= row.tick <= max(changed)+1]
                                assert max(abs(a-b) for a,b in zip(interval,interval[1:])) <= 40.01
                    checks += 1
    finally:
        current.close()
        if prior: prior.close()
    sentence = actual_sentence(library, 'ظهور ناگهانی')
    assert len(r_frames(sentence)) >= 5, 'the reported sentence lost its final R'
    print(f'Passed {checks} connected final-R releases, source/timing preservation and the reported sentence.')


def integration(library):
    owner = make_language_engine(library, library, None, Path('/unavailable'), Path('/unavailable'))
    original = continuous_plan.smooth_final_r_joins
    selected = []
    def observe(plan, parts):
        before = [tuple(row.controls) for row in plan.frames]
        counts, marks = plan.counts, plan.source_marks
        changed = original(plan, parts)
        assert (plan.counts, plan.source_marks) == (counts, marks)
        bounds = continuous_plan._edges_by_count(counts)
        for edge in changed:
            at = bounds[edge-1]
            assert before[at-1] == tuple(plan.frames[at-1].controls)
            assert any(abs(before[at][c]-before[at-1][c]) >= 150 and
                       abs(plan.frames[at].controls[c]-before[at-1][c]) <
                       abs(before[at][c]-before[at-1][c]) for c in (9,13,15,17))
            for i, row in enumerate(plan.frames):
                for col in range(62):
                    if at <= i < at+10 and col in (9,13,15,17):
                        continue
                    assert row.controls[col] == before[i][col], (i-at, col)
            selected.append((parts[edge-1].text, parts[edge].text))
        return changed
    try:
        for rate, gender, speed in ((1,0,40),(3,0,65),(3,0,82),(6,1,100)):
            for k, v in ((SAMPLE_RATE,rate),(GENDER,gender),(NATIVE_SPEED,speed)):
                owner.set_parameter(k,v)
            for text in ('آذر heading', 'ظهور heading', 'قرمز heading', 'روح zero'):
                before = len(selected); audio, marks = bytearray(), []
                with patch.object(continuous_plan,'smooth_final_r_joins',new=observe):
                    assert owner.speak((EciIndex(11),text,EciIndex(12)), audio.extend,
                                       lambda value: marks.append((value,len(audio))))
                assert audio and marks == [(11,0),(12,len(audio))], (text, marks)
                if text in ('قرمز heading','روح zero'):
                    assert len(selected) == before, text
        assert selected and any('heading' in right for _, right in selected), selected
    finally:
        owner.close()
    print(f'Passed final-R resonance-only joins at {len(selected)} actual switches.')


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--library', type=Path)
    ap.add_argument('--previous', type=Path)
    args = ap.parse_args()
    with tempfile.TemporaryDirectory(prefix='reloq-final-r-') as tmp:
        if args.library:
            library, name = args.library.resolve(), 'eciPersianSmoothFinalRJoin'
        else:
            library, name = Path(tmp)/'transition.so', 'reloq_persian_smooth_final_r_join'
            subprocess.run(['cc','-shared','-fPIC','-std=c11','-O2','-Wall','-Wextra','-Werror',
                            '-I',str(ROOT/'include'),'-I',str(ROOT/'src'),
                            str(ROOT/'src/reloq_persian_transition.c'),'-lm','-o',str(library)],check=True)
        unit(library,name)
        if args.library:
            connected_release(library, args.previous.resolve() if args.previous else None)
            integration(library)
    print('Passed final-R resonance guards and source/timing preservation.')

if __name__ == '__main__':
    main()
