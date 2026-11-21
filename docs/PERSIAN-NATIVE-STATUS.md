# Persian native-core status

This is development work, not a statement that Persian is ready for release.
The accepted Persian U sound is in the shared acoustic tuning. The independent
Persian/Pinglish analyzer produces language-owned phone and boundary events.
An experimental C speech entry point can synthesize a subset without NVDA or
an eSpeak process, using the existing Finnish-based acoustic bridge.

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
extended renderer rejects English language events and English-only character
names, as well as phonetic groups with ambiguous implicit syllables, before
sending output; the established driver retains those cases. The
native complete Pinglish analysis also accepts bracket boundaries directly.
These paths still use the accepted Finnish-based acoustic bridge and the US
prosody pilot; they do not constitute an independently registered voice.

The language is not yet a normal, independent ECI voice. The remaining work
includes registering Persian and Pinglish dialects, completing the C sentence
renderer for mixed languages, controls, full contour rules and streaming, and
retaining the accepted sound, US sentence contours, cancellation, controls,
character reading, source indexes and mixed English speech. The current
experimental speech entry point does not support all of these and must not be
presented as a finished replacement. Unknown written words still need
linguistic and pronunciation validation.

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
