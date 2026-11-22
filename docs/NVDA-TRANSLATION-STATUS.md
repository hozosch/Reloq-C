# NVDA add-on translations

The development package contains gettext catalogs for German, Aragonese,
Greek, Spanish, Persian, Finnish, French, Italian, Japanese, Korean, Brazilian
Portuguese, Simplified Chinese and Traditional Chinese. English is the
built-in fallback. Language names and the Persian test-package manifest are
also localized.

All 161 currently extracted messages have populated entries in every catalog,
including the Eloquence category and its dictionary, equalizer and update
dialogs. The count includes technical names and values that are unchanged
across languages. Reused platform wording and the newly written translations,
especially Persian and Aragonese, require review by native speakers before the
2.0 release. Newly added messages should be added to every catalog; absent
translations fall back to English.

`python addon/build_translations.py --update-template` refreshes the template
using GNU xgettext. `python addon/test_translation_catalogs.py` builds the
catalogs with GNU msgfmt and checks format arguments and test manifests. The
normal add-on still loads translations through NVDA's translation mechanism;
no translation package is required at runtime.
