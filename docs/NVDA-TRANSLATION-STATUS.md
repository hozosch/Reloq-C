# NVDA add-on translations

The development package contains gettext catalogs for German, Aragonese,
Greek, Spanish, Persian, Finnish, French, Italian, Japanese, Korean, Brazilian
Portuguese, Simplified Chinese and Traditional Chinese. English is the
built-in fallback. Language names and the Persian test-package manifest are
also localized.

These catalogs are not yet ready to be described as complete: the current
template has 161 messages. Persian has 161 populated entries, the ten existing
SAPI/Android locales each have 97, and Aragonese has 52. These counts include
technical names and values that are unchanged across languages. Missing entries
fall back to English. Reused platform wording, locale names, and the newly
written Persian and Aragonese strings require review by native speakers before
the 2.0 release; several longer notices and help texts in other locales are
still untranslated.

`python addon/build_translations.py --update-template` refreshes the template
using GNU xgettext. `python addon/test_translation_catalogs.py` builds the
catalogs with GNU msgfmt and checks format arguments and test manifests. The
normal add-on still loads translations through NVDA's translation mechanism;
no translation package is required at runtime.
