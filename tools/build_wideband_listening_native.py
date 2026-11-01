"""Build a listening-only five-profile host, leaving release source unchanged.

The private copy of the rate-transfer module changes only the lower S
target blend. Original upper banks, pole widths, power compensation and
native output clocks are retained. A link wrapper colours periodic voice.
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
    text = (ROOT / 'src/reloq_rate_transfer.c').read_text()
    replacements = [
        ('struct noise_grid {', 'extern unsigned reloq_listening_profile(void);\nstruct noise_grid {'),
        ('    memcpy(g->coefficients,p->target,sizeof g->coefficients);g->upper=p->upper_target;',
         '    g->sibilant=p->kind==1;\n    memcpy(g->coefficients,p->target,sizeof g->coefficients);g->upper=p->upper_target;'),
        ('    double coefficients[6][5],upper,profile[FFT_SIZE/2+1];',
         '    int sibilant;\n    double coefficients[6][5],upper,profile[FFT_SIZE/2+1];'),
        ('        wanted[k]=((1-mix)*ours[k]+mix*wanted[k]*a/b)*mag*mag;',
         '''        double blend=mix;
        if(grid->sibilant && (reloq_listening_profile()&4)) {
            /* Anchor only the lower flank. Preserve the existing wide
             * companion above 5 kHz, with no broadening or upper boost. */
            double hz=(double)k*rate/FFT_SIZE;
            double t=fmax(0,fmin(1,(5000-hz)/1200));
            blend+=.65*(1-blend)*t*t*(3-2*t);
        }
        wanted[k]=((1-blend)*ours[k]+blend*wanted[k]*a/b)*mag*mag;'''),
        ('float key[144]={rate,mix,(float)profile->kind,(float)!!historic84};',
         'float key[144]={rate,mix,(float)profile->kind,(float)((!!historic84)|(reloq_listening_profile()&12))};'),
        ('    causal_kernel(s,spectrum,0);\n    /* Correct the finite-kernel residual',
         '''    if(grid->sibilant && (reloq_listening_profile()&8)) {
        /* Listening hypothesis for roughness: suppress narrow ripple in
         * the inverse correction, not the upper frication band. Two small
         * moving averages in log magnitude form a smooth triangular
         * frequency window; existing power compensation follows below. */
        double logs[FFT_SIZE/2+1],prefix[FFT_SIZE/2+2];
        unsigned radius=(unsigned)fmax(1,floor(140*FFT_SIZE/rate+.5));
        for(unsigned k=0;k<=FFT_SIZE/2;++k)logs[k]=log(spectrum[k]);
        for(unsigned pass=0;pass<2;++pass) {
            prefix[0]=0;
            for(unsigned k=0;k<=FFT_SIZE/2;++k)prefix[k+1]=prefix[k]+logs[k];
            for(unsigned k=0;k<=FFT_SIZE/2;++k) {
                unsigned lo=k>radius?k-radius:0;
                unsigned hi=k+radius<FFT_SIZE/2?k+radius:FFT_SIZE/2;
                logs[k]=(prefix[hi+1]-prefix[lo])/(hi-lo+1);
            }
        }
        for(unsigned k=0;k<=FFT_SIZE/2;++k)spectrum[k]=exp(logs[k]);
    }
    causal_kernel(s,spectrum,0);
    /* Correct the finite-kernel residual'''),
    ]
    for old, new in replacements:
        assert text.count(old) == 1, old
        text = text.replace(old, new)
    source = output.parent / 'rate-transfer-listening.c'
    source.write_text(text)
    extra = []
    for path in (source, ROOT / 'tests/reloq_listening_wideband.c'):
        obj = output.parent / (path.stem + '.o')
        subprocess.run(['cc', '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror',
            '-pedantic', '-ffp-contract=off', '-fPIC', '-fvisibility=hidden',
            '-I' + str(ROOT / 'include'), '-c', str(path), '-o', str(obj)], check=True)
        extra.append(str(obj))
    linked = [str(ROOT / 'eci' / p) for p in objects if p.name != 'reloq_rate_transfer.o']
    subprocess.run(['cc', '-O3', '-flto=2', '-fno-strict-aliasing', '-ffp-contract=off',
        '-shared', *linked, *extra, str(provider), '-pthread',
        '-Wl,--gc-sections,--no-undefined', '-Wl,--wrap=reloq_rate_transfer_process_voice',
        '-lm', '-o', str(output)], check=True)
    print(output)


if __name__ == '__main__':
    main()
