"""Build an explicitly labelled Persian/Pinglish NVDA test package.

Only use Windows DLLs which have passed the focused tests. The bounded
sentence planner uses the selected cross-language transition. Uncovered words
use the core's orthographic inference without eSpeak and still need listening
checks. This script does not publish or replace a release.
"""
from __future__ import annotations

import argparse
import gettext
import io
import json
from pathlib import Path
import re
import sys
import zipfile

ROOT = Path(__file__).resolve().parents[2]
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT / 'addon'))
from build_synth import make_entries

PREFIX = 'synthDrivers/reloqC/'
MODULES = ('text_frontend', 'pinglish_frontend', 'persian_symbols',
           'persian_unicode_names',
           'reference_alignment', 'hybrid_fa_frontend', 'direct_fa_sequence', 'continuous_plan',
           'continuous_driver', 'parallel_pilot', 'korean_contour', 'nvda_driver_probe')


def add_persian(entries, *, version='1.5.0-dev142', sound_variant=None, core_melody=False):
    """Extend a baseline package without changing the normal release builder."""
    entries = dict(entries)
    if sound_variant is not None:
        # Read by nvda/_reloq_persian_bridge.py when the driver starts.
        if sound_variant not in (0, 1, 2, 3, 4, 5, 6, 7):
            raise ValueError('unknown Persian sound variant')
        entries[PREFIX + 'persian_sound_variant.txt'] = f'{sound_variant}\n'.encode('ascii')
    if core_melody:
        # Read by nvda/_reloq_persian_bridge.py when the driver starts.
        entries[PREFIX + 'persian_native_melody.txt'] = b'1\n'
    driver = PREFIX + '__init__.py'
    source = entries[driver].decode('utf8')
    old_import = 'from ._speech_tags import EciTagCommand'
    old_languages = 'self._languages = dict(LANGUAGES)'
    old_factory = ('lambda: LanguageEngine(DLL, languages=self._languages,\n'
                   '                language=initial_language, voice=initial_voice,\n'
                   '                engine_factory=DirectEciEngine)')
    if any(source.count(value) != 1 for value in (old_import, old_languages, old_factory)):
        raise ValueError('the NVDA driver changed; its Persian adapter must be reviewed')
    source = source.replace(old_import,
                            'from ._reloq_persian_bridge import add_profiles, make_engine\n'
                            'from ._reloq_persian_decimals import decimal_pending, dotted_version_word, protect_decimals\n'
                            'from ._reloq_persian_joiners import (ReloqPersianJoinerGuard, joiner_pending,\n'
                            '    emoji_selector_pending, prepare_persian_symbols, protect_joiners,\n'
                            '    protect_emoji_selectors, restore_joiners)\n' + old_import, 1)
    source = source.replace(old_languages, 'self._languages = add_profiles(dict(LANGUAGES), DLL)', 1)
    source = source.replace(old_factory,
                            'lambda: make_engine(DLL, languages=self._languages,\n'
                            '                language=initial_language, voice=initial_voice)', 1)
    substitutions = (
        ('        initial_language, initial_voice = self._selectedLanguage, self._voice\n',
         '        initial_language, initial_voice = self._selectedLanguage, self._voice\n'
         '        prepare_persian_symbols(self._languages[initial_language].locale)\n'),
        ('    def _select_language(self, selected):\n        with self._submit_lock:\n',
         '    def _select_language(self, selected):\n'
         '        if selected in self._languages:\n'
         '            prepare_persian_symbols(self._languages[selected].locale)\n'
         '        with self._submit_lock:\n'),
        ('    segmentation_guarded = False\n    last_text_alnum = False',
         '    segmentation_guarded = False\n    persian_joiners = False\n    last_text_alnum = False'),
        ('            said = item\n', '            said = restore_joiners(item) if persian_joiners else item\n'),
        ('        elif isinstance(item, SegmentationGuard):\n',
         '        elif isinstance(item, ReloqPersianJoinerGuard):\n'
         '            persian_joiners = item.enabled\n'
         '        elif isinstance(item, SegmentationGuard):\n'),
        ('NativeDictionaryWord, CapitalWordCommand}',
         'NativeDictionaryWord, CapitalWordCommand, ReloqPersianJoinerGuard}'),
        ('any(isinstance(item, str) and _BUILTIN_BOUNDARY.search(item) for item in sequence))',
         'any(isinstance(item, str) and _BUILTIN_BOUNDARY.search(item) for item in sequence) or\n'
         '                            joiner_pending(sequence) or decimal_pending(sequence) or\n'
         '                            emoji_selector_pending(sequence))'),
        ('        for item in sequence:\n            if isinstance(item, LangChangeCommand):',
         '        for _sequence_index, item in enumerate(sequence):\n            if isinstance(item, LangChangeCommand):'),
        ('                        if protect and not any(left < end and right > start for left, right in personal) and not any(\n',
         '                        if dotted_version_word(selected, item, start, end, sequence, _sequence_index):\n'
         '                            protect = False\n'
         '                        if protect and not any(left < end and right > start for left, right in personal) and not any(\n'),
        ('        speechSequence[:] = self._protect_tags(prepared, symbolLevel=symbolLevel)',
         '        prepared = self._protect_tags(prepared, symbolLevel=symbolLevel)\n'
         '        prepared = protect_decimals(self, prepared, symbol_level=symbolLevel)\n'
         '        prepared = protect_emoji_selectors(self, prepared, symbol_level=symbolLevel)\n'
         '        speechSequence[:] = protect_joiners(self, prepared, symbol_level=symbolLevel)'),
    )
    for old, new in substitutions:
        if source.count(old) != 1:
            raise ValueError('the NVDA Persian joiner adapter must be reviewed')
        source = source.replace(old, new, 1)
    entries[driver] = source.encode('utf8')
    entries[PREFIX + '_reloq_persian_projection.py'] = (ROOT / 'nvda/_reloq_persian_projection.py').read_bytes()
    entries[PREFIX + '_reloq_persian_frames.py'] = (ROOT / 'nvda/_reloq_persian_frames.py').read_bytes()
    entries[PREFIX + '_reloq_persian_glide.py'] = (ROOT / 'nvda/_reloq_persian_glide.py').read_bytes()
    entries[PREFIX + '_reloq_persian_bridge.py'] = (ROOT / 'nvda/_reloq_persian_bridge.py').read_bytes()
    entries[PREFIX + '_reloq_persian_joiners.py'] = (ROOT / 'nvda/_reloq_persian_joiners.py').read_bytes()
    entries[PREFIX + '_reloq_persian_decimals.py'] = (ROOT / 'nvda/_reloq_persian_decimals.py').read_bytes()
    entries[PREFIX + '_persian/__init__.py'] = b'"""Private Persian test frontend."""\n'
    for name in MODULES:
        entries[PREFIX + '_persian/' + name + '.py'] = (HERE / (name + '.py')).read_bytes()
    entries['doc/licenses/Persian-city-names-NOTICE.txt'] = (
        ROOT / 'reference-persian-cities/README.md').read_bytes()
    if 'doc/source.json' in entries:
        metadata = json.loads(entries['doc/source.json'])
        metadata['build'] = ('shared Windows Persian acoustic core followed by '
                             'experiments/persian/build_experimental_nvda.py')
        entries['doc/source.json'] = (json.dumps(metadata, indent=2) + '\n').encode('utf8')
    manifest = entries['manifest.ini'].decode('utf8')
    if manifest.count('version = "1.4.1"') != 1:
        raise ValueError('the normal version changed; choose an explicit test version')
    # Keep the existing updater's recognized dev-version grammar. The updater
    # offers a published release with the same base version to a development
    # build, so a test build made after release 2.0.0 must be 2.0.1-devN: a
    # 2.0.0-devN build is asked to "update" to the release without Persian.
    if not re.fullmatch(r'(?:1\.5\.0|2\.0\.[01])-dev[0-9]+', version):
        raise ValueError('the Persian test version must use the 1.5.0-devN, 2.0.0-devN or 2.0.1-devN form')
    manifest = manifest.replace('version = "1.4.1"', f'version = "{version}"', 1)
    manifest = manifest.replace('summary = "Eloquence C: fifteen language variants"',
                                'summary = "Eloquence C: Persian and Pinglish test"', 1)
    manifest = re.sub(r'(?m)^description = .*$',
                      'description = "Test build with Persian and Pinglish."', manifest)
    entries['manifest.ini'] = manifest.encode('utf8')
    for name in tuple(entries):
        if not name.startswith('locale/') or not name.endswith('/manifest.ini'):
            continue
        german = name.startswith('locale/de/')
        catalog = entries.get(name.replace('/manifest.ini', '/LC_MESSAGES/nvda.mo'))
        if catalog:
            translate = gettext.GNUTranslations(io.BytesIO(catalog)).gettext
            summary = translate('Eloquence C: Persian and Pinglish test')
            description = translate('Test build with Persian and Pinglish.')
        else:
            # Tests may construct only the manifest without a compiled catalog.
            summary = 'Eloquence C: Persisch- und Pinglish-Test' if german else 'Eloquence C: Persian and Pinglish test'
            description = ('Testversion mit Persisch und Pinglish.' if german
                           else 'Test build with Persian and Pinglish.')
        localized = entries[name].decode('utf8')
        localized = re.sub(r'(?m)^summary = .*$', 'summary = ' + json.dumps(summary, ensure_ascii=False), localized)
        localized = re.sub(r'(?m)^description = .*$', 'description = ' + json.dumps(description, ensure_ascii=False), localized)
        entries[name] = localized.encode('utf8')
    return entries


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--dll', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--version', default='1.5.0-dev142')
    parser.add_argument('--sound-variant', type=int, choices=(0, 1, 2, 3, 4, 5, 6, 7),
                        help='bundle a Persian listening variant: 0 previous sound, 1 to 7 candidates')
    parser.add_argument('--core-melody', action='store_true',
                        help='use the US sentence melody that is fixed in the core')
    args = parser.parse_args()
    named_version = re.search(r'((?:1\.5\.0|2\.0\.[01])-dev[0-9]+)\.nvda-addon$', args.output.name)
    if named_version and named_version.group(1) != args.version:
        parser.error('the output filename and add-on version must agree')
    entries = add_persian(make_entries(args.dll), version=args.version,
                          sound_variant=args.sound_variant, core_melody=args.core_melody)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(args.output, 'w') as archive:
        for name, data in sorted(entries.items()):
            info = zipfile.ZipInfo(name, date_time=(1980, 1, 1, 0, 0, 0))
            info.compress_type = zipfile.ZIP_LZMA if name == PREFIX + 'eci.dll' else zipfile.ZIP_DEFLATED
            archive.writestr(info, data, compresslevel=None if info.compress_type == zipfile.ZIP_LZMA else 9)
    print('Created private NVDA test package:', args.output)


if __name__ == '__main__':
    main()
