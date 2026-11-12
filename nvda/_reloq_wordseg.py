"""Optional English word segmentation, adapted from OpenEVV's MIT code.

The tables and algorithm come from OpenEVV commit 103277c72b4b1a4dc4c5a91fb9d0ddf0d75f87db;
see OpenEVV-NOTICE.txt.  This is a text adapter for NVDA, not an ECI rule.
"""
from __future__ import annotations

from dataclasses import dataclass, field
from functools import lru_cache
from pathlib import Path
import re
import unicodedata

if __package__:
    from ._reloq_dictionary_manager import PREFIXES, SUFFIXES
else:
    from _reloq_dictionary_manager import PREFIXES, SUFFIXES

_WORDS = re.compile(r"[A-Za-z]{2,32}")
_VOWELS = frozenset("aeiou")
_ENDINGS = ("ings", "ing", "ers", "er", "ed", "es", "s")


def _vowel(word, at):
    return word[at] in _VOWELS or (word[at] == "y" and at > 0 and word[at - 1] not in _VOWELS)


@dataclass(frozen=True)
class NativeDictionaryKeys:
    exact: frozenset
    roots: frozenset
    english: bool = False
    compound: dict = field(default_factory=dict)

    def __contains__(self, word):
        # ECI main/abbreviation entries retain case. Its English root lookup
        # sees folded mixed-case words, but all-capital words are treated as
        # abbreviations instead of using the lower-case root entry.
        if word in self.exact:
            return True
        lower = word.casefold()
        if not (self.english and word.isupper()) and lower in self.roots:
            return True
        # Unlike main entries, root entries can cover inflected forms. ECI
        # also checks these roots in upper-case plurals and participles.
        for ending in _ENDINGS:
            if len(lower) <= len(ending) + 2 or not lower.endswith(ending):
                continue
            stem = lower[:-len(ending)]
            if (stem in self.roots or stem + "e" in self.roots or
                len(stem) > 2 and stem[-1] == stem[-2] and stem[:-1] in self.roots):
                return True
        return False


def _in_dictionary(word, dictionary):
    if isinstance(dictionary, NativeDictionaryKeys):
        return word in dictionary
    return word.casefold() in dictionary


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

    def _compound(self, word):
        # A pronounceable technical compound can still need word boundaries.
        # Require two independently known pieces, rather than a word-specific
        # exception or a merely pronounceable arbitrary substring.
        return any(self._word(word[:i]) and self._word(word[i:])
                   for i in range(3, len(word) - 2))

    def _leave(self, word, dictionary, original):
        if len(word) < 2 or len(word) > 32 or not any(_vowel(word, i) for i in range(len(word))):
            return True
        if all(c in "ivxlc" for c in word) or word.startswith("mc"):
            return True
        if word in self.protected or self._word(word) or _in_dictionary(original, dictionary):
            return True
        # An ECI root may also cover inflected forms. Avoid changing a form
        # that could be supplied by the user's active pronunciation data.
        if not isinstance(dictionary, NativeDictionaryKeys) and any(
               word.endswith(ending) and len(word) > len(ending) + 1 and
               _in_dictionary(original[:-len(ending)], dictionary) for ending in _ENDINGS):
            return True
        return self._sayable(word) and not self._compound(word)

    def split(self, word, dictionary=frozenset()):
        original = word
        word = word.lower()
        if self._leave(word, dictionary, original):
            return original
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
                    return original
                value = word[at:end]
                if value in self.abbreviations:
                    value = " ".join(c.upper() for c in value)
                by_letter = letters[end] < other[end]
            output.append(value)
            at = end
        return " ".join(output)

    def text(self, text, dictionary=frozenset(), *, preserve_upper=False):
        def replace(match):
            start, end = match.span()
            # Never split a part of an accented, numeric or apostrophized
            # token; underscore and punctuation delimit names.
            if ((start and (text[start - 1].isalnum() or text[start - 1] in "'’")) or
                (end < len(text) and (text[end].isalnum() or text[end] in "'’"))):
                return match.group()
            if preserve_upper and match.group().isupper():
                return match.group()
            return self.split(match.group(), dictionary)
        return _WORDS.sub(replace, text)


def uppercase_candidates(text, segmenter, dictionary, protected=()):
    """Defer only changed uppercase words until the native voice can inspect them."""
    result = []
    for match in _WORDS.finditer(text):
        start, end = match.span()
        word = match.group()
        if not word.isupper():
            continue
        if ((start and (text[start - 1].isalnum() or text[start - 1] in "'’")) or
                (end < len(text) and (text[end].isalnum() or text[end] in "'’"))):
            continue
        if any(left < end and right > start for left, right in protected):
            continue
        segmented = segmenter.split(word, dictionary)
        if segmented != word:
            result.append((start, end, segmented))
    return tuple(result)


def _word_char(char):
    return char.isalnum() or char in "_'’" or unicodedata.category(char).startswith("M")


def word_runs(text):
    """Yield complete word-like spans, retaining Unicode combining marks."""
    start = None
    for index, char in enumerate(text):
        if _word_char(char):
            if start is None:
                start = index
        elif start is not None:
            yield start, index
            start = None
    if start is not None:
        yield start, len(text)


def _upper(char):
    return char.isupper() or (char.isalpha() and char.istitle())


def plural_acronym(word):
    """Keep an uppercase initialism with a lowercase plural ending intact."""
    if not word.endswith("s"):
        return False
    for ending in ("es", "s"):
        if not word.endswith(ending):
            continue
        stem = word[:-len(ending)]
        letters = [character for character in stem
                   if not unicodedata.category(character).startswith("M")]
        if len(letters) >= 2 and all(_upper(character) for character in letters):
            return True
    return False


def builtin_acronym(word):
    """Recognize initialism plurals and uppercase initialisms with digits.

    Mixed-case names such as iPhone remain ordinary CamelCase unless a
    pronunciation dictionary explicitly identifies them.
    """
    if plural_acronym(word):
        return True
    head = word.rstrip('0123456789')
    if len(head) != len(word) and plural_acronym(head):
        return True
    letters = [c for c in word if not unicodedata.category(c).startswith("M")]
    return (sum(_upper(c) for c in letters) >= 2 and any(c.isdecimal() for c in letters)
            and all(_upper(c) or c.isdecimal() for c in letters))


def camel_word(word):
    """Split lower/upper and acronym/word boundaries without changing case."""
    if plural_acronym(word):
        return word
    boundaries = []
    for index, char in enumerate(word):
        if not _upper(char) or index == 0:
            continue
        before = index - 1
        while before >= 0 and unicodedata.category(word[before]).startswith("M"):
            before -= 1
        after = index + 1
        while after < len(word) and unicodedata.category(word[after]).startswith("M"):
            after += 1
        if before >= 0 and word[before].islower():
            boundaries.append(index)
        elif before >= 0 and _upper(word[before]) and after < len(word) and word[after].islower():
            start = before
            while start >= 0 and (_upper(word[start]) or unicodedata.category(word[start]).startswith("M")):
                start -= 1
            if not plural_acronym(word[start + 1:]):
                boundaries.append(index)
    if not boundaries:
        return word
    pieces, start = [], 0
    for end in boundaries:
        pieces.append(word[start:end])
        start = end
    pieces.append(word[start:])
    return " ".join(pieces)


def split_camel(text, dictionary=frozenset()):
    output, previous = [], 0
    for start, end in word_runs(text):
        word = text[start:end]
        output.append(text[previous:start])
        output.append(word if _in_dictionary(word, dictionary) else camel_word(word))
        previous = end
    output.append(text[previous:])
    return "".join(output)


def transform_unmatched(text, protected, transform):
    """Transform word runs except those touched by a personal dictionary."""
    output, previous = [], 0
    spans, at = sorted(protected), 0
    for start, end in word_runs(text):
        output.append(text[previous:start])
        word = text[start:end]
        while at < len(spans) and spans[at][1] <= start:
            at += 1
        overlaps = at < len(spans) and spans[at][0] < end
        output.append(word if overlaps else transform(word))
        previous = end
    output.append(text[previous:])
    return "".join(output)


def native_compound_spans(text, dictionary):
    """Find case-sensitive native entries that contain punctuation."""
    if not isinstance(dictionary, NativeDictionaryKeys) or not dictionary.compound:
        return ()
    result = []
    for start, end in word_runs(text):
        for word in dictionary.compound.get(text[start:end], ()):
            finish = start + len(word)
            if (text.startswith(word, start) and
                    (finish == len(text) or not _word_char(text[finish]))):
                result.append((start, finish))
    return tuple(result)


def protected_native_spans(text, dictionary, personal_spans=()):
    """Preserve native entries that NVDA's builtin.dic would otherwise split.

    NVDA processes its speech dictionaries before the engine sees the text.
    Personal matches are left as text so NVDA can apply them first.
    """
    previous = 0
    candidates = list(native_compound_spans(text, dictionary))
    for start, end in word_runs(text):
        word = text[start:end]
        if not _in_dictionary(word, dictionary):
            continue
        if camel_word(word) == word and not re.search(r"[^\W\d_]\d", word):
            continue
        candidates.append((start, end))
    spans, at = sorted(personal_spans), 0
    for start, end in sorted(candidates, key=lambda span: (span[0], -span[1])):
        if start < previous:
            continue
        while at < len(spans) and spans[at][1] <= start:
            at += 1
        if at < len(spans) and spans[at][0] < end:
            continue
        if previous < start:
            yield text[previous:start], False
        yield text[start:end], True
        previous = end
    if previous < len(text):
        yield text[previous:], False


def dictionary_keys(directory):
    """Collect protected keys per language from the immutable ECI snapshot."""
    by_prefix = {prefix: {"exact": set(), "roots": set(), "compound": {}} for prefix in PREFIXES.values()}
    if directory is None:
        return {language: NativeDictionaryKeys(frozenset(), frozenset(), prefix in ("ENU", "ENG"))
                for language, prefix in PREFIXES.items()}
    for prefix, groups in by_prefix.items():
        for suffix in SUFFIXES:
            path = Path(directory) / (prefix + suffix + ".dic")
            try:
                with path.open("rb") as stream:
                    for line in stream:
                        key, separator, value = line.partition(b"\t")
                        if separator and value.strip():
                            try:
                                word = key.strip().decode("cp1252")
                            except UnicodeDecodeError:
                                continue
                            if not 1 < len(word) <= 128 or any(c.isspace() for c in word):
                                continue
                            if all(_word_char(c) for c in word):
                                if suffix == "Root":
                                    groups["roots"].add(word.casefold())
                                else:
                                    groups["exact"].add(word)
                            elif suffix in ("main", "abbr"):
                                runs = iter(word_runs(word))
                                first = next(runs, None)
                                if first is not None and first[0] == 0:
                                    groups["compound"].setdefault(word[:first[1]], set()).add(word)
            except FileNotFoundError:
                pass
    return {language: NativeDictionaryKeys(frozenset(by_prefix[prefix]["exact"]),
                                           frozenset(by_prefix[prefix]["roots"]),
                                           prefix in ("ENU", "ENG"),
                                           {key: tuple(sorted(values, key=len, reverse=True))
                                            for key, values in by_prefix[prefix]["compound"].items()})
            for language, prefix in PREFIXES.items()}
