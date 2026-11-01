# Feedback on the slower body, clarity and S trials

The six-way listening comparison on `codex/body-clarity-listening100` was
rejected: the added body was not audible, and the S-only trial sounded too
similar to 11.025 kHz without the intended wideband clarity. Those profiles
are listening experiments, not accepted release changes.

The energy matching largely cancels the low shelf in the body sentence
"Eine warme, ruhige Melodie am Meer." The actual voice compensation in its
validation report gives these maximum net gains below the shelf:

| Rate | Nominal shelf | Voice compensation | Maximum net low-band gain |
| --- | ---: | ---: | ---: |
| 16 kHz | +1.0 dB | -0.850 dB | +0.150 dB |
| 22.05 kHz | +1.5 dB | -1.279 dB | +0.221 dB |
| 32 kHz | +2.0 dB | -1.711 dB | +0.289 dB |
| 44.1 kHz | +2.5 dB | -2.155 dB | +0.345 dB |
| 48 kHz | +3.0 dB | -2.589 dB | +0.411 dB |

The S trial explicitly excludes the wide companion's noise bank. A separate
noise-only "Ass. Ass. Ass. Ass." probe at 48 kHz measured about 13.1% of
reference noise energy above 5.5 kHz, versus about 0.23% in that trial. Its
loss of extended bandwidth explains the reported narrow sound; this is not
a suitable target for the requested clearer wideband S.

The 4.9-kHz clarity shelf acts before the serial tract. Synthetic stationary
vowel probes find upper harmonics at that input which are strongly reduced
at the tract output. That motivates examining voiced excitation and tract
balance together. These probes are not real-speech listening judgments and
do not establish that more high-frequency power always improves a voice.

A subsequent listening design should retain the extended S band, correct
its spectral balance without the rejected broad resonance widening, and
verify that a proposed body change survives its level comparison. Separate
periodic voice and frication controls remain necessary. The user requested
slower listening packages before another acoustic add-on or release.
