ReLoq C 1.1.1 — unified SAPI5 Windows installer

Installation
------------
Run the supplied ReLoq C SAPI5 setup executable. Inno Setup offers the supported interface
languages and initially selects the Windows display language. Administrator
permission is requested. The single installer installs both x64 and x86 voices
on compatible 64-bit Windows, and x86 voices on 32-bit Windows. Each installed
architecture registers 14 languages. Restart existing speech applications.
Choose SAPI5 and a “ReLoq C” voice.

The installation is under %ProgramFiles%\ReLoq C\SAPI5. The x86 components
live in its x86 subfolder. Both architectures use one common dictionaries
folder. Existing dictionary edits and user settings survive upgrades. Uninstall
through Windows Installed apps or the Start menu shortcut; the same Inno
uninstaller removes both architectures and their voice registrations, retaining
user dictionary files and per-user settings. Future installers update this
installation in place.

Voice settings
--------------
Open Start > ReLoq C SAPI5 > ReLoq C - Voice settings, ReLoqSettings.exe,
or the voice's Settings / Properties in a SAPI program. The same Start menu
group contains this guide and the uninstaller. Shortcut names follow the
installer language and are refreshed when the installation language changes.
The dialog follows the Windows display language for all supported languages,
including both Chinese scripts, with English as fallback. Standard Windows
controls support keyboard and screen readers. The standalone dialog initially
selects the Windows language; voice properties initially select that voice.

Changes are stored immediately and apply to the next utterance in running
applications. No Save button is needed. “Use the same settings for all
languages” shares profiles, anatomy, analysis and dictionary settings. Enabling
it takes the current language's parameters. Disabling it restores the separate
language settings. The x86 and x64 versions share these user settings.

The synthesis sample rate applies to all languages, independently of that checkbox.
It remains the selected Eloquence rate even if an application requests a different
output rate. With Eloquence at 11,025 Hz and the application at 48,000 Hz, SAPI
or the application upsamples the 11,025-Hz signal. ReLoq has no extra resampler.
Only formats actually advertised by this voice are accepted by Speak; an
application's target cannot silently select a wider acoustic profile.
Existing clients must renegotiate, reopen the voice or restart to adopt a changed
Eloquence synthesis rate. Legacy rate migration uses
the saved German rate, then US English, then the first configured language.
The default is 11,025 Hz. Reset preserves the common sample rate.

Eight profiles, male/female model, head size, pitch, inflection, roughness,
breathiness, seven rates, pause shortening, speed boost, ECI tags, abbreviations,
phrase prediction and dictionaries are available. SAPI controls speed and
volume. Normal speed follows SAPI5IBMTTS: ECI = 60 + 10 × (SAPI rate + XML
adjustment), bounded to 0–250. SAPI 0 is ECI 60; +10 is ECI 160; values at
-6 or lower select ECI 0. Optional speed boost multiplies this by 1.6, capped
at 250. Leave boost off for the reference scale. Profiles start at volume 100. Normal SAPI XML pitch, rate, volume and
spelling fragments use the shared C engine.

Responsiveness and dictionaries
-------------------------------
The initialized language prepares speech on a worker while SAPI delivers it
through a bounded queue. A short announcement can therefore finish rendering
while it is being played, making interruptions more responsive. For the first
output, the adapter collects approximately 60 ms of source audio in one block.
Additional short blocks during the first 500 ms help clients that apply speed-up;
later blocks contain up to 4,096 samples. Event and fragment boundaries are
flushed at their exact positions. The samples and native synthesis rate are unchanged.

Unchanged dictionary paths retain initialized language data. Small metadata
checks detect edits, replacement, creation and deletion of native .dic files
between utterances. No text result or synthesized speech is cached. Aborted
audio is discarded, and the next announcement is generated afresh.

Enable dictionaries and choose either the bundled IBM TTS Dictionaries or
Alternative IBM TTS Dictionaries in the settings window. The alternative is
downloaded and verified in the background and can be updated there. The
alternative source currently provides US English and Castilian Spanish files.
A successful alternative dictionary download displays a dialog with the
number of new distinct entries. Modified and removed entries do not count as
additions. The first download shows the total; if an earlier copy cannot be
compared, the dialog reports that the count is unavailable.
A custom dictionary folder overrides the selected source and stays unchanged.
SAPI's lexicon editor is not implemented.

Validation and limits
---------------------
The user confirmed basic x64 SAPI operation. Host tests cover the production
backend with the real core: languages, rates, events, fragments, partial writes,
abort/reuse, priority voices and dictionary edits. PE architectures, exports
and native imports are checked. Automated Windows installation, SAPI stream
conversion and removal are exercised by the installer workflow. This does not
measure audible latency or dropouts on individual sound devices.

Test-SAPI.exe checks all 14 installed languages with eight successive application
format requests, including reverse changes. It verifies that native synthesis
stays at the selected rate, the application stream uses its requested rate and
utterance duration remains consistent. Speech stays in memory: no audio file
or audio device is opened. Run:
    Test-SAPI.exe > sapi-diagnostics.txt

The optional --all-native-rates switch repeats this with all seven Eloquence
rates (784 cases), temporarily changing and restoring the user's common rate.

These are native x86/x64 builds, not native ARM64. Phoneme-ID input, SAPI
lexicons and visemes are not implemented. Sentence skip reports zero skipped
sentences and stops the stream. Mid-utterance speed changes apply to the next
utterance; volume and abort are handled while streaming. XML rate/volume
changes apply at fragment-state boundaries. Settings do not change queued PCM.

Neither engine needs NVDA, Python, original Eloquence DLL/SYN files, bytecode,
a helper process, temporary audio files or an audio cache.

Project: https://github.com/hozosch/Reloq-C

Building from source
--------------------
Install the x64 and/or i686 MinGW-w64 C toolchains and Python for packaging.
    make -C eci windows-x64
    python3 sapi/build.py --arch x64
    make -C eci windows-x86
    python3 sapi/build.py --arch x86
The two core/object/archive trees are separate. The x86 core uses SSE2 scalar
arithmetic and exports the same undecorated ECI function names. Python is a
build tool and is not installed as a runtime dependency.

Build both architecture payloads, then run the Inno Setup 6.7 compiler on Windows:
    python sapi/build_installer.py --iscc "C:\Program Files (x86)\Inno Setup 6\ISCC.exe"
The .github/workflows/sapi-installer.yml workflow builds, installs, tests and
uninstalls the combined package. Internal payload ZIPs are not installers.
