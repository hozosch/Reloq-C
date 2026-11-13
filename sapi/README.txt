ReLoq C — unified SAPI5 Windows installer

Installation
------------
Run the supplied ReLoq C SAPI5 setup executable. Inno Setup offers the supported interface
languages and initially selects the Windows display language. Administrator
permission is requested. The single installer installs both x64 and x86 voices
on compatible 64-bit Windows, and x86 voices on 32-bit Windows. Each installed
architecture registers 15 languages. Restart existing speech applications.
Choose SAPI5 and a “ReLoq C” voice.

Aragonese is an initial test voice; number and character names are not yet
fully localized. SAPI has no assigned Aragonese Windows language identifier,
so the test voice uses the identifier for Spanish (Spain) in SAPI metadata.
Applications that filter voices by language may therefore group it with Spanish.

The installation is under %ProgramFiles%\ReLoq C\SAPI5. The x86 components
live in its x86 subfolder. Both architectures share the current user's settings
and dictionary data under %LOCALAPPDATA%\ReLoq C\SAPI5\Dictionaries. Existing
installed dictionary files are copied to the user profile on first use and
are not changed. Existing dictionary edits and user settings survive upgrades. Uninstall
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
phrase prediction and dictionaries are available. By default, SAPI controls
speed and volume. Normal speed follows SAPI5IBMTTS: ECI = 60 + 10 × (SAPI rate + XML
adjustment), bounded to 0–250. SAPI 0 is ECI 60; +10 is ECI 160; values at
-6 or lower select ECI 0. Optional speed boost multiplies this by 1.6, capped
at 250. Leave boost off for the reference scale. Profiles start at volume 100. Normal SAPI XML pitch, rate, volume and
spelling fragments use the shared C engine.

Choose “Application overrides” to independently use ReLoq's own speed, pitch
or volume instead of the application's values. Choose between native ECI speed
0–250 and 1–6000 words per minute using the radio buttons and arrow keys; Tab
moves to the selected value. Speed boost applies only when SAPI controls
speed. Choose the pitch in ECI units 0–100 or in hertz; this is the same base
pitch also shown in the main window. Volume uses 0–100. These settings affect only
ReLoq C and can be enabled separately for each language or shared across them.

Choose “Equalizer” for bass, midrange, treble and presets. The “Advanced
filters” dialog edits one of five independently enabled filters at a time:
frequency 1–24000 Hz, gain -12 to +12 dB, and bandwidth 10–400 percent
of an octave (100 percent is one octave). Frequencies at or above half the
synthesis sample rate are inactive but remain saved. Simple and advanced
filters combine; switching off the equalizer bypasses them. Peaks are limited
only when necessary. EQ settings follow the chosen language or shared settings.

In the “Base pitch” group, “Use Hz” is followed by one value field. With the
checkbox off the field edits the ECI value from 0 to 100; with it on the same
field edits a whole-number base pitch from 1 to 24,000 Hz. Each scale retains
its saved value when switching.
SAPI application pitch 50 remains neutral and uses this base; application pitch
adjustments still work unless the pitch override is enabled. Inflection 0 makes the base easier to measure, although
tonal languages can retain linguistic contours. The synthesis rate must exceed
twice the requested frequency. Voice filters can attenuate very high pitches;
an insufficient rate can distort or interrupt synthesis.

With ECI tags enabled, `vwpm150 sets the following speech to 150 using the
native WPM timing rule, and `vs50 returns to the ECI speed scale. Whole numbers
from 1 to 6000 are accepted. At 1 to 4 WPM, the synthesis control frames are
slowed to avoid overflow while retaining pitch. Audible WPM depends on language,
text and pauses; very high values can also hit timing limits. Tags do not change
saved settings.

Responsiveness and dictionaries
-------------------------------
The language synthesizes on the SAPI caller's thread and delivers small audio
batches directly to SAPI. The first batch supplies the usual 50 ms startup
reserve; events retain their exact audio positions. A caller reuses its own
initialized language contexts; contexts are not transferred between threads.
The samples and synthesis rate are unchanged.

“Use original question intonation” selects the native question contour for
German and US English; it is off by default. Existing settings are retained
when upgrading. “Mellow mode” softens periodic voice presence around 3–4 kHz at synthesis
rates of 16 kHz and above, omits added richness from 22 kHz and reduces
voiced breathiness; it is off by default.
“Original timing (5 ms)” selects the clock for the chosen language. It is on
by default for Castilian Spanish, Greek and Aragonese, and off for the other languages.
With shared settings enabled, a manual choice applies to every language.
The speed and WPM scales do not change.
Unchanged dictionary paths retain initialized language data. Small metadata
checks detect edits, replacement, creation and deletion of native .dic files
between utterances. No text result or synthesized speech is cached. Aborted
audio is discarded, and the next announcement is generated afresh.

Enable community dictionaries and choose either IBM TTS Dictionaries or
Alternative IBM TTS Dictionaries in the settings window. Both sources can be
downloaded and updated there. The bundled community source supports German and
US English; the alternative supports US English and Castilian Spanish. Downloads
are verified in the background and stored in the current user's profile. A
manual update displays the numbers of new distinct entries and files. An
automatic initial download for the alternative source shows a temporary Windows
notification; it does not open a modal dialog. Changes and removals do not
count as additions. Windows 10 retains these notifications in its notification
center, while Windows 11 treats them as temporary.

Choose “Own dictionaries” to add Eloquence .dic files or NVDA speech
dictionaries. Select the language and, for Eloquence files, Main, Root or
Abbreviations. For NVDA files, select all voices or a specific voice. The
dialog suggests the format; change it if an ambiguous file was identified
incorrectly. Existing automatically selected NVDA files are recognized on
reload. The original files are never altered and personal files remain active
when community dictionaries are disabled. Changes to selected source files are
detected between utterances.

Eloquence files use the native dictionary stages; later own files override
earlier files and the selected community source. NVDA files use UTF-8, retain
their entry order, case setting and match type, and are applied before speech
and word segmentation. They support ordinary regular-expression captures.
SAPI uses PCRE2 for regular expressions, so some Python-specific patterns and
Unicode case-folding details can differ from NVDA. Invalid patterns are
reported when selecting the file. A previously configured custom folder still
overrides the chosen community source. SAPI's lexicon editor is not implemented.

Validation and limits
---------------------
The user confirmed basic x64 SAPI operation. Host tests cover the production
backend with the real core: languages, rates, events, fragments, partial writes,
abort/reuse, priority voices and dictionary edits. PE architectures, exports
and native imports are checked. Automated Windows installation, SAPI stream
conversion and removal are exercised by the installer workflow. This does not
measure audible latency or dropouts on individual sound devices.

Test-SAPI.exe checks all 15 installed languages with eight successive application
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
changes apply at fragment-state boundaries. Settings do not change audio
already delivered to the application.

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

Program updates
---------------
Open the voice settings and choose “Program updates…” to enable or disable
automatic checks or check immediately. A separate background process checks
at login and daily thereafter. With your confirmation, it downloads the newer
installer, verifies its published SHA-256 and starts the normal setup.
