"""Measure production S texture and bandwidth; never write audio files.

Requires a C compiler, NumPy and SciPy for development measurements only.
--baseline-ref compares with an earlier Git revision with the same numeric
core headers; use 8cee7c3 for the official 1.0 source.
"""
from pathlib import Path
import argparse
import ctypes as C
import json
import subprocess
import tempfile

import numpy as np
from scipy.signal import welch

ROOT = Path(__file__).resolve().parents[1]
SOURCES = ('reloq_core.c', 'reloq_rate_transfer.c', 'reloq_noise_profile.c',
           'reloq_german_voice_defaults.c', 'reloq_frame_adapter.c')
RATES = (8000, 11025, 16000, 22050, 32000, 44100, 48000)


def build(folder, baseline, compiler):
    sources = []
    for name in SOURCES:
        source = ROOT / 'src' / name
        if baseline:
            source = folder / name
            source.write_bytes(subprocess.check_output(
                ['git', 'show', f'{baseline}:src/{name}'], cwd=ROOT))
        sources.append(source)
    library = folder / 'probe.so'
    subprocess.run([compiler, '-std=c11', '-O2', '-fPIC', '-shared',
                    '-ffp-contract=off', '-I' + str(ROOT / 'include'),
                    '-I' + str(ROOT / 'src'), str(ROOT / 'tests/reloq_sibilant_probe.c'),
                    *map(str, sources), '-lm', '-o', str(library)], check=True)
    native = C.CDLL(str(library))
    native.reloq_test_sibilant_probe.argtypes = (
        C.c_int, C.c_int, C.c_int, C.c_int, C.c_float,
        C.POINTER(C.c_float), C.c_uint)
    native.reloq_test_sibilant_probe.restype = C.c_int
    return native


def capture(native, rate, profile, kind, seed, centre):
    samples = np.empty(rate * 2, dtype=np.float32)
    count = native.reloq_test_sibilant_probe(
        rate, profile, kind, seed, centre,
        samples.ctypes.data_as(C.POINTER(C.c_float)), len(samples))
    assert count > 0, (rate, profile, kind, count)
    return samples[:count]


def measure(samples, rate):
    assert np.isfinite(samples).all()
    # Discard startup. One-ms energy windows measure envelope variation;
    # this is a texture proxy, not a perceptual quality or distortion score.
    samples = samples[len(samples) // 5:].astype(np.float64)
    block = rate // 1000
    blocks = samples[:len(samples) // block * block].reshape(-1, block)
    envelope = np.sqrt(np.mean(blocks * blocks, axis=1))
    frequency, spectrum = welch(samples, fs=rate, nperseg=round(rate * .040))
    total = spectrum.sum()
    assert total > 0 and envelope.mean() > 0
    return dict(rms=float(np.sqrt(np.mean(samples * samples))),
                envelope_cv=float(envelope.std() / envelope.mean()),
                peak_fraction=float(spectrum.max() / total),
                peak_hz=float(frequency[np.argmax(spectrum)]),
                centroid_hz=float((frequency * spectrum).sum() / total),
                below_3000=float(spectrum[frequency < 3000].sum() / total),
                above_5500=float(spectrum[frequency >= 5500].sum() / total),
                above_8000=float(spectrum[frequency >= 8000].sum() / total),
                above_11025=float(spectrum[frequency >= 11025].sum() / total))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline-ref')
    parser.add_argument('--cc', default='cc')
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    records, controls, unchanged = [], 0, 0
    with tempfile.TemporaryDirectory(prefix='reloq-sibilant-measure-') as temp:
        folder = Path(temp)
        current_dir = folder / 'current'
        current_dir.mkdir()
        current = build(current_dir, None, args.cc)
        old = None
        if args.baseline_ref:
            old_dir = folder / 'old'
            old_dir.mkdir()
            old = build(old_dir, args.baseline_ref, args.cc)
        for rate in RATES:
            for profile in (1, 2):
                for centre in (3800, 4200, 4600):
                    for seed in (8, 100):
                        after_pcm = capture(current, rate, profile, 0, seed, centre)
                        after = measure(after_pcm, rate)
                        row = dict(rate=rate, profile=profile, centre=centre,
                                   seed=seed, after=after)
                        if old:
                            before_pcm = capture(old, rate, profile, 0, seed, centre)
                            row['before'] = before = measure(before_pcm, rate)
                            assert len(before_pcm) == len(after_pcm)
                            if rate < 16000:
                                assert np.array_equal(before_pcm, after_pcm)
                                unchanged += 1
                            else:
                                row['level_db'] = float(20 * np.log10(after['rms'] / before['rms']))
                                assert abs(row['level_db']) < .8, row
                        records.append(row)
            # SH, F, a voiced vowel and the 8/11-kHz voiced S keep their
            # original numeric output. Hashes are evidence, not cached audio.
            for kind in (1, 2, 3, 4):
                if kind == 4 and rate >= 16000:
                    continue
                after = capture(current, rate, 1, kind, 8, 4200)
                if old:
                    assert np.array_equal(capture(old, rate, 1, kind, 8, 4200), after), (rate, kind)
                controls += 1
    summary = []
    for rate in RATES:
        rows = [row for row in records if row['rate'] == rate]
        result = dict(rate=rate, cases=len(rows))
        for key in ('envelope_cv', 'peak_fraction', 'centroid_hz', 'below_3000', 'above_5500',
                    'above_8000', 'above_11025'):
            result[key] = float(np.mean([row['after'][key] for row in rows]))
        if old and rate >= 16000:
            result['largest_level_change_db'] = max(abs(row['level_db']) for row in rows)
            for key in ('envelope_cv', 'peak_fraction'):
                before = np.mean([row['before'][key] for row in rows])
                result[key + '_ratio'] = result[key] / before
                assert result[key + '_ratio'] < .95, result
            before_low = np.mean([row['before']['below_3000'] for row in rows])
            assert result['below_3000'] < before_low, result
        summary.append(result)
    extended = [row for row in summary if row['rate'] >= 16000]
    assert all(b['above_5500'] > a['above_5500'] + .008
               for a, b in zip(extended, extended[1:])), extended
    report = dict(baseline_ref=args.baseline_ref, no_audio_files=True,
                  steady_s_cases=len(records), exact_legacy_s_cases=unchanged,
                  exact_other_controls=controls, summary=summary, records=records)
    if args.output:
        args.output.write_text(json.dumps(report, indent=2, allow_nan=False) + '\n')
    print(json.dumps({key: value for key, value in report.items() if key != 'records'}, indent=2, allow_nan=False))


if __name__ == '__main__':
    main()
