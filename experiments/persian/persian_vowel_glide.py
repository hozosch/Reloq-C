"""Private U-to-A inter-word transition comparisons for Persian.

Work with one native, continuously planned clause. The user-approved
first-word onset, word boundaries, explicit glottal phones and native
vowel targets are retained. This replay experiment is not an NVDA path.
"""
from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import re
import sys
import tempfile

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, EciIndex
from _reloq_persian_glide import apply_u_a_glides, plan_u_a_glides
from contextual_continuation import _validate_replay_settings, replay
from espeak_ng_probe import EspeakPhonemeProbe
from shared_sentence_contour import capture
from text_frontend import translate_text


_TAG = re.compile(r'`\[[^]]+\]')


@dataclass(frozen=True)
class VowelGlideComparison:
    baseline: bytes
    glide: bytes
    glide_pitch: bytes
    original_controls: np.ndarray
    glide_controls: np.ndarray
    glide_pitch_controls: np.ndarray
    # word index, first frame, native destination frame, pitch peak/end
    changed: tuple[tuple[int, int, int, int, int], ...]
    indexes: dict[int, int]
    tags: tuple[str, ...]
    ticks: np.ndarray

    @property
    def selected(self) -> bytes:
        """The user's selected second candidate, including its local F0 glide."""
        return self.glide_pitch


def _marked_words(text: str) -> tuple[list, list[re.Match[str]]]:
    matches = list(_TAG.finditer(text))
    if not matches:
        raise ValueError('no Persian phoneme tags')
    sequence: list = []
    cursor = 0
    for i, match in enumerate(matches):
        sequence.append(text[cursor:match.start()])
        if i:
            sequence.append(EciIndex(i))
        sequence.append(match.group())
        cursor = match.end()
    sequence.append(text[cursor:])
    return sequence, matches


def compare_vowel_glide(api: EspeakPhonemeProbe, fa: DirectEciEngine,
                        library: Path, text: str, rate: int, *,
                        gender: int = 0, head: int = 50,
                        anticipation_ms: float = 25.0,
                        pitch_window_ms: float = 45.0) -> VowelGlideComparison:
    """Try a longer native-target U→A glide, and optionally ease local F0.

    The candidates do not insert a /w/ or /ʔ/ phone. They change only the
    existing tract interpolation, plus F0 for the second comparison. The
    initial and settled vowel targets and every frame duration are preserved.
    """
    if not 0 < anticipation_ms <= 50 or not 15 <= pitch_window_ms <= 80:
        raise ValueError('unsupported transition window')
    _validate_replay_settings((fa,), rate, gender, head)
    parts = translate_text(api, text)
    if len(parts) != 1 or parts[0].language != 'fa':
        raise ValueError('vowel-glide experiment needs one Persian-only clause')
    tagged = parts[0].text
    sequence, tags = _marked_words(tagged)
    with tempfile.TemporaryDirectory(prefix='reloq-fa-vowel-glide-') as name:
        folder = Path(name)
        whole = capture(fa, [tagged], folder / 'whole.f0', frames=folder / 'whole.frames')
        marked = capture(fa, sequence, folder / 'marked.f0', frames=folder / 'marked.frames')
        if whole.frames is None or marked.frames is None:
            raise RuntimeError('native acoustic frames unavailable')
        if whole.pcm != marked.pcm or not np.array_equal(whole.frames, marked.frames):
            raise RuntimeError('word markers changed the native clause')
        ticks = marked.frames['tick'].astype(np.float64)
        original = marked.frames['controls'].copy()
        if replay(library, original, rate, gender, head) != marked.pcm:
            raise RuntimeError('reference frame replay differs from native audio')
        glide = original.copy()
        glide_pitch = original.copy()
        tags_text = tuple(match.group() for match in tags)
        separators = tuple(tagged[a.end():b.start()] for a, b in zip(tags, tags[1:]))
        boundaries = {i: round(sample * 1000 / rate) for i, sample in marked.indexes.items()}
        plans = plan_u_a_glides(original, ticks, tags_text, separators, boundaries,
                                anticipation_ms=anticipation_ms, pitch_window_ms=pitch_window_ms)
        apply_u_a_glides(glide, ticks, plans, pitch=False)
        apply_u_a_glides(glide_pitch, ticks, plans)
        changes = tuple(plan.summary for plan in plans)
        return VowelGlideComparison(marked.pcm, replay(library, glide, rate, gender, head),
                                    replay(library, glide_pitch, rate, gender, head),
                                    original, glide, glide_pitch, changes,
                                    marked.indexes, tags_text, ticks)
