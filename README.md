# ReLoq C

ReLoq C is a speech synthesis project that preserves the familiar character of Eloquence while improving pronunciation, responsiveness and control over the voice. Apple Eloquence serves as the reference for language behaviour and sound.

The shared synthesis engine is written in C. NVDA, Android ARM64 and Windows SAPI5 use the same core. The project is developed and tested with screen-reader users; language reliability, responsiveness and lower resource use remain active priorities.

The development branch is preparing **1.1.0-rc25**. German now uses the
shared native sentence and acoustic path, including long sentences and
expressions with hyphens or slashes. Native word boundaries carry voice tags,
while caller dictionaries and the accepted male-only initial /ts/ adjustment
remain available. The established C high-rate sound and Greek/Castilian filter
are retained. German number transitions, “Homepage”, “veröffentlicht” and
“samt” have further listening-test adjustments; the new voice needs feedback;
see [the current work and verification limits](docs/CURRENT-WORK.md).
Android Direct Boot retains separate copies of active settings and dictionaries
before first unlock.

## Download and install

The [latest release](https://github.com/hozosch/Reloq-C/releases/latest) provides three installable packages:

| Platform | Package | Requirements |
| --- | --- | --- |
| NVDA | `.nvda-addon` | NVDA 2026.1 or later, 64-bit process |
| Android | ARM64 `.apk` | Android 8 or later, ARM64 device |
| Windows SAPI5 | `-setup.exe` | Installs x64 and x86 voices together on compatible Windows |

Release **1.0.1** restores the original S/Z source levels and improves interruption of obsolete text analysis. New NVDA configurations use NVDA's language when supported. Install packages over an existing installation to retain settings. Existing Android package and signing identities, and the NVDA installation identity, are retained for in-place updates. Source archives are for developers and are not installable packages. [Release notes](docs/releases/1.0.1.md) describe the update and its limits.

## Current features

- **14 language variants**, including German, American and British English, Chinese, Japanese, Korean and Greek.
- **Eight voice presets:** Reed, Shelly, Bobby, Rocko, Glen, Sandy, Grandma and Grandpa, with adjustable head size, pitch, roughness and breathiness.
- **Seven sample rates from 8 to 48 kHz**, covering the classic sound and an extended frequency range.
- **Unicode and language-specific character names**, extending the handling of letters, accents and symbols.
- **Optional pronunciation dictionaries:** community data for German and American English, plus your own Dic files managed through NVDA settings or the Android dictionary screen.
- **Pause shortening, optional ECI commands, abbreviation dictionaries and phrase prediction.**

## Goals and platform roadmap

First, make the supported languages and everyday speech reliable. This includes word analysis, compounds, sentence melody and transitions between sounds. Accepted sound improvements, such as the German consonantal r and adjusted nasal vowels, are retained. Speech behaviour is currently evaluated in NVDA.

Next, reduce binary size, memory use and CPU demand, and keep navigation responsive even at high sample rates. These improvements belong in the shared C core so that every platform can benefit. The project aims for compact, independently expressed C rules without runtime bytecode or original DLL/SYN modules.

All three integrations are available. Android offers an offline TTS service, live controls and pronunciation dictionaries. SAPI5 provides one voice per language and a native dialog with live settings, optional shared parameters and a common synthesis rate. Its unified installer supplies both architectures, localized installation and Start menu entries for settings, the guide and removal. See the [SAPI guide](sapi/README.txt). Native Windows ARM64 SAPI support remains future work; the current Windows components are x64 and x86.

## Using the NVDA add-on

The synthesizer is listed as **Eloquence C**. It requires NVDA 2026.1 or later in a 64-bit process. The DLL is currently built for Windows x64.

Install the `.nvda-addon` file from [Releases](https://github.com/hozosch/Reloq-C/releases) with NVDA, restart NVDA and press **NVDA+Control+S** to select **Eloquence C**. Choose your language and voice in NVDA's Speech settings. The ReLoq settings category manages dictionaries and checks for package updates on request.

The [English user guide](docs/USER-GUIDE.en.md) and [German user guide](docs/USER-GUIDE.de.md) cover languages, settings and dictionaries. [Changes in 1.0.1](docs/changes/1.0.1.md) are documented separately.

## Android

The Android app targets **ARM64 devices running Android 8 or later**. It includes all 14 language variants and eight voice presets, runs offline, and appears as **ReLoq C** in Android's text-to-speech settings. Its launcher screen provides all voice/output controls, independent Android speed and pitch overrides, and optional community/custom dictionaries.

The [Android instructions](android/README.md) explain installation, building and validation limits. Android's live settings and speech engine have been confirmed in practical use. Automated host tests complement device testing; they do not establish audible performance on every device.

## Feedback and contributions

Please report pronunciation errors, incomplete speech, freezes and everyday usability problems through [Issues](https://github.com/hozosch/Reloq-C/issues). Include a short example, language, voice preset, version, sample rate, and the expected and actual behaviour. Mention whether pronunciation dictionaries are enabled.

Developer information:

- [Build and packaging instructions, in German](docs/BUILD.md)
- [Technical work status and open faults, in German](docs/CURRENT-WORK.md)
- [Agreed pronunciation and sound choices, in German](GEWUENSCHTE-ABWEICHUNGEN-DE.md)
- [Earlier version notes](docs/history/README.md)

The original ReLoq C code is licensed under [GPL version 3 or any later version](LICENSE). See [NOTICE](NOTICE) for its scope and the redistribution terms for `eci.dll`. Matching source and build scripts are available in this repository. Third-party components and data retain their own licenses.

The NVDA adapter is written in Python. Component and data notices are retained with their sources, including the [OpenEVV license](vendor/openevv61/LICENSE), [OpenEVV notices](vendor/openevv61/NOTICE) and [dictionary license](data/community/LICENSE.md).
