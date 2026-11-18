#!/usr/bin/env python3
"""Reuse only a verified dev144 Windows host cache after Persian rule edits."""
from __future__ import annotations

import os
from pathlib import Path
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[1]
BASE = '3c3d3f27635e48974487d6c8769a2b98dd97ae0c'
EXPECTED_KEY = ('persian-windows-host-gcc14-v2-'
                '095ec9297122d7ae23141480a61b623ca85d47fc13ed7e5a61e180300e5207bf')
RECENT_BASE = '5675c59f29e690b2e15ba7ba9c209934a09632c9'
RECENT_KEY = ('persian-windows-host-gcc14-v2-'
              'e69d174816ad2c632f7786280780e6d8f87ca561fcef6adc1212928b2e58f6b6')
RECENT_ALLOWED = {'include/reloq_eci.h', 'include/reloq_persian_numbers.h',
                  'src/reloq_eci.c', 'experiments/persian/direct_fa_numbers.c',
                  'experiments/persian/direct_fa_numbers.h',
                  'experiments/persian/direct_fa_characters.c',
                  'experiments/persian/direct_fa_characters.h'}
NEWEST_BASE = 'ca30814b6041007ecbe94c7681ef2e39a2ec09d9'
NEWEST_KEY = ('persian-windows-host-gcc14-v2-'
              'f4aa931982b6b718f119c59251c1b630a0645e043c0d790f358088e893c079a6')
NEWEST_ALLOWED = {'experiments/persian/direct_fa_lexicon.c',
                  'experiments/persian/direct_fa_text.c'}
TRACE_BASE = '14bfa3132c9936d2798d3281ac5693b550f002cb'
TRACE_KEY = ('persian-windows-host-gcc14-v2-'
             '9fbaf447f57f00e8aa4e28add7806baf049e17728506d6b7be26561f5313afaa')
DEV149_BASE = 'ccdc6680e8d698c6cf4834556706021aa0027f25'
DEV149_KEY = ('persian-windows-host-gcc14-v2-'
              '4419e090258b54c489fe7ecfba709236760771647c2b523f2865fd3ebe3fd944')
DEV149_ALLOWED = {'include/reloq_syllable_trace.h',
                  'experiments/persian/direct_fa_lexicon.c',
                  'vendor/openevv61/src/reloq61_bridge.c',
                  'vendor/openevv61/lang/fifi/delta_rules_c30_fifi.c',
                  'vendor/openevv61/lang/enus61/delta_rules_c29_enus61.c'}
DEV150_BASE = '0c2120cc341b6f9034564df99539f3c16e94e51c'
DEV150_KEY = ('persian-windows-host-gcc14-v2-'
              'dac446486a22d19a77d825d49f9646341087763174c54ba85cd8cda4c8791601')
DEV150_ALLOWED = {'experiments/persian/build_native_probe.py',
                  'src/reloq_persian_acoustics.c'}
DEV151_BASE = '64cecf6a0541bcc74a6f39677b726b576b9f1b25'
DEV151_KEY = ('persian-windows-host-gcc14-v2-'
              'a9bee5067b5866b06b93d8cc979749e88a6150b3e977d397276e2e911f96023e')
DEV151_ALLOWED = {'experiments/persian/build_native_probe.py',
                  'src/reloq_persian_acoustics.c'}
DEV152_BASE = '812e03b4fe7f0a8331b7a433c352a86a34710498'
DEV152_KEY = ('persian-windows-host-gcc14-v2-'
              '56a9101fabd669ff59d3badaef2dc36a73677c46a44d562608d88877b274f13a')
DEV152_ALLOWED = {'experiments/persian/build_native_probe.py',
                  'src/reloq_persian_acoustics.c',
                  'vendor/openevv61/lang/jajp61/delta_rules_c17_jajp61.c',
                  'vendor/openevv61/lang/jajp61/delta_rules_c30_jajp61.c'}
TRACE_ALLOWED = {'include/reloq_eci.h', 'include/reloq_persian_frames.h',
                 'include/reloq_syllable_trace.h', 'src/reloq_eci.c',
                 'src/reloq_persian_frames.c', 'src/reloq_persian_frame_internal.h',
                 'experiments/persian/direct_fa_lexicon.c',
                 'vendor/openevv61/src/reloq61_bridge.c',
                 'vendor/openevv61/lang/fifi/delta_rules_c30_fifi.c',
                 'vendor/openevv61/lang/enus61/delta_rules_c29_enus61.c'}
ALLOWED = {'experiments/persian/direct_fa_lexicon.c',
           'experiments/persian/direct_fa_lexicon.h',
           'experiments/persian/direct_fa_text.c'}
INPUTS = ('eci/Makefile', 'eci/reloq_eci_fixed_math.c', 'src', 'include',
          'vendor/openevv61', 'tools/prepare_persian_provider.py',
          'experiments/persian/build_native_probe.py',
          'experiments/aragonese/build_native_probe.py',
          'experiments/persian/direct_fa_*.c', 'experiments/persian/direct_fa_*.h',
          'experiments/persian/direct_fa_*.inc')


def eligible(key, changed):
    return bool(changed) and ((key == EXPECTED_KEY and set(changed) <= ALLOWED) or
                              (key == RECENT_KEY and set(changed) <= RECENT_ALLOWED) or
                              (key == NEWEST_KEY and set(changed) <= NEWEST_ALLOWED) or
                              (key == TRACE_KEY and set(changed) <= TRACE_ALLOWED) or
                              (key == DEV149_KEY and set(changed) == DEV149_ALLOWED) or
                              (key == DEV150_KEY and set(changed) == DEV150_ALLOWED) or
                              (key == DEV151_KEY and set(changed) == DEV151_ALLOWED) or
                              (key == DEV152_KEY and set(changed) == DEV152_ALLOWED))


def main():
    build, dll = ROOT / 'eci/build/windows', ROOT / 'eci/eci.dll'
    try:
        key = os.environ.get('PERSIAN_HOST_CACHE_KEY', '')
        base = DEV152_BASE if key == DEV152_KEY else DEV151_BASE if key == DEV151_KEY else DEV150_BASE if key == DEV150_KEY else DEV149_BASE if key == DEV149_KEY else TRACE_BASE if key == TRACE_KEY else NEWEST_BASE if key == NEWEST_KEY else RECENT_BASE if key == RECENT_KEY else BASE
        changed = subprocess.check_output(['git', 'diff', '--name-only', base, 'HEAD', '--', *INPUTS],
                                          cwd=ROOT, text=True).splitlines()
        if not eligible(key, changed):
            raise ValueError('cached host source transition was not verified')
        if not build.is_dir() or not dll.is_file():
            raise ValueError('cached host objects or DLL are missing')
        for path in build.rglob('*'):
            if path.is_file():
                os.utime(path, None)
        os.utime(dll, None)
        # All native Persian direct components depend on direct_fa_*.h.
        for obj in build.glob('reloq_fa_direct_*.o'):
            obj.unlink()
        if key in (RECENT_KEY, TRACE_KEY, DEV149_KEY, DEV150_KEY, DEV151_KEY, DEV152_KEY):
            # These are the only host sources which include reloq_eci.h;
            # direct Persian objects also cover the changed private headers.
            for name in ('reloq_eci.o', 'reloq_rules61.o'):
                (build / name).unlink(missing_ok=True)
            if key in (TRACE_KEY, DEV149_KEY):
                (build / 'reloq_persian_frames.o').unlink(missing_ok=True)
            dll.unlink()
        print('Reused verified Windows host objects; rebuilding Persian text components.')
    except (OSError, ValueError, subprocess.CalledProcessError) as exc:
        print(f'Cannot safely reuse host cache ({exc}); rebuilding the host cleanly.')
        shutil.rmtree(build, ignore_errors=True)
        dll.unlink(missing_ok=True)


if __name__ == '__main__':
    main()
