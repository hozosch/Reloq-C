"""Private listening experiment for glottalization at Persian word boundaries.

This is not a production rule. It compares the accepted Finnish-based Persian
plan against a gentler opening-quotient trajectory at eligible vowel-initial
words. The eSpeak phone tags, source levels, timing and lexical glottal stops
are preserved. The private frame replay is offline only and unsuitable for NVDA.
"""
from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import re
import tempfile

import numpy as np

from _reloq_eci_direct import DirectEciEngine, EciIndex
from contextual_continuation import _validate_replay_settings, replay
from espeak_ng_probe import EspeakPhonemeProbe
from shared_sentence_contour import capture
from text_frontend import translate_text


_TAG = re.compile(r'`\[[^]]+\]')
_VOWEL_INITIAL = re.compile(r"^`\[\.[012](?:[aAeiou]|'[aAeiou]:')")


@dataclass(frozen=True)
class OnsetComparison:
    baseline: bytes
    softer: bytes
    original_controls: np.ndarray
    softer_controls: np.ndarray
    changed: tuple[tuple[int, int, int, float, float], ...]
    tags: tuple[str, ...]
    indexes: dict[int, int]


def compare_onsets(api: EspeakPhonemeProbe, fa: DirectEciEngine, library: Path,
                   text: str, rate: int, *, gender: int = 0, head: int = 50,
                   retention: float = 0.5) -> OnsetComparison:
    """Soften detected Finnish constriction, without touching explicit /ʔ/."""
    if not 0.0 <= retention <= 1.0:
        raise ValueError('retention must be between zero and one')
    _validate_replay_settings((fa,), rate, gender, head)
    parts = translate_text(api, text)
    if len(parts) != 1 or parts[0].language != 'fa':
        raise ValueError('word-onset comparison needs one Persian-only clause')
    tagged = parts[0].text
    matches = list(_TAG.finditer(tagged))
    if not matches:
        raise ValueError('no Persian word tags')
    sequence: list = []
    cursor = 0
    for i, match in enumerate(matches):
        sequence.append(tagged[cursor:match.start()])
        if i:
            sequence.append(EciIndex(i))
        sequence.append(match.group())
        cursor = match.end()
    sequence.append(tagged[cursor:])

    with tempfile.TemporaryDirectory(prefix='reloq-fa-onsets-') as name:
        path = Path(name)
        unmarked = capture(fa, [tagged], path / 'original.f0', frames=path / 'original.frames')
        marked = capture(fa, sequence, path / 'marked.f0', frames=path / 'marked.frames')
        if unmarked.frames is None or marked.frames is None:
            raise RuntimeError('missing control frames')
        if (unmarked.pcm != marked.pcm or not np.array_equal(unmarked.frames, marked.frames)
                or not np.array_equal(unmarked.f0, marked.f0)):
            raise RuntimeError('word indexes changed the native utterance')
        frames = marked.frames
        original = frames['controls'].copy()
        if replay(library, original, rate, gender, head) != marked.pcm:
            raise RuntimeError('replayed reference differs from native synthesis')
        output = original.copy()
        tick = frames['tick'].astype(np.float64)
        changed = []
        for i in range(1, len(matches)):
            current, previous = matches[i].group(), matches[i - 1].group()
            separator = tagged[matches[i - 1].end():matches[i].start()]
            if (not _VOWEL_INITIAL.match(current) or 'w' in current or 'w' in previous
                    or not separator.isspace()):
                continue
            if i not in marked.indexes:
                raise RuntimeError('missing marked word boundary')
            t = round(marked.indexes[i] * 1000 / rate)
            begin = int(np.searchsorted(tick, t, side='left'))
            limit_t = min(t + 100, round(marked.indexes[i + 1] * 1000 / rate)
                          if i + 1 in marked.indexes else tick[-1] + 1)
            limit = int(np.searchsorted(tick, limit_t, side='left'))
            early_end = int(np.searchsorted(tick, min(t + 45, limit_t), side='left'))
            late_start = int(np.searchsorted(tick, min(t + 60, limit_t), side='left'))
            if early_end <= begin or limit <= late_start:
                continue
            normal = float(np.median(original[late_start:limit, 3]))
            trough = begin + int(np.argmin(original[begin:early_end, 3]))
            minimum = float(original[trough, 3])
            # The female source uses OQ 100 normally and reaches 33 at the
            # same automatic onset; the male defaults are 56 and 18.
            if not 48 <= normal <= 100 or minimum > normal * 0.5 or normal - minimum < 20:
                continue
            recovery = np.flatnonzero(original[trough:limit, 3] >= normal - 1)
            if not len(recovery):
                continue
            end = trough + int(recovery[0]) + 1
            old = original[begin:end, 3]
            output[begin:end, 3] = np.where(
                old < normal, normal - retention * (normal - old), old)
            changed.append((i, begin, end, minimum, normal))
        softer = replay(library, output, rate, gender, head)
        return OnsetComparison(marked.pcm, softer, original, output, tuple(changed),
                               tuple(match.group() for match in matches), marked.indexes)
