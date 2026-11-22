"""Validate packaged NVDA translations and the Persian test manifests."""
from __future__ import annotations

import configparser
import gettext
import io
from pathlib import Path
import sys

from build_translations import translation_entries

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'experiments/persian'))
from build_experimental_nvda import PREFIX, add_persian


def main():
    entries = translation_entries()
    locales = ('an', 'de', 'el', 'es', 'fa', 'fi', 'fr', 'it', 'ja', 'ko',
               'pt_BR', 'zh_CN', 'zh_TW')
    base = {
        'manifest.ini': (ROOT / 'addon/synth/manifest.ini').read_bytes(),
        PREFIX + '__init__.py': (ROOT / 'addon/synth/synthDrivers/reloqC/__init__.py').read_bytes(),
        **entries,
    }
    persian = add_persian(base)
    for locale in locales:
        mo = entries[f'locale/{locale}/LC_MESSAGES/nvda.mo']
        trans = gettext.GNUTranslations(io.BytesIO(mo))
        # Missing entries deliberately retain English. Selected common and
        # test-build strings must nonetheless work in every new catalog.
        for message in ('Eloquence C: Persian and Pinglish test',
                        'Test build with Persian and Pinglish.'):
            assert trans.gettext(message) != message, (locale, message)
        assert trans.gettext('Version %s is available.') % '2.0'  # preserve %s
        parser = configparser.ConfigParser(interpolation=None)
        parser.read_string('[addon]\n' + persian[f'locale/{locale}/manifest.ini'].decode('utf8'))
        assert parser['addon']['summary'].strip('"') == trans.gettext('Eloquence C: Persian and Pinglish test')
        assert parser['addon']['description'].strip('"') == trans.gettext('Test build with Persian and Pinglish.')
    assert gettext.GNUTranslations(io.BytesIO(entries['locale/fa/LC_MESSAGES/nvda.mo'])).gettext('Persian') == 'فارسی'
    assert gettext.GNUTranslations(io.BytesIO(entries['locale/de/LC_MESSAGES/nvda.mo'])).gettext('Persian') == 'Persisch'
    print(f'Validated {len(locales)} NVDA catalogs, fallback and localized Persian test manifests.')


if __name__ == '__main__':
    main()
