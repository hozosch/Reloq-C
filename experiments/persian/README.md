# Persian: first listening comparison

The comparison tests acoustic building blocks, not an implemented Persian
voice or general Persian grapheme-to-phoneme conversion. It uses the existing
German and Finnish native engines for manually entered phoneme sequences.
The audio labels explicitly distinguish those approximations from the
Persian and Arabic eSpeak NG recordings used as references.

`نقطه` (point) is commonly transcribed /noqte/; in Iranian Persian the
uvular consonant has context-dependent stop and fricative realizations,
including devoicing before voiceless consonants. The Arabic /q/ is a
voiceless uvular stop; the existing ordinary German /k/ is velar and is
included as a contrast, not described as a correct uvular /q/. German /r/
and /g/ are also comparisons rather than a native Persian phoneme.

Persian has six vowels. Finnish front `ä`, `e`, `i`, `o` and `u` offer
useful starting points. Finnish `a` is a comparison for Persian's back
vowel, not necessarily the correct rounded Persian realization. The
prosody section contrasts a statement and question in the Persian eSpeak
voice with the existing Finnish and German contours. Neither is claimed
as a verified Persian intonation model.

Research references:

* [Uvular lenition in Persian](https://cla-acl.ca/pdfs/actes-2021/Ariyaee_and_Kochetov-Acoustics_of_Persian_uvular_lenition_in_consonant_clusters.pdf)
* [Persian intonation, eight speakers](https://www.isca-archive.org/speechprosody_2008/sadattehrani08_speechprosody.html)
* [Persian interrogative intonation](https://journals.dartmouth.edu/journals/xmlpage/1/article/389?htmlOnce=yes)
* [eSpeak NG Persian voice source](https://github.com/espeak-ng/espeak-ng/tree/master/dictsource)

To reproduce, install eSpeak NG (the initial recordings used version 1.51)
and Python NumPy and SciPy, then run:

```
python3 experiments/persian/espeak_reference.py --output /tmp/persian-reference
python3 experiments/persian/listening.py --library eci/libreloq_eci.so \
  --reference-dir /tmp/persian-reference --wav /tmp/persian-comparison.wav
```

The eSpeak samples are comparison recordings, not copied phoneme tables or
embedded software in the ReLoq binary. Record the eSpeak version and source
when producing them; eSpeak NG is distributed under GPL-3.0-or-later.

## Finnish-based long-a and uvular-stop experiment

`build_native_probe.py` produces a separate, disposable Linux library from
two temporary copies of the Finnish provider. It is not a Persian module and
does not modify the normal engine. In this library only, `RELOQ_FA_PROBE_A=1`
adjusts Finnish /a/; `RELOQ_FA_PROBE_Q=1` and `=2` modify the regular Finnish
/k/ when the input spelling is `q`. The settings are listening probes, not
production controls, and must not be enabled in a normal Finnish voice.

eSpeak NG commit `688cb60368d6e0998f3632af8d1c3c2ad4191fc2`,
`phsource/ph_farsi`, selects `phsource/vowel/0_3` for its Persian /A/;
`dictsource/fa_list` transcribes Iran as `i:RAn`. The spectral file's two
middle frames have peak frequencies F1=656, F2=1000, F3=2620 Hz. The Finnish
/a/ rule starts at 625, 1050, 2600 Hz, so the experiment shifts positive
target frequencies by +31, -50, +20 Hz while retaining contextual differences
and native inherited-value sentinels. eSpeak's additive spectral frames and
Eloquence's Klatt parameters are different models, so matching these nominal
frequencies is only an approximation, not a demonstrated acoustic match.
The cited eSpeak reference is GPL-3.0-or-later; this root experiment follows
the enclosing project's GPL license.

eSpeak's common `q` source describes a voiceless uvular stop and uses the
short `ustop/q` release sample. Its Persian dictionary transcribes the two
test words with `q1`; the Persian voice audio is retained as a separate
reference instead of assuming it is identical to the Arabic `q`. The
two original exploratory variants shift F2/F3 for the existing /k/ and reduce
its higher burst amplitudes. They do not copy the eSpeak sample and have not
been validated as uvular or as the voiced Persian stop/fricative. The input
spellings `qalam` and `noqte` are used only to select those release experiments;
their Finnish stress, vowels, syllabification and phoneme name remain Finnish.

After building the ordinary Linux engine, reproduce the experiment with:

```
python3 experiments/persian/build_native_probe.py --work-dir /tmp/fa-probe
python3 experiments/persian/inspect_espeak_spectrum.py /path/to/espeak-ng/phsource/vowel/0_3
python3 experiments/persian/espeak_reference.py --output /tmp/fa-reference
python3 experiments/persian/test_native_probe.py --baseline eci/libreloq_eci.so \
  --probe /tmp/fa-probe/libreloq_eci.so
python3 experiments/persian/compare_native_probe.py --library /tmp/fa-probe/libreloq_eci.so \
  --reference-dir /tmp/fa-reference --wav /tmp/persian-long-a-q.wav
```

The reference recording uses eSpeak NG 1.51. `test_native_probe.py` checks the
probe and baseline PCM at several rates, genders and contexts and checks that
disabled settings, normal `k`, an unaffected Finnish vowel, and other
languages remain unchanged. It does not verify perceptual or phonetic quality.
