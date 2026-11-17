"""Approximate eSpeak pitch on identical ReLoq Persian frames for listening."""
from __future__ import annotations
import argparse
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import wave

import numpy as np
from scipy import signal

ROOT = Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT / 'experiments/persian'), str(ROOT / 'nvda')]
from compare_vowel_corpus import EspeakEvents
from direct_fa_sequence import EciIndex, project
from hybrid_fa_frontend import DirectFirstPersian
from test_us_articulation import capture, voice
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE
from _reloq_persian_frames import FrameCapture, FrameReplay

CASES = ('سلام دنیا.', 'صحبت می‌کنم.', 'این یک آزمایش است.', 'این یک آزمایش است؟')


def replay(engine, scope, frames):
    pcm = bytearray()
    previous = scope(1)
    try:
        assert FrameReplay(engine._dll).replay(engine, frames, (), pcm.extend, lambda _: None)
    finally:
        scope(previous)
    return bytes(pcm)


def estimate(path):
    with wave.open(str(path), 'rb') as w:
        if w.getnchannels() != 1 or w.getsampwidth() != 2:
            raise ValueError('unexpected eSpeak audio format')
        rate = w.getframerate()
        samples = np.frombuffer(w.readframes(w.getnframes()), dtype='<i2').astype(float) / 32768
    samples = signal.sosfiltfilt(signal.butter(4, 900, fs=rate, output='sos'), samples)
    size, hop = int(rate * .05), int(rate * .005)
    lo, hi = int(rate / 300), int(rate / 70)
    times, pitches = [], []
    for offset in range(0, len(samples) - size, hop):
        frame = samples[offset:offset + size]
        frame = frame - frame.mean()
        if np.dot(frame, frame) < .001:
            continue
        correlation = signal.correlate(frame, frame, method='fft')[size - 1:]
        score = []
        for lag in range(lo, hi + 1):
            denominator = np.sqrt(np.dot(frame[:-lag], frame[:-lag]) * np.dot(frame[lag:], frame[lag:]))
            score.append(correlation[lag] / denominator if denominator else 0)
        peaks, _ = signal.find_peaks(score, height=.72, distance=8)
        if peaks.size:
            times.append((offset + size / 2) * 1000 / rate)
            pitches.append(rate / (peaks[0] + lo))
    if len(pitches) < 20:
        raise ValueError('too few periodic eSpeak frames')
    return np.array(times), np.array(pitches)


def pair(text, frontend, fa, scope, events, cli, data, temporary):
    parts = []
    for number, match in enumerate(re.finditer(r'\S+\s*', text), 1):
        parts.extend((EciIndex(number), match.group()))
    job = project(parts, frontend)
    previous = scope(1)
    try:
        captured = FrameCapture(fa._dll).capture(fa, job.parts)
    finally:
        scope(previous)
    frames = captured.frames()
    assert replay(fa, scope, frames) == captured.pcm
    original = [(r.tick, r.controls[1]) for r in frames if r.controls[2] > 10 and r.controls[1] > 0]
    assert len(original) > 20
    target_starts = [sample / 16 for _, sample in captured.indexes]
    assert len(target_starts) == len(parts) // 2
    events.vowels(text, 140)
    donor_starts = [at for kind, _, at, _ in events.events if kind == 1]
    ends = [at for kind, _, at, _ in events.events if kind == 5]
    assert len(target_starts) == len(donor_starts) and len(ends) == 1
    target_edges = target_starts + [original[-1][0]]
    donor_edges = donor_starts + [ends[0]]
    assert all(a < b for a, b in zip(target_edges, target_edges[1:]))
    assert all(a < b for a, b in zip(donor_edges, donor_edges[1:]))
    subprocess.run([str(cli), '--path=' + str(data), '-v', 'fa', '-s', '140', '-w', str(temporary), text],
                   check=True, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
    times, pitches = estimate(temporary)
    def mapped(tick):
        donor_time = np.interp(tick, target_edges, donor_edges)
        return float(np.interp(donor_time, times, pitches)) * 10
    scale = np.median([value for _, value in original]) / np.median([mapped(tick) for tick, _ in original])
    for row in frames:
        if row.controls[1] > 0:
            row.controls[1] = mapped(row.tick) * scale
    candidate = replay(fa, scope, frames)
    assert len(candidate) == len(captured.pcm) and candidate != captured.pcm
    return captured.pcm, candidate, target_edges, donor_edges


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--labels', choices=('de', 'en'), required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--candidate', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--espeak-cli', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    args = parser.parse_args()
    native, data = args.candidate, args.data_parent
    frontend = DirectFirstPersian(native, None, args.espeak_library, data)
    events = EspeakEvents(args.espeak_library, data)
    fa, scope = voice(native, 0x90000)
    labels, label_scope = voice(native, 0x40000 if args.labels == 'de' else 0x10000)
    for engine in (fa, labels):
        for parameter, value in ((SAMPLE_RATE, 3), (NATIVE_SPEED, 65), (GENDER, 0)):
            engine.set_parameter(parameter, value)
    audio = bytearray()
    def append(pcm):
        assert pcm
        audio.extend(pcm)
        audio.extend(bytes(9600))
    def say(text):
        sample, _ = capture(labels, label_scope, text, False)
        append(sample.pcm)
    try:
        if args.labels == 'de':
            say('Vergleich der persischen Melodie. Zuerst die bisherige Version, '
                'dann eine geschätzte eSpeak Tonhöhenkurve auf denselben persischen '
                'Lauten. Sie wird an den Wortgrenzen ausgerichtet. '
                'Betonungspositionen und Lautlängen bleiben gleich. Nur eine Hörprobe.')
        else:
            say('Persian intonation comparison. First the current version, then an estimated '
                'eSpeak pitch contour transferred to the same Persian sounds, aligned at word '
                'boundaries. Stress positions and sound lengths stay the same. Listening experiment only.')
        with tempfile.TemporaryDirectory() as folder:
            for number, text in enumerate(CASES, 1):
                before, after, target, donor = pair(text, frontend, fa, scope, events, args.espeak_cli,
                                                    data, Path(folder) / 'reference.wav')
                say(f'Beispiel {number}. Bisher.' if args.labels == 'de' else f'Example {number}. Current.')
                append(before)
                say('eSpeak Melodie.' if args.labels == 'de' else 'eSpeak pitch contour.')
                append(after)
                print(number, text, 'target edges', target, 'donor edges', donor)
        say('Ende des Vergleichs.' if args.labels == 'de' else 'End of comparison.')
    finally:
        events.close()
        frontend.close()
        fa.close()
        labels.close()
    with wave.open(str(args.output), 'wb') as out:
        out.setnchannels(1)
        out.setsampwidth(2)
        out.setframerate(16000)
        out.writeframes(audio)
    print('Wrote', args.output, 'seconds', round(len(audio) / 32000, 1))


if __name__ == '__main__':
    main()
