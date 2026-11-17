"""Request-local Korean pitch pilot for ordinary Persian speech.

The listening comparison used translated Korean sentences. For arbitrary
Persian input, a syllable-count proxy supplies Korean sentence structure;
it is an approximation that still needs native-speaker listening feedback.
No speech result or pitch curve is cached across requests.
"""
from __future__ import annotations

import ctypes
import re

if __package__:
    from .._reloq_eci_direct import (DirectEciEngine, GENDER, NATIVE_INFLECTION, NATIVE_PITCH,
                                     NATIVE_SPEED, NATIVE_WPM, SAMPLE_RATE)
    from .._reloq_persian_frames import FrameCapture, NativeFrame
else:
    from _reloq_eci_direct import (DirectEciEngine, GENDER, NATIVE_INFLECTION, NATIVE_PITCH,
                                   NATIVE_SPEED, NATIVE_WPM, SAMPLE_RATE)
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
        count = len(_SYLLABLE.findall(match.group()))
        if not count:
            return None
        if index:
            gap = tagged[matches[index - 1].end():match.start()]
            punctuation = _punctuation(gap)
            parts.append((punctuation if punctuation else '') + ' ')
        parts.append('가' * count)
    last = tagged[matches[-1].end():]
    ending = _punctuation(last)
    parts.append(ending or (',' if continuation else '.'))
    return ''.join(parts)


class KoreanContour:
    """Own a private Korean donor handle on the existing speech worker."""

    def __init__(self, library):
        self.voice = DirectEciEngine(library, dialect=0xa0000, encoding='utf8', native_voice=True)
        self.api = FrameCapture(self.voice._dll)
        self.parameters = {}

    def cancel(self):
        self.voice.cancel()

    def close(self):
        self.voice.close()

    def apply(self, rows, count, tagged, target_voice, cancelled, *, edges=0, continuation=False):
        pilot = proxy(tagged, continuation=continuation)
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
        self.voice._cancelled.clear()
        donor = api.capture(self.voice, (pilot,), maximum=30000, audio=False)
        if cancelled.is_set() or donor is None:
            raise InterruptedError('Korean pitch planning cancelled')
        frames = donor.frames()
        pointer = ctypes.cast(rows, ctypes.POINTER(NativeFrame))
        status = api.transfer_korean_pitch(pointer, count, frames, len(frames), edges, 45)
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
        delta = plan.frames[target_boundary].controls[1] - anchor.controls[1]
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
