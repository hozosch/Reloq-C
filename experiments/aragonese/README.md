# Aragonese: isolated Spanish-backend probe

This is an initial research probe, not an available voice or a production
Aragonese language implementation. It does not modify the Spanish voice, the
language catalogue, user settings or release packages. Invoke it explicitly:

```
python3 experiments/aragonese/test_probe.py
python3 experiments/aragonese/probe.py --library eci/libreloq_eci.so --wav /tmp/aragonese-probe.wav
python3 experiments/aragonese/compare_sh.py --library eci/libreloq_eci.so --wav /tmp/aragonese-sh-comparison.wav
python3 experiments/aragonese/castilian_sh_listening.py --library eci/libreloq_eci.so --wav /tmp/aragonese-castilian-sh.wav
python3 experiments/aragonese/inventory.py --library eci/libreloq_eci.so
# After building the ordinary Linux provider, make separate disposable probes:
python3 experiments/aragonese/build_native_probe.py sh
python3 experiments/aragonese/build_native_probe.py au
python3 experiments/aragonese/build_native_probe.py refine
```

The first experiment maps Aragonese **ny** to the Spanish spelling **ñ**,
which represents the same palatal nasal in the reference orthography. It
then asks the existing Castilian Spanish voice for phonemes and optional PCM.
The mapping is not safe as a general text preprocessor: names and borrowed
words, character spelling, source offsets, language changes and dictionaries
need dedicated treatment.

In particular, **x/ix**, **ll**, regional alternatives and word stress need
phonetic investigation. Numbers, dates, symbols and character names must be
localized rather than left to Spanish. The experiment does not establish
that the Spanish backend pronounces the full Aragonese words correctly.

The separate, labelled listening comparison uses the existing Latin American
Spanish /tʃ/ (as in *ocho*) and /ʃ/ (as in *show*) in manually specified
phoneme contexts. It also includes German *acht*, because the intended source
of the suggestion "8" is ambiguous. This tests how the existing sounds differ;
it does not claim to have derived a new /ʃ/ sound from either pronunciation
of eight. The whole recording uses the unchanged native providers.

## Castilian-base listening sketch

Aragonese is diverse: the Academy's 2023 orthography describes /θ/ for many
varieties, but /s/ for Benasqués; Mott's Chistabino description records an
apical /s/. The choice of Castilian Spanish as a first acoustic basis is an
experiment, not a claim that its /s/ represents every speaker. Castilian
already has /θ/, but its provider does not accept the Mexican /ʃ/ phoneme.
The 2023 official orthography describes the ordinary s as an alveolar /s/;
it does not prescribe whether its articulation is apical. Chistabino's apical
variant is evidence for one variety, not a standard pronunciation for all.

`castilian_sh_listening.py` creates a labelled, offline PCM sketch using two
explicit phoneme probes. It takes Mexican /ʃ/, shortens its unvoiced portion
in two alternatives and crossfades it into a Castilian carrier; the final
examples contrast Castilian /s/ with /θ/. The fixed splice locations apply
only to the voice, rate and two probes in this experiment. This is not a
general synthesizer implementation and cannot demonstrate production-quality
transitions, word analysis, sentence prosody, other voices or rates. Those
need synthesis-time integration and their own tests.

For these probes the listener preferred **Probe zwei**, with 30 ms removed
from the measured fricative window. That listening preference is the starting
point for a real synthesis-time implementation, not a fixed 30-ms cut to
apply to arbitrary voices, rates or contexts.

Further reference: [Mott, *Chistabino (Pyrenean Aragonese)*](https://www.cambridge.org/core/journals/journal-of-the-international-phonetic-association/article/chistabino-pyrenean-aragonese/02F4CE9705DC9B69280662963C61FA46).

`inventory.py` lists independently sourced examples for developing and
checking the language, alongside the Castilian provider's actual phonemes.
It covers ny/ñ, x/ix, the Castilian-style interdental, palatal ll, rounded
au, silent final t in -nt/-st/-rt, and word stress. In particular, x can
also mean /ks/, even at the beginning of a word (*xenofobo*, *taxi*, *sexo*,
*examen*); blindly mapping every x to /ʃ/ would therefore be wrong. Likewise,
dropping the t in *rebost* before passing it to Spanish changes the Spanish
stress rule. The inventory is a diagnostic baseline, not a claim that these
features or numbers, punctuation, character names, dictionaries and platform
integration have been implemented.

With the current Castilian provider, `auca` is already analysed as `aw`;
spelling the sound as `au` in SPR creates two syllables instead, so it is
not a drop-in correction for the insufficiently rounded off-glide. The
Castilian tables also contain an `S` acoustic rule, but the SPR reader lacks
the Mexican provider's mapping from the input `S` to that phoneme. Enabling
that mapping and choosing a duration must be scoped to Aragonese before the
sound can be used safely in ordinary synthesis.

Two **temporary, non-production** patches record the native experiments:
`native-sh-probe.patch` admits the missing S and scales its native duration to
80%; `native-au-probe.patch` lowers the w F1/F2 targets toward the existing u
targets. Apply them only to private copies of the provider: they intentionally
alter Castilian rather than checking a separate Aragonese identity. The short
S probe successfully generated explicit phoneme groups for word-initial,
medial, and final S, including the still-available Castilian T. Its first two
test items were approximately 31 and 36 ms shorter than the Mexican reference
for the same voice and rate. These are measurements, not a listener's
validation and not evidence that running text or other contexts work. The
first shortening also changed the /tʃ/ affricate. A later private probe avoids
shortening the S portion immediately following /t/: initial, medial and final
/tʃ/, the Castilian /θ/, and the control words *hola* and *casa* remain
byte-identical at three sample rates, two speeds and two voice genders. This adjacency
heuristic is not yet a complete distinction between affricates and every
possible /t/ + /ʃ/ sequence, and must not be shipped unchanged.
The first w target change affected an onset in *puent*, but had no effect on
*auca*, *pauto* or *cudiau*. The AU patch now tries the other native w target
rule instead. In the private binary those three AU words change their PCM
without changing phones or duration; *puent*, *hola*, *casa* and *mocet*
remain byte-identical at 11.025, 16 and 48 kHz, two speeds and two voice
genders. The candidate still needs a listening test and safe language scoping.
The builder prints each private library path. Given those two paths and the
unmodified baseline, `native_listening.py --baseline BASELINE --native-sh
SH_LIBRARY --rounded-au AU_LIBRARY --wav FILE` makes the labelled comparison.

The listener found the native 80% /ʃ/ longer than the earlier listening
sketch and cautioned that the stronger AU glide could sound disyllabic. A further
private `refine` build combines the patches and allows serial audition of
multiple values in the same binary. It scales the native /ʃ/ duration to 80%,
65% or 50%, and interpolates the AU target change at 0%, 40%, 65% or 100%.
The 80% /ʃ/ and 100% AU settings reproduce the earlier isolated candidates;
they are references, not approved production settings. Environment variables
used by this throwaway binary are not an application API, and their controls
are process-wide. Tests and rendering must run serially. To verify and compare:

```
python3 experiments/aragonese/test_refinement_probe.py --baseline BASELINE --previous-sh SH_LIBRARY --previous-au AU_LIBRARY --refined REFINED_LIBRARY
python3 experiments/aragonese/refinement_listening.py --baseline BASELINE --refined REFINED_LIBRARY --wav FILE
```

The shorter variants retain the same S phone, and the AU variants retain the
same phone sequence and total duration. Neither property proves that the AU
will be perceived as one syllable; that remains a listening question. The
limited /tʃ/ heuristic and the lack of language-specific scoping still prevent
using these temporary patches as a production Aragonese voice.

The listener selected **Sch, Probe eins**, the 65% duration setting, as the
target for future integration. For AU, the listener preferred the earlier 40%
rounding over 65% in the middle, but noted insufficient rounding word-finally
and a two-syllable impression in *auca*. These are distinct requirements:
rounding should not turn the second part into a weighted vowel.

The next disposable `glide` build independently adjusts the first and second
formant targets. The listening comparison keeps the earlier 40% F2 change for
nonfinal AU; one candidate uses no F1 change and 65% F2 for final AU, the
other a 20% F1 change and 80% F2 for final AU. These are experiments, not
perceptual conclusions. A duration-control investigation in the same build
showed that `span_ph_w_dur` does not change these AU samples, while it can
change an onset /w/; do not treat it as a working AU duration control. The
listening comparison leaves the duration setting unchanged at 100%. A native
word-position detector is included only for investigation; its behavior needs
the regression checks below. The optional manual mode sets the position per
isolated word and is **not** a text-processing or platform implementation.

```
python3 experiments/aragonese/build_native_probe.py glide
python3 experiments/aragonese/test_glide_probe.py --baseline BASELINE --previous REFINE_LIBRARY --glide GLIDE_LIBRARY
python3 experiments/aragonese/glide_listening.py --baseline BASELINE --glide GLIDE_LIBRARY --wav FILE
```

The final-AU example *costau* is attested alongside *cudiau* in Bayo's study
of Cheso Aragonese, [published by Institución Fernando el Católico](https://ifc.dpz.es/recursos/publicaciones/07/70/03bayo.pdf).
It is an acoustic test item, not proof that the parent Spanish voice has the
appropriate pronunciation of the word as a whole.

## AU timing against the preferred pauto

The listener could barely hear the latest F1/F2 alternatives, identified
duration as the more likely problem, and found *pauto* in the earlier 40%
rounding probe good. That earlier setting is the reference for the following
experiments. The isolated probe at 16 kHz and the recording's default voice
and speed reported a native nucleus budget of approximately 148.5 ms for
*auca* versus 125.4 ms for *pauto*. These are scheduler values, not measured
boundaries of a separate /u/ phone; they depend on speech context and settings.
Scaling the first budget to 85% gives about 126 ms in that example.

The private `timing` build tries both vowel-transition timing (mode 1) and
whole-nucleus timing (mode 2). Mode 2 at 85% or 70% changes the whole initial
AU nucleus, not only its /u/ portion. Its formant target stays at the earlier
40% setting; *pauto* is a byte-for-byte reference for the tested cases. A
candidate detector distinguishes word-initial /aw/ and conservatively checks
the word boundary. An earlier version falsely matched /a/ followed by /w/ in
separate words such as *a hueso*. The revised mode-2 candidate passed 912
checks across 11.025, 16 and 48 kHz, two speeds and two voice genders. They
cover *a hueso*, *a huevo*, *la huerta*, *la pausa*, initial AU within phrases,
and byte-identical *pauto* and *pauca* references. These checks do not prove
that every word boundary or phonetic environment is correct. This remains a
disposable Castilian-based experiment, not an Aragonese module.

```
python3 experiments/aragonese/build_native_probe.py timing
python3 experiments/aragonese/test_au_timing_probe.py --reference REFINE_LIBRARY --timing TIMING_LIBRARY --mode 2
python3 experiments/aragonese/au_timing_listening.py --baseline BASELINE --reference REFINE_LIBRARY --timing TIMING_LIBRARY --mode 2 --wav FILE
```

The spoken comparison repeats *pauto* unchanged and includes old and new
versions of *auca*, *auto* and *la auca*. Listening remains necessary to tell
whether any of the experiments preserve the preferred sound while avoiding a
two-syllable impression.

The listener selected timing **Probe 2**, the 70% initial-AU nucleus, with
the earlier 40% rounding. A private provider identity now shares the
Castilian rules and immutable tables and applies that timing, rounding and
the selected 65% native /ʃ/ duration only to that identity. In one Linux
build, 156 comparisons of provider control frames against the selected
disposable timing probe matched, including initial and medial AU, word
boundaries, /ʃ/, /tʃ/ and /θ/, across three output rates, two speeds and two
voice genders. The Spanish, German, English and Greek controls also matched
the previous library's PCM in the cases checked.

The next development candidate exposes this identity as an **initial test
voice**. Its dictionary slots are separate from Spanish, and its native
orthography includes ny/ñ and some x/ix cases. The remaining ambiguous x
spellings still need lexical coverage. Number words and character names still
inherit much of Castilian and are not yet a localized Aragonese system. The
/tʃ/ adjacency heuristic still cannot distinguish a genuine /t/ followed
by /ʃ/. The pronunciation experiments remain usable because their builder
removes scoped hooks only in temporary Castilian source copies.

On Linux with unstripped local ELF builds, the frame comparison can be
repeated against an existing disposable timing-probe library:

```
python3 experiments/aragonese/test_private_core_linux.py --library eci/libreloq_eci.so --timing TIMING_LIBRARY
```

`phoneme_probe.py` now also experiments with the latent final t rule on the
Castilian phoneme output of an isolated word. It removes that sound after n,
s or r while retaining the original stress and keeps the existing ny/ñ probe.
This is not connected to any platform: dictionaries, names, loanwords, source
positions and continuous speech must be handled before it can be used there.

References used as starting points:

- [Academia Aragonesa de la Lengua: official Aragonese orthography (2023)](https://www.academiaaragonesadelalengua.org/sites/default/files/ficheros-pdf/ortografia-del-aragones_web_es.pdf), especially the descriptions of **ny/ñ**, **x/ix**, **ll** and regional variation.
- [eSpeak NG: Aragonese language sources](https://github.com/espeak-ng/espeak-ng/tree/master/dictsource), as a separate implementation to compare rather than an authority for all dialects.
