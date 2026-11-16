#!/usr/bin/env python3
"""Safely migrate the verified pre-Persian Windows provider build cache.

The old archive cannot be linked unchanged: the selected Persian translation
units and the two initialization fixes need new objects. Reuse the unaffected
language objects only when the exact historical cache and its known source
transition have been verified. Any unexpected change falls back to a clean
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
EXPECTED = {
    'include/reloq_eci.h',
    'vendor/openevv61/Makefile.native',
    'vendor/openevv61/src/delta/delta_low.c',
    'vendor/openevv61/src/eci/lang/eci_voicetable.c',
    'vendor/openevv61/src/port/evv_arena.c',
    'vendor/openevv61/src/reloq61_bridge.c',
    'experiments/aragonese/build_native_probe.py',
}
CHECKED = {
    'vendor/openevv61/Makefile.native': '5ac5f6d658e5c92b61c66d1a21fb591ceccdeab2',
    'experiments/aragonese/build_native_probe.py': 'deed0d350d8e913c7b678bbfd7c561f3b046ca19',
}
INPUTS = ('vendor/openevv61', 'include', 'src/reloq_voice_filter.c',
          'src/reloq_orthography.c', 'src/reloq_german_word_edges.c',
          'src/reloq_english_word_edges.c', 'experiments/aragonese/build_native_probe.py',
          'experiments/persian/build_native_probe.py')


def git(*arguments):
    return subprocess.check_output(['git', *arguments], cwd=ROOT, text=True).strip()


def main():
    try:
        changed = set(git('diff', '--name-only', BASE, 'HEAD', '--', *INPUTS).splitlines())
        if changed != EXPECTED:
            raise ValueError('provider sources differ from the verified migration')
        for path, expected in CHECKED.items():
            if git('rev-parse', 'HEAD:' + path) != expected:
                raise ValueError('provider build instructions changed')
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
                        BUILD / 'src/eci/lang/eci_voicetable.o',
                        BUILD / 'src/port/evv_arena.o',
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
