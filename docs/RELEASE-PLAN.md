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
  Compare high rates separately. Windows/Dell and Windows/ARM listening tests
  remain distinct from host benchmarks.
- Retain the current Android package identity, signing continuity and browser
  update handoff. The user reports that Play Protect accepts 0.2.197. This is
  a device observation for that package, not a general security certification.
- Finalize production signing and the preview-to-release update path before
  public Android distribution; this iteration keeps the preview identity.
- Publish actual versioned package assets with the first GitHub release so
  the existing user-requested update checks can offer them. A source commit
  alone is not an installable release.

## SAPI5 integration

SAPI5 is not implemented yet. Build a small C COM adapter around the same
private ECI core DLL; do not copy the synthesizer, depend on NVDA/Python or add
another pronunciation frontend. Generate voice registration data from the
existing catalogue so names, profiles and language ordering remain consistent.

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

Start with a Windows x64 preview. Native ARM64 and 32-bit hosts require their
own builds and activation tests; an x64 package is not a claim of native
ARM64 support. Validate registration, settings persistence, interruption and
real SAPI clients before including that adapter in the first public release.

Microsoft's primary interface references:

- [TTS Engine Vendor Porting Guide](https://learn.microsoft.com/en-us/previous-versions/windows/desktop/ee431802(v=vs.85))
- [ISpTTSEngine::Speak](https://learn.microsoft.com/en-us/previous-versions/windows/desktop/ee413475(v=vs.85))
- [ISpTTSEngineSite::Write](https://learn.microsoft.com/en-us/previous-versions/windows/desktop/ms719570(v=vs.85))
