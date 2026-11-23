"""Verify all active NVDA interface messages and package translations."""
import ast
import gettext
import io
from pathlib import Path
import re
from build_translations import translation_entries

ROOT=Path(__file__).resolve().parents[1]
SOURCES=('addon/synth/synthDrivers/reloqC/__init__.py',
         'addon/synth/globalPlugins/reloqCDictionaries/__init__.py',
         'nvda/_reloq_languages.py','addon/build_translations.py')
LOCALES=('an','de','el','es','fa','fi','fr','it','ja','ko','pt_BR','zh_CN','zh_TW')


def main():
    messages=set()
    for path in SOURCES:
        for node in ast.walk(ast.parse((ROOT/path).read_text(encoding='utf8'))):
            if (isinstance(node,ast.Call) and isinstance(node.func,ast.Name) and
                node.func.id in ('_', 'N_') and node.args and
                isinstance(node.args[0],ast.Constant) and isinstance(node.args[0].value,str)):
                messages.add(node.args[0].value)
    entries=translation_entries()
    for locale in LOCALES:
        catalog=gettext.GNUTranslations(io.BytesIO(entries[f'locale/{locale}/LC_MESSAGES/nvda.mo']))
        missing=sorted(m for m in messages if m not in catalog._catalog)
        assert not missing,(locale,missing)
        for msg in messages:
            assert re.findall(r'(?<!%)%s',msg)==re.findall(r'(?<!%)%s',catalog.gettext(msg)),(locale,msg)
        assert catalog.gettext('Pronunciation dictionaries') != 'Pronunciation dictionaries',locale
        assert f'locale/{locale}/manifest.ini' in entries
    # Language names stay the catalog's original stable names in the selector;
    # adding UI translations must not force them through gettext.
    source=(ROOT/'addon/synth/synthDrivers/reloqC/__init__.py').read_text(encoding='utf8')
    assert 'info.displayName = language.name' in source
    print(f'Passed {len(messages)} interface strings in {len(LOCALES)} locales and preserved language names.')

if __name__=='__main__':main()
