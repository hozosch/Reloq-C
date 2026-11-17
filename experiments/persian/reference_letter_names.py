"""Pinned eSpeak spelling references for the Persian letter-name table."""
from __future__ import annotations

from espeak_worker import EspeakWorker
from text_frontend import UnsupportedPronunciation, word_tag

LETTERS = tuple(sorted('اآأبپتثجچحخدذرزژسشصضطظعغفقکگلمنوهیءۀةئؤإ'
                       '\u064b\u064c\u064d\u064e\u064f\u0650\u0651\u0652\u0654\u0670', key=ord))


def reference_tag(worker: EspeakWorker, char: str) -> str:
    """Use eSpeak's spell mode, excluding its final spelling pause."""
    rows = worker.translate('\x0118Y' + char + '\x01Y', separator='|')
    words = [word for row in rows for word in row.phonemes.split()]
    if len(words) != 1 or not words[0].endswith('|_!'):
        raise UnsupportedPronunciation('letter name has an unsupported word structure')
    phones = words[0][:-3].split('|')
    # eSpeak lengthens these initial consonants. Its colon notation is not
    # accepted directly by the Finnish tag parser; keep the consonant and
    # vowel with ordinary consonant length until a listening comparison.
    if char == 'ر' and phones[0] == 'R:':
        phones[0] = 'R'
    elif char == 'گ' and phones[0] == 'g:':
        phones[0] = 'g'
    return word_tag('|'.join(phones))
