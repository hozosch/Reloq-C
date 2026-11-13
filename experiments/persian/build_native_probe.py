"""Build a private Persian acoustic experiment for Linux.

This is a listening prototype, not a Persian language module.  Its copy of
the Finnish and US providers can be changed using process environment variables;
never distribute this binary as a normal Finnish or Persian synthesizer.
The repository's ordinary binaries and native provider sources are untouched.

The experimental long-a offsets approximate the centre-frame formants in
eSpeak NG's GPL-3.0-or-later phsource/vowel/0_3 (commit 688cb60368d6e0998f3632af8d1c3c2ad4191fc2).
They are interpreted through the existing Klatt synthesizer, which is not
the eSpeak synthesizer. The q settings are exploratory and do not copy its
samples. This experiment is distributed under the project's GPL license.
"""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
PROVIDER = ROOT / 'vendor' / 'openevv61'
sys.path.insert(0, str(ROOT / 'experiments' / 'aragonese'))
from build_native_probe import host_objects

Q_HELPER = r'''
/* Only the disposable listening build uses source spelling as a q switch. */
static int persian_probe_source_q(void *state, int32_t position)
{
    delta_state *d = state;
    if (!d || !position) return 0;
    delta_node *point = vmovel(d, EVV_AT(delta_node *, position), 1);
    if (!point) return 0;
    int32_t node = vgetsc(d, 1, 0, EVV_REF(point), 1);
    if (!node || (*EVV_AT(int32_t *, node) & 2)) return 0;
    unsigned key = *(const unsigned char *)delta_record_field(
        &vstmtbl[1], 0, TFLDS(EVV_AT(void *, node)));
    return key == 25 || key == 45; /* Finnish source projection: q, Q. */
}
static int persian_probe_setting(const char *name)
{
    const char *value = getenv(name);
    return value && value[0] >= '1' && value[0] <= '4' && !value[1]
        ? value[0] - '0' : 0;
}
static int16_t persian_probe_lower(int16_t value, int offset, int minimum)
{
    if (value <= 0) return value; /* -1/-2 are native inheritance markers. */
    return value - offset < minimum ? minimum : value - offset;
}
'''

US_A_PATCH = r'''
    /* Experimental US /a/, as in father. The nominal base is 750/1200/2440
       Hz. Two intermediate settings move it towards eSpeak's 0_3 nucleus
       656/1000/2620, preserving context-dependent endpoints and sentinels. */
    {
        const char *setting = getenv("RELOQ_FA_PROBE_US_A");
        int variant = setting && !setting[1] && (setting[0]=='1' || setting[0]=='2')
            ? setting[0]-'0' : 0;
        if (variant) {
            int f1 = variant==1 ? 47 : 94;
            int f2 = variant==1 ? 100 : 200;
            int f3 = variant==1 ? 90 : 180;
            if (STATE(int16_t,s448)>0) STATE(int16_t,s448)-=f1;
            if (STATE(int16_t,s449)>0) STATE(int16_t,s449)-=f1;
            if (STATE(int16_t,s450)>0) STATE(int16_t,s450)-=f2;
            if (STATE(int16_t,s451)>0) STATE(int16_t,s451)-=f2;
            if (STATE(int16_t,s452)>0) STATE(int16_t,s452)+=f3;
            if (STATE(int16_t,s453)>0) STATE(int16_t,s453)+=f3;
        }
    }
'''

A_PATCH = r'''
    /* Preserve coarticulation and the native inheritance markers.  This is
       an approximation of the eSpeak 0_3 nucleus, not an imported spectrum. */
    if (getenv("RELOQ_FA_PROBE_A") && *getenv("RELOQ_FA_PROBE_A") == '1') {
        if (STATE(int16_t,s448)>0) STATE(int16_t,s448)+=31;
        if (STATE(int16_t,s449)>0) STATE(int16_t,s449)+=31;
        if (STATE(int16_t,s450)>0) STATE(int16_t,s450)-=50;
        if (STATE(int16_t,s451)>0) STATE(int16_t,s451)-=50;
        if (STATE(int16_t,s452)>0) STATE(int16_t,s452)+=20;
        if (STATE(int16_t,s453)>0) STATE(int16_t,s453)+=20;
    }
'''

Q_PATCH = r'''
    {
        int variant = persian_probe_setting("RELOQ_FA_PROBE_Q");
        if (variant && persian_probe_source_q(state, STATE(int32_t,w61))) {
            /* Tentative lower resonances and reduced high-frequency burst.
               Different release and coarticulation from a velar K, but not
               yet a phonetically validated Persian uvular consonant. */
            int f2 = variant == 2 ? 300 : 150;
            int f3 = variant == 2 ? 450 : 250;
            int burst = variant == 2 ? 24 : 12;
            STATE(int16_t,s450)=persian_probe_lower(STATE(int16_t,s450),f2,650);
            STATE(int16_t,s451)=persian_probe_lower(STATE(int16_t,s451),f2,650);
            STATE(int16_t,s452)=persian_probe_lower(STATE(int16_t,s452),f3,1200);
            STATE(int16_t,s453)=persian_probe_lower(STATE(int16_t,s453),f3,1200);
            STATE(int16_t,s440)=persian_probe_lower(STATE(int16_t,s440),burst,0);
            STATE(int16_t,s441)=persian_probe_lower(STATE(int16_t,s441),burst,0);
            STATE(int16_t,s442)=persian_probe_lower(STATE(int16_t,s442),burst,0);
            if (variant >= 3) {
                /* Short post-release frication. The German /x/ rule uses
                   similar mid-band levels (70 and 55); this is not a
                   concatenated /k+x/ and not a validated [q]. f11 is the
                   native release end timestamp in units of 1/10000 ms. */
                int32_t extension = variant==3 ? 100000 : 250000;
                if (STATE(int32_t,f11)>=0 && STATE(int32_t,f11)<=INT32_MAX-extension)
                    STATE(int32_t,f11)+=extension;
                if (STATE(int16_t,s438)<65) STATE(int16_t,s438)=65;
                if (STATE(int16_t,s440)<(variant==3 ? 48 : 55))
                    STATE(int16_t,s440)=variant==3 ? 48 : 55;
            }
        }
    }
'''


def prepare(language: str, name: str, rule: str, label: str, patch: str, helper: str, work: Path) -> Path:
    source = PROVIDER / 'lang' / language / name
    text = source.read_text(encoding='utf8')
    start = text.index('int32_t reloq_' + language + '_' + rule + '(')
    end = text.index('\nstatic int32_t evv_' + rule + '(', start)
    rule_text = text[start:end]
    marker = label + ':;\n    r0 = reloq61_typed_succeed'
    if rule_text.count(marker) != 1:
        raise RuntimeError('Cannot locate unique success point in ' + name)
    rule_text = rule_text.replace(marker, label + ':;\n' + patch +
                                  '    r0 = reloq61_typed_succeed', 1)
    text = text[:start] + rule_text + text[end:]
    include = '#include "reloq61_rule_calls_' + language + '.h"\n'
    if text.count(include) != 1:
        raise RuntimeError('Cannot locate unique include point in ' + name)
    text = text.replace(include, include + '#include <stdlib.h>\n' + helper, 1)
    destination = work / name
    destination.write_text(text, encoding='utf8')
    return destination


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--work-dir', type=Path)
    parser.add_argument('--dry-run', action='store_true')
    parser.add_argument('--jobs', type=int, default=4, choices=range(1, 9))
    args = parser.parse_args()
    if args.work_dir:
        work = args.work_dir.resolve()
        work.mkdir(parents=True, exist_ok=False)
    else:
        work = Path(tempfile.mkdtemp(prefix='reloq-persian-probe-'))
    modified = [
        prepare('fifi', 'delta_rules_c17_fifi.c', 'stanfin_ph_a', 'L13086', A_PATCH, '', work),
        prepare('fifi', 'delta_rules_c22_fifi.c', 'fin_ph_k', 'L5915', Q_PATCH, Q_HELPER, work),
        prepare('enus61', 'delta_rules_c06_enus61.c', 'ga_ph_a', 'L3010', US_A_PATCH, '', work),
    ]
    if args.dry_run:
        print('Prepared isolated source files:', work)
        return
    archive = PROVIDER / 'build' / 'release' / 'libreloq61.a'
    if not archive.is_file():
        raise RuntimeError('Build the ordinary Linux engine and provider first')
    objects = host_objects()
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
        print('Compiling', source.name, flush=True)
        subprocess.run(['cc', *flags, *['-I' + p for p in dirs], '-c', str(source), '-o', str(obj)],
                       cwd=PROVIDER, check=True)
        compiled.append(obj)
    private_archive = work / 'libreloq61.a'
    shutil.copyfile(archive, private_archive)
    subprocess.run(['gcc-ar', 'r', str(private_archive), *map(str, compiled)], check=True)
    library = work / 'libreloq_eci.so'
    print('Linking private experimental library; this may take several minutes', flush=True)
    subprocess.run(['cc', '-O3', f'-flto={args.jobs}', '-fno-strict-aliasing', '-ffp-contract=off', '-shared',
                    *map(str, objects), str(private_archive), '-pthread', '-Wl,--gc-sections,--no-undefined',
                    '-lm', '-o', str(library)], check=True, env={**os.environ, 'TMPDIR': str(work)})
    print('Experimental library:', library)


if __name__ == '__main__':
    main()
