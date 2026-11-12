"""Build a disposable Linux-only Castilian pronunciation experiment.

This changes Spanish inside the output binary and MUST NOT be distributed as
an application or used as a language module. Its source tree and baseline
binary are never edited. Build the ordinary Linux engine first.
"""
from __future__ import annotations

import argparse
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
PROVIDER = ROOT / 'vendor' / 'openevv61'
HERE = Path(__file__).resolve().parent


def split_patches(source: str):
    """Yield the single-file diffs from our checked-in experimental patch."""
    chunks = []
    for line in source.splitlines(keepends=True):
        if line.startswith('--- a/'):
            chunks.append([])
        if not chunks:
            raise ValueError('Patch has no original filename')
        chunks[-1].append(line)
    for chunk in chunks:
        relative = chunk[0][len('--- a/'):].strip()
        target = chunk[1][len('+++ b/'):].strip() if chunk[1].startswith('+++ b/') else None
        if relative != target or not relative.startswith('vendor/openevv61/lang/eses61/'):
            raise ValueError('Unexpected patch target')
        yield Path(relative), ''.join(chunk)


def host_objects():
    excluded = {'reloq_host.c', 'reloq_voice_filter.c', 'reloq_orthography.c',
                'reloq_german_word_edges.c', 'reloq_english_word_edges.c'}
    sources = [p for p in (ROOT / 'src').glob('*.c')
               if p.name not in excluded and not p.name.startswith('reloq_english')]
    sources.append(ROOT / 'src' / 'reloq_english_spr.c')
    objects = [ROOT / 'eci' / 'build' / 'posix' / (p.stem + '.o') for p in sources]
    if not all(p.is_file() for p in objects):
        raise RuntimeError('Build the ordinary Linux engine first: make -C eci libreloq_eci.so')
    return sorted(objects)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('kind', choices=('sh', 'au'), help='sh = admitted/shortened S; au = rounded glide')
    parser.add_argument('--dry-run', action='store_true', help='prepare and verify patches without compiling')
    args = parser.parse_args()
    work = Path(tempfile.mkdtemp(prefix='reloq-aragonese-probe-'))
    patch_file = HERE / ('native-sh-probe.patch' if args.kind == 'sh' else 'native-au-probe.patch')
    modified = []
    for relative, patch_text in split_patches(patch_file.read_text(encoding='utf8')):
        original = ROOT / relative
        destination = work / original.name
        subprocess.run(['patch', '--silent', '-o', str(destination), str(original)],
                       input=patch_text, text=True, check=True)
        modified.append(destination)
    if args.dry_run:
        print(f'Prepared {len(modified)} isolated source files in {work}')
        return

    objects = host_objects()
    parent = PROVIDER / 'build' / 'release' / 'libreloq61.a'
    if not parent.is_file():
        raise RuntimeError('The ordinary provider archive is missing')
    languages = ('enus61 dede61 ptbr fifi engb61 eses61 esmx61 frfr61 frca61 '
                 'itit61 jajp61 kokr61 zhcn61 zhtw61 elgr61').split()
    dirs = ['include', '../../include', 'rom/jajp', 'rom/cjk', 'rom/greek', 'src']
    dirs += [str(p.relative_to(PROVIDER)) for p in sorted((PROVIDER / 'src').glob('*')) if p.is_dir()]
    dirs += [str(p.relative_to(PROVIDER)) for p in sorted((PROVIDER / 'src').glob('*/*')) if p.is_dir()]
    dirs += ['lang/' + name for name in languages]
    flags = ['-O3', '-flto=4', '-fPIC', '-std=gnu11', '-fno-strict-aliasing',
             '-ffp-contract=off', '-fvisibility=hidden', '-ffunction-sections', '-fdata-sections',
             '-DEVV_ARENA=1', '-DEVV_NO_BYTECODE', '-DEVV_ROM_JAJP', '-DRELOQ61_PARAMETERS_ONLY=1', '-Os']
    compiled = []
    for source in modified:
        obj = work / (source.stem + '.o')
        subprocess.run(['cc', *flags, *['-I' + p for p in dirs], '-c', str(source), '-o', str(obj)],
                       cwd=PROVIDER, check=True)
        compiled.append(obj)
    archive = work / 'libreloq61.a'
    shutil.copyfile(parent, archive)
    subprocess.run(['gcc-ar', 'r', str(archive), *map(str, compiled)], check=True)
    library = work / 'libreloq_eci.so'
    subprocess.run(['cc', '-O3', '-flto=4', '-fno-strict-aliasing', '-ffp-contract=off', '-shared',
                    *map(str, objects), str(archive), '-pthread', '-Wl,--gc-sections,--no-undefined',
                    '-lm', '-o', str(library)], check=True)
    print('Experimental library:', library)


if __name__ == '__main__':
    main()
