"""Build a disposable Linux probe with thread-scoped Persian acoustics.

Only the handful of modified provider objects are rebuilt. This does not
register a language or change the ordinary provider archive or add-on.
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


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--work', type=Path, required=True)
    parser.add_argument('--jobs', type=int, default=4, choices=range(1, 9))
    args = parser.parse_args()
    work = args.work.resolve()
    if work.exists():
        parser.error('work directory must not exist')
    archive = PROVIDER / 'build/release/libreloq61.a'
    if not archive.is_file():
        parser.error('build the regular provider first')
    subprocess.run([sys.executable, str(ROOT / 'experiments/persian/build_native_probe.py'),
                    '--dry-run', '--work-dir', str(work)], check=True)
    languages = ('enus61 dede61 ptbr fifi engb61 eses61 esmx61 frfr61 frca61 '
                 'itit61 jajp61 kokr61 zhcn61 zhtw61 elgr61').split()
    dirs = ['include', '../../include', 'rom/jajp', 'rom/cjk', 'rom/greek', 'src']
    dirs += [str(p.relative_to(PROVIDER)) for p in sorted((PROVIDER / 'src').glob('*')) if p.is_dir()]
    dirs += [str(p.relative_to(PROVIDER)) for p in sorted((PROVIDER / 'src').glob('*/*')) if p.is_dir()]
    dirs += ['lang/' + name for name in languages]
    flags = ['-O3', '-flto=4', '-fPIC', '-std=gnu11', '-fno-strict-aliasing',
             '-ffp-contract=off', '-fvisibility=hidden', '-ffunction-sections', '-fdata-sections',
             '-DEVV_ARENA=1', '-DEVV_NO_BYTECODE', '-DEVV_ROM_JAJP', '-DRELOQ61_PARAMETERS_ONLY=1', '-Os']
    sources = sorted(work.glob('*.c'))
    for source in sources:
        text = source.read_text(encoding='utf8')
        if 'getenv(' not in text:
            continue
        text = 'extern const char *reloq_fa_scoped_get(const char *);\n' + text.replace(
            'getenv(', 'reloq_fa_scoped_get(')
        source.write_text(text, encoding='utf8')

    def compile_one(source):
        obj = work / (source.stem + '.o')
        print('Compiling', source.name, flush=True)
        subprocess.run(['cc', *flags, *['-I' + d for d in dirs], '-c', str(source), '-o', str(obj)],
                       cwd=PROVIDER, check=True)
        return obj

    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        objects = list(pool.map(compile_one, sources))
    local_archive = work / 'libreloq61.a'
    shutil.copyfile(archive, local_archive)
    subprocess.run(['gcc-ar', 'r', str(local_archive), *map(str, objects)], check=True)
    scope = work / 'scoped_acoustics_probe.o'
    subprocess.run(['cc', *flags, '-c', str(ROOT / 'experiments/persian/scoped_acoustics_probe.c'),
                    '-o', str(scope)], check=True)
    output = work / 'libreloq_eci_scoped.so'
    print('Linking scoped acoustic probe', flush=True)
    subprocess.run(['cc', '-O3', f'-flto={args.jobs}', '-fno-strict-aliasing', '-ffp-contract=off',
                    '-shared', *map(str, host_objects()), str(scope), str(local_archive), '-pthread',
                    '-Wl,--gc-sections,--no-undefined', '-lm', '-o', str(output)], check=True,
                   env={**os.environ, 'TMPDIR': str(work)})
    print('Ready:', output, flush=True)


if __name__ == '__main__':
    main()
