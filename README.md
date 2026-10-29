# ReLoq C

ReLoq C is a speech synthesis project that preserves the familiar character of Eloquence while improving pronunciation, responsiveness and control over the voice. Apple Eloquence serves as the reference for language behaviour and sound.

The shared synthesis engine is written in C. **NVDA is the first integration and the current testing platform. Windows SAPI5 and Android are planned next.** Work on those integrations will follow the remaining language and stability fixes, and reductions in the core's size and resource use.

## Current features

- **14 language variants**, including German, American and British English, Chinese, Japanese, Korean and Greek.
- **Eight voice presets:** Reed, Shelly, Bobby, Rocko, Glen, Sandy, Grandma and Grandpa, with adjustable head size, pitch, roughness and breathiness.
- **Seven sample rates from 8 to 48 kHz**, covering the classic sound and an extended frequency range.
- **Unicode and language-specific character names**, extending the handling of letters, accents and symbols.
- **Optional pronunciation dictionaries:** community data for German and American English, plus your own Dic files managed through NVDA settings.
- **Pause shortening and optional ECI commands.**

## Goals and platform roadmap

First, make the supported languages and everyday speech reliable. This includes word analysis, compounds, sentence melody and transitions between sounds. Accepted sound improvements, such as the German consonantal r and adjusted nasal vowels, are retained. Speech behaviour is currently evaluated in NVDA.

Next, reduce binary size, memory use and CPU demand, and keep navigation responsive even at high sample rates. These improvements belong in the shared C core so that every platform can benefit. The project aims for compact, independently expressed C rules without runtime bytecode or original DLL/SYN modules.

Then add a **SAPI5 integration for Windows** and an **Android speech engine** using the same core. These are planned targets, not currently available packages.

## Using the NVDA add-on

The synthesizer is listed as **Eloquence C (experimental)**. It requires NVDA 2026.1 or later in a 64-bit process. The DLL is currently built for Windows x64.

Installable packages belong under [Releases and downloads](https://github.com/hozosch/Reloq-C/releases). If no `.nvda-addon` file is listed there, GitHub does not yet provide an installable package. The source download is intended for building the project yourself.

Once a package is available, install the `.nvda-addon` file with NVDA, restart NVDA and press **NVDA+Control+S** to select **Eloquence C (experimental)**. Choose your language and voice in NVDA's Speech settings.

The [English user guide](docs/USER-GUIDE.en.md) and [German user guide](docs/USER-GUIDE.de.md) cover languages, settings and dictionaries. [Changes and known limitations in 0.2.188](docs/changes/0.2.188.md) are documented separately.

## Feedback and contributions

Please report pronunciation errors, incomplete speech, freezes and everyday usability problems through [Issues](https://github.com/hozosch/Reloq-C/issues). Include a short example, language, voice preset, version, sample rate, and the expected and actual behaviour. Mention whether pronunciation dictionaries are enabled.

Developer information:

- [Build and packaging instructions, in German](docs/BUILD.md)
- [Technical work status and open faults, in German](docs/CURRENT-WORK.md)
- [Agreed pronunciation and sound choices, in German](GEWUENSCHTE-ABWEICHUNGEN-DE.md)
- [Earlier version notes](docs/history/README.md)

The NVDA adapter is written in Python. Component and data notices are retained with their sources, including the [OpenEVV license](vendor/openevv61/LICENSE), [OpenEVV notices](vendor/openevv61/NOTICE) and [dictionary license](data/community/LICENSE.md).
