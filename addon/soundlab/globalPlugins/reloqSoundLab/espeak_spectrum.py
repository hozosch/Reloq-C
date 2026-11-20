"""Read eSpeakEdit spectral sequences without loading the eSpeak runtime.

Only frequencies of active formant peaks are imported into the listening
editor. eSpeak peak heights and widths do not have the same meaning as
Eloquence's serial resonators, so they are retained for inspection only.
"""
from __future__ import annotations

from dataclasses import dataclass
import math
import struct

MAX_BYTES = 8 * 1024 * 1024
MAX_FRAMES = 2048
MAX_NAME = 4096
N_PEAKS = 9
N_KLATT = 14


@dataclass(frozen=True)
class Peak:
    frequency: int
    height: int
    left_width: int
    right_width: int


@dataclass(frozen=True)
class Frame:
    index: int
    time: float
    pitch: float
    length: float
    peaks: tuple[Peak, ...]

    @property
    def active(self):
        return any(p.height > 0 for p in self.peaks)


@dataclass(frozen=True)
class Spectrum:
    name: str
    kind: str
    frames: tuple[Frame, ...]


def _extended80(data):
    word, mantissa = struct.unpack('>HQ', data)
    exponent = word & 0x7fff
    if not exponent and not mantissa:
        return 0.0
    if exponent == 0x7fff:
        raise ValueError('Die Spektraldatei enthält einen ungültigen Zahlenwert.')
    try:
        value = math.ldexp(mantissa / (1 << 63), (exponent or 1) - 16383)
    except OverflowError as exc:
        raise ValueError('Die Spektraldatei enthält einen ungültigen Zahlenwert.') from exc
    return -value if word & 0x8000 else value


def parse(data):
    """Parse SPECTSEQ, SPECTSEK and SPECTSQ2 source files."""
    if not isinstance(data, (bytes, bytearray)) or len(data) > MAX_BYTES:
        raise ValueError('Die Spektraldatei ist ungültig oder zu groß.')
    view = memoryview(data)
    position = 0

    def take(size):
        nonlocal position
        if size < 0 or size > len(view) - position:
            raise ValueError('Die Spektraldatei ist unvollständig.')
        result = view[position:position + size]
        position += size
        return result

    magic = bytes(take(8))
    if magic not in (b'SPECTSEQ', b'SPECTSEK', b'SPECTSQ2'):
        raise ValueError('Keine unterstützte eSpeak-Spektraldatei.')
    name_length, = struct.unpack('<I', take(4))
    if name_length > MAX_NAME:
        raise ValueError('Der Name in der Spektraldatei ist zu lang.')
    name = bytes(take(name_length)).rstrip(b'\0').decode('utf8', errors='replace')
    count, _amplitude, _max_y, _unused = struct.unpack('<hhhh', take(8))
    if not 1 <= count <= MAX_FRAMES:
        raise ValueError('Die Anzahl der Spektralbilder ist ungültig.')
    frames = []
    for index in range(count):
        time, pitch, length, _dx = (_extended80(take(10)) for _ in range(4))
        nx, _markers, _adjustment = struct.unpack('<Hhh', take(6))
        if magic == b'SPECTSQ2':
            take(4)
        peaks = []
        for _ in range(N_PEAKS):
            _estimate, _bandwidth, freq, height, left, right = struct.unpack('<hhhhhh', take(12))
            if magic == b'SPECTSQ2':
                take(6)
            peaks.append(Peak(freq, height, left, right))
        if magic != b'SPECTSEQ':
            take(2 * N_KLATT)
        take(nx * 2)
        frames.append(Frame(index, time, pitch, length, tuple(peaks)))
    return Spectrum(name, magic.decode('ascii'), tuple(frames))


def load(path):
    if path.stat().st_size > MAX_BYTES:
        raise ValueError('Die Spektraldatei ist zu groß.')
    return parse(path.read_bytes())
