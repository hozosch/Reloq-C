# First public release

The release target is a reliable shared engine with measured resource use.
Optimization continues after that milestone; it does not require proving that
no further speed or size improvement is possible.

## Core acceptance

- Resolve concrete language regressions, including the APK filename and the
  `SAPI5 ... Vorbereitung` sentence reported against 0.2.197. Keep the native
  source-token analysis and the documented German sound departures.
- Keep all language/rate/model output comparisons unchanged outside intended
  language fixes. Exercise cancellation, source indexes, dictionary edits and
  reloads, ECI controls and mixed Unicode through the public API.
- Record package/DLL sizes, requested heap peaks and paired CPU measurements.
  Compare high rates separately. Host benchmarks and Windows/ARM listening tests
  remain distinct from host benchmarks.
- Read CLDR emoji names in the shared core, including Android sentence
  reading, joined sequences and source indexes. Keep native punctuation and
  accent rules. Audit actual linked code separately from source/archive size:
  release rule entries must not retain interpreter descriptions or a second
  PCM renderer.
- Retain the current Android package identity, signing continuity and browser
  update handoff. The user reports that Play Protect accepts 0.2.197. This is
  a device observation for that package, not a general security certification.
- The first public package release explicitly includes the existing Android
  **preview**, retaining its package and public test-signing identity for
  updates. A privately signed production APK needs a deliberate migration
  path; it is not claimed for this release.
- Publish actual versioned package assets with the first GitHub release so
  the existing user-requested update checks can offer them. A source commit
  alone is not an installable release.

## SAPI5 integration

The small C COM adapter wraps the shared private ECI core and registers one
voice per language. Native settings apply immediately to later utterances,
with optional shared parameters and a common synthesis rate. Version 0.2.205
uses SAPI's output thread directly and combines x64 and x86 in a localized
Inno Setup installer. Automated Windows checks cover both COM views,
all native/application rate combinations, and removal. Basic x64 use was
also confirmed by the user. The user has confirmed SAPI response and sample-rate behaviour. Ongoing
physical-device and screen-reader checks complement the automated coverage. See the [preview guide](../sapi/README.txt)
for supported controls and the explicit limitations.

| Boundary | Required implementation |
| --- | --- |
| COM activation | Class factory, lifetime, voice token selection and registration/removal |
| `GetOutputFormat` | Negotiate supported mono 16-bit PCM rates |
| `Speak` | Pass UTF-16 text fragments and their states into the shared C input/control path |
| Audio site | Bounded PCM streaming through `Write`, without temporary audio files |
| Control site | Respond to abort, rate, volume and skip actions while rendering |
| Events | Bridge source positions to word/sentence/bookmark events before their audio is written |
| Engine controls | Reuse current voice anatomy, model, pause and dictionary settings |

SAPI supplies parsed text fragments. Their source offsets and state changes
must survive the bridge. Existing explicit ECI index callbacks are useful,
but do not by themselves implement automatic SAPI word/sentence events.
Pronunciation/phoneme fragments also need a defined mapping and tests.

Version 0.2.205 combines the x64 and x86 builds in one localized Inno Setup installer. Both retain separate COM registry views and share dictionaries and user settings. The installer workflow checks installation, in-memory SAPI conversion and removal on Windows; physical-device response and UI accessibility still require user tests. Native ARM64
still requires its own build and activation tests; an x64 package is not a
claim of native ARM64 support. Registration, settings persistence, interruption and real SAPI clients have
been exercised for the first release. Continue testing device-specific audio
and UI accessibility after that milestone.

Microsoft's primary interface references:

- [TTS Engine Vendor Porting Guide](https://learn.microsoft.com/en-us/previous-versions/windows/desktop/ee431802(v=vs.85))
- [ISpTTSEngine::Speak](https://learn.microsoft.com/en-us/previous-versions/windows/desktop/ee413475(v=vs.85))
- [ISpTTSEngineSite::Write](https://learn.microsoft.com/en-us/previous-versions/windows/desktop/ms719570(v=vs.85))
