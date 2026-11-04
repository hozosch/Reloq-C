"""Verify native S/Z levels without changing durations or unrelated phones.

Compare a production host against the separate 1.0 host. Listening hosts
are also supported with their extra source experiment disabled. Audio
remains in memory.
"""
from pathlib import Path
import argparse
import ctypes as C
import hashlib
import json
import sys

import numpy as np

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import (DirectEciEngine, SAMPLE_RATE, NATIVE_SPEED,
    NATIVE_PITCH, NATIVE_INFLECTION, NATIVE_VOLUME, GENDER, HEAD_SIZE)

TEXTS = {
    0x40000: 'Susi sieht sieben weiße Segel.',
    0x10000: 'Sue sees seven sisters. Lazy bees buzz.',
    0x10001: 'Sister says seventy six.',
    0x20001: 'Susana usa seis vasos.',
    0x30000: 'Ceci est aussi un son.',
    0x30001: 'Ceci est aussi un son.',
    0x50000: 'Sassi, pasta, stazione.',
    0x70000: 'Seis sapatos são seus.',
    0x90000: 'Susi saa sisua.',
    0x60000: 'si4 si4 si4.',
    0xa0000: '사시사사 쌍.',
}
EXCEPTIONS = {0x20000: 'Susana usa seis vasos.',
              0x80000: 'すしとさかな。', 0x120000: 'Σίσυφος.'}


def engine(path, dialect):
    e = DirectEciEngine(path, dialect=dialect, encoding='utf8',
                        native_voice=True, lexicon_path=None)
    for p, v in ((NATIVE_SPEED, 75), (NATIVE_PITCH, 65),
                 (NATIVE_INFLECTION, 30), (NATIVE_VOLUME, 100),
                 (GENDER, 0), (HEAD_SIZE, 50)):
        e.set_parameter(p, v)
    return e


def capture(e, text):
    chunks = []
    assert e.speak([text], chunks.append, None)
    data = b''.join(chunks)
    x = np.frombuffer(data, dtype='<i2')
    assert len(x)
    return x, dict(samples=len(x), sha256=hashlib.sha256(data).hexdigest(),
        clipped_samples=int(np.count_nonzero((x == 32767) | (x == -32768))))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--baseline', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    library, baseline = args.library.resolve(), args.baseline.resolve()
    dll = C.CDLL(str(library))
    if hasattr(dll, 'reloq_source_listening_set'):
        setter = dll.reloq_source_listening_set
        setter.argtypes = (C.c_uint, C.c_double); setter.restype = C.c_int
        assert setter(0, 1) == 0
    rows, controls = [], []
    for dialect, text in {**TEXTS, **EXCEPTIONS}.items():
        before, after = engine(baseline, dialect), engine(library, dialect)
        try:
            for rate in range(7):
                before.set_parameter(SAMPLE_RATE, rate)
                after.set_parameter(SAMPLE_RATE, rate)
                x, a = capture(before, text); y, b = capture(after, text)
                assert len(x) == len(y), (hex(dialect), rate, 'duration')
                assert b['clipped_samples'] <= a['clipped_samples'], (hex(dialect), rate, 'new clipping')
                delta = float(10 * np.log10(np.mean(y.astype(float) ** 2) / np.mean(x.astype(float) ** 2)))
                if dialect in TEXTS:
                    assert a != b and delta < -.01, (hex(dialect), rate, 'S not reduced', delta)
                else:
                    assert a == b, (hex(dialect), rate, 'native exception changed')
                rows.append(dict(language=hex(dialect), rate=rate,
                    text=text, level_db=delta, before=a, after=b))
                for control in ('mama.', 'papa.', 'fava.'):
                    _, a = capture(before, control); _, b = capture(after, control)
                    assert a == b, (hex(dialect), rate, control)
                    controls.append(dict(language=hex(dialect), rate=rate,
                        text=control, **a))
                if dialect == 0x40000:
                    for control in ('Zeit.', 'Katze.', 'Platz.'):
                        _, a = capture(before, control); _, b = capture(after, control)
                        assert a == b, (rate, control, 'affricate')
                        controls.append(dict(language=hex(dialect), rate=rate,
                            text=control, **a))
        finally:
            before.close(); after.close()
        print(hex(dialect), 'passed', flush=True)
    report = dict(native_s_source_gain_db=0, removed_source_gain_db=4,
        changed_languages=len(TEXTS), unchanged_languages=len(EXCEPTIONS),
        rate_cases=len(rows), unchanged_controls=len(controls), no_audio_files=True,
        baseline_sha256=hashlib.sha256(baseline.read_bytes()).hexdigest(),
        library_sha256=hashlib.sha256(library.read_bytes()).hexdigest(),
        cases=rows, controls=controls)
    args.output.write_text(json.dumps(report, ensure_ascii=False, indent=2) + '\n')


if __name__ == '__main__':
    main()
