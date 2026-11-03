# Shared German sentence path

German uses the same native sentence and acoustic planning path as the other
languages by default. `RELOQ_DE_LEGACY=1` selects the previous German composer
only for diagnostics; native synthesis failures do not automatically fall back.

The caller's case-sensitive RLEX pronunciations are attached to the native
Root stage after explicit native Root entries. Explicit RLEX words over 100
letters are projected before the native lexer splits long tokens; the sentence
continues through the same native rules. Main aliases are normalized before
the sentence rules, and already loaded dictionary snapshots survive source-file
removal and overlay reload. Equivalent EUR/Euro/euro-sign amounts use the
native German currency rule, and phoneme output follows the same normalization.
Native pause boundaries support the existing final and internal silence policy.

The male-only 2-dB /ts/ and /tsv/ correction is carried as acoustic endpoint
metadata and selected at each actual word's voice model. It does not alter the
female path or unrelated word-internal /ts/. Explicit gender changes therefore
remain word-scoped.

For “der Zahn”, the native acoustic rules produce a different vocalic-r
formant trajectory from the old composer. Short-window measurements are
consistent with the original DLL in part of this region; the perceived colour,
other German speech and the final NVDA experience still require listening.
This is a test candidate, not a stable release.
