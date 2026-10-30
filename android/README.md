# Android ARM64 preview

ReLoq C Preview uses the same C pronunciation and acoustic core as the NVDA
add-on. Version `0.2.198-preview10` targets ARM64 (`arm64-v8a`), Android 8/API 26
or later. Speech runs offline. Network access is used only when you explicitly update pronunciation dictionaries or check for an app update. Package downloads open in the browser.

## Install or update

Install `reloq-c-android-arm64-0.2.198-preview10.apk` over the existing preview.
The package remains `org.reloq.tts.preview`, the preview signing certificate is
unchanged, and versionCode increases from 20193 to 20194. Android can therefore
update the existing app and retain its settings. Do not uninstall it first.

Open **ReLoq C Preview** to adjust its settings or read a test text. To use it
with TalkBack or another client, open **Android speech settings** and choose
**ReLoq C Preview** as the preferred text-to-speech engine. All 14 language
variants and eight named voice presets are available. Both Chinese scripts
use the single Mandarin Chinese module; Japanese retains its existing analysis.

The user confirmed Android 0.2.192, including live settings, as stable.
0.2.194 fixes German abbreviation/quantity analysis and removes its supplemental lexicon; these
new controls still need their ordinary TalkBack check.

## Voice and output settings

The screen uses labelled standard Android checkboxes, sliders and lists, with
English and German UI text. Checkboxes, lists and sliders apply and save changes
immediately; there is no Save settings button. The next synthesis request uses
the new values. Speech already submitted keeps its request settings. **Read test
text** starts a speech sample, and **Stop** interrupts it.

- **Control language and voice through Android** lets clients choose a voice.
  Turn it off to use the saved language and voice for all requests.
- **Control speed through Android** and **Control pitch through Android** work
  independently. Turning either off enables its manual slider and overrides
  the corresponding value supplied by Android or the client.
- Manual speed is expressed as an approximate **140–1400 words per minute**,
  initially 190. Android's TTS request supplies a percentage with 100 meaning
  normal, rather than an absolute WPM count. Normal is mapped to native speed
  55; the inverse C duration curve makes 2× approximately twice as fast. Actual
  WPM depends on language, word lengths and pauses. Rate boost multiplies the
  requested speed by 1.6, within the native limits.
- Manual pitch, inflection, voice volume, male/female model, head size,
  roughness and breathiness are available. Voice values are stored separately
  for each of the eight presets. **Reset selected voice profile** restores that
  preset's anatomy and voice values; speed and output settings stay global.
- All seven sample rates from 8 to 48 kHz, the three native pause policies and
  optional ECI command processing are available. **Use abbreviation dictionary**
  starts enabled; **Phrase prediction** starts disabled. Prediction uses the
  language core to infer phrase boundaries in unpunctuated text. Main and Root
  pronunciation volumes remain separate from abbreviation expansion. Initial output is 11,025 Hz.
  Android's media volume continues to control playback independently of the
  engine's voice-volume setting.

## Pronunciation dictionaries

**Pronunciation dictionaries** opens the manager. Community dictionaries are
optional and initially disabled. Enabling them installs the verified bundled
snapshot for German and American English. **Update community dictionaries from
current master** resolves the current `eigencrow/IBMTTSDictionaries` master,
verifies the downloaded Git blobs and activates a complete new snapshot.
Synthesis does not access the network.

Select a language and Main, Root or Abbreviations before adding your own Dic
file through Android's file picker. Files use Windows-1252 and tab-separated
entries. You can reorder or remove sources; later own files override earlier
ones and community entries. Main preserves case; German Root retains its native
case folding. Replacements and phonetic entries go through the C core's normal
dictionary stages, including compound/context handling, rather than a Java
text substitution.

Selected sources remain linked using persistent file permissions. While the
speech service is running, the monitor checks modification metadata every two
seconds and reloads a stable change in the background. Changes made while the
app is closed are also detected after the service restarts. Automatic detection
depends on the document provider reporting modification time or size. **Reload
selected files** always reads them again, including with providers that do not
report those fields. A failed merge leaves the previous active snapshot intact.

Downloads, merges and source checks run outside the speech worker. With no own
files selected there is no polling thread; the dictionary worker also exits
after 15 seconds of inactivity. A request
holds its immutable snapshot until native synthesis finishes. Native tables
load once per used language and remain resident across ordinary requests and
cancellation; selecting a changed snapshot reloads that language when next used.

## Build on Linux x86-64

Requirements: Python 3, GNU Make, JDK 17, Android SDK platform 35, SDK build
tools 35.0.1, and Android NDK r30. The app contains no Python, JVM implementation,
Gradle, emulator or original DLL/SYN module. Each normal build resolves the
latest dictionary master and reuses existing verified data when possible.

```sh
export ANDROID_SDK_ROOT=/path/to/android-sdk
export ANDROID_NDK_HOME=/path/to/android-ndk-r30
export JAVA_HOME=/path/to/jdk-17
python3 android/build.py --upgrade-from dist/reloq-c-android-arm64-0.2.191-preview3.apk
```

Explicit `--sdk`, `--ndk`, `--java-home`, `--build-tools` and `--android-jar`
paths are also accepted. Omit `--upgrade-from` when there is no previous APK.
When given, it compares both signed artifacts' package names, versionCodes
and certificates and rejects an incompatible update. `--verified-dictionaries`
is reserved for a build where the current master commit has already been
resolved independently and the local snapshot has been verified. Output goes
under `dist/`. Android uses separate object directories and preserves the
Windows/Linux outputs. Rebuild the Android objects after changing compiler flags.

The checked-in `preview-debug.p12` is an intentionally public **test signing
identity**, alias `androiddebugkey`, password `android`. It keeps preview updates
installable over earlier previews. Production distribution needs a separate
private signing identity and a deliberate migration plan.

## Transport and resources

One persistent worker owns one native handle per used language. Profiles share
that handle, applying only changed controls. PCM buffers are allocated only for
used languages: 2,200 bytes for the first instead of eagerly reserving 30,800
bytes for all fourteen. No speech results are cached. A bounded queue carries
at most 16 blocks of 1,100 mono 16-bit samples. Only Android's synthesis thread
calls `SynthesisCallback`; callbacks do not outlive the request. Cancellation
wakes both sides, and delayed stops are scoped to their own requests.

German uses the built-in native C dictionary, just like the other languages.
The former 712,343-byte supplemental lexicon is neither packaged nor installed.
Updates remove only its generated content-named and old preview-named copies;
user `.dic` files are retained. Dictionary snapshots are reused by content and
unused old snapshots are removed after their last active request finishes.

Packaging uses DEFLATE level 9 while preserving aapt2's uncompressed resource
table. The APK additionally includes the optional community data, so its total
size is larger than preview 2 despite the stronger compression. All language
and acoustic data remain included. Release rule tracing is compiled out. German source knots now grow with
actual use rather than reserving the sum of six worst-case target buffers.
The 4,000-word host test requests 91,652,850 peak live heap bytes instead of
100,477,682. This is a requested-allocation measurement, not Android RSS.
The new AVX2 variants apply to supported native x64 hosts; native ARM64 keeps
its existing arithmetic path with shared constant-window/logarithm savings.

Text enters C as standard UTF-8, including supplementary Unicode characters.
The ARM64 backtracking boundary saves the AAPCS64 preserved registers, Android's
platform register, stack/link registers and floating-point state. Native `char`
is explicitly signed. ELF segments use 16 KiB alignment; final resource-table
storage/alignment, signatures and ZIP alignment are checked during the build.

## Validation

Host tests exercise 112 voice identifiers, locale routing, streaming order,
backpressure and cancellation. The settings test covers speed inversion,
physical pitch scaling, manual overrides, preset separation, preference
migration and dictionary byte/case/priority handling. A three-passage native
duration comparison measures about 174 WPM at normal and 353 WPM at 2×, a 2.03×
duration ratio. These figures illustrate why the slider's WPM label is nominal.

```sh
python3 android/test_settings.py --java-home "$JAVA_HOME" \
  --android-jar "$ANDROID_SDK_ROOT/platforms/android-35/android.jar"
python3 android/test_native_transport.py --java-home "$JAVA_HOME"
```

The JNI test compares 35 actual native PCM hashes/lengths with direct C calls:
all language variants, all US presets, mixed UTF-8, cancellation, stale-stop
isolation, customized female anatomy, 48 kHz, tags and dictionary switches.
PCM remains in RAM; no audio files are written. These are host transport tests,
not Android execution or real-device acoustic measurements.

The NDK-compiled ARM64 register/stack test uses Unicorn and pyelftools:

```sh
python3 android/test_arm64_landing.py \
  vendor/openevv61/build/android-arm64/src/port/evv_land.o
python3 android/verify_apk.py dist/reloq-c-android-arm64-0.2.198-preview10.apk
```

The final APK passes signature, install-critical resource-table and 16-KiB ELF checks. The package/signature comparison supports an in-place update directly from 0.2.192-preview4. The user confirmed that version as stable. The browser download handoff still needs its ordinary device/TalkBack check once a release package is published. The user reports that Play Protect accepts 0.2.197. This build keeps its browser update flow and INTERNET-only permission profile.

For an isolated build, pass `--build-dir /absolute/fresh/build-directory`; the output APK path remains controlled by `--output`.

## App updates

Choose **App updates** on the settings screen, then **Check for updates**. The app checks published GitHub Releases on request. If a newer ARM64 package is available, choose **Open download in browser**. The release link must name this project’s newer ARM64 APK on GitHub, with valid size and SHA-256 metadata. The browser downloads the file and Android handles installation and signature checks. ReLoq does not request package-install permission or provide an APK installation service. Keep Play Protect enabled; a warning is not a successful security check.

The repository currently has no published package release. Until packages are published, the screen reports that no newer published app package is available. There is no background polling and the speech service does not perform update downloads. Package updates and dictionary updates are separate actions.
