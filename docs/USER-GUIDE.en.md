# Eloquence C – user guide

## Version 1.3.0 – test version

The Eloquence C NVDA settings category can optionally split unfamiliar English words. It is off by default and applies to American and British English. Known words and dictionary entries are preserved where possible; unfamiliar names and foreign words can still be misidentified.

Choose “Import NVDA dictionary” to select an existing NVDA speech dictionary from another driver, then choose its language and either one voice or all voices. Regular expressions, case sensitivity and comments are retained. The imported copy can be edited or removed in the same category; the original file is not changed. Imported rules are applied before NVDA's other speech dictionaries. Native ECI dictionaries use a different format and remain under “Own dictionary files”.

## Version 1.2.2

The base pitch can optionally be set exactly in hertz. German final A is now also used before commas, colons and other pauses. For new NVDA installations the default pitch of 65 corresponds to the default Reed voice.

## Version 1.2.1

Mellow mode has been refined with a broader reduction in voiced presence, no added richness and less breathiness during voicing.

## Version 1.2.0

Greek pronunciation and sentence melody have been refined. The first utterance waits for the selected dictionaries, speech volume and SAPI responsiveness have been improved, and a full stop after a single letter can end an English sentence. Mellow mode and original question intonation are available as optional settings.

## Version 1.1.3

A crash on certain long texts has been fixed. HTTPS addresses use the HTTP reader. English pronunciation in the Greek voice has been refined.

## Version 1.1.2

A separated dash before a number becomes a pause. Chinese and English switch more smoothly while Chinese inflection remains available, and German number-word corrections also apply in thousands.

## Version 1.1.1

The speech engine handles dashes in running text in tested European languages even without surrounding spaces. In German character reading, underscore uses noun stress.

## Version 1.1.0

The German E-I glide defaults to opening 85, backness 45 and softness 45. German sentence and sound planning improve hyphenated words, number words and question contours. English analysis retains the whole stem in “recosp” and the final vowel in “tzsche”.

## English clock readings with seconds

English clock times such as `13:28:25` keep the ordinary hours/minutes reading and append “and twenty five seconds”. One second uses the singular; fractional seconds remain in the normal number reader. This applies to US and British English in the native core, including NVDA and other ECI clients. Source indices remain attached to their original text positions.

Eloquence C is the NVDA add-on from [ReLoq C](https://github.com/hozosch/Reloq-C). It develops the familiar Eloquence sound with pronunciation corrections, Unicode support and additional voice controls. NVDA is the first integration and current speech testing platform. The same engine is also available for Android ARM64 and Windows SAPI5.

## Install and select

You need NVDA 2026.1 or later in a 64-bit process. The package contains a Windows x64 DLL.

1. Download a `.nvda-addon` file from [Releases](https://github.com/hozosch/Reloq-C/releases) when a package is available there.
2. Open it with NVDA, confirm installation and restart NVDA.
3. Press **NVDA+Control+S**, choose **Eloquence C** and confirm.
4. Choose a language and voice in NVDA's **Speech** settings.

On first use, Eloquence C selects NVDA's language when supported. A saved
language selection takes precedence.

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

**Use original question intonation** uses the language core's question contour for German and American English. It is off by default, preserving the rising question contour.

**Mellow mode** softens voiced presence around 3–4 kHz at sample rates of 16 kHz and above and omits the added richness from 22 kHz. It also reduces breathiness during voicing and is off by default.

Under **NVDA settings → Eloquence C → Exact base pitch**, enable **Use base pitch in hertz**, enter a whole number, and choose **OK** or **Apply**. NVDA's regular pitch slider retains the 0–100 ECI scale. Changing that slider switches back to the ECI scale with an announcement; the Hertz value is retained for later. Voice and language changes preserve an active Hertz baseline. Inflection 0 is useful for checking the pitch; tonal languages can retain their linguistic contours.

**Process ECI tags** is off by default. When enabled, supported backtick commands in text can control pronunciation, spelling, emphasis, voice parameters, voices and languages. This option is not needed for ordinary reading. Pronunciation dictionaries can contain pronunciation annotations independently of this setting.

`` `vwpm150 `` sets the following speech to 150 using the native WPM timing rule. Whole numbers from 1 to 6000 are accepted; `` `vs50 `` returns to the usual ECI speed scale. At 1 to 4 WPM the control frames are slowed during synthesis to avoid overflow in the language rules; pitch is retained. Audible words per minute depend on the language, text and pauses; very high values may hit timing limits. These commands do not change saved sliders.

`` `vbhz150 `` sets the base pitch of the following text to 150 hertz; integer values from 1 to 24,000 hertz are supported. `` `vb65 `` returns to the usual ECI pitch scale. For measurements, `` `vf0 `` can turn off inflection; linguistic tones in tonal languages may remain. The sample rate must exceed twice the desired frequency, and the voice filters can attenuate very high values. An insufficient sample rate can distort the result or cause synthesis to fail. These tags do not change the saved settings.

**Use abbreviation dictionary** controls native abbreviation expansion and starts enabled. Main and Root pronunciation dictionaries remain active independently. **Phrase prediction** starts disabled; enabled, it uses the language core to infer phrase boundaries in text without punctuation. Both settings also appear on Android and apply to the next synthesis request. Equal speed values retain each language’s own duration model; they do not promise equal words or syllables per minute.

German uses the built-in C dictionary, as do the other languages. The supplemental German lexicon from earlier versions is no longer required. Optional pronunciation dictionaries remain available.

Emoji names are read in the selected language during ordinary reading and character navigation, including skin tones, flags, keycaps and joined emoji. This works in the shared engine, including Android. Ordinary digits and punctuation retain their existing treatment; text presentation selectors are respected. NVDA may also apply its own symbol pronunciation settings before sending text to the engine.

## Pronunciation dictionaries

Select Eloquence C first, then open **NVDA Settings → Eloquence C**.

**Use community pronunciation dictionaries** adds German and American English entries from [eigencrow/IBMTTSDictionaries](https://github.com/eigencrow/IBMTTSDictionaries). It is off by default. **Update dictionaries now** checks the current `master` branch and applies a complete update in the background. Updating requires internet access.

Under **Dictionary source**, you can also select **Alternative IBM TTS Dictionaries**. This source currently provides US English and Castilian Spanish, but no German dictionary. It is downloaded in the background the first time it is needed; own files take priority.

After an update, a dialog reports the number of new distinct entries and new dictionary files. Changes and removals do not count as additions. The first download reports both totals; if the previous version cannot be compared, the dialog says so. Online updates currently retrieve a fixed set of supported filenames; other new files are not downloaded automatically.

Use **Add file** to select your own Dic files and assign a language and dictionary type. Filenames can be arbitrary; files stay at their original locations. Later files in the list take priority. Choose **OK** or **Apply** to save the selection.

**Open dictionary folder** opens the existing `synthDrivers/reloqC/communityDictionaries` folder inside the add-on. This is where the bundled files live and editor changes are detected. With community dictionaries enabled, you can add files using a prefix from the language table and one of these suffixes:

- `main.dic` for Main words, such as `DEUmain.dic` or `ENUmain.dic`.
- `Root.dic` for word roots, such as `DEURoot.dic`.
- `abbr.dic` for abbreviations, such as `DEUabbr.dic`.

Files use the native Dic format: Windows-1252 encoding, one entry per line and a tab between the lookup word and replacement. Main keys are case-sensitive, and ordinary text replacements subsequently undergo normal text analysis. Root and pronunciation entries are handled at their respective dictionary stages.

Edits to selected files and files in the dictionary folder are detected in the background, including newly created files and editor-style file replacements. Invalid data leaves the last valid set active. Community updates refresh the files in this add-on folder. Selected own files and locally edited or removed dictionary files are preserved. A locally edited volume takes priority over the downloaded volume. No additional editing folder is created in NVDA’s main directory.

The first utterance waits until the selected dictionaries have loaded completely. If no valid set is available at startup, an error dialog appears. Check the dictionary settings or temporarily choose another synthesizer.

Under **Add-on updates**, choose **Check for add-on updates** to check published GitHub Releases. If a newer package is available, **Download and install update** checks the download and opens NVDA’s normal installation and confirmation flow. Checks run only when requested. If no package has been published, the panel says so. **Update dictionaries now** remains independent of package updates.

## Reporting problems

NVDA can process symbols before sending text to the synthesizer; its symbol settings therefore also affect which characters reach ReLoq. See the [technical work status](https://github.com/hozosch/Reloq-C/blob/main/docs/CURRENT-WORK.md) for details.

When [reporting an issue](https://github.com/hozosch/Reloq-C/issues), include a short example, language, voice, version, sample rate and whether dictionaries are enabled. Explain whether speech stops, pronunciation is wrong or NVDA itself exits.

## SAPI5 x64 and x86

Run the single SAPI5 `setup.exe`. Its Inno Setup language chooser offers the supported interface languages. It installs x64 and x86 voices together on compatible Windows, and x86 on 32-bit Windows.
After restarting existing SAPI applications, choose a **ReLoq C** language voice.
There is one registered voice per language. Open `ReLoqSettings.exe` or the
voice's SAPI properties to select a profile and additional language parameters.
Changes are saved immediately for the next utterance. The dialog follows the
supported Windows display language; the installer also offers an explicit language chooser. Its checkbox shares
all parameters across languages; turning it off restores separate language
settings. The synthesis sample rate is common to every language. SAPI or the
application converts this signal to its own output format; requesting a higher
output rate does not change Eloquence's synthesis rate or bandwidth. Speed
and volume belong to SAPI. Without speed boost, SAPI 0 maps to native ECI 60,
with ten native units per SAPI step. All profile volumes default to 100. Updates use the same installer. Windows Installed apps or the Start menu uninstall shortcut removes both architectures, retaining user dictionary files and settings. Basic x64 use is confirmed; further device and screen-reader UI testing remains necessary.
See [the SAPI guide](../sapi/README.txt) for controls, installation and limitations.
