"""Offline boundary experiment for Persian and ReLoq US-English chunks.

The normal NVDA engine is unchanged. Preserve the synthesizers' own prosody;
shorten only the long silent tail at an unpunctuated language switch. Real
streaming integration must also preserve indexes, cancellation and latency.
"""
from __future__ import annotations

import re

import numpy as np


def last_phone_is_stop(phones: str) -> bool:
    tags = re.findall(r'`\[([^]]+)\]', phones)
    return bool(tags and re.search(r'[ptk]$', tags[-1]))


def shorten_boundary(pcm: bytes, rate: int, text: str, *, final_stop: bool = False) -> bytes:
    """Keep natural sound, a small gap, and extra silence after a final stop."""
    if not isinstance(rate, int) or rate <= 0 or len(pcm) % 2:
        raise ValueError('expected 16-bit mono PCM and a positive sample rate')
    if text.rstrip().endswith(('.', '?', '!', ',', ';', ':', '،', '؟', '؛')):
        return pcm  # Punctuation is an intentional clause boundary.
    values = np.frombuffer(pcm, dtype='<i2')
    audible = np.flatnonzero(np.abs(values.astype(np.int32)) > 4)
    if not len(audible):
        return pcm
    last = int(audible[-1]) + 1
    remaining = len(values) - last
    # Do not touch a tail that is already short. Only trim up to 500 ms of
    # actual silence, keeping a larger interval for unreleased stop closure.
    keep = round(rate * (0.070 if final_stop else 0.025))
    if remaining <= keep or remaining > rate // 2:
        return pcm
    return pcm[:2 * (last + keep)]


def splice(parts: list[tuple[bytes, str, bool]], rate: int) -> bytes:
    """Parts contain PCM, visible text, and a word-final-stop indicator."""
    if not parts:
        return b''
    output = bytearray()
    for index, (pcm, text, stop) in enumerate(parts):
        output.extend(shorten_boundary(pcm, rate, text, final_stop=stop)
                      if index + 1 < len(parts) else pcm)
    return bytes(output)
