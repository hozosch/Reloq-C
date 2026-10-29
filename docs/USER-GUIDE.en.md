# Eloquence C – user guide

Eloquence C is the NVDA add-on from [ReLoq C](https://github.com/hozosch/Reloq-C). It develops the familiar Eloquence sound with pronunciation corrections, Unicode support and additional voice controls.

## Install and select

You need NVDA 2026.1 or later in a 64-bit process. The package contains a Windows x64 DLL.

1. Download a `.nvda-addon` file from [Releases](https://github.com/hozosch/Reloq-C/releases) when a package is available there.
2. Open it with NVDA, confirm installation and restart NVDA.
3. Press **NVDA+Control+S**, choose **Eloquence C (experimental)** and confirm.
4. Choose a language and voice in NVDA's **Speech** settings.

The repository source ZIP is not an installable add-on. See the [NVDA user guide](https://download.nvaccess.org/documentation/en/userGuide.html) for general NVDA instructions.

## Languages and voices

There are 14 language choices. Both Chinese writing systems share one choice.

| Language | Dictionary prefix |
| --- | --- |
| Mandarin Chinese | CHS |
| German | DEU |
| British English | ENG |
| American English | ENU |
| Latin American Spanish | ESM |
| Castilian Spanish | ESP |
| Finnish | FIN |
| French | FRA |
| Canadian French | FRC |
| Italian | ITA |
| Japanese | JPN |
| Korean | KOR |
| Brazilian Portuguese | PTB |
| Greek | ELL |

The voice presets are **Reed, Shelly, Bobby, Rocko, Glen, Sandy, Grandma and Grandpa**. Language selects pronunciation; voice selects a sound profile. NVDA can automatically switch between supported languages in appropriately marked text when its automatic language switching option is enabled.

## Voice settings

These controls are in NVDA's Speech settings:

- **Rate and rate boost:** reading speed and its extended range.
- **Pitch, inflection and volume:** base pitch, sentence melody range and output level.
- **Head size:** resonance and voice character.
- **Roughness and breathiness:** additional voice-source characteristics.
- **Sample rate:** 8, 11.025, 16, 22.05, 32, 44.1 or 48 kHz; the default is 11.025 kHz. Higher rates extend the frequency range and use more CPU. If speech stutters, compare with 11.025 or 22.05 kHz to help identify the cause.

**Shorten pauses** offers **Never**, **Only at text end** and **Always**. It changes automatically generated pauses. Explicitly requested pauses retain their duration.

**Process ECI tags** is off by default. When enabled, supported backtick commands in text can control pronunciation, spelling, emphasis, voice parameters, voices and languages. This option is not needed for ordinary reading. Pronunciation dictionaries can contain pronunciation annotations independently of this setting.

## Pronunciation dictionaries

Select Eloquence C first, then open **NVDA Settings → Eloquence C**.

**Use community pronunciation dictionaries** adds German and American English entries from [eigencrow/IBMTTSDictionaries](https://github.com/eigencrow/IBMTTSDictionaries). It is off by default. **Update dictionaries now** checks the current `master` branch and applies a complete update in the background. Updating requires internet access.

Use **Add file** to select your own Dic files and assign a language and dictionary type. Filenames can be arbitrary; files stay at their original locations. Later files in the list take priority. Choose **OK** or **Apply** to save the selection.

**Open dictionary folder** opens an additional folder whose files are discovered automatically. Combine a prefix from the language table with one of these suffixes:

- `main.dic` for Main words, such as `DEUmain.dic` or `ENUmain.dic`.
- `Root.dic` for word roots, such as `DEURoot.dic`.
- `abbr.dic` for abbreviations, such as `DEUabbr.dic`.

Files use the native Dic format: Windows-1252 encoding, one entry per line and a tab between the lookup word and replacement. Main keys are case-sensitive, and ordinary text replacements subsequently undergo normal text analysis. Root and pronunciation entries are handled at their respective dictionary stages.

Edits to selected files and files in the dictionary folder are detected in the background, including newly created files and editor-style file replacements. Invalid data leaves the last valid set active. Community updates do not overwrite your own files. Local changes to bundled Dic files are also taken into account when community dictionaries are enabled.

The update button updates **dictionaries**. Automatic **add-on** updates are not yet implemented; install a new `.nvda-addon` package to update the add-on.

## Reporting problems

The project is still experimental. The reported American English crash with dictionaries, and Greek-letter differences in Finnish and Korean NVDA sentences, remain under investigation. See the [technical work status](https://github.com/hozosch/Reloq-C/blob/main/docs/CURRENT-WORK.md) for details.

When [reporting an issue](https://github.com/hozosch/Reloq-C/issues), include a short example, language, voice, version, sample rate and whether dictionaries are enabled. Explain whether speech stops, pronunciation is wrong or NVDA itself exits.
