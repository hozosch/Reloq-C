"""Plan the selected Persian U-to-A transition on native control frames.

This module has no dependency on NumPy, native libraries, or global state.
It is a reusable planning building block, not a complete Persian driver.
The caller must supply verified word boundaries and continuously planned
frames; punctuation and explicit glottal phones are never smoothed across.
"""
from __future__ import annotations

from bisect import bisect_left, bisect_right
from dataclasses import dataclass
import re
import struct


TRACT = (9, 10, *range(13, 35))
_FINAL_U = re.compile(r"(?:u|'u:')\]$")
_INITIAL_A = re.compile(r"^`\[\.[012](?:a|'a:')")


def _f32(value: float) -> float:
    """Round an arithmetic result to the format of native control frames."""
    return struct.unpack('=f', struct.pack('=f', value))[0]


def _ease(x: float) -> float:
    x = max(0.0, min(1.0, x))
    return x * x * (3.0 - 2.0 * x)


@dataclass(frozen=True)
class Glide:
    word: int
    first: int
    end: int
    peak: int
    pitch_end: int
    start_values: tuple[float, ...]
    differences: tuple[float, ...]
    pitch_start: float = 0.0
    pitch_difference: float = 0.0

    @property
    def summary(self) -> tuple[int, int, int, int, int]:
        return self.word, self.first, self.end, self.peak, self.pitch_end


def plan_u_a_glides(controls, ticks, tags, separators, boundaries_ms, *,
                    anticipation_ms: float = 25.0,
                    pitch_window_ms: float = 45.0) -> tuple[Glide, ...]:
    """Identify safe transitions using the original, unmodified frames.

    ``boundaries_ms[i]`` is the start time of word ``i`` in the same clock
    as ``ticks``; word 0 starts at tick zero. ``separators[i-1]`` is the text
    between words ``i-1`` and ``i``. A missing eligible boundary is an error,
    never a reason to guess its position. The native frame layout is 62 floats.
    """
    if not 0 < anticipation_ms <= 50 or not 15 <= pitch_window_ms <= 80:
        raise ValueError('unsupported transition window')
    if len(controls) != len(ticks) or not len(ticks):
        raise ValueError('nonempty frames and matching ticks are required')
    if len(separators) != max(0, len(tags) - 1):
        raise ValueError('word separators do not match word tags')
    if len(controls[0]) != 62:
        raise ValueError('expected 62 native controls per frame')
    occupied = [False] * len(controls)
    result = []
    for i in range(1, len(tags)):
        previous, current = tags[i - 1], tags[i]
        if (not _FINAL_U.search(previous) or not _INITIAL_A.match(current)
                or 'w' in previous or 'w' in current or not separators[i - 1].isspace()):
            continue
        if i not in boundaries_ms or (i > 1 and i - 1 not in boundaries_ms):
            raise RuntimeError('missing vowel word boundary')
        t0 = boundaries_ms[i]
        left_t = boundaries_ms[i - 1] if i > 1 else 0
        right_t = boundaries_ms.get(i + 1, float(ticks[-1]) + 1)
        boundary = bisect_left(ticks, t0)
        # Never consume the whole first word, including its utterance onset.
        earliest = max(left_t + (t0 - left_t) / 2, t0 - anticipation_ms)
        first = bisect_left(ticks, earliest)
        limit = bisect_left(ticks, min(right_t, t0 + 70))
        if boundary - first < 3 or limit - boundary < 8:
            continue
        for j in range(first, limit):
            row = controls[j]
            if row[2] <= 0 or row[8] != 0 or row[6] != 0:
                limit = j
                break
        if limit - boundary < 8:
            continue
        # End at the first A plateau, never a later vowel in the same word.
        end = boundary
        rising = False
        for j in range(boundary + 1, limit):
            delta = _f32(float(controls[j][9]) - float(controls[j - 1][9]))
            if delta > 0.25:
                rising = True
                end = j
            elif rising or float(ticks[j]) - t0 > 5:
                break
        if (end - boundary < 4 or float(ticks[end]) - t0 > 65 or
                float(controls[end][9]) - float(controls[boundary][9]) < 100 or
                float(controls[end][13]) - float(controls[boundary][13]) < 100 or
                any(occupied[first:end + 1]) or ticks[end] <= ticks[first]):
            continue
        for j in range(first, end + 1):
            occupied[j] = True
        start_values = tuple(float(controls[first][column]) for column in TRACT)
        differences = tuple(_f32(float(controls[end][column]) - value)
                            for column, value in zip(TRACT, start_values))

        # Preserve the selected peak and the later native F0 endpoint.
        peak_limit = min(limit, bisect_right(ticks, t0 + 15))
        peak = max(range(boundary, peak_limit), key=lambda j: controls[j][1])
        after_target = min(15.0, max(0.0, (float(ticks[end]) - t0) / 2))
        pitch_until = min(t0 + pitch_window_ms, float(ticks[end]) + after_target)
        pitch_end = min(limit - 1, bisect_left(ticks, pitch_until))
        pitch_start = pitch_difference = 0.0
        if (pitch_end - peak >= 3 and ticks[pitch_end] > ticks[peak] and
                _f32(float(controls[peak][1]) - float(controls[pitch_end][1])) >= 50):
            pitch_start = float(controls[peak][1])
            pitch_difference = _f32(float(controls[pitch_end][1]) - pitch_start)
        else:
            peak = pitch_end = -1
        result.append(Glide(i, first, end, peak, pitch_end, start_values,
                            differences, pitch_start, pitch_difference))
    return tuple(result)


def apply_u_a_glides(controls, ticks, glides: tuple[Glide, ...], *, pitch: bool = True) -> None:
    """Apply planned transitions in place; ``pitch=True`` is selected variant 2."""
    for glide in glides:
        span = float(ticks[glide.end]) - float(ticks[glide.first])
        for frame in range(glide.first, glide.end + 1):
            progress = _ease((float(ticks[frame]) - float(ticks[glide.first])) / span)
            row = controls[frame]
            for column, start, difference in zip(TRACT, glide.start_values, glide.differences):
                row[column] = _f32(start + progress * difference)
        if pitch and glide.peak >= 0:
            span = float(ticks[glide.pitch_end]) - float(ticks[glide.peak])
            for frame in range(glide.peak, glide.pitch_end + 1):
                progress = _ease((float(ticks[frame]) - float(ticks[glide.peak])) / span)
                controls[frame][1] = _f32(glide.pitch_start + progress * glide.pitch_difference)
