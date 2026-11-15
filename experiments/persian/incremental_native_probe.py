"""Incrementally rebuild explicitly changed files in a private Persian probe.

Prepare the new directory with build_native_probe.py --dry-run first. This
reuses an earlier private probe archive; it does not build a release or an
NVDA add-on. The ordinary engine and the rest of its source must be unchanged.
"""
from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor
import os
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
PROVIDER = ROOT / 'vendor/openevv61'
sys.path.insert(0, str(ROOT / 'experiments/aragonese'))
from build_native_probe import host_objects


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--previous', required=True, type=Path)
    parser.add_argument('--work', required=True, type=Path)
    parser.add_argument('--changed', nargs='+', required=True,
                        help='generated C files permitted to differ from the previous probe')
    parser.add_argument('--jobs', type=int, default=4)
    parser.add_argument('--archive-only', action='store_true',
                        help='prepare the updated archive without an intermediate library link')
    parser.add_argument('--scoped-onsets', action='store_true',
                        help='enable the private, thread-scoped native-onset table')
    args = parser.parse_args()
    previous, work = args.previous.resolve(), args.work.resolve()
    if previous == work or not 1 <= args.jobs <= 64:
        parser.error('use distinct directories and 1–64 jobs')
    changed = set(args.changed)
    sources = sorted(work.glob('*.c'))
    if not sources:
        parser.error('the new directory has no generated C files')
    for source in sources:
        old = previous / source.name
        if source.name not in changed and (not old.is_file() or source.read_bytes() != old.read_bytes()):
            raise RuntimeError(f'Unexpected changed or absent source: {source.name}')
    for name in changed:
        if Path(name).name != name or not (work / name).is_file():
            raise RuntimeError(f'Missing or invalid changed source: {name}')
    languages = ('enus61 dede61 ptbr fifi engb61 eses61 esmx61 frfr61 frca61 '
                 'itit61 jajp61 kokr61 zhcn61 zhtw61 elgr61').split()
    includes = ['include', '../../include', 'rom/jajp', 'rom/cjk', 'rom/greek', 'src']
    includes += [str(p.relative_to(PROVIDER)) for p in sorted((PROVIDER / 'src').glob('*')) if p.is_dir()]
    includes += [str(p.relative_to(PROVIDER)) for p in sorted((PROVIDER / 'src').glob('*/*')) if p.is_dir()]
    includes += ['lang/' + name for name in languages]
    flags = ['-O3', f'-flto={args.jobs}', '-fPIC', '-std=gnu11', '-fno-strict-aliasing',
             '-ffp-contract=off', '-fvisibility=hidden', '-ffunction-sections', '-fdata-sections',
             '-DEVV_ARENA=1', '-DEVV_NO_BYTECODE', '-DEVV_ROM_JAJP', '-DRELOQ61_PARAMETERS_ONLY=1', '-Os']
    if args.scoped_onsets:
        flags.append('-DRELOQ_FA_SCOPED_ONSETS=1')

    def compile_one(name: str) -> Path:
        source = work / name
        obj = work / (source.stem + '.o')
        print('Compiling', source.name, flush=True)
        subprocess.run(['cc', *flags, *['-I' + d for d in includes], '-c', str(source), '-o', str(obj)],
                       cwd=PROVIDER, check=True)
        return obj

    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        objects = list(pool.map(compile_one, sorted(changed)))
    archive = work / 'libreloq61.a'
    shutil.copyfile(previous / 'libreloq61.a', archive)
    subprocess.run(['gcc-ar', 'r', str(archive), *map(str, objects)], check=True)
    if args.archive_only:
        print('Ready:', archive, flush=True)
        return
    print('Linking focused experimental library', flush=True)
    output = work / 'libreloq_eci.so'
    subprocess.run(['cc', '-O3', f'-flto={args.jobs}', '-fno-strict-aliasing', '-ffp-contract=off',
                    '-shared', *map(str, host_objects()), str(archive), '-pthread',
                    '-Wl,--gc-sections,--no-undefined', '-lm', '-o', str(output)],
                   check=True, env={**os.environ, 'TMPDIR': str(work)})
    print('Ready:', output, flush=True)


if __name__ == '__main__':
    main()
