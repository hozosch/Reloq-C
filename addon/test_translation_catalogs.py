"""Validate packaged NVDA translations and the Persian test manifests."""
from __future__ import annotations

import configparser
import ast
import gettext
import io
from pathlib import Path
import re
import sys

from build_translations import translation_entries

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'experiments/persian'))
from build_experimental_nvda import PREFIX, add_persian


def main():
    entries = translation_entries()
    sources = (
        ROOT / 'addon/synth/synthDrivers/reloqC/__init__.py',
        ROOT / 'addon/synth/globalPlugins/reloqCDictionaries/__init__.py',
        ROOT / 'nvda/_reloq_languages.py',
        ROOT / 'experiments/persian/nvda_driver_probe.py',
        ROOT / 'addon/build_translations.py',
    )
    messages = set()
    for source in sources:
        for node in ast.walk(ast.parse(source.read_text(encoding='utf8'))):
            if (isinstance(node, ast.Call) and isinstance(node.func, ast.Name) and
                    node.func.id in ('_', 'N_') and node.args and
                    isinstance(node.args[0], ast.Constant) and isinstance(node.args[0].value, str)):
                messages.add(node.args[0].value)
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
        missing = sorted(message for message in messages if message not in trans._catalog)
        assert not missing, (locale, missing)
        for message in messages:
            expected = re.findall(r'(?<!%)%s', message)
            actual = re.findall(r'(?<!%)%s', trans.gettext(message))
            assert actual == expected, (locale, message, trans.gettext(message))
        # The development manifest must not silently use its English text.
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
    print(f'Validated {len(locales)} complete NVDA catalogs, placeholders and localized Persian test manifests.')


if __name__ == '__main__':
    main()
