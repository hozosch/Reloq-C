"""Request-local Korean pitch pilot for ordinary Persian speech.

The listening comparison used translated Korean sentences. For arbitrary
Persian input, a syllable-count proxy supplies Korean sentence structure;
it is an approximation that still needs native-speaker listening feedback.
No speech result or pitch curve is cached across requests.
"""
from __future__ import annotations

import ctypes
from bisect import bisect_left, bisect_right
import re

if __package__:
    from .._reloq_eci_direct import (DirectEciEngine, GENDER, NATIVE_INFLECTION, NATIVE_PITCH,
                                     NATIVE_SPEED, NATIVE_WPM, PITCH_HZ, PROCESS_TAGS, SAMPLE_RATE)
    from .._reloq_persian_frames import FrameCapture, NativeFrame
else:
    from _reloq_eci_direct import (DirectEciEngine, GENDER, NATIVE_INFLECTION, NATIVE_PITCH,
                                   NATIVE_SPEED, NATIVE_WPM, PITCH_HZ, PROCESS_TAGS, SAMPLE_RATE)
    from _reloq_persian_frames import FrameCapture, NativeFrame

_TAG = re.compile(r'`\[[^]]+\]')
_SYLLABLE = re.compile(r'\.[012]')
_PUNCTUATION = ',;:.!?'


def _punctuation(text):
    explicit = ''.join(char for char in text if char in _PUNCTUATION)
    return explicit or (',' if any(char in '()[]{}' for char in text) else '')


def proxy(tagged: str, *, continuation: bool = False) -> str | None:
    """Keep syllable counts and phrase punctuation without Korean words."""
    matches = tuple(_TAG.finditer(tagged))
    if not matches:
        return None
    parts = []
    for index, match in enumerate(matches):
        # Some explicit phone tags have no syllable marker. They still need
        # a Korean word, rather than dropping the whole sentence's contour.
        count = max(1, len(_SYLLABLE.findall(match.group())))
        if index:
            gap = tagged[matches[index - 1].end():match.start()]
            punctuation = _punctuation(gap)
            parts.append((punctuation if punctuation else '') + ' ')
        parts.append('가' * count)
    last = tagged[matches[-1].end():]
    ending = _punctuation(last)
    parts.append(ending or (',' if continuation else '.'))
    return ''.join(parts)


def _donor_span(frames, leading, syllables, trailing):
    """Select the actual Persian segment from a contextual Korean donor.

    Each proxy syllable normally yields one voiced run. If the Korean core
    merges runs, fall back to a proportional slice of the voiced interval;
    both routes keep the donor's surrounding phrase context during synthesis.
    """
    if not leading and not trailing:
        return 0, len(frames)
    runs, start = [], None
    for i, row in enumerate(frames):
        active = row.controls[2] > 10 and row.controls[1] > 0
        if active and start is None:
            start = i
        elif not active and start is not None:
            runs.append((start, i))
            start = None
    if start is not None:
        runs.append((start, len(frames)))
    if len(runs) == leading + syllables + trailing:
        return runs[leading][0], runs[leading + syllables - 1][1]
    if not runs:
        return 0, 0
    total = leading + syllables + trailing
    first, last = frames[runs[0][0]].tick, frames[runs[-1][1] - 1].tick
    ticks = [row.tick for row in frames]
    begin = first + (last - first) * leading / total
    end = first + (last - first) * (leading + syllables) / total
    return bisect_left(ticks, begin), bisect_right(ticks, end)


class KoreanContour:
    """Own a private Korean donor handle on the existing speech worker."""

    def __init__(self, library):
        self.voice = DirectEciEngine(library, dialect=0xa0000, encoding='utf8', native_voice=True)
        self.voice.set_parameter(PROCESS_TAGS, 1)
        self.api = FrameCapture(self.voice._dll)
        self.parameters = {}

    def cancel(self):
        self.voice.cancel()

    def close(self):
        self.voice.close()

    def apply(self, rows, count, tagged, target_voice, cancelled, *, edges=0, continuation=False,
              preceded=False, prefix=()):
        pilot = proxy(tagged)
        api = self.api
        if pilot is None or api.transfer_korean_pitch is None:
            return False
        if cancelled.is_set():
            raise InterruptedError('Korean pitch planning cancelled')
        for parameter in (SAMPLE_RATE, NATIVE_SPEED, NATIVE_WPM, NATIVE_PITCH, NATIVE_INFLECTION, GENDER):
            value = target_voice._dll.eciGetParam(target_voice._handle, parameter)
            if value >= 0 and self.parameters.get(parameter) != value:
                self.voice.set_parameter(parameter, value)
                self.parameters[parameter] = value
        pitch_hz = getattr(target_voice, '_pitch_hz', 0)
        if self.voice._pitch_hz != pitch_hz:
            self.voice.set_parameter(PITCH_HZ, pitch_hz)
        matches = tuple(_TAG.finditer(tagged))
        syllable_counts = tuple(max(1, len(_SYLLABLE.findall(match.group()))) for match in matches)
        # Synthesize a little Korean context on either side of an embedded
        # Persian span. Otherwise the final Persian word is pronounced as a
        # Korean sentence ending just before English, producing a large fall.
        leading, trailing = (2 if preceded else 0), (2 if continuation else 0)
        if continuation and not _punctuation(tagged[matches[-1].end():]):
            pilot = pilot[:-1]  # remove the proxy's implicit full stop
        pilot = ('가가 ' if leading else '') + pilot + (' 가가.' if trailing else '')
        self.voice._cancelled.clear()
        donor = api.capture(self.voice, (*prefix, pilot), maximum=30000, audio=False)
        if cancelled.is_set() or donor is None:
            raise InterruptedError('Korean pitch planning cancelled')
        frames = donor.frames()
        first, last = _donor_span(frames, leading, sum(syllable_counts), trailing)
        if first >= last:
            return False
        donor_pointer = ctypes.cast(ctypes.byref(frames, first * ctypes.sizeof(NativeFrame)),
                                    ctypes.POINTER(NativeFrame))
        pointer = ctypes.cast(rows, ctypes.POINTER(NativeFrame))
        if api.transfer_korean_aligned is not None and not edges:
            syllables = (ctypes.c_uint32 * len(matches))(*syllable_counts)
            # The accepted original listening sample mapped the whole
            # Korean phrase continuously. Word-by-word time warping changed
            # that melody and proved distracting in the NVDA candidate.
            status = api.transfer_korean_aligned(pointer, count, donor_pointer, last - first, None,
                                                  syllables, len(matches))
        else:
            status = api.transfer_korean_pitch(pointer, count, donor_pointer, last - first, edges, 45)
        if status < 0:
            raise RuntimeError('Korean pitch transfer failed: ' + str(status))
        return status == 0


def contour(owner):
    result = getattr(owner, '_persian_korean_contour', None)
    if result is None:
        result = KoreanContour(owner._dll)
        owner._persian_korean_contour = result
    return result


def align_english_edges(plan, parts, is_bracket_boundary):
    """Fit only English switch edges to their neighboring Persian pitch."""
    offsets = [0]
    for count in plan.counts:
        offsets.append(offsets[-1] + count)
    for edge in range(1, len(parts)):
        left, right = parts[edge - 1].language, parts[edge].language
        if {left, right} != {'fa', 'en-us'} or is_bracket_boundary(parts, edge):
            continue
        a, b, c = offsets[edge - 1:edge + 2]
        li = next((i for i in range(b - 1, a - 1, -1) if plan.frames[i].controls[2] > 10 and
                   plan.frames[i].controls[1] > 0), None)
        ri = next((i for i in range(b, c) if plan.frames[i].controls[2] > 10 and
                   plan.frames[i].controls[1] > 0), None)
        if li is None or ri is None:
            continue
        english_boundary = li if left == 'en-us' else ri
        target_boundary = ri if left == 'en-us' else li
        anchor = plan.frames[english_boundary]
        target = plan.frames[target_boundary].controls[1]
        # Keep the English word's own contour. A large target mismatch used
        # to drag its first or last 45 ms by almost an octave; constrain the
        # correction to two semitones instead of forcing exact equality.
        limit = 2 ** (2 / 12)
        target = max(anchor.controls[1] / limit, min(anchor.controls[1] * limit, target))
        delta = target - anchor.controls[1]
        indexes = range(li, a - 1, -1) if left == 'en-us' else range(ri, c)
        for index in indexes:
            row = plan.frames[index]
            distance = abs(row.tick - anchor.tick)
            if distance > 45:
                break
            if row.controls[2] <= 10 or row.controls[1] <= 0:
                continue
            x = 1.0 - distance / 45.0
            weight = x * x * (3.0 - 2.0 * x)
            row.controls[1] = max(1.0, row.controls[1] + weight * delta)
