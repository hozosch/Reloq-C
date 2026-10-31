ReLoq C 0.2.202 — SAPI5 Windows x64 / x86 preview 2

Installation
------------
Use the package matching your application's architecture. On 64-bit Windows,
install both packages for both 32-bit and 64-bit speech applications. Extract
each ZIP into its own folder and run Install-SAPI.exe, then Install / update.
Do not mix architectures. Normal Windows administrator permission is requested.
Each installer registers 14 voices in its matching registry view. Restart
applications already open. Select SAPI5, then e.g. “ReLoq C - German”.

x64: %ProgramFiles%\ReLoq C\SAPI5
x86 on 64-bit Windows: %ProgramFiles(x86)%\ReLoq C\SAPI5
x86 on 32-bit Windows: %ProgramFiles%\ReLoq C\SAPI5
Each installation contains one native core DLL. Its Install-SAPI.exe removes
only that architecture's voice/COM registration, keeping user dictionaries
and settings. Both architectures can be installed together.

Voice settings
--------------
Open ReLoqSettings.exe or the voice's Settings / Properties in a SAPI program.
The dialog follows the Windows display language for all supported languages,
including both Chinese scripts, with English as fallback. Standard Windows
controls support keyboard and screen readers. The standalone dialog initially
selects the Windows language; voice properties initially select that voice.

Changes are stored immediately and apply to the next utterance in running
applications. No Save button is needed. “Use the same settings for all
languages” shares profiles, anatomy, analysis and dictionary settings. Enabling
it takes the current language's parameters. Disabling it restores the separate
language settings. The x86 and x64 versions share these user settings.

Sample rate ALWAYS applies to all languages, independently of that checkbox.
It is advertised during format negotiation; SAPI converts to the application's
output format if necessary. Existing clients must renegotiate the format,
reopen the voice or restart to adopt a changed rate. Preview-1 migration uses
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
The owner thread retains the initialized language. Unchanged dictionary paths
no longer reload it. Small metadata checks detect edits, replacement, creation
or deletion of native .dic files between utterances. Only initialized linguistic
data is retained, never text results or synthesized speech. Eight small live
PCM packets bridge the owner thread to SAPI's caller. Abort also reaches the
native planner. No helper process or speech-result cache is used.

Enable dictionaries for the bundled master snapshot from
eigencrow/IBMTTSDictionaries. A writable custom folder may contain native .dic
files; an empty path uses the bundled folder. Upgrades preserve edited files.
SAPI's lexicon editor and a dictionary download/update UI are not implemented.
NVDA and Android retain their existing dictionary managers.

Validation and limits
---------------------
The user confirmed basic x64 SAPI operation. The new x86 package still needs
real Windows installation and client testing. Both COM layouts match the
Microsoft SDK. PE architecture, exports and native imports are checked. Host
tests exercise the production backend with the real core: languages, rates,
events, fragments, partial writes, abort/reuse, priority voices and .dic edits.
Host first-Write timing is distinct from audible Windows/device latency.

Test-SAPI.exe checks actual SAPI activation of the 14 installed languages.
Speech stays in memory: no audio file or audio device is used. Run:
    Test-SAPI.exe > sapi-diagnostics.txt

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
