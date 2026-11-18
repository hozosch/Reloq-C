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
TRACE_BASE = '14bfa3132c9936d2798d3281ac5693b550f002cb'
TRACE_KEY = ('persian-windows-provider-gcc14-v2-'
             'eb978682e83612fa0fef1940e0c321d3072f9cf8db4b6d11c243532189d602cc')
DEV149_BASE = 'ccdc6680e8d698c6cf4834556706021aa0027f25'
DEV149_KEY = ('persian-windows-provider-gcc14-v2-'
              'cab206af509e0977cc552bd0723a1a6d2f8c9b886d82357a931432e54d7a0047')
DEV149_ALLOWED = {
    'include/reloq_syllable_trace.h',
    'vendor/openevv61/src/reloq61_bridge.c',
    'vendor/openevv61/lang/fifi/delta_rules_c30_fifi.c',
    'vendor/openevv61/lang/enus61/delta_rules_c29_enus61.c',
}
DEV150_BASE = '0c2120cc341b6f9034564df99539f3c16e94e51c'
DEV150_KEY = ('persian-windows-provider-gcc14-v2-'
              '0b8cb1dd6f2d585d653cae4a821bb432b804501524673c7444edc72155df053e')
DEV150_ALLOWED = {'experiments/persian/build_native_probe.py',
                  'src/reloq_persian_acoustics.c'}
DEV151_BASE = '64cecf6a0541bcc74a6f39677b726b576b9f1b25'
DEV151_KEY = ('persian-windows-provider-gcc14-v2-'
              '0b0acae8031297dc4ce2baa29429934bf1d6f52250760b7380ab0759babb3356')
DEV151_ALLOWED = {'experiments/persian/build_native_probe.py',
                  'src/reloq_persian_acoustics.c'}
DEV152_BASE = '812e03b4fe7f0a8331b7a433c352a86a34710498'
DEV152_KEY = ('persian-windows-provider-gcc14-v2-'
              '910f15fd293dbd45854c54fe6b3907d1b381bf561b04cd64dfd5690d86db9ea1')
DEV152_ALLOWED = {'experiments/persian/build_native_probe.py',
                  'src/reloq_persian_acoustics.c',
                  'vendor/openevv61/lang/jajp61/delta_rules_c17_jajp61.c',
                  'vendor/openevv61/lang/jajp61/delta_rules_c30_jajp61.c'}
DEV153_BASE = '0d8ccd6e773bcd9e1af0a3ecbd4b3ddc956fbd82'
DEV153_KEY = ('persian-windows-provider-gcc14-v2-'
              'c50e521eab044c336168b3ce125379a2f21d61f9d8cc49cee89e3447c0632f28')
DEV153_ALLOWED = {'experiments/persian/build_native_probe.py',
                  'src/reloq_persian_acoustics.c',
                  'vendor/openevv61/Makefile.native', 'tools/prepare_persian_provider.py'}
DEV153_CHECKED = {'vendor/openevv61/Makefile.native': '0a483b13d681dfcb0e61f886cc677259d40b0ba9',
                  'tools/prepare_persian_provider.py': 'b384362ec6ba15c0fd412b6f4f94201d738c2470'}
DEV154_BASE = 'bd28d74c148d9b88b2995d9267293c40a0f78be5'
DEV154_KEY = ('persian-windows-provider-gcc14-v2-'
              'c76d65b3e35168941848c062148a2ad922b4949b8ce8c9293883666c1055f88f')
DEV154_ALLOWED = {'experiments/persian/build_native_probe.py',
                  'src/reloq_persian_acoustics.c',
                  'vendor/openevv61/src/eci/bridge/eci_stmarray.c'}
DEV155_BASE = 'b434e5cb96f9f824cc4d0287d1e1427a96853f41'
DEV155_KEY = ('persian-windows-provider-gcc14-v2-'
              'c83463169692296a905dc6098785f24b7577e72820b955b60437ebfc5e5c420b')
DEV155_ALLOWED = {'src/reloq_persian_acoustics.c'}
TRACE_ALLOWED = {
    'include/reloq_eci.h', 'include/reloq_persian_frames.h', 'include/reloq_syllable_trace.h',
    'vendor/openevv61/src/reloq61_bridge.c',
    'vendor/openevv61/lang/fifi/delta_rules_c30_fifi.c',
    'vendor/openevv61/lang/enus61/delta_rules_c29_enus61.c',
}
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
        if key == DEV155_KEY:
            changed = set(git('diff', '--name-only', DEV155_BASE, 'HEAD', '--', *INPUTS).splitlines())
            if changed != DEV155_ALLOWED:
                raise ValueError('restored Persian R provider changes differ from the verified scope')
        elif key == DEV154_KEY:
            changed = set(git('diff', '--name-only', DEV154_BASE, 'HEAD', '--', *INPUTS).splitlines())
            if changed != DEV154_ALLOWED:
                raise ValueError('Persian R provider changes differ from the verified scope')
        elif key == DEV153_KEY:
            changed = set(git('diff', '--name-only', DEV153_BASE, 'HEAD', '--', *INPUTS).splitlines())
            if changed != DEV153_ALLOWED or any(git('rev-parse', 'HEAD:' + path) != blob
                                                 for path, blob in DEV153_CHECKED.items()):
                raise ValueError('Brazilian R provider changes differ from the verified scope')
        elif key == DEV152_KEY:
            changed = set(git('diff', '--name-only', DEV152_BASE, 'HEAD', '--', *INPUTS).splitlines())
            if changed != DEV152_ALLOWED:
                raise ValueError('Persian R changes differ from the verified scope')
        elif key == DEV151_KEY:
            changed = set(git('diff', '--name-only', DEV151_BASE, 'HEAD', '--', *INPUTS).splitlines())
            if changed != DEV151_ALLOWED:
                raise ValueError('Persian glide and vowel changes differ from the verified scope')
        elif key == DEV150_KEY:
            changed = set(git('diff', '--name-only', DEV150_BASE, 'HEAD', '--', *INPUTS).splitlines())
            if changed != DEV150_ALLOWED:
                raise ValueError('Persian glide and vowel changes differ from the verified scope')
        elif key == DEV149_KEY:
            changed = set(git('diff', '--name-only', DEV149_BASE, 'HEAD', '--', *INPUTS).splitlines())
            if changed != DEV149_ALLOWED:
                raise ValueError('absolute syllable clock changes differ from the verified scope')
        elif key == TRACE_KEY:
            changed = set(git('diff', '--name-only', TRACE_BASE, 'HEAD', '--', *INPUTS).splitlines())
            if changed != TRACE_ALLOWED:
                raise ValueError('syllable trace provider changes differ from the verified scope')
        elif key == PREVIOUS_KEY:
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
        if names is None or len(names.group(1).split()) != 21 or not BUILD.is_dir():
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
        if key in (TRACE_KEY, DEV149_KEY):
            objects.append(BUILD / 'lang/enus61/delta_rules_c29_enus61.o')
        if key == DEV154_KEY:
            objects.append(BUILD / 'src/eci/bridge/eci_stmarray.o')
        if key == DEV153_KEY:
            objects.extend(BUILD / f'src/reloq61_{name}.o'
                           for name in ('acoustic_output', 'spr_output', 'input_counts'))
        if key == DEV152_KEY:
            objects.extend(BUILD / f'lang/jajp61/delta_rules_c{part}_jajp61.o'
                           for part in ('17', '30'))
        # No provider object should depend on the changed public ECI header.
        # If one does, rebuilding that object is safer than assuming this.
        for dep in BUILD.rglob('*.d'):
            value = dep.read_text(encoding='utf8', errors='replace')
            if ('reloq_eci.h' in value or key in (TRACE_KEY, DEV149_KEY) and
                    ('reloq_persian_frames.h' in value or 'reloq_syllable_trace.h' in value)):
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
