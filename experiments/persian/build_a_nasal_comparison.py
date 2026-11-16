"""Build a listening-only comparison with the prior back-A nasal setting.

All other provider objects, including the accepted L and the 1.5.0-dev115
timing candidate, are reused unchanged. This deliberately does not alter
the default source or expose a user-facing control.
"""
from __future__ import annotations

import argparse
from pathlib import Path
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[2]
PROVIDER = ROOT / 'vendor/openevv61'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--work', type=Path, required=True)
    parser.add_argument('--windows', action='store_true')
    parser.add_argument('--cc', default='cc')
    parser.add_argument('--ar', default='gcc-ar')
    args = parser.parse_args()
    work = args.work.resolve()
    if work.exists():
        parser.error('the comparison work directory must not already exist')
    work.mkdir(parents=True)
    build = 'windows-release' if args.windows else 'release'
    host = ROOT / ('eci/build/windows' if args.windows else 'eci/build/posix')
    archive = PROVIDER / 'build' / build / 'libreloq61.a'
    if not archive.is_file() or not host.is_dir():
        parser.error('build the standard matching C engine first')
    source = (ROOT / 'src/reloq_persian_acoustics.c').read_text(encoding='utf8')
    marker = '{"RELOQ_FA_PROBE_A_NASAL","1"}'
    if source.count(marker) != 1:
        raise RuntimeError('the accepted back-A nasal selection changed')
    modified = work / 'reloq_persian_acoustics.c'
    modified.write_text(source.replace(marker, '{"RELOQ_FA_PROBE_A_NASAL","0"}'), encoding='utf8')
    obj = work / 'reloq_persian_acoustics.o'
    flags = ['-O3', '-flto=4', '-std=gnu11', '-fno-strict-aliasing', '-ffp-contract=off',
             '-fvisibility=hidden', '-ffunction-sections', '-fdata-sections']
    if not args.windows:
        flags.append('-fPIC')
    subprocess.run([args.cc, *flags, '-c', str(modified), '-o', str(obj)], check=True)
    comparison = work / 'libreloq61.a'
    shutil.copyfile(archive, comparison)
    subprocess.run([args.ar, 'r', str(comparison), str(obj)], check=True)
    excluded = {'reloq_host.c', 'reloq_voice_filter.c', 'reloq_orthography.c',
                'reloq_german_word_edges.c', 'reloq_english_word_edges.c',
                'reloq_persian_acoustics.c'}
    sources = [item for item in (ROOT / 'src').glob('*.c')
               if item.name not in excluded and not item.name.startswith('reloq_english')]
    sources.append(ROOT / 'src/reloq_english_spr.c')
    objects = [host / (item.stem + '.o') for item in sorted(sources)]
    if args.windows:
        objects.append(host / 'reloq_eci_fixed_math.o')
    if any(not item.is_file() for item in objects):
        raise RuntimeError('the matching native host objects are incomplete')
    output = work / ('eci.dll' if args.windows else 'libreloq_eci.so')
    link = [args.cc, '-O3', '-flto=2', '-fno-strict-aliasing', '-ffp-contract=off',
            '-shared', *map(str, objects), str(comparison)]
    if args.windows:
        link += ['-static', '-Wl,--strip-all,--no-insert-timestamp']
    else:
        link.append('-pthread')
    link += ['-Wl,--gc-sections,--no-undefined', '-lm', '-o', str(output)]
    subprocess.run(link, check=True)
    print('Comparison engine:', output, flush=True)


if __name__ == '__main__':
    main()
