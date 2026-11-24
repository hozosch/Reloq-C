# Persian native-core status

This is development work, not a statement that Persian is ready for release.
The accepted Persian U sound is in the shared acoustic tuning. The independent
Persian/Pinglish analyzer produces language-owned phone and boundary events.
An experimental C speech entry point can synthesize a subset without NVDA or
an eSpeak process, using the existing Finnish-based acoustic bridge.

The private NVDA test package now selects the complete C analyzer even when
NVDA's eSpeak DLL is installed; it does not package or load the eSpeak helper
modules. Covered words use the existing native analyses. For uncovered
written words the C analyzer infers short vowels, which can differ from the
earlier reference-backed development voice and needs listening validation.
To recover the earlier reference-backed readings, the shared C analyzer now
also includes previously uncovered spellings from the existing Lilak word
inventory, provided their old eSpeak readings agree in four contexts and are
accepted by the native phone parser. Existing native entries and productive
rules have priority. Half-space compounds remain on their existing native
morphology and fallback paths so the recovered whole-word table cannot bypass
verbal prefixes or compound rules. This is generated offline; the runtime needs no eSpeak
library. Agreement with the earlier reference is not proof that every
unvowelled Persian spelling has only one correct pronunciation.
Recovered stems now also participate in the native productive plural and
comparative suffix rules. A written half-space supplies the boundary; when
the half-space is omitted, a compact offline-checked admission mask avoids
splitting unrelated spellings. Both variants retain the stem's known vowels,
and previously covered whole words still take precedence.
The same boundary-checked endings now cover stems from the original native
lexicon and the frequency table. This restores plural-ezafe forms such as
`گلرهای` from their known stems, rather than guessing the whole word again.
Pinglish retains the explicit a/aa distinction. In the complete native
analyzer, runs of three or more identical vowels are treated like a doubled
vowel; this permits informal elongated spellings while leaving strict English
routing and the single/double distinction unchanged. Tests also cover the
source indexes and following weak words. This removes the optional eSpeak
pronunciation fallback from that test package; it does not remove its Python
contour orchestration or register an independent ECI voice.

The native analyzer also composes a written numeral and an independently
known, classified nominal component when their unspaced compound is not
already in the lexicon. This covers the form `دونقطه` used by NVDA for the
colon while preserving admitted whole words and rejecting short ambiguous
verb endings. The numeral retains its short vowel, and the compounded noun
receives final-syllable stress. The shared ECI UTF-8 input can carry original
utterance context across a host language switch. A single ordinary symbol in that larger source
is then not mistaken for an isolated character, while explicit spelling,
genuinely isolated characters and emoji remain available. Older ECI DLLs use
the existing single-character protection as a compatibility fallback.

The silent US contour proxy and its pronunciation checks now run in shared C.
The NVDA planner uses that implementation when the matching ECI library is
available; the Python implementation remains for older libraries. It does not
change the Persian phones. Word boundaries, weak words, punctuation and the
full generated proxy are compared against the previous implementation.
Sample-clock word bounds are also converted and fitted in C, including the
same rounding and syllable alignment. Native text preparation is reusable
independently of the synthesis call, so a future C renderer can capture and
replay the prepared event plan without an NVDA object. An experimental C
entry point now owns the silent Persian target frames, syllable timing and
source-index sample positions. It restores the caller's output and callback
after capture; the owned data remain valid for a subsequent donor pass. A
second C-only entry point produces and captures the US prosody pilot with its
word marks, without NVDA. A third experimental C entry point fits and replays
the two captures, retaining the caller's frame-index marks and leaving both
captures unchanged. A standalone test compares that step with the existing
separate contour transfer and replay for short phrases at all seven sample
rates, including cancellation. A further experimental C entry point now
prepares ordinary Persian or Pinglish text, derives word boundaries and
syllable counts, captures both contexts, applies the selected U-to-A glide,
fits the US contour and replays source indexes without NVDA or eSpeak. The
extended C sentence entry point accepts separate normal, character-name and
explicit Persian phonetic runs, including the `l20.0/`l20.1 switches between
Persian script and Pinglish. Its US donor is constructed from the same owned
phone events, preserving geminated affricates and glottal boundaries. The
extended renderer still rejects English language events and English-only names
inside longer text, as well as phonetic groups with ambiguous implicit
syllables, before sending output; the established driver retains those cases.
The separate incremental native backend maps source indices inside an English
event to positions within its original English text instead of collapsing them
to the beginning of that word. The event itself stays intact for English
analysis. Standalone tests include English mixed with Persian digits and
compare audio with and without those indices. This fixes one input mapping,
not the missing shared English acoustic bank or the accepted US contour.
For default dictionaries, the same experimental incremental backend now owns
its English worker and reuses it between requests. It does not
share that mutable worker with the parallel US contour pilot. Directory and
explicit ECI dictionaries retain their existing separate path. Comparisons
cover repeated mixed text, clocks and voice settings, cancellation and
dictionary replacement; other languages retain their existing workers. This
prepares the English analyzer in C but does not move NVDA's mixed sentence
planning, register Persian as an ECI voice, or provide the selected contour.
An isolated character is named even without spelling mode. If no Persian name
is available, the same native transaction uses the US-English Unicode name.
For ordinary runs, the renderer uses the shared Unicode policy to omit
non-Persian symbol names without changing source byte offsets. Run-mode bit 4 carries a host's wider utterance
context, so a lone symbol at a language boundary is not mistaken for an
isolated character; explicit spelling and default emoji still get names.
Compound emoji and English names inside longer mixed input need further work.
The target and the US donor are now built from one owned linguistic event
plan instead of independently analyzing ordinary Persian text twice. The
native complete Pinglish analysis also accepts bracket boundaries directly.
These paths still use the accepted Finnish-based acoustic bridge and the US
prosody pilot; they do not constitute an independently registered voice.

The experimental C sentence handle can now own, preload and reuse its US
pilot. Before each utterance it applies the Persian voice's current acoustic
clock, rate, anatomy and other shared controls; stopping the Persian handle
also stops the owned pilot. The NVDA short, ordinary-sentence route selects
this C-owned pilot when available and keeps the previous explicit pilot as a
compatibility fallback. Tests compare the output and source indexes with the
existing route at all seven sample rates, across both genders and clocks,
including cancellation and subsequent speech. This removes one driver-owned
handle from that route; it does not yet move mixed-language planning,
long-text streaming or voice registration into the shared core.

The C-only US-contour transaction now uses the Persian voice's effective
acoustic clock for its pilot, restoring the caller's independently selected
English clock afterwards, including on cancellation. The current NVDA mixed
route likewise applies the Persian choice to its US parts while a Persian or
Pinglish voice owns the utterance. This fixes inconsistent clock changes but
does not merge the providers or remove the existing Python mixed-sentence
planner. US English and Persian selection both finish preparing their
background workers before accepting speech. A further scoped R adjustment
attenuates a band around the third formant on the periodic source before
aspiration and frication are mixed. It covers R after A, coda R after U and R
before I after a rounded vowel; other languages and other R contexts keep their
existing paths. Objective tests compare the isolated noise and voiced signals,
but the audible result still needs the listeners' validation. These changes do
not establish Persian as a finished platform voice.
The U before a selected R and the upper R band have since received additional
voice-only smoothing. A conservative phonetic lookahead lets the U correction
start before the provider reports the following R; uncertain or mixed-text
input keeps the existing streaming path. A separately verified E-R-H context
also softens the R voice, preserving the noise source. These remain listening
tests and do not establish that every pronunciation or transition is correct.
The following test revision retains the accepted final U-R, strengthens the
periodic-source upper-band cut for R before I, and fades that cut through the
first 25 ms of the I. The separate noise source and the I's steady state are
unchanged. The captured reference and the two NVDA routes are tested across
voices and sample rates; the perceived result still requires listener review.
The NVDA mixed planner also uses a shared C transition helper for a verified
English stop, including a verified voiced closure, followed by a Persian
initial R. If the target context contains
a stale voiced lead before the independently measured first syllable, it
removes that lead and limits a few inherited voiced frames at the start of R.
The English stop, all real R frames and the aspiration and frication controls
are retained. Source indexes and the existing streaming route are checked for
the reported grouped-number example and for negative controls. This is still
an NVDA mixed-planner path, not completed mixed-language C integration.

The language is not yet a normal, independent ECI voice. The remaining work
includes registering Persian and Pinglish dialects, completing the C sentence
renderer for mixed languages, controls, full contour rules and streaming, and
retaining the accepted sound, US sentence contours, cancellation, controls,
character reading, source indexes and mixed English speech. The current
experimental speech entry point does not support all of these and must not be
presented as a finished replacement. Unknown written words still need
linguistic and pronunciation validation.

## Reviewed readings and linked compounds

A short hand-checked table in `experiments/persian/direct_fa_lexicon.c` now
takes precedence over the generated tables. It holds frequent words that the
generator dropped (several begin with ع, such as `عادت`, `عمر` and `عملیات`)
and a few readings that were wrong in the reference, such as `نخست`. Their
inflected forms (`عمرم`, `عادتش`) are still guessed; the reason those stems
are missing from the generated tables has not been found.

Three productive rules were added before the tables or before the letter by
letter guess. A half-space compound with a written `و` keeps it as a linking
/o/ (`گفت‌وگو`), unless the tail is a classified word of its own (`هم‌وطن`).
The Arabic article inside a compound is read /ol/ with sun-letter
assimilation and a shortened preceding vowel (`حق‌الزحمه`, `فی‌البداهه`,
`روح‌الله`). An existing reading that already links the article is kept; the
rule replaces only a missing reading or one that says a separate "al". A
Roman-script spelling follows its Persian word, so these readings reach
Pinglish too. A word that begins with the article and is otherwise unknown
uses the known reading of the rest (`الوصل`). An unknown word made of two
known words written together is read as those words (`سیدمهدی`). A final
tanwin in a guessed word no longer makes the whole phrase fail, and a guessed
word no longer begins with an explicit glottal stop, matching the lexicon.
Numbers from 1000 to 1999 are read without a leading «یک».

`experiments/persian/test_direct_fa_linked.py` checks these cases against the
direct text library. It is not yet part of a workflow. These readings were
checked by reading and by synthesis, not by a native listener.

## Switchable sound variant for listening tests

`eciPersianSoundVariant(n)` selects a candidate Persian sound for the whole
process; `0`, the default, is the previous sound and produces identical audio.
Variants 1 to 3 share the new R and uvular described below. Variant 1 also
shifts both vowels fully, variant 2 keeps the previous vowels, and variant 3
shifts them half as far (about 1490 Hz for /æ/ and 950 Hz for /o/). A first
listener found the full shift possibly too far forward, so all three are
built for comparison.
Nothing selects the candidate automatically. It is offered for comparison by
Persian listeners and has only been checked by measurement so far.

The candidate changes four things. The front vowel of `بد` has its second
resonance about 200 Hz higher (roughly 1390 to 1590 Hz for Reed), because the
previous value is closer to a central vowel than to Persian /æ/. The /o/ of
`دو` moves from about 900 to 1000 Hz. An R between a voiced sound and a vowel
is a single voiced tap: the donor R closed twice to near silence and carried
strong aspiration that began inside the preceding vowel. The voice now dips
once by about 11 dB and both noise sources stay low. R after a voiceless
sound, before a consonant and at the end of a word is unchanged, and so is an
R followed by H. Finally, a uvular directly before another consonant, as in
`مغرور`, is a stop without the added post-release frication.

The NVDA test package applies a variant when it contains a one-line file
`persian_sound_variant.txt` next to the driver. The NVDA direct-test workflow
builds the usual add-on without that file and, for the listening branch, one add-on
per candidate; only the first is run through the NVDA test.
`experiments/persian/test_persian_sound_variant.py` checks the switch, that
switching back restores the previous audio exactly, and the tap's voice dip.

## First-audio latency

The direct Persian acoustic path already uses the same native span delivery
as the other languages, but it has the old Finnish contour and is not an
acceptable substitute for the chosen US-derived melody. An experimental
incremental entry point now accepts text, character-name and unambiguous
phonetic runs and emits through that same renderer. It is not selected by
the voice; user controls and some English contexts still need a fallback.
The complete C
sentence path currently captures both the entire Persian target and a US
pilot before it replays anything. This makes its first audio grow with the
whole sentence, even though the underlying engine can emit spans as it goes.

The goal is a native Persian phrase/intonation implementation that drives
the shared incremental renderer with the accepted phonemes, timing and US
contour, retaining source indexes and prompt cancellation. Do not make the
two-pass route the normal voice merely because it has been moved out of
Python. `experiments/persian/benchmark_native_streaming.py --library <path>`
compares first PCM for the direct and current US-contour C paths with English
and German; its measurements have no hardware-independent pass threshold.
`experiments/persian/benchmark_active_vs_native.py --library <path>` also
compares the active no-eSpeak planner with the two-pass C renderer after
requiring matching PCM and source indexes. It reports mixed text separately,
because that C renderer cannot yet process it. On a local warm-run comparison,
the C-only two-pass path did not consistently reduce first-audio latency;
this is not a measurement of NVDA, Windows audio or a user's hardware.

The US melody needs to be fixed in the Persian core itself, with Persian
syllables, lexical stress, weak words and punctuation as its inputs. A direct
experimental call of the US provider's intonation rule on the current
Finnish-backed Persian state failed in isolation. The providers have
language-specific symbol tables and records, so their entry points cannot be
treated as interchangeable. Adapt the rule inputs to the Persian plan and
native duration clock, then compare the resulting F0 and audio with the
accepted US-derived contour before enabling it in any platform voice.
The current provider runs `generate_durations`, `generate_intonation`, and
`generate_acoustic_vals` in that order for each syllable in
`vendor/openevv61/lang/fifi/delta_rules_c04_fifi.c`. That per-syllable
boundary is the intended integration point: the Persian duration clock is
already available, and the ordinary span renderer can emit the result
without either full-utterance capture. The Persian event plan owns stress
and weak-word metadata; the existing US proxy describes the established
phonetic and phrase mapping to use as a reference, not as another render.
`eci/test_persian_syllable_plan_clock.py` compares the ordered plan syllables
and their source-word event marks to the current provider's syllable clocks
over several rates and both genders. Source words can yield multiple
internal pronunciation groups, so their boundaries must come from the
analyzer's event map rather than counting groups. Internal punctuation is
not yet covered by the source-word clock invariant: in a comma example the
next syllable overlaps its index boundary. Feed the ordered syllable events
to the per-syllable duration/intonation boundary instead of treating output
index marks as syllable boundaries; verify or reject any input where that
event order cannot be matched. Mixed English needs its own alignment policy.
The tone targets also need a vowel or sonorant clock within each syllable.
`eci/test_persian_phone_plan_clock.py` compares the provider's phone clocks
with the event-plan syllables for ordinary Persian and Pinglish phrases,
including comma pauses. Equal counts are not sufficient to prove alignment:
the current doubled /dʒ/ bridge can place a phone clock in the following
syllable even when counts match. An experimental C adapter now compares the
event plan with both provider clocks and captured provider phone slots before
returning vowel anchors. It accounts for the backend's two-part affricates,
H features and the cross-syllable doubled /dʒ/, while retaining linguistic
stress and weak-word metadata. When an identified phone lies in the gap
around a provider syllable, as in `قشنگ`, the adapter extends only its returned
anchor through that verified phone, without crossing another syllable or a
pause. Ambiguous or inconsistent alignments can still be declined
without returning partial anchors. The current diagnostic
obtains syllable and phone clocks in separate captures. The adapter does not
yet drive intonation, handle mixed English or remove full-utterance planning.
`eci/test_persian_nucleus_clock.py` checks the mappings across voices, rates,
comma pauses, grouped numbers and rejected mismatches.

An additional opt-in provider trace now observes each vowel after duration
assignment and before intonation. It identifies the vowel from the provider's
current phone record and includes any marked /q/ release before it, whose
existing acoustic rule extends that release by 25 ms. A C cursor validates
the linguistic event plan before synthesis and consumes these vowel clocks
one at a time, carrying word-group, syllable, stress and weak-word metadata.
This makes alignment available at the per-syllable boundary without a full
acoustic capture. It remains experimental: the current voice does not install
the cursor in its live renderer, the direct US intonation rules and
source-word/mixed-English boundaries are incomplete, and matching provider
vowel slots alone does not prove those boundaries. The test compares early
clocks and identities with independently traced phone clocks, checks cursor
output and rejection without partial state, and compares control frames with
and without the trace. It also checks final /q/ independently: its later
overlap with a pause does not prevent identifying the earlier vowel interval.

The current backend adds a short provider-slot carrier for word-final /h/;
the adapter consumes it only when its identity and position agree with that
word's syllable. Tests cover final /h/ in isolation and before another word,
as well as /h/ followed by a vowel. In the fixed 300-word sample, two words
are still declined: final /q/ can overlap the following pause. That mapping
must be verified before accepting it; the current voice continues to use the
existing contour path.
For an all-unstressed word whose phones the US provider cannot represent,
the current proxy can add an artificial primary stress while retaining the
weak-word control. That synthetic marker is not a Persian lexical accent;
reproducing the selected contour requires both pieces of context.
The existing scalar US tone helpers in `src/reloq_english_intonation.c` can
express nuclear and phrase-final targets on a supplied clock, but do not yet
cover all US phrase and accent decisions. They are candidates for a shared
rule implementation, not a drop-in replacement for the accepted contour.

## Apple integration

[ReLoq Apple](https://github.com/Quinton1110/ReLoq-Apple) is an independently
maintained app that already provides iOS/iPadOS and Apple-silicon Mac voices.
Its 0.1.1 source pins ReLoq C 1.4.1 with the Apple changes proposed in
[PR 53](https://github.com/hozosch/Reloq-C/pull/53). Its build uses the C
sources from the pinned engine as a static XCFramework; it does not turn a new
engine dialect into a system voice automatically.

Once the Persian ECI voice is complete, update the pinned core in the Apple
app, add Persian to its bridge language list and system voice catalog, and
route Persian input through UTF-8. Check preservation of Persian text
boundaries such as ZWNJ, as well as SSML, interruption, word indexes, settings
and mixed English. The app's current bridge explicitly filters some Unicode
format characters, so Persian must not simply be appended to its language
list without checking that path. A Mac build and device/VoiceOver validation
remain necessary; the author tested the older 1.4.1-based Apple build, not
this development branch.

For review, `tools/apple/reloq-apple-0.1.1-core2.patch` prepares that app's
0.1.1 source for the newer headers, provider archive and Persian joiners. It
does not enable or advertise the incomplete Persian voice. The separate Apple
app compatibility workflow pins the author's 0.1.1 commit, applies this patch
and builds unsigned apps against the development core. Passing CI will not
replace testing the signed app and speech extension on Apple devices.

The in-tree Apple build can be invoked on a Mac with
`make -f tools/apple/Makefile xcframework`. It prepares iOS, iOS simulator
and Apple-silicon macOS static libraries and the public ECI headers.
`.github/workflows/apple-native-build.yml` is prepared to build the same
framework on a GitHub-hosted Mac and run a native ECI smoke test. A hosted
Mac has successfully built all three library slices and passed the native
Mac ECI smoke test after both Apple build recipes gained the small existing
fixed-point table. The source selection, headers and smoke test also passed
on Linux. The separate compatibility workflow also passed the Mac bridge
tests and built unsigned iPhone and Mac app targets. No signed app based on
this branch or VoiceOver test on a Mac or iPhone has been completed.
