# Cross-platform NVDA speech dictionary compatibility

This branch adds NVDA-format import and speech preprocessing to SAPI and
Android using the same format, language and voice selection as the add-on.
It does not replace either platform's regular-expression implementation with
the NVDA process or claim identical behavior for every Python regex.

## Reference behavior

The reference for this work is the `SpeechDict` and `SpeechDictEntry` in NVDA
2026.3.1, as used by `nvda/_reloq_nvda_dicts.py`. Each UTF-8 file contains
pattern, replacement, case sensitivity and entry type, separated by tabs.
The UTF-8 BOM and comment lines are allowed; `\#` denotes a literal hash.
Entry types 0 through 6 are anywhere, regular expression, whole word, part
of word, start of word, end of word, and shell wildcard, respectively. Files
are selected for a language and either all eight profiles or one profile.
Entries are applied in sequence. A replacement for a non-regex entry is
literal; regex replacements can use numbered and named capture references.

The actual NVDA implementation is the authority for details. In particular,
NVDA 2026.3.1 uses Python `re` for regular-expression entries by default and
the Python `regex` module with VERSION1 semantics for word-boundary types.
Its `speechDictsUseModernRegex` feature flag also changes regex entries to
`regex`. Unicode case folding and word boundaries therefore matter.

An investigation using PCRE2 10.45 found concrete differences from NVDA's
default Python `re`: case-insensitive `\bi\b` matches `İ` and `ı` in Python
but not in PCRE2, and Python's `\w` does not treat a standalone combining
mark as a word character while PCRE2 with Unicode properties does. Word-type
entries in NVDA intentionally use different word-boundary behavior. A simple
replacement of NVDA's regex engine with a PCRE2 call is not sufficient.

`common/nvda_dictionary.c` parses the file structure. The SAPI adapter compiles
and applies entries with PCRE2; Android uses its built-in regex engine. Invalid
expressions are rejected at import. The format can be selected explicitly so
an ambiguous four-column native Eloquence file can remain a native source.

## Integration checks

Host-side tests cover entry types, regex captures, case, malformed inputs,
BOM and comments, language and voice scope, source order, and independent
operation with community dictionaries enabled or disabled. They also cover
mistakenly selected NVDA-format files without modifying the original source.

SAPI host tests check original source offsets, speech events, word segmentation,
and ordinary fragmented input. Android host tests exercise snapshot publication,
the device-protected copy before unlock, and actual JNI synthesis. Windows
installer and Android package builds and platform-specific tests must still pass
before the feature can be considered release-ready.
