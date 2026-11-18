#!/usr/bin/env python3
"""Safely reuse a verified Windows provider build cache.

The selected Persian translation units must be rebuilt after a generator
change. Reuse unaffected objects only for a specifically named cache whose
source transition has been checked. Any other change falls back to a clean
provider build. No synthesized text, phones or audio are cached here.
"""
from __future__ import annotations

from pathlib import Path
import os
import re
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[1]
PROVIDER = ROOT / 'vendor/openevv61'
BUILD = PROVIDER / 'build/windows-release'
BASE = '0218903296e1828d468933fad04d7745779b5629'
PREVIOUS_TREE = 'caf3eaa2e08ac9fbd58dfcc9c532aab19405911e'
PREVIOUS_KEY = ('persian-windows-provider-gcc14-v2-'
                'f106c4723d7f6e32a226c721308d5e5b0f676a41f86a43f7a3c940b0ef20f128')
RECENT_BASE = '5675c59f29e690b2e15ba7ba9c209934a09632c9'
RECENT_KEY = ('persian-windows-provider-gcc14-v2-'
              '258cf46181dbfeb1b6d1f9c2e9146e82d09ebaa88f092c1c1ac5e757e1fe95fc')
RECENT_HEADERS = {
    'include/reloq_eci.h': 'f568cdd0b995f7ed7229cbe91f405cb0165b61f9',
    'include/reloq_persian_numbers.h': '5704f39488eec88d9095191df7dc64d151cfd6d8',
}
MIGRATION_KEY = ('persian-windows-provider-gcc14-v1-'
                 'eafb701ff0daa266b36f21741a91cf44935fe7d1d5fc200e54f509f1b878b075')
EXPECTED = {
    'include/reloq_eci.h',
    'vendor/openevv61/Makefile.native',
    'vendor/openevv61/src/delta/delta_low.c',
    'vendor/openevv61/src/eci/lang/eci_voicetable.c',
    'vendor/openevv61/src/eci/api/eci_dllobj.c',
    'vendor/openevv61/src/port/evv_arena.c',
    'vendor/openevv61/src/port/port_win32.c',
    'vendor/openevv61/src/reloq61_bridge.c',
    'experiments/aragonese/build_native_probe.py',
}
CHECKED = {
    'vendor/openevv61/Makefile.native': '5ac5f6d658e5c92b61c66d1a21fb591ceccdeab2',
    'experiments/aragonese/build_native_probe.py': 'deed0d350d8e913c7b678bbfd7c561f3b046ca19',
}
INPUTS = ('vendor/openevv61', 'include', 'src/reloq_voice_filter.c',
          'src/reloq_orthography.c', 'src/reloq_german_word_edges.c',
          'src/reloq_english_word_edges.c', 'src/reloq_persian_acoustics.c',
          'src/reloq_persian_acoustics.h', 'tools/prepare_persian_provider.py',
          'experiments/aragonese/build_native_probe.py',
          'experiments/persian/build_native_probe.py')


def git(*arguments):
    return subprocess.check_output(['git', *arguments], cwd=ROOT, text=True).strip()


def eligible_recent(key, changed, blobs):
    return (key == RECENT_KEY and set(changed) == set(RECENT_HEADERS) and
            blobs == RECENT_HEADERS)


def main():
    try:
        key = os.environ.get('PERSIAN_PROVIDER_CACHE_KEY', '')
        if key == RECENT_KEY:
            changed = set(git('diff', '--name-only', RECENT_BASE, 'HEAD', '--', *INPUTS).splitlines())
            blobs = {path: git('rev-parse', 'HEAD:' + path) for path in RECENT_HEADERS}
            if not eligible_recent(key, changed, blobs):
                raise ValueError('provider headers differ from the verified additive API change')
            if not BUILD.is_dir() or not (BUILD / 'libreloq61.a').is_file():
                raise ValueError('cached provider archive is missing')
            # The provider source does not include these headers. Reject the
            # cache if dependency files contradict that checked relationship.
            for dep in BUILD.rglob('*.d'):
                value = dep.read_text(encoding='utf8', errors='replace')
                if any(Path(path).name in value for path in RECENT_HEADERS):
                    raise ValueError('a provider object depends on a changed public header')
            for path in BUILD.rglob('*'):
                if path.is_file():
                    os.utime(path, None)
            print('Reused verified provider cache for the additive public API change.')
            return
        if key == PREVIOUS_KEY:
            changed = set(git('diff', '--name-only', PREVIOUS_TREE, 'HEAD', '--', *INPUTS).splitlines())
            # Both of these inputs are rebuilt below. The scope-selection C
            # file is its own object; it is not part of the other language
            # objects in this verified provider cache.
            allowed = {'experiments/persian/build_native_probe.py',
                       'src/reloq_persian_acoustics.c'}
            if not changed or not changed <= allowed:
                raise ValueError('provider sources differ from the verified previous build')
        elif key == MIGRATION_KEY:
            # This older cache predates the Persian provider and requires the
            # originally verified, more substantial migration.
            migration_inputs = tuple(path for path in INPUTS if path not in (
                'src/reloq_persian_acoustics.c', 'src/reloq_persian_acoustics.h',
                'tools/prepare_persian_provider.py'))
            changed = set(git('diff', '--name-only', BASE, 'HEAD', '--', *migration_inputs).splitlines())
            if changed != EXPECTED:
                raise ValueError('provider sources differ from the verified migration')
            for path, expected in CHECKED.items():
                if git('rev-parse', 'HEAD:' + path) != expected:
                    raise ValueError('provider build instructions changed')
        else:
            raise ValueError('the restored cache is not one of the verified keys')
        makefile = (PROVIDER / 'Makefile.native').read_text(encoding='utf8')
        names = re.search(r'^PERSIAN_NAMES := (.+)$', makefile, re.MULTILINE)
        if names is None or len(names.group(1).split()) != 18 or not BUILD.is_dir():
            raise ValueError('cached provider has an unexpected layout')
        objects = []
        for name in names.group(1).split():
            sources = list((PROVIDER / 'lang').rglob(name))
            if len(sources) != 1:
                raise ValueError('cannot identify a Persian provider object')
            objects.append(BUILD / sources[0].relative_to(PROVIDER).with_suffix('.o'))
        objects.extend((BUILD / 'src/delta/delta_low.o',
                        BUILD / 'src/eci/api/eci_dllobj.o',
                        BUILD / 'src/eci/lang/eci_voicetable.o',
                        BUILD / 'src/port/evv_arena.o',
                        BUILD / 'src/port/port_win32.o',
                        BUILD / 'src/reloq61_bridge.o',
                        BUILD / 'reloq_persian_acoustics.o', BUILD / 'libreloq61.a'))
        # No provider object should depend on the changed public ECI header.
        # If one does, rebuilding that object is safer than assuming this.
        for dep in BUILD.rglob('*.d'):
            if 'reloq_eci.h' in dep.read_text(encoding='utf8', errors='replace'):
                objects.append(dep.with_suffix('.o'))
        for path in BUILD.rglob('*'):
            if path.is_file():
                os.utime(path, None)
        for path in objects:
            path.unlink(missing_ok=True)
        shutil.rmtree(BUILD / 'persian-provider', ignore_errors=True)
        print(f'Reused verified provider cache; rebuilding {len(objects) - 1} changed objects.')
    except (OSError, subprocess.CalledProcessError, ValueError) as error:
        print(f'Cannot safely reuse the provider cache ({error}); rebuilding it cleanly.')
        shutil.rmtree(BUILD, ignore_errors=True)


if __name__ == '__main__':
    main()
