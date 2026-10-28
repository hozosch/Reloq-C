# Translating the NVDA interface

The UI source language is English. Runtime messages use NVDA's standard
`addonHandler.initTranslation()` and `_()`. Translations are selected by NVDA's
UI language, independently of the language spoken by the synthesizer.

GNU gettext (`msgfmt`, `xgettext`, `msgmerge`, and optionally `msginit`) is needed
to build or update translations. It is not needed on the user's NVDA system.

The English template is `synth/locale/reloqC.pot`. Each translation belongs in
`synth/locale/<language>/LC_MESSAGES/nvda.po`, using NVDA locale names such as
`de`, `fr` or `pt_BR`. German is currently included. An absent catalog or missing
message falls back to the English source text.

From the project root, update the template after changing UI strings:

```sh
python3 addon/build_translations.py --update-template
msgmerge --update --backup=none addon/synth/locale/de/LC_MESSAGES/nvda.po addon/synth/locale/reloqC.pot
```

To start another language, create its `LC_MESSAGES` directory and initialize its
PO file with `msginit`, for example:

```sh
mkdir -p addon/synth/locale/fr/LC_MESSAGES
msginit --no-translator --locale=fr --input=addon/synth/locale/reloqC.pot --output-file=addon/synth/locale/fr/LC_MESSAGES/nvda.po
```

Edit the `msgstr` translations and metadata, preserving parameters and appropriate
`&` accelerator marks. Do not translate setting IDs, language codes or saved
voice IDs. Sample-rate labels may use the locale's decimal separator.

`python3 addon/build_synth.py` validates and compiles every PO using GNU msgfmt.
It packages `locale/<language>/LC_MESSAGES/nvda.mo` and generates the localized
`locale/<language>/manifest.ini` from the same PO. Add-on summary and description
therefore have one translation source. The source ZIP retains the PO and POT
files; the installed add-on receives the compiled catalogs.

The shared `_reloq_languages.py` module marks English display names with `N_()`.
The driver translates them when constructing UI choices; the worker has no NVDA
or gettext runtime dependency. Text normalization and pronunciation remain in
the selected speech language and do not change when the UI language changes.

Checks use the actual packaged MO through Python's gettext reader and the
supplied NVDA property/settings contract:

```sh
python3 -B addon/test_synth.py en
python3 -B addon/test_synth.py de_DE
python3 -B addon/test_synth.py fr
```

The French case currently checks English fallback, not an included French
translation. The German regional locale checks `de_DE` to `de` catalog lookup.

See the official [NVDA add-on localization documentation](https://download.nvaccess.org/releases/2026.2/documentation/developerGuide.html#localizingAddons).
