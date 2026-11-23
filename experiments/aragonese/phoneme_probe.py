"""Experimental, isolated phoneme-level Aragonese pronunciation probes.

This is intentionally not called by the synth. Whole-word dictionary lookup,
foreign words, text/source indices and platform integration remain unsolved.
"""
from __future__ import annotations

import re
import unicodedata

from probe import spanish_probe_input


_GROUP = re.compile(r'`\[[^\]]+\]')


def latent_final_t(word: str, phonemes: str) -> str:
    """Omit latent final t after n/s/r without changing the word stress.

    Work on the final phoneme rather than respelling the word for Spanish:
    a respelling such as *rebos* can move the stress away from its original
    final syllable. This is deliberately limited to one isolated letter word.
    It has not yet been integrated with dictionaries or context.
    """
    word = unicodedata.normalize('NFC', word)
    if not word.isalpha() or not word.casefold().endswith(('nt', 'st', 'rt')):
        return phonemes
    groups = list(_GROUP.finditer(phonemes))
    if not groups:
        return phonemes
    group = groups[-1]
    if not group.group().endswith('t]'):
        return phonemes
    return phonemes[:group.end() - 2] + phonemes[group.end() - 1:]


def probe_word(engine, word: str) -> tuple[str, str]:
    """Return the Castilian starting phones and an isolated candidate."""
    baseline = engine._phonemes_for_probe(word)
    adapted = engine._phonemes_for_probe(spanish_probe_input(word))
    return baseline, latent_final_t(word, adapted)
