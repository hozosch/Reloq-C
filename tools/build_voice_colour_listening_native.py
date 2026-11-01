"""Build a listening-only host with independently selectable voice colours.

Uses the unmodified 1.0 runtime source and the normal ECI make object rules.
The extra sections wrap only the periodic source function at link time.
No prototype setter or filter is compiled into the public NVDA/Android/SAPI
build targets. The setter is serialized between utterances by the WAV tool.
"""
from pathlib import Path
import argparse
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--cc', default='cc')
    args = parser.parse_args()
    changed = subprocess.check_output(['git', 'diff', '--name-only',
        '8cee7c3956358d03856b047198f0e2e57f3fa86b', '--', 'src', 'include', 'vendor'],
        cwd=ROOT, text=True)
    assert not changed, 'Listening prototype must retain the 1.0 acoustic/noise source'
    sources = [p for p in sorted((ROOT / 'src').glob('*.c'))
               if p.name not in ('reloq_host.c', 'reloq_voice_filter.c',
                                 'reloq_orthography.c', 'reloq_german_word_edges.c')
               and (not p.name.startswith('reloq_english') or p.name == 'reloq_english_spr.c')]
    objects = [Path('build/posix') / (source.stem + '.o') for source in sources]
    rules = Path('../vendor/openevv61/build/release/libreloq61.a')
    subprocess.run(['make', '-C', str(ROOT / 'eci'), '-o', 'Makefile', '-j2',
                    'PROVIDER_LTO=-flto=2', *map(str, objects), str(rules)], check=True)
    output = args.output.resolve()
    output.parent.mkdir(parents=True, exist_ok=True)
    wrapper = output.parent / 'voice-colour-wrapper.o'
    subprocess.run([args.cc, '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror',
                    '-pedantic', '-ffp-contract=off', '-fPIC', '-fvisibility=hidden',
                    '-I' + str(ROOT / 'include'), '-c',
                    str(ROOT / 'tests/reloq_listening_voice_colour.c'), '-o', str(wrapper)], check=True)
    subprocess.run([args.cc, '-O3', '-flto=2', '-fno-strict-aliasing', '-ffp-contract=off',
                    '-shared', *[str(ROOT / 'eci' / obj) for obj in objects], str(wrapper),
                    str(ROOT / 'eci' / rules), '-pthread', '-Wl,--gc-sections,--no-undefined',
                    '-Wl,--wrap=reloq_rate_transfer_process_voice',
                    '-Wl,--wrap=reloq_rate_transfer_prepare_noise_mix_profile',
                    '-lm', '-o', str(output)], check=True)
    print(output)


if __name__ == '__main__':
    main()
