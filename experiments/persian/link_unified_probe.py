"""Link Persian text analysis into an existing scoped Linux ECI probe.

Reuses the already compiled provider archive and host objects. This is an
experimental library, not an installed NVDA voice or a Windows build.
"""
from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor
import os
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT / 'experiments/aragonese'))
from build_native_probe import host_objects


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--scope-work', type=Path, required=True,
                        help='completed build_scoped_acoustics_probe work directory')
    parser.add_argument('--work', type=Path, required=True)
    parser.add_argument('--jobs', type=int, choices=range(1, 9), default=4)
    parser.add_argument('--continuous-capture', action='store_true',
                        help='add private request-scoped frame capture')
    parser.add_argument('--timing-trace', action='store_true',
                        help='build a diagnostic-only scope that forwards RELOQ_FA_TRACE_NUC')
    args = parser.parse_args()
    archive = args.scope_work.resolve() / 'libreloq61.a'
    scope = args.scope_work.resolve() / 'scoped_acoustics_probe.o'
    if not archive.is_file() or not scope.is_file():
        parser.error('the compiled scoped provider archive and scope object are required')
    work = args.work.resolve()
    if work.exists():
        parser.error('work directory must not exist')
    work.mkdir(parents=True)
    if args.timing_trace:
        # Keep the regular and NVDA scope independent of process environment.
        # This temporary scope forwards only the existing tracing switch and
        # retains the exact same selected acoustic values.
        text = (HERE / 'scoped_acoustics_probe.c').read_text(encoding='utf8')
        marker = '    if(!fa_acoustics || !name)return 0;\n'
        if text.count(marker) != 1:
            raise ValueError('the scoped acoustic selector changed; inspect it before tracing')
        text = text.replace(marker, marker +
                            '    if(!strcmp(name,"RELOQ_FA_TRACE_NUC"))return getenv(name);\n', 1)
        source = work / 'scoped_acoustics_timing.c'
        source.write_text(text, encoding='utf8')
        scope = work / 'scoped_acoustics_timing.o'
        subprocess.run(['cc', '-O3', f'-flto={args.jobs}', '-fPIC', '-std=gnu11',
                        '-fno-strict-aliasing', '-ffp-contract=off', '-fvisibility=hidden',
                        '-ffunction-sections', '-fdata-sections', '-Os', '-c', str(source),
                        '-o', str(scope)], check=True)
    sources = tuple(HERE / ('direct_fa_' + name + '.c')
                    for name in ('lexicon', 'numbers', 'normalize', 'text', 'letters', 'pinglish'))

    def compile_one(source):
        output = work / (source.stem + '.o')
        subprocess.run(['cc', '-O2', '-std=c11', '-fPIC', '-ffunction-sections',
                        '-fdata-sections', '-I' + str(ROOT / 'include'),
                        '-c', str(source), '-o', str(output)], check=True)
        return output

    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        objects = list(pool.map(compile_one, sources))
    hosts = host_objects()
    if args.continuous_capture:
        from prepare_continuous_host import prepare, prepare_eci
        source = prepare(work)
        continuous = (source, prepare_eci(work), HERE / 'continuous_frames.c')
        flags = ['-O2', '-std=c11', '-fPIC', '-ffunction-sections', '-fdata-sections',
                 '-ffp-contract=off', '-fvisibility=hidden', '-I' + str(ROOT / 'include'),
                 '-I' + str(ROOT / 'src'), '-I' + str(HERE)]
        for item in continuous:
            output = work / (item.stem + '.o')
            subprocess.run(['cc', *flags, '-c', str(item), '-o', str(output)], check=True)
            objects.append(output)
        hosts = [obj for obj in hosts if obj.name not in ('reloq_rules61.o', 'reloq_eci.o')]
    output = work / 'libreloq_eci_persian_probe.so'
    subprocess.run(['cc', '-O3', f'-flto={args.jobs}', '-fno-strict-aliasing', '-ffp-contract=off',
                    '-shared', *map(str, hosts), str(scope), *map(str, objects), str(archive),
                    '-pthread', '-Wl,--gc-sections,--no-undefined', '-lm', '-o', str(output)],
                   check=True, env={**os.environ, 'TMPDIR': str(work)})
    print('Ready:', output)


if __name__ == '__main__':
    main()
