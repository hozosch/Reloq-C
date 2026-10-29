# Android ARM64 preview

ReLoq C Preview uses the same C pronunciation and acoustic core as the NVDA
add-on. Version `0.2.189-preview1` targets ARM64 (`arm64-v8a`), Android 8/API 26
or later. It runs offline and requests no network permission.

## Try it

Install `reloq-c-android-arm64-0.2.189-preview1.apk`, then open **ReLoq C Preview**.
Choose a test language, voice, sample rate and pause policy, and press **Read test
text**. The screen uses ordinary labelled Android controls; English and German
UI text are included. **Stop** interrupts the current test.

To use the engine with TalkBack or another client, open **Android speech
settings** from the app and select **ReLoq C Preview** as the preferred engine.
The engine exposes 14 language variants with all eight named presets. The
client's voice selection takes precedence over the preview's saved default.
Speech rate and pitch come from Android's normal TTS request settings.

The initial sample rate is 11,025 Hz. The app supports the same seven rates
from 8 to 48 kHz as NVDA. Its test language selection affects the test screen;
each Android client supplies its own language or voice. Both Chinese scripts
use the single Mandarin Chinese module. Japanese retains its existing analysis.

The optional community/custom dictionary manager is currently NVDA-only.
The built-in German lexicon is bundled in the APK. No additional voice-data
download or original DLL/SYN module is needed.

## Build on Linux x86-64

Requirements: Python 3, GNU Make, JDK 17, Android SDK platform 35, SDK build
tools 35.0.1, and Android NDK r30. These are build-time dependencies; the app
does not contain Python, a JVM implementation, Gradle or an emulator.

```sh
export ANDROID_SDK_ROOT=/path/to/android-sdk
export ANDROID_NDK_HOME=/path/to/android-ndk-r30
export JAVA_HOME=/path/to/jdk-17
python3 android/build.py
```

Explicit `--sdk`, `--ndk`, `--java-home`, `--build-tools` and `--android-jar`
paths are also accepted. The APK is written under `dist/`. The native build
uses separate object/archive directories and preserves Windows/Linux outputs.
Rebuild those Android directories after changing compiler flags.

The checked-in `preview-debug.p12` is an intentionally public **test signing
identity**, alias `androiddebugkey`, password `android`. It keeps local preview
updates installable over earlier previews. It is not a production credential;
production distribution must use a separate private signing identity.

## Transport and resource use

One persistent worker owns one native handle per used language. Voice changes
apply anatomy/source controls to that language's existing handle. No speech
result is cached. A bounded queue carries at most 16 blocks of 1,100 mono
16-bit samples between the native producer and Android's synthesis thread.
Only the latter calls `SynthesisCallback`; callbacks do not outlive the request.
Cancellation wakes both sides, and a delayed stop is scoped to its own queue.
Engine destruction is queued on the owner worker rather than blocking the UI.

Spoken text enters C as standard UTF-8 bytes, including supplementary Unicode
characters. The JNI modified-UTF-8 string representation is not used for text.
The ARM64 backtracking boundary saves the AAPCS64 preserved registers, Android's
platform register, stack/link registers and floating-point state. Native `char`
is explicitly signed to match the established Windows/Linux core semantics.
ELF segments use 16 KiB alignment; the APK is alignment-checked and signed.

## Validation and remaining work

The host JNI test exercises actual native synthesis for all 14 language
variants, all eight US presets, accented/mixed UTF-8, bounded streaming,
cancellation, stale-stop isolation and speech after cancellation. Its 26 PCM
hash/length pairs match direct C calls. PCM stays in RAM. This is a transport
test on the host, not Android execution or a real-device acoustic comparison.

```sh
javac -d android/build/test-classes android/java/org/reloq/tts/preview/PcmQueue.java \
  android/java/org/reloq/tts/preview/Catalog.java android/test/PcmQueueTest.java
java -cp android/build/test-classes org.reloq.tts.preview.PcmQueueTest
python3 android/test_native_transport.py --java-home "$JAVA_HOME"
```

The ARM64 register/stack test executes the NDK-compiled instructions using
Unicorn and pyelftools:

```sh
python3 android/test_arm64_landing.py \
  vendor/openevv61/build/android-arm64/src/port/evv_land.o
```

The APK's manifest, DEX, native architecture, dependencies, 16 KiB segment
alignment and signature have been inspected. Actual installation, service
discovery, TalkBack navigation, sustained playback and 48 kHz performance on a
Pixel have **not yet been tested**. Those are the next practical checks.
