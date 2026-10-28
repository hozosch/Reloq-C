# NVDA development boundaries

## Direct ECI path

`../eci/` now builds a native Windows x64 `eci.dll` directly from the C
source. It has no external helper process or Apple/OpenEVV runtime. Its
11,025-Hz German voice currently covers stressed syllables of the
forms `hV[p/t/k]`, `V[p/t/k]` and `[p/t/k/b/d/g/m/n]V[p/t/k]`, each with a period,
and bare/h/p/t/k-initial `V[s/f]`, plus checked k→t, k→p and p→t
two-word transitions and sequences of these complete sentences. ECI text
fragments may split a word; the native layer aggregates them before
preflight. An index at a sentence boundary or at a measured connected
two-word boundary is supported, including a split text feed; an index
within another unmodeled position rejects the request before audio output.
`make -C eci test` checks native
text-to-PCM, indexed callbacks, rejection of uncovered text, cancellation,
and controlled numeric PCM; `make -C eci windows-x64`
checks PE32+ architecture, 18 named exports and system-only imports. It also
builds `eci_windows_x64_runtime_smoke.exe` to check loading, callbacks,
cancellation and reuse on a Windows machine. These checks do not execute the
DLL on Windows here. The default exact ECI path still advertises zero complete
languages: it queues CP1252 text and refuses uncovered input before emitting
audio. Opt-in generic mode (`eciSetParam` 0x1003=1) has a productive German
G2P and approximate acoustic voice. This broadens acceptance but does not
claim Apple-exact pronunciation or sound.
The 0.2.6 package merges the original 1,205 observed pronunciation records
with 1,337 separately checked, context-stable records at build time. The
product C loader still receives one sorted 2,542-entry lexicon. On a disjoint
952-form reference holdout, the separate `ab-`/`auf-`/`aus-` composition rule
improved exact G2P matches from 318 to 340 with no exact regression; the
supplement has no entries from that holdout. Controlled target changes also
cover `/v/` into a vowel (initial F1) and pre-nasal B1 plus a proportional
`/n/` FNZ rise. Neither the G2P count nor the target checks is an audible
quality score for ordinary text.

`_reloq_eci_direct.py` binds this API in-process; `_reloq_eci_transport.py`
is a FIFO worker with playback-gated index and completion
events. Run `python3 -B nvda/test_eci_direct.py`,
`python3 -B nvda/test_eci_transport.py`, and after building the Linux
development library `python3 -B nvda/test_eci_direct_native.py`. The last
test checks native PCM and index delivery, cancellation and explicit failure
for uncovered text. `python3 -B nvda/test_eci_generic_native.py` checks a
natural sentence, interior/EOF index timing, unchanged PCM with marks, and
native rate control in generic mode.
The selectable driver also accepts NVDA `PitchCommand` and
`CharacterModeCommand`. It sends a native inline ECI pitch mark for a changed
pitch, including NVDA's capital-letter pitch, and keeps spelled letters at
separate word boundaries. A reset mark restores the selected pitch in that
utterance. Other inline prosody commands are not implemented.
The direct ECI German byte interface is CP1252 in the measured reference;
the internal German text scanner uses UTF-8. The native boundary decodes
CP1252 before coverage checks and synthesis.

## Earlier numeric host path

`_reloq_native.py` loads the authored C renderer through `reloq_host.h`. It
accepts **already scheduled 62-control frames** and copies each PCM16 mono
block before handing it to an audio queue. Its `analyze_german` method now
queries the native restricted German UTF-8 → SPR → phone-context frontend;
it returns bounded annotation bytes and fixed-width phone records without
producing any audio. Uncovered words and unsupported characters report
their UTF-8 byte offset. Native cancellation is checked within UTF-8 and
text scanning and between phone parser events; an individual G2P word or
one scanner event remains synchronous. The preview does not yet expose caller-owned
lexicon overrides to Python.

There are now two separate x64 NVDA add-ons. The exact `../addon/preview/`
global plugin keeps the user's existing NVDA voice and provides manual text
entry. The `../addon/synth/` package registers a selectable experimental
German SynthDriver. It speaks ordinary NVDA text sequences using the opt-in
generic C mode, without a preview shortcut. The Python modules transport
ECI PCM; acoustic control processing remains in C. The earlier exact preview
was loaded on the user's ARM64EC NVDA; the selectable package still needs a
Windows hearing test.

Since 0.2.78, the selectable driver's `_reloq_eci_transport.py` separates
ECI work from WavePlayer feeds with a bounded 64-operation audio queue.
Its actual initial reserve is 40 ms, with 80-ms subsequent batches and a
40-ms tail reserve. Shortened-pause handoff follows the final real feed.
Cancellation clears queued audio and markers; a rate change waits for a
late cancelled feed before closing its old player. See
`test_eci_playback_queue.py` and `test_eci_transport.py`.

`_reloq_dev_transport.py` is an unregistered development transport for
precomputed numeric control frames. A persistent worker owns the native
session; separate utterances remain in FIFO order, and cancellation clears
pending work, cancels native rendering and stops playback. It gives the last
PCM block an `nvwave.WavePlayer.feed(..., onDone=...)` compatible completion
callback. Thus it can start preparing a subsequent chunk without calling
`WavePlayer.idle()` between chunks. Its generation checks discard callbacks
and PCM from cancelled work. Run `python3 -B nvda/test_dev_transport.py` to
exercise FIFO, cancellation during blocked audio feed, the stop/next-chunk
race, blocked-worker shutdown and callback reentrancy with fake native and
audio objects. There is no `idle()` call at chunk boundaries. This transport
has no text entry point, `SynthDriver`, or NVDA registration.

After `make -C host test`, run
`python3 -B nvda/test_dev_transport_native.py [host/libreloq_host.so]`.
This second check uses the real C library: two numeric utterances both feed
their expected PCM before either playback callback is triggered, and only
those callbacks complete the jobs. It requires neither NVDA nor Windows.

The selectable driver sends the whole NVDA speech sequence, including every
`IndexCommand`, as one native ECI transaction. Interior marks reach a PCM word
boundary, with the corresponding index notification held until that PCM has
played; the single done notification is likewise playback gated. The player
stops before native cancellation, and there is no player `idle()` wait between
jobs. No artificial text length limit is imposed. Rate, pitch and volume are
worker-owned native controls that survive speech cancellation. Other languages,
sample rates and pause modes still need native support. Only spelling letter
boundaries and inline pitch marks are implemented; full inline prosody is not.
ECI stop is
thread-safe; settings, rendering and destruction run on the worker thread.

The developer build under `../host/` makes a Linux `.so` and runs a C ABI
test. With an installed 64-bit MinGW-w64 GCC and binutils, the Windows x64
DLL build and packaging checks can be run from the project root with:

```sh
make -C host windows-x64
```

The target creates `host/reloq_host.dll`, links the C ABI test client for
Windows, and compares the DLL's named exports to every public declaration in
`include/reloq_host.h`. It rejects non-system
runtime DLL dependencies; these would otherwise be missing on another NVDA
installation. The x64 cross-build has passed this check: the PE32+ DLL has
seven named exports and imports only `KERNEL32.dll` and `msvcrt.dll`; the
linked PE32+ test client imports `reloq_host.dll`. The
check does not execute the DLL under Windows; Wine and the Windows SDK are
absent here. `WINDOWS_CC` and `WINDOWS_OBJDUMP` may be set if the
cross-compiler has another name.

The x64 DLL can be loaded by an x64 or ARM64EC NVDA process on Windows 11;
a 32-bit or pure ARM64 NVDA process needs its own corresponding build. The
loader determines whether the supplied DLL actually works in the process.
A Linux `.so` cannot be packaged for NVDA on Windows. The selectable driver
now supports normal reading through a broad but inaccurate German
approximation; additional languages, rates and full speech commands remain
open. Both add-ons include a matching x64 DLL, manifest, audio worker and
package tests. Neither contains Apple/OpenEVV binaries, captured audio or
acoustic target sequences; the generic package includes an optional observed
pronunciation lexicon.
