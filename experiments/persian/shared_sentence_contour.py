"""Experimental Persian/English pitch plan from one Finnish-based sentence.

This is an offline listening experiment. It uses private, process-wide native
trace hooks and temporary files, and must not be used inside NVDA. English is
rendered by its own voice; the Finnish proxy for English is used only to plan
stress and sentence F0. No proxy PCM is delivered to the listener.
"""
from __future__ import annotations

from contextlib import contextmanager
from dataclasses import dataclass
import os
from pathlib import Path
import re
import sys
import tempfile
import threading
from typing import Callable, Iterator

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, EciIndex, PAUSE_MODE
from mixed_language_probe import last_phone_is_stop, shorten_boundary
from streaming_boundary import StreamingBoundary
from text_frontend import SpeechPart, native_tag_accepted

TRACE = 'RELOQ_FA_F0_TRACE'
OVERRIDE = 'RELOQ_FA_F0_OVERRIDE'
FRAME_TRACE = 'RELOQ_FA_FRAME_TRACE'
ROW = np.dtype([('tick', '<i4'), ('f0', '<f4'), ('av', '<f4'), ('ah', '<f4')])
FRAME_ROW = np.dtype([('tick', '<i4'), ('controls', '<f4', (62,))])


@dataclass(frozen=True)
class Captured:
    pcm: bytes
    f0: np.ndarray
    av: np.ndarray
    indexes: dict[int, int]  # mono sample positions
    frames: np.ndarray | None = None


@dataclass(frozen=True)
class Comparison:
    baseline: bytes
    shared: bytes
    short_pause: bytes
    pilot: Captured
    before: tuple[Captured, ...]
    after: tuple[Captured, ...]
    pilot_boundaries_ms: tuple[int, ...]
    native_cuts: tuple[int, ...]    # sample positions, before terminal pause
    connected_cuts: tuple[int, ...] # include all audible release/decay
    parts: tuple[SpeechPart, ...]
    rate: int


@contextmanager
def native_f0_files(trace: Path, override: Path | None = None,
                    frames: Path | None = None) -> Iterator[None]:
    old = {key: os.environ.get(key) for key in (TRACE, OVERRIDE, FRAME_TRACE)}
    os.environ[TRACE] = os.fspath(trace)
    if override is None:
        os.environ.pop(OVERRIDE, None)
    else:
        os.environ[OVERRIDE] = os.fspath(override)
    if frames is None:
        os.environ.pop(FRAME_TRACE, None)
    else:
        os.environ[FRAME_TRACE] = os.fspath(frames)
    try:
        yield
    finally:
        for key, value in old.items():
            if value is None:
                os.environ.pop(key, None)
            else:
                os.environ[key] = value


@contextmanager
def native_terminal_pause_removed(e: DirectEciEngine) -> Iterator[None]:
    """Read the core's own terminal-pause boundary without changing settings."""
    previous = e._dll.eciGetParam(e._handle, PAUSE_MODE)
    if previous not in (0, 1, 2):
        raise RuntimeError('cannot read native pause setting')
    e.set_parameter(PAUSE_MODE, 1)
    try:
        yield
    finally:
        e.set_parameter(PAUSE_MODE, previous)


def capture(e: DirectEciEngine, parts: list, trace: Path, override: Path | None = None,
            frames: Path | None = None) -> Captured:
    audio, indexes = bytearray(), {}

    def on_index(index: int) -> None:
        if index in indexes:
            raise RuntimeError(f'duplicate index: {index}')
        indexes[index] = len(audio) // 2

    with native_f0_files(trace, override, frames):
        if not e.speak(parts, audio.extend, on_index):
            raise RuntimeError('native synthesis was cancelled')
    if not audio or not trace.is_file():
        raise RuntimeError('missing audio or native F0 trace: use the contour probe build')
    rows = np.fromfile(trace, dtype=ROW)
    if not len(rows) or np.any(np.diff(rows['tick']) <= 0) or rows['tick'][0] < 0:
        raise RuntimeError('invalid native F0 clock')
    if not np.all(np.isfinite(rows['f0'])) or np.any(rows['f0'] <= 0):
        raise RuntimeError('invalid native F0 values')
    ticks = np.arange(int(rows['tick'][-1]) + 1)
    f0 = np.interp(ticks, rows['tick'], rows['f0']).astype(np.float32)
    av = np.interp(ticks, rows['tick'], rows['av']).astype(np.float32)
    frame_rows = None
    if frames is not None:
        if not frames.is_file():
            raise RuntimeError('missing native acoustic frames: use the frame-contour probe')
        frame_rows = np.fromfile(frames, dtype=FRAME_ROW)
        if len(frame_rows) != len(rows) or not np.array_equal(frame_rows['tick'], rows['tick']):
            raise RuntimeError('F0 and native acoustic frame clocks differ')
    return Captured(bytes(audio), f0, av, indexes, frame_rows)


def _proxy_with_display_punctuation(display: str, tags: list[str]) -> str:
    """Reinsert punctuation from the native phone display between proxy tags."""
    matches = list(re.finditer(r'`\[[^]]+\]', display))
    if len(matches) != len(tags):
        raise ValueError('native phone and proxy tag counts differ')
    pieces = []
    previous = 0
    for index, (match, tag) in enumerate(zip(matches, tags)):
        marks = ''.join(char for char in display[previous:match.start()] if char in '.,?!:;')
        if index:
            pieces.append(marks + ' ')
        else:
            pieces.append(marks)
        pieces.append(tag)
        previous = match.end()
    pieces.append(''.join(char for char in display[previous:] if char in '.,?!:;'))
    return ''.join(pieces)


def english_proxy(e: DirectEciEngine, text: str) -> str:
    """Preserve English syllable counts/stresses in an inaudible Finnish proxy.

    English's own ECI phonemes, not eSpeak's, are used for syllable structure.
    Vowels are only rough duration hints: the proxy must never be spoken.
    """
    phonemes = e._phonemes_for_probe(text)
    tags = re.findall(r'`\[([^]]+)\]', phonemes)
    vowel_map = {'i': 'i', 'I': 'i', 'e': 'e', 'E': 'A', 'a': 'a', 'A': 'a',
                 'o': 'o', 'O': 'o', 'u': 'u', 'U': 'u', 'x': 'A', 'X': 'A',
                 'y': 'i', 'Y': 'a', '@': 'A', 'V': 'A'}
    result = []
    for tag in tags:
        syllables = re.findall(r'\.([012])([^.]+)', tag)
        if not syllables:
            raise ValueError(f'English pronunciation has no usable syllable: {tag!r}')
        pieces = []
        for stress, body in syllables:
            vowel = next((vowel_map[ch] for ch in body if ch in vowel_map), 'A')
            pieces.append('.' + stress + 'm' + vowel)
        result.append('`[' + ''.join(pieces) + ']')
    if not result:
        raise ValueError(f'English pronunciation has no phoneme tags: {text!r}')
    if re.search(r'[.?!,;:]\s*\S', text):
        return _proxy_with_display_punctuation(phonemes, result)
    punctuation = re.search(r'([.?!,;:])\s*$', text)
    return ' '.join(result) + (punctuation.group(1) if punctuation else '')


def _last_audible(pcm: bytes, rate: int) -> int:
    samples = np.frombuffer(pcm, '<i2').astype(np.int32)
    audible = np.flatnonzero(np.abs(samples) > 4)
    if not len(audible):
        raise ValueError('silent speech part')
    return round((int(audible[-1]) + 1) * 1000 / rate)


def _continuation_cut(pcm: bytes, native_cut: int, text: str) -> int:
    """Keep native phonetic timing and all audible decay, with no padding."""
    if len(pcm) % 2 or not 0 <= native_cut <= len(pcm) // 2:
        raise ValueError('invalid native pause boundary')
    if text.rstrip().endswith(('.', '?', '!', ',', ';', ':', '،', '؟', '؛')):
        return len(pcm) // 2
    data = np.frombuffer(pcm, '<i2').astype(np.int32)
    audible = np.flatnonzero(np.abs(data) > 4)
    if not len(audible):
        return len(data)
    return max(native_cut, int(audible[-1]) + 1)


def _voiced_interval(item: Captured) -> tuple[int, int]:
    voiced = np.flatnonzero(item.av > 0)
    if not len(voiced):
        raise ValueError('speech part has no voiced pitch anchor')
    return int(voiced[0]), int(voiced[-1])


def _fit_pitch(pilot: Captured, start: int, end: int, item: Captured) -> np.ndarray:
    if not 0 <= start < end < len(pilot.f0):
        raise ValueError(f'invalid pilot interval: {start}, {end}, {len(pilot.f0)}')
    first, last = _voiced_interval(item)
    if last <= first:
        raise ValueError('voiced interval too short')
    # Boundary values are shared exactly. This makes language changes part of
    # one F0 curve, while unvoiced transitions keep each engine's own sound.
    positions = np.linspace(start, end, last - first + 1)
    plan = np.empty(len(item.f0), dtype='<f4')
    plan[:first] = pilot.f0[start]
    plan[first:last + 1] = np.interp(positions, np.arange(len(pilot.f0)), pilot.f0)
    plan[last + 1:] = pilot.f0[end]
    return plan


def compare(parts: tuple[SpeechPart, ...], fa: DirectEciEngine, en: DirectEciEngine,
            rate: int) -> Comparison:
    """Return the independently voiced baseline and shared-F0 comparison."""
    if not parts or any(p.language not in ('fa', 'en-us') for p in parts):
        raise ValueError('expected supported Persian and English parts')
    if len(parts) > 32 or rate <= 0 or {p.language for p in parts} != {'fa', 'en-us'}:
        raise ValueError('unsupported sentence size or language')
    for p in parts[:-1]:
        if p.text.rstrip().endswith(('.', '?', '!', ',', ';', ':', '،', '؟', '؛')):
            raise ValueError('explicit punctuation must be planned as a separate clause')
    with tempfile.TemporaryDirectory(prefix='reloq-fa-shared-f0-') as name:
        folder = Path(name)
        proxy_parts = []
        for index, part in enumerate(parts):
            if index:
                proxy_parts.extend([' ', EciIndex(index)])
            proxy = part.text if part.language == 'fa' else english_proxy(en, part.text)
            for tag in re.findall(r'`\[[^]]+\]', proxy):
                if not native_tag_accepted(tag, fa._phonemes_for_probe(tag)):
                    raise ValueError(f'Finnish planning tag changed by native engine: {tag!r}')
            proxy_parts.append(proxy)
        pilot = capture(fa, proxy_parts, folder / 'pilot.trace')
        boundaries = [0]
        for index in range(1, len(parts)):
            if index not in pilot.indexes:
                raise RuntimeError(f'missing pilot boundary: {index}')
            boundaries.append(round(pilot.indexes[index] * 1000 / rate))
        # A pause-suppressed pilot can end immediately after its last frame;
        # clamp the sample-derived edge to the last valid F0 observation.
        boundaries.append(min(_last_audible(pilot.pcm, rate), len(pilot.f0) - 1))
        if any(a >= b for a, b in zip(boundaries, boundaries[1:])):
            raise RuntimeError(f'non-monotone sentence boundaries: {boundaries!r}')
        voices = {'fa': fa, 'en-us': en}
        before, after, baseline_parts, shared_parts = [], [], [], []
        native_cuts, connected_cuts = [], []
        for index, part in enumerate(parts):
            voice = voices[part.language]
            prior = capture(voice, [part.text], folder / f'before-{index}.trace')
            plan = _fit_pitch(pilot, boundaries[index], boundaries[index + 1], prior)
            override = folder / f'f0-{index}.bin'
            plan.tofile(override)
            adjusted = capture(voice, [part.text], folder / f'after-{index}.trace', override)
            if len(adjusted.pcm) != len(prior.pcm) or len(adjusted.f0) != len(prior.f0):
                raise RuntimeError('pitch plan changed native segment timing')
            if not np.allclose(adjusted.f0, plan, rtol=0, atol=.01):
                raise RuntimeError('pitch plan did not reach the acoustic frames')
            if index + 1 < len(parts):
                # Policy 1 removes only the native terminal phrase pause; it
                # leaves its own stop timing and explicit pauses intact. The
                # signal must remain a byte-identical prefix, otherwise do not
                # risk cutting a changed pronunciation or leaking settings.
                with native_terminal_pause_removed(voice):
                    no_terminal = capture(voice, [part.text], folder / f'native-cut-{index}.trace', override)
                if len(no_terminal.pcm) > len(adjusted.pcm) or not adjusted.pcm.startswith(no_terminal.pcm):
                    raise RuntimeError('native pause policy did not preserve the spoken prefix')
                native_cut = len(no_terminal.pcm) // 2
                connected_cut = _continuation_cut(adjusted.pcm, native_cut, part.text)
            else:
                native_cut = connected_cut = len(adjusted.pcm) // 2
            native_cuts.append(native_cut)
            connected_cuts.append(connected_cut)
            stop = last_phone_is_stop(voice._phonemes_for_probe(part.text))
            baseline_parts.append((prior.pcm, part.text, stop))
            shared_parts.append((adjusted.pcm, part.text, stop))
            before.append(prior)
            after.append(adjusted)
        # Match both pause cuts so the A/B comparison changes pitch, not
        # length. The larger cut point keeps all audible samples from either.
        baseline, short_pause, connected = bytearray(), bytearray(), bytearray()
        for index, (old, new) in enumerate(zip(baseline_parts, shared_parts)):
            connected.extend(new[0][:2 * connected_cuts[index]])
            if index + 1 < len(parts):
                old_cut = shorten_boundary(old[0], rate, old[1], final_stop=old[2])
                new_cut = shorten_boundary(new[0], rate, new[1], final_stop=new[2])
                length = max(len(old_cut), len(new_cut))
                baseline.extend(old[0][:length])
                short_pause.extend(new[0][:length])
            else:
                baseline.extend(old[0])
                short_pause.extend(new[0])
        return Comparison(bytes(baseline), bytes(connected), bytes(short_pause),
                          pilot, tuple(before), tuple(after), tuple(boundaries),
                          tuple(native_cuts), tuple(connected_cuts), parts, rate)


def stream_shared(parts: tuple[SpeechPart, ...], outcome: Comparison,
                  fa: DirectEciEngine, en: DirectEciEngine, rate: int,
                  on_pcm: Callable[[bytes], None], on_index: Callable[[int], None],
                  cancelled: threading.Event | None = None, *, seamless: bool = True) -> bool:
    """Stream a precomputed experiment; indexes here identify parts only.

    A cancellation immediately mutes and discards held PCM, but this research
    adapter does not yet interrupt the native call in progress. It is not an
    NVDA integration and its synthetic indexes are not source positions.
    """
    if parts != outcome.parts or rate != outcome.rate:
        raise ValueError('plan and language parts must agree')
    cancelled = cancelled or threading.Event()

    def deliver(pcm: bytes) -> None:
        if not cancelled.is_set():
            on_pcm(pcm)
        if cancelled.is_set():
            boundary.cancel()

    def deliver_index(index: int) -> None:
        if not cancelled.is_set():
            on_index(index)
        if cancelled.is_set():
            boundary.cancel()

    boundary = StreamingBoundary(rate, deliver, deliver_index)
    voices = {'fa': fa, 'en-us': en}
    with tempfile.TemporaryDirectory(prefix='reloq-fa-stream-f0-') as name:
        folder = Path(name)
        for index, (part, planned) in enumerate(zip(parts, outcome.after)):
            if cancelled.is_set():
                boundary.cancel()
                return False
            voice = voices[part.language]
            plan = folder / f'f0-{index}.bin'
            planned.f0.astype('<f4').tofile(plan)
            remaining = 2 * outcome.connected_cuts[index]

            def deliver_until_native_cut(pcm: bytes) -> None:
                nonlocal remaining
                take = min(remaining, len(pcm))
                if take:
                    deliver(pcm[:take])
                    remaining -= take

            with native_f0_files(folder / f'trace-{index}', plan):
                completed = voice.speak([EciIndex(index + 1), part.text],
                                        deliver_until_native_cut if seamless else boundary.audio,
                                        deliver_index if seamless else boundary.index)
            if not completed or cancelled.is_set():
                boundary.cancel()
                return False
            if seamless:
                if remaining:
                    raise RuntimeError('rendered part shorter than its planned boundary')
            else:
                stop = last_phone_is_stop(voice._phonemes_for_probe(part.text))
                boundary.end_part(part.text, switch=index + 1 < len(parts), final_stop=stop)
    return not cancelled.is_set()
