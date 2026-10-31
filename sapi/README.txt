ReLoq C 0.2.201 — SAPI5 Windows x64 preview 1

Extract the ZIP into a folder, then run Install-SAPI.exe and choose Install /
update. Installation requests normal Windows administrator permission and
registers 14 language voices. Restart applications which were already open.
Select SAPI5 in NVDA, then a voice named “ReLoq C - German”, for example.

Voice settings
-------------
Run ReLoqSettings.exe or use the voice's Settings / Properties button in a
SAPI application. Choose a language, then its profile or individual controls.
Changes are stored immediately for your Windows account and apply to the
next utterance, including speech in already running applications. There is no
Save button. A sample-rate change takes effect when the SAPI client next
negotiates its audio format; a client may request a different supported rate.

The dialog offers all eight profiles, male/female model, head size, base pitch,
inflection, roughness, breathiness, seven sample rates, pause shortening, speed
boost, ECI tags, abbreviations, phrase prediction and optional dictionaries.
SAPI controls rate and volume; profiles all start at volume 100. Ordinary
SAPI XML pitch, rate, volume and spelling fragments use the same C engine.

Optional dictionaries
---------------------
Enable the dictionary checkbox to use the bundled current master snapshot
from eigencrow/IBMTTSDictionaries. You can select a writable custom folder
containing native .dic files; edits are detected between utterances. Leave
the path empty for the bundled folder. Installer upgrades preserve existing
dictionary files. SAPI's lexicon editor and a dictionary download/update UI
are not implemented in this first preview. NVDA and Android retain their
existing dictionary managers.

Installation location: %ProgramFiles%\ReLoq C\SAPI5
ReLoqSettings.exe and Install-SAPI.exe remain available there after extraction
files are removed. Uninstall voices using Install-SAPI.exe. This unregisters
only ReLoq tokens; user settings and dictionaries are preserved.

Testing and preview limits
--------------------------
The production streaming backend is checked with the real C core on the
build host: 14 languages, all seven rates, fragments, source events, partial
writes, cancellation, changed profiles and simultaneous priority voices.
The x64 COM DLL and installer are cross-compiled and their exports/imports
are inspected. Actual Windows COM activation, installation and screen-reader
navigation still need device testing. This is not the first public release.

Test-SAPI.exe checks real SAPI activation for all 14 installed languages,
keeping the audio in memory. It prints a text result and does not create an
audio file or open an audio device. It may be run from a command prompt as:
    Test-SAPI.exe > sapi-diagnostics.txt

This build supports 64-bit SAPI clients. It is not a native ARM64 or 32-bit
engine. Phoneme-ID input, SAPI lexicons and viseme events are not implemented.
Sentence skip currently reports zero skipped sentences and stops the stream.
Rate changes made during an already rendering utterance apply to the next
utterance; volume and abort actions are handled during streaming. Ordinary
XML rate/volume changes take effect at explicit fragment-state boundaries.
Engine settings affect the next utterance, not already queued PCM.

The package contains one copy of the shared C synthesizer. It has no NVDA,
Python, original Eloquence DLL/SYN, bytecode or helper-process dependency.
No audio cache or temporary speech file is used.

Project: https://github.com/hozosch/Reloq-C
