# German publicly checked pronunciation lexicons

The 0.2.12 add-on loads 15,921 exact-case entries from three disjoint files.
`german_apple_dictionary.rlex` contributes 13,379 new records. Its spelling
candidates are selected from checked original German dictionary descriptors;
stored pronunciations come exclusively from fresh public Eloquence phoneme
responses, with each candidate confirmed in isolation and two contexts.
Action numbers and set membership are never decoded into product rules or
assumed to be pronunciations. Original table payloads are not shipped.
Case variants are checked separately. Context-dependent readings and words
that expand into multiple groups are excluded.

The supplement SHA-256 is
`ccaf1e449112a3f4f9485a4efd2b9d1deaea6d48472835bf42ae05bdacf9f6bf`.
All original 2,542 rows remain byte-identical in their source files. The merged
add-on lexicon SHA-256 is
`94f049bdd0db20af2f71d5b8facb1ca4bc0cc28af272c91c78669182354e1ff5`.
All 15,921 rows were checked through native lookup, complete-text annotation,
and ECI text-to-PCM with completion marks. This is a whole-word dictionary,
not the complete original productive morphology. Method and limitations:
[`GERMAN-DICTIONARY-0.2.12-DE.md`](../reference-apple/GERMAN-DICTIONARY-0.2.12-DE.md).

Regenerate this supplement with
`reference-apple/capture_german_dictionary_lexicon.py`, supplying a separate
original `deu.so`, `eci.so`, public phoneme probe, ini working directory,
runtime directory, and both earlier files via `--base`. The original module
revision is guarded by SHA-256. Use `--verify` to check an existing supplement
without rewriting it. Apple modules are needed only for reference checking;
the portable C engine loads the saved lexicon independently.

`german_apple_observed.rlex` has 1,205 complete German word-to-SPR records.
The source JSONL was a user-supplied collection of external ECI public
phoneme responses. Its origin is **not presumed to be Apple**, and the raw
collection is not included in this project. The later German-letter additions
were captured separately with explicit CP1252 input. Every included record
was checked against a fresh public Apple ECI 6.1 German response for that
isolated word. This checks the stored word pronunciations, not
their realization in the C synthesizer.

The exporter selects only successful `dede` records with a single ASCII word
and exactly one complete SPR group. It rejects sentence contexts, expansions,
and non-ASCII spellings in that original source because some labels passed
UTF-8 bytes into a Windows-1252 ECI interface. Separately captured German
letters must explicitly use CP1252. Duplicate words with disagreeing outputs
abort export. The present base-file SHA-256 is
`22c4df3cb699163d22e72d6d2b93e24c61b78055735d9d6efd1bc21eed70b6c5`.

The portable ASCII file starts with `RELOQ-DE-SPR-1`, then contains byte-sorted
lines of uppercase hex-encoded Windows-1252 word bytes, a tab, and uppercase
hex-encoded SPR bytes. `reloq_german_lexicon_file_load_path` validates every
row, ordering, phone symbol and bracket structure before exposing a
caller-owned `reloq_lexicon` view. Its load operation is atomic; bad files do
not replace an earlier valid view. Pass `&owned.view` to
`reloq_german_text_annotate_checked` or `_with_lexicon` and clear the owner
after that call. The experimental native ECI path can load it with
`eciLoadLexicon`; the NVDA add-on passes the packaged file at initialization.
The owned representation holds one row array and one shared byte pool after
loading, rather than allocating every key and pronunciation separately.
A pronunciation group cannot supply missing syllable timing, transitions,
prosody, acoustic parameters, or audio frames.

The earlier 1,081-word snapshot covered 908 spellings beyond the then-current
strict C G2P; that is a historical annotation-coverage measurement. The
current add-on also merges `german_apple_hunspell_context.rlex`, a **separate**
1,337-row set whose public ECI readings were verified in isolation and two
sentence contexts. The original file remains byte-identical. Its spelling
candidate origin, disjoint holdout, verifier, metrics and load cost are recorded
in [`german_context_lexicon_notes.md`](../reference-apple/german_context_lexicon_notes.md).

## Regeneration against the public Apple ECI API

Build `reference-apple/phoneme_capture.c` and arrange the Apple ELF `eci.so`,
its German language module, `eci.ini`, and runtime dependencies separately.
For example, from the repository root:

```sh
cc -std=c11 -O2 -Wall -Wextra -o /tmp/phoneme_capture \
  reference-apple/phoneme_capture.c -ldl
LD_LIBRARY_PATH=/path/to/runtime \
python3 -B tools/export_observed_lexicon.py \
  /path/to/eloquence-bulk-phonemes.jsonl \
  data/german_apple_observed.rlex \
  --cp1252-corrections /path/to/german-apple-cp1252-corrections.jsonl \
  --apple-probe /tmp/phoneme_capture \
  --apple-eci /path/to/apple/eci.so \
  --apple-workdir /path/to/directory-containing-eci.ini \
  --check
```

Omit `--check` to write a new file after reviewing source and comparison
counts. The checked-in data can be loaded and tested **without Apple modules**:

```sh
make test_reloq_german_lexicon_file
./test_reloq_german_lexicon_file
```
