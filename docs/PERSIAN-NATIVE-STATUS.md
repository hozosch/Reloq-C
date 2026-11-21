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

The language is not yet a normal, independent ECI voice. The remaining work
includes registering Persian and Pinglish dialects, moving target/donor frame
capture, contour fitting, replay and streaming orchestration into C, and
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

The in-tree Apple build can be invoked on a Mac with
`make -f tools/apple/Makefile xcframework`. It prepares iOS, iOS simulator
and Apple-silicon macOS static libraries and the public ECI headers.
`.github/workflows/apple-native-build.yml` is prepared to build the same
framework on a GitHub-hosted Mac and run a native ECI smoke test. The source
selection, headers and smoke test were checked on Linux; this new Apple build
has not yet run on a Mac or iPhone.
