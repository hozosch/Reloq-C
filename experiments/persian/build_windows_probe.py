"""Build an isolated Windows x64 Persian acoustic and text probe.

Reuse the ordinary Windows provider archive and ECI host objects. This is a
development DLL, not an installed NVDA driver or a release build.
"""
from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import shutil
import subprocess
import sys

from prepare_continuous_host import prepare, prepare_eci

ROOT = Path(__file__).resolve().parents[2]
HERE = Path(__file__).resolve().parent
PROVIDER = ROOT / 'vendor/openevv61'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--work', type=Path, required=True)
    parser.add_argument('--cc', default='gcc')
    parser.add_argument('--ar', default='gcc-ar')
    parser.add_argument('--jobs', type=int, choices=range(1, 9), default=2)
    parser.add_argument('--prepare-only', action='store_true')
    args = parser.parse_args()
    work = args.work.resolve()
    if work.exists():
        parser.error('work directory must not exist')
    archive = PROVIDER / 'build/windows-release/libreloq61.a'
    objects = sorted((ROOT / 'eci/build/windows').glob('*.o'))
    if not args.prepare_only and (not archive.is_file() or not objects):
        parser.error('build the ordinary Windows ECI core and provider first')
    subprocess.run([sys.executable, str(HERE / 'build_native_probe.py'),
                    '--dry-run', '--work-dir', str(work)], check=True)
    sources = sorted(work.glob('*.c'))
    for source in sources:
        text = source.read_text(encoding='utf8')
        if 'getenv(' in text:
            text = 'extern const char *reloq_fa_scoped_get(const char *);\n' + text.replace(
                'getenv(', 'reloq_fa_scoped_get(')
            source.write_text(text, encoding='utf8')
    if args.prepare_only:
        print('Prepared scoped Windows probe sources:', work)
        return
    languages = ('enus61 dede61 ptbr fifi engb61 eses61 esmx61 frfr61 frca61 '
                 'itit61 jajp61 kokr61 zhcn61 zhtw61 elgr61').split()
    dirs = ['include', '../../include', 'rom/jajp', 'rom/cjk', 'rom/greek', 'src']
    dirs += [str(p.relative_to(PROVIDER)) for p in sorted((PROVIDER / 'src').glob('*')) if p.is_dir()]
    dirs += [str(p.relative_to(PROVIDER)) for p in sorted((PROVIDER / 'src').glob('*/*')) if p.is_dir()]
    dirs += ['lang/' + name for name in languages]
    flags = ['-O3', '-flto=2', '-std=gnu11', '-fno-strict-aliasing', '-ffp-contract=off',
             '-fvisibility=hidden', '-ffunction-sections', '-fdata-sections', '-DEVV_ARENA=1',
             '-DEVV_NO_BYTECODE', '-DEVV_ROM_JAJP', '-DRELOQ61_PARAMETERS_ONLY=1',
             '-DRELOQ_FA_SCOPED_ONSETS=1', '-Os']

    def compile_one(source):
        output = work / (source.stem + '.o')
        print('Compiling', source.name, flush=True)
        subprocess.run([args.cc, *flags, *['-I' + d for d in dirs], '-c', str(source),
                        '-o', str(output)], cwd=PROVIDER, check=True)
        return output

    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        modified = list(pool.map(compile_one, sources))
    private_archive = work / 'libreloq61.a'
    shutil.copyfile(archive, private_archive)
    subprocess.run([args.ar, 'r', str(private_archive), *map(str, modified)], check=True)
    direct_sources = [HERE / ('direct_fa_' + name + '.c')
                      for name in ('lexicon', 'numbers', 'normalize', 'text', 'letters', 'pinglish')]
    direct_sources.extend((HERE / 'scoped_acoustics_probe.c', HERE / 'continuous_frames.c'))
    direct = []
    for source in direct_sources:
        obj = work / (source.stem + '.o')
        subprocess.run([args.cc, '-O2', '-std=c11', '-ffunction-sections', '-fdata-sections',
                        '-I' + str(ROOT / 'include'), '-c', str(source), '-o', str(obj)], check=True)
        direct.append(obj)
    # Only the private Persian DLL gains frame capture and continuous replay.
    # All other host objects and the baseline release remain unchanged.
    objects = [obj for obj in objects if obj.name not in ('reloq_rules61.o', 'reloq_eci.o')
               and not obj.name.startswith('reloq_fa_direct_')]
    for source in (prepare(work), prepare_eci(work)):
        obj = work / (source.stem + '.o')
        subprocess.run([args.cc, '-O2', '-std=c11', '-Wall', '-Wextra', '-Werror', '-pedantic',
                        '-ffunction-sections', '-fdata-sections', '-ffp-contract=off',
                        '-fvisibility=hidden', '-I' + str(ROOT / 'include'),
                        '-I' + str(ROOT / 'src'), '-I' + str(HERE),
                        '-I' + str(PROVIDER / 'include'),
                        '-c', str(source), '-o', str(obj)], check=True)
        direct.append(obj)
    exports = work / 'persian.def'
    exports.write_text('EXPORTS\nreloq_fa_direct_text\nreloq_fa_direct_project\nreloq_fa_direct_number\n'
                       'reloq_fa_direct_letter\nreloq_fa_direct_free\n'
                       'reloq_fa_pinglish_word\n', encoding='ascii')
    output = work / 'eci_persian_probe.dll'
    print('Linking private Windows acoustic and text probe', flush=True)
    subprocess.run([args.cc, '-O3', '-flto=2', '-fno-strict-aliasing', '-ffp-contract=off',
                    '-shared', *map(str, objects), *map(str, direct), str(private_archive),
                    str(exports), '-static-libgcc', '-lm', '-static',
                    '-Wl,--strip-all,--no-insert-timestamp', '-Wl,--gc-sections',
                    '-Wl,--no-undefined', '-o', str(output)], check=True)
    print('Ready:', output, flush=True)


if __name__ == '__main__':
    main()
