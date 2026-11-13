"""Inspect an eSpeak SPECTSEQ vowel file's recorded peak frequencies.

This small read-only inspector implements the SPECTSEQ (not SPECTSEK or
SPECTSQ2) layout described by eSpeak NG's spect.c and ieee80.c. It does not
load or distribute eSpeak's data and is not part of the synthesizer.
"""
from __future__ import annotations

import argparse
import hashlib
import math
from pathlib import Path
import struct


def extended80(data: bytes) -> float:
    if len(data) != 10:
        raise ValueError('Truncated 80-bit value')
    word, mantissa = struct.unpack('>HQ', data)
    exponent = word & 0x7fff
    if not exponent and not mantissa:
        return 0.0
    if exponent == 0x7fff:
        raise ValueError('Non-finite 80-bit value')
    sign = -1 if word & 0x8000 else 1
    return sign * math.ldexp(mantissa / (1 << 63), (exponent or 1) - 16383)


def inspect(path: Path) -> None:
    data = path.read_bytes()
    pos = 0

    def take(length: int) -> bytes:
        nonlocal pos
        if length < 0 or pos + length > len(data):
            raise ValueError('Truncated or invalid spectral file')
        item = data[pos:pos + length]
        pos += length
        return item

    if take(8) != b'SPECTSEQ':
        raise ValueError('Only ordinary SPECTSEQ files are supported')
    name_length, = struct.unpack('<I', take(4))
    name = take(name_length).decode('ascii')
    count, amplitude, _, _ = struct.unpack('<hhhh', take(8))
    if not 0 < count <= 100:
        raise ValueError('Unexpected frame count')
    print('File:', path, 'name:', name, 'frames:', count, 'amplitude:', amplitude)
    print('SHA-256:', hashlib.sha256(data).hexdigest())
    for i in range(count):
        time, pitch, length, dx = (extended80(take(10)) for _ in range(4))
        nx, markers, adjustment = struct.unpack('<hhh', take(6))
        peaks = [struct.unpack('<hhhhhh', take(12)) for _ in range(9)]
        take(nx * 2)
        frequencies = [peak[2] for peak in peaks[1:4]]
        print(f'frame {i}: time={time:.3f}s length={length:.1f}ms '
              f'markers={markers} F1/F2/F3={frequencies}')
    if pos != len(data):
        raise ValueError('Unexpected trailing spectral data')


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('path', type=Path)
    inspect(parser.parse_args().path)


if __name__ == '__main__':
    main()
