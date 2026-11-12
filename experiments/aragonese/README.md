# Aragonese: isolated Spanish-backend probe

This is an initial research probe, not an available voice or a production
Aragonese language implementation. It does not modify the Spanish voice, the
language catalogue, user settings or release packages. Invoke it explicitly:

```
python3 experiments/aragonese/test_probe.py
python3 experiments/aragonese/probe.py --library eci/libreloq_eci.so --wav /tmp/aragonese-probe.wav
python3 experiments/aragonese/compare_sh.py --library eci/libreloq_eci.so --wav /tmp/aragonese-sh-comparison.wav
python3 experiments/aragonese/castilian_sh_listening.py --library eci/libreloq_eci.so --wav /tmp/aragonese-castilian-sh.wav
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

`castilian_sh_listening.py` creates a labelled, offline PCM sketch using two
explicit phoneme probes. It takes Mexican /ʃ/, shortens its unvoiced portion
in two alternatives and crossfades it into a Castilian carrier; the final
examples contrast Castilian /s/ with /θ/. The fixed splice locations apply
only to the voice, rate and two probes in this experiment. This is not a
general synthesizer implementation and cannot demonstrate production-quality
transitions, word analysis, sentence prosody, other voices or rates. Those
need synthesis-time integration and their own tests.

Further reference: [Mott, *Chistabino (Pyrenean Aragonese)*](https://www.cambridge.org/core/journals/journal-of-the-international-phonetic-association/article/chistabino-pyrenean-aragonese/02F4CE9705DC9B69280662963C61FA46).

References used as starting points:

- [Academia Aragonesa de la Lengua: official Aragonese orthography (2023)](https://www.academiaaragonesadelalengua.org/sites/default/files/ficheros-pdf/ortografia-del-aragones_web_es.pdf), especially the descriptions of **ny/ñ**, **x/ix**, **ll** and regional variation.
- [eSpeak NG: Aragonese language sources](https://github.com/espeak-ng/espeak-ng/tree/master/dictsource), as a separate implementation to compare rather than an authority for all dialects.
