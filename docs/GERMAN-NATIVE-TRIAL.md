# German native sentence path: experimental work, not a release

This branch is an isolated continuation of `codex/hearing-rc7`. The production
German frontend is unchanged and the native route is enabled only with
`RELOQ_DE_NATIVE_TRIAL=1`.

The experimental change attaches caller-supplied RLEX entries to the native
Root dictionary stage, rather than replacing input text with a phonetic string.
It preserves explicit native Root dictionary precedence, and retains an already
loaded dictionary snapshot across an overlay reload. Tests for German
dictionary compounds, overlay lifecycle, tags, case handling in native Main
dictionaries, discretionary hyphens, and the analysis controls pass locally.
Normal-profile PCM comparisons still match the parent build in 188 cases.

Do not enable this path by default until the remaining compatibility questions
are resolved and the actual output has been evaluated:

* In a male model, word-initial `Zehn` and `Zwei` are about 2 dB louder than
  the accepted parent frontend; the existing phonetic context attenuation is
  not yet applied by the native renderer.
* The experimental overlay folds Root keys to lowercase. Explicit RLEX input
  is case-sensitive and may contain conflicting entries (the existing
  `german_apple_observed.rlex` contains `Sep` and `sep`). Those differences
  must be preserved before this overlay can replace the current implementation.
* Equivalent Euro amount spellings no longer produce identical output in
  `test_eci_numeric_native.py`, which passes on the parent frontend.
* The perceptual problem with vocalic r before /ts/ has not been diagnosed.
  The legacy phone plan recognizes r as a coda; that does not prove the
  resulting sound is correct.
* `test_eci_pause_modes_native.py` currently also fails on the parent frontend;
  the trial fails an earlier pause assertion and needs separate investigation.

This branch is a work checkpoint, not a candidate 1.1 release or a request to
merge. No claim of listening verification is made.
