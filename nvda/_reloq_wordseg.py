"""Optional English word segmentation, adapted from OpenEVV's MIT code.

The tables and algorithm come from OpenEVV commit 103277c72b4b1a4dc4c5a91fb9d0ddf0d75f87db;
see OpenEVV-NOTICE.txt.  This is a text adapter for NVDA, not an ECI rule.
"""
from __future__ import annotations

from functools import lru_cache
from pathlib import Path
import re

_WORDS = re.compile(r"[a-z]{2,32}")
_VOWELS = frozenset("aeiou")
_ENDINGS = ("ings", "ing", "ers", "er", "ed", "es", "s")


def _vowel(word, at):
    return word[at] in _VOWELS or (word[at] == "y" and at > 0 and word[at - 1] not in _VOWELS)


class WordSegmenter:
    def __init__(self, onsets, codas, protected, pieces, abbreviations=()):
        self.onsets, self.codas = frozenset(onsets), frozenset(codas)
        self.protected, self.pieces = frozenset(protected), frozenset(pieces)
        self.abbreviations = frozenset(abbreviations)

    @classmethod
    def from_files(cls, data_dir, language):
        root = Path(data_dir)
        groups = {"onset": set(), "coda": set(), "word": set()}
        for line in (root / (language + "-sayable")).read_text(encoding="utf-8").splitlines():
            if not line or line.startswith("#"):
                continue
            kind, word = line.split(maxsplit=1)
            groups[kind].add(word)
        pieces, abbreviations, section = set(), set(), False
        for line in (root / (language + "-pieces")).read_text(encoding="utf-8").splitlines():
            if line.startswith("# Abbreviations said by their letters"):
                section = True
            elif line.startswith("# Prefixes"):
                section = False
            elif line.strip() and not line.startswith("#"):
                pieces.add(line.strip())
                if section: abbreviations.add(line.strip())
        return cls(groups["onset"], groups["coda"], groups["word"], pieces, abbreviations)

    @lru_cache(maxsize=4096)
    def _coda(self, value):
        if not value or value in self.codas:
            return True
        collapsed = "k" if value == "q" else re.sub(r"(.)\1+", r"\1", value)
        return collapsed != value and collapsed in self.codas

    @lru_cache(maxsize=16384)
    def _sayable(self, word):
        n = len(word)
        first = next((i for i in range(n) if _vowel(word, i)), None)
        if first is None or (first and word[:first] not in self.onsets):
            return False
        last = n - 1
        while not _vowel(word, last):
            last -= 1
        if not self._coda(word[last + 1:]):
            return False
        i = first
        while i < last:
            while i < last and _vowel(word, i):
                i += 1
            if i == last:
                break
            j = i
            while j < n and not _vowel(word, j):
                j += 1
            run = word[i:j]
            if not any(self._coda(run[:k]) and (not run[k:] or run[k:] in self.onsets)
                       for k in range(len(run) + 1)):
                return False
            i = j
        return True

    def _stem(self, word):
        if len(word) < 2:
            return False
        if word in self.pieces or word + "e" in self.pieces:
            return True
        return len(word) > 3 and word[-1] == word[-2] and word[-1] not in _VOWELS and word[:-1] in self.pieces

    @lru_cache(maxsize=16384)
    def _word(self, word):
        if word in self.pieces:
            return True
        for ending in _ENDINGS:
            if len(word) < len(ending) + 3 or not word.endswith(ending):
                continue
            if ending == "s" and word[-2] == "s":
                continue
            if ending == "es" and word[-3] not in "sxzh":
                continue
            if self._stem(word[:-len(ending)]):
                return True
        return False

    def _leave(self, word, dictionary):
        if len(word) < 2 or len(word) > 32 or not any(_vowel(word, i) for i in range(len(word))):
            return True
        if all(c in "ivxlc" for c in word) or word.startswith("mc"):
            return True
        if word in self.protected or word in self.pieces or word in dictionary:
            return True
        # An ECI root may also cover inflected forms. Avoid changing a form
        # that could be supplied by the user's active pronunciation data.
        if any(word.endswith(ending) and len(word) > len(ending) + 1 and
               word[:-len(ending)] in dictionary for ending in _ENDINGS):
            return True
        return self._sayable(word)

    def split(self, word, dictionary=frozenset()):
        if self._leave(word, dictionary):
            return word
        n = len(word)
        inf = 0x3fffffff
        letters, other, target = [inf] * (n + 1), [inf] * (n + 1), [0] * (n + 1)
        other[n] = 0
        for i in range(n - 1, -1, -1):
            letters[i] = 30 + min(letters[i + 1], other[i + 1] + 1)
            for j in range(i + 2, n + 1):
                piece = word[i:j]
                cost = 39 if self._word(piece) else 132 if len(piece) >= 3 and self._sayable(piece) else inf
                score = min(letters[j], other[j]) + cost
                if score < other[i]:
                    other[i], target[i] = score, j
        by_letter, at, output = letters[0] <= other[0], 0, []
        while at < n:
            if by_letter:
                value, end = word[at].upper(), at + 1
                if word[at] == "a": value = "ay"
                elif word[at] == "i": value = "igh"
                by_letter = letters[end] < other[end] + 1
            else:
                end = target[at]
                if end <= at:
                    return word
                value = word[at:end]
                if value in self.abbreviations:
                    value = " ".join(c.upper() for c in value)
                by_letter = letters[end] < other[end]
            output.append(value)
            at = end
        return " ".join(output)

    def text(self, text, dictionary=frozenset()):
        def replace(match):
            start, end = match.span()
            # Never split a part of a mixed-case, accented, numeric or
            # apostrophized token; underscore and punctuation delimit names.
            if ((start and (text[start - 1].isalnum() or text[start - 1] in "'’")) or
                (end < len(text) and (text[end].isalnum() or text[end] in "'’"))):
                return match.group()
            return self.split(match.group(), dictionary)
        return _WORDS.sub(replace, text)


def dictionary_keys(directory):
    """Collect exact protected keys from the same immutable ECI snapshot."""
    result = {"enus": set(), "engb": set()}
    if directory is None:
        return {k: frozenset() for k in result}
    for language, prefix in (("enus", "ENU"), ("engb", "ENG")):
        for suffix in ("main", "Root", "abbr"):
            path = Path(directory) / (prefix + suffix + ".dic")
            try:
                with path.open("rb") as stream:
                    for line in stream:
                        key, separator, value = line.partition(b"\t")
                        if separator and value.strip():
                            try:
                                word = key.strip().decode("cp1252").lower()
                            except UnicodeDecodeError:
                                continue
                            if 1 < len(word) <= 32 and word.isascii() and word.isalpha():
                                result[language].add(word)
            except FileNotFoundError:
                pass
    return {k: frozenset(v) for k, v in result.items()}
