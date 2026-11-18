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
    return key == EXPECTED_KEY and bool(changed) and set(changed) <= ALLOWED


def main():
    build, dll = ROOT / 'eci/build/windows', ROOT / 'eci/eci.dll'
    try:
        changed = subprocess.check_output(['git', 'diff', '--name-only', BASE, 'HEAD', '--', *INPUTS],
                                          cwd=ROOT, text=True).splitlines()
        if not eligible(os.environ.get('PERSIAN_HOST_CACHE_KEY', ''), changed):
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
        print('Reused verified Windows host objects; rebuilding Persian text components.')
    except (OSError, ValueError, subprocess.CalledProcessError) as exc:
        print(f'Cannot safely reuse host cache ({exc}); rebuilding the host cleanly.')
        shutil.rmtree(build, ignore_errors=True)
        dll.unlink(missing_ok=True)


if __name__ == '__main__':
    main()
