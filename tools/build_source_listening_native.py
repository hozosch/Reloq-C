"""Build an upper-harmonic listening host with the production S rollback.

Only the periodic-source wrapper is experimental. Normal ECI targets include
only the S rollback.
"""
from pathlib import Path
import argparse
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--provider', type=Path)
    args = parser.parse_args()
    output = args.output.resolve()
    output.parent.mkdir(parents=True, exist_ok=True)
    sources = [p for p in sorted((ROOT / 'src').glob('*.c'))
        if p.name not in ('reloq_host.c', 'reloq_voice_filter.c',
                         'reloq_orthography.c', 'reloq_german_word_edges.c')
        and (not p.name.startswith('reloq_english') or p.name == 'reloq_english_spr.c')]
    objects = [Path('build/posix') / (p.stem + '.o') for p in sources]
    provider = args.provider.resolve() if args.provider else ROOT / 'vendor/openevv61/build/release/libreloq61.a'
    targets = [str(p) for p in objects]
    if not args.provider:
        targets.append(str(provider))
    subprocess.run(['make', '-C', str(ROOT / 'eci'), '-o', 'Makefile', '-j2',
                    'PROVIDER_LTO=-flto=2', *targets], check=True)
    wrapper = output.parent / 'source-listening.o'
    subprocess.run(['cc', '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror',
        '-pedantic', '-ffp-contract=off', '-fPIC', '-fvisibility=hidden',
        '-I' + str(ROOT / 'include'), '-c', str(ROOT / 'tests/reloq_listening_source.c'),
        '-o', str(wrapper)], check=True)
    subprocess.run(['cc', '-O3', '-flto=2', '-fno-strict-aliasing', '-ffp-contract=off',
        '-shared', *(str(ROOT / 'eci' / p) for p in objects), str(wrapper), str(provider),
        '-pthread', '-Wl,--gc-sections,--no-undefined',
        '-Wl,--wrap=reloq_rate_transfer_process_voice', '-lm', '-o', str(output)], check=True)
    print(output)


if __name__ == '__main__':
    main()
