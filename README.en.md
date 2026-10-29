# ReLoq C

[Deutsch](README.md)

ReLoq C is a speech synthesizer for NVDA that preserves the familiar character of Eloquence while improving pronunciation, reading and everyday responsiveness.

Apple Eloquence provides a reference for language behaviour and sound. ReLoq C aims to correct pronunciation errors and limitations, extend Unicode support, offer higher sample rates and give users more control over their voices.

## Features

- **14 language variants**, including German, American and British English, Chinese, Japanese, Korean and Greek.
- **Eight voice presets:** Reed, Shelly, Bobby, Rocko, Glen, Sandy, Grandma and Grandpa, with adjustable head size, pitch, roughness and breathiness.
- **Seven sample rates from 8 to 48 kHz**, covering the classic sound and an extended frequency range.
- **Unicode and language-specific character names**, extending the handling of letters, accents and symbols.
- **Optional pronunciation dictionaries:** community data for German and American English, plus your own Dic files managed through NVDA settings.
- **Pause shortening and optional ECI commands.**

## Project goals

The goal is fast, intelligible and reliable speech for daily NVDA use. Work includes word analysis, compounds, sentence melody and transitions between sounds. Accepted sound improvements, such as the German consonantal r and adjusted nasal vowels, are retained.

A compact, efficient C engine is another priority: short response times for navigation and long texts, and lower CPU use at high sample rates. Improvements should benefit every supported language.

## Using it with NVDA

The synthesizer is listed as **Eloquence C (experimental)**. It requires NVDA 2026.1 or later in a 64-bit process. The DLL is currently built for Windows x64.

Installable packages belong under [Releases and downloads](https://github.com/hozosch/Reloq-C/releases). If no `.nvda-addon` file is listed there, GitHub does not yet provide an installable package. The source download is intended for building the project yourself.

Once a package is available, install the `.nvda-addon` file with NVDA, restart NVDA and press **NVDA+Control+S** to select **Eloquence C (experimental)**. Choose your language and voice in NVDA's Speech settings.

The [user guide](docs/USER-GUIDE.en.md) covers languages, settings and dictionaries. ReLoq C is still under development; [changes and known limitations in 0.2.187](docs/changes/0.2.187.md) are documented separately.

## Feedback and contributions

Please report pronunciation errors, incomplete speech and everyday usability problems through [Issues](https://github.com/hozosch/Reloq-C/issues). Include a short example, language, voice preset, version, sample rate, and the expected and actual behaviour.

Developer information:

- [Build and packaging instructions, in German](docs/BUILD.md)
- [Technical work status and open faults, in German](docs/CURRENT-WORK.md)
- [Agreed pronunciation and sound choices, in German](GEWUENSCHTE-ABWEICHUNGEN-DE.md)
- [Earlier version notes](docs/history/README.md)

The synthesis engine is compiled as C, with a Python adapter for NVDA. Simplifying and optimizing the engine remains part of the work. Component and data notices are retained with their sources, including the [OpenEVV license](vendor/openevv61/LICENSE), [OpenEVV notices](vendor/openevv61/NOTICE) and [dictionary license](data/community/LICENSE.md).
