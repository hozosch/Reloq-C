"""Build an explicitly labelled Persian/Pinglish NVDA test package.

Only use a Windows DLL and worker which have passed the focused tests. The
streaming cross-language contour and the complete native rules remain work in
progress. This script does not publish or replace a release.
"""
from __future__ import annotations

import argparse
from pathlib import Path
import sys
import zipfile

ROOT = Path(__file__).resolve().parents[2]
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT / 'addon'))
from build_synth import make_entries
from pe_format import assert_x64_pe

PREFIX = 'synthDrivers/reloqC/'
MODULES = ('espeak_ng_probe', 'espeak_worker', 'text_frontend', 'pinglish_frontend',
           'reference_alignment', 'hybrid_fa_frontend', 'direct_fa_sequence', 'nvda_driver_probe')


def add_persian(entries, worker: bytes):
    """Extend a baseline package without changing the normal release builder."""
    entries = dict(entries)
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
                            'from ._reloq_persian_bridge import add_profiles, make_engine\n' + old_import, 1)
    source = source.replace(old_languages, 'self._languages = add_profiles(dict(LANGUAGES), DLL)', 1)
    source = source.replace(old_factory,
                            'lambda: make_engine(DLL, languages=self._languages,\n'
                            '                language=initial_language, voice=initial_voice)', 1)
    entries[driver] = source.encode('utf8')
    entries[PREFIX + '_reloq_persian_projection.py'] = (ROOT / 'nvda/_reloq_persian_projection.py').read_bytes()
    entries[PREFIX + '_reloq_persian_bridge.py'] = (ROOT / 'nvda/_reloq_persian_bridge.py').read_bytes()
    entries[PREFIX + '_persian/__init__.py'] = b'"""Private Persian test frontend."""\n'
    for name in MODULES:
        entries[PREFIX + '_persian/' + name + '.py'] = (HERE / (name + '.py')).read_bytes()
    entries[PREFIX + 'fa-espeak-worker.exe'] = worker
    manifest = entries['manifest.ini'].decode('utf8')
    if manifest.count('version = "1.4.1"') != 1:
        raise ValueError('the normal version changed; choose an explicit test version')
    # Keep the existing updater's recognized dev-version grammar.
    manifest = manifest.replace('version = "1.4.1"', 'version = "1.5.0-dev91"', 1)
    manifest = manifest.replace('summary = "Eloquence C: fifteen language variants"',
                                'summary = "Eloquence C: Persian and Pinglish test"', 1)
    entries['manifest.ini'] = manifest.encode('utf8')
    return entries


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--dll', type=Path, required=True)
    parser.add_argument('--worker', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    assert_x64_pe(args.worker.read_bytes())
    entries = add_persian(make_entries(args.dll), args.worker.read_bytes())
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(args.output, 'w') as archive:
        for name, data in sorted(entries.items()):
            info = zipfile.ZipInfo(name, date_time=(1980, 1, 1, 0, 0, 0))
            info.compress_type = zipfile.ZIP_LZMA if name == PREFIX + 'eci.dll' else zipfile.ZIP_DEFLATED
            archive.writestr(info, data, compresslevel=None if info.compress_type == zipfile.ZIP_LZMA else 9)
    print('Created private NVDA test package:', args.output)


if __name__ == '__main__':
    main()
