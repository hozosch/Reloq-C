# Cross-platform NVDA speech dictionary compatibility

This branch establishes parsing and reference tests. It does not yet enable
NVDA speech-dictionary processing in the SAPI or Android speech pipelines.

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

`common/nvda_dictionary.c` only parses the file structure; it does not compile
patterns or make replacements. Callers must validate regexes as well, and must
avoid treating an ambiguous four-column native Eloquence file as NVDA without
allowing the user to override the detected format.

## Integration checks

The completed feature needs tests for all entry types, regex captures,
case-insensitive Unicode, empty and malformed inputs, BOM and comments,
language and voice scope, source order, and independent operation with
community dictionaries both enabled and disabled. Existing mistakenly selected
NVDA-format files need a migration path that leaves source bytes intact.

SAPI must preserve original source offsets and word, sentence and bookmark
events when replacement text has a different length. Replacements must also
work across its ordinary fragmented speech input. Android must include the
new dictionary data in the device-protected snapshot used before unlock, while
preserving the existing snapshot publication and lifetime rules. In both
adapters the replacement stage must cooperate with word segmentation and not
affect native Eloquence dictionary handling.
