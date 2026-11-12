"""Build a disposable Linux-only Castilian pronunciation experiment.

This changes Spanish inside the output binary and MUST NOT be distributed as
an application or used as a language module. Its source tree and baseline
binary are never edited. Build the ordinary Linux engine first.
"""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
PROVIDER = ROOT / 'vendor' / 'openevv61'
HERE = Path(__file__).resolve().parent

PROBE_PERCENT_HELPER = '''
/* Private listening builds only; never include this in a real language. */
static int aragonese_probe_percent(const char *name, int fallback)
{
    const char *text = getenv(name);
    char *end;
    long value;
    if (!text || !*text) return fallback;
    value = strtol(text, &end, 10);
    if (*end) return fallback;
    if (value < 0) return 0;
    if (value > 100) return 100;
    return (int)value;
}
'''

# Used only inside the throwaway glide build. Test an /a w/ phone sequence
# against a word fence, preserving the provider's cursor and fence arrays.
# The scanner's context result is verified against real audio before use.
PROBE_AW_CONTEXT_HELPER = r'''
static int aragonese_probe_aw_context(void *state, int32_t token_ref)
{
    delta_state *d = state;
    if (!d || !d->vars || !token_ref) return 0;
    delta_vars *v = EVV_AT(delta_vars *,d->vars);
    delta_tpos left = d->lpta, right = d->rpta;
    unsigned char scan[8], index[256], marks[256], chars[256];
    int8_t compared = v->compared_equal, fences = v->fence_count, testing = v->testing;
    memcpy(scan,&v->scan_ptr,sizeof scan);
    memcpy(index,EVV_AT(const void *,d->fence_index),d->nstmts);
    memcpy(marks,EVV_AT(const void *,d->fence_marks),d->nstmts);
    memcpy(chars,EVV_AT(const void *,d->fence_chars),d->nstmts);
    v->testing = 0;
    const uint8_t word_fence[] = {2};
    fence(d,1,word_fence);
    lpta_loadp(d,EVV_AT(const delta_token *,token_ref));
    int after_a = !setscan_l(d,4) && !testFldeq(d,4,0,31) && !advance_tok(d);
    lpta_loadp(d,EVV_AT(const delta_token *,token_ref));
    int at_w = !setscan_r(d,4) && !testFldeq(d,4,0,28);
    int final = at_w && (advance_tok(d) || advance_tok(d));
    d->lpta = left; d->rpta = right; memcpy(&v->scan_ptr,scan,sizeof scan);
    v->compared_equal = compared; v->fence_count = fences; v->testing = testing;
    memcpy(EVV_AT(void *,d->fence_index),index,d->nstmts);
    memcpy(EVV_AT(void *,d->fence_marks),marks,d->nstmts);
    memcpy(EVV_AT(void *,d->fence_chars),chars,d->nstmts);
    if (!after_a || !at_w) return 0;
    return final ? 2 : 1;
}

static int aragonese_probe_glide_context(void *state, int32_t token_ref)
{
    int mode = aragonese_probe_percent("RELOQ_ARAGON_PROBE_GLIDE_SCOPE",0);
    if (mode == 1) return aragonese_probe_aw_context(state,token_ref);
    if (mode == 2) return aragonese_probe_percent("RELOQ_ARAGON_PROBE_MANUAL_FINAL",0) ? 2 : 1;
    return 0;
}
'''


def replace_once(text: str, before: str, after: str) -> str:
    if text.count(before) != 1:
        raise ValueError('Experimental patch target is missing or ambiguous')
    return text.replace(before, after, 1)


def enable_refinement(source: Path) -> None:
    """Parameterize the two private patches without modifying the checkout."""
    text = source.read_text(encoding='utf8')
    if source.name.endswith('c31_eses61.c'):
        text = replace_once(text, 'reloq_local_196.i32*4/5',
                            'reloq_local_196.i32*aragonese_probe_percent('
                            '"RELOQ_ARAGON_PROBE_SH_PERCENT",80)/100')
    elif source.name.endswith('c30_eses61.c'):
        old = ('    if(STATE(int16_t,s448)>300)STATE(int16_t,s448)=300;\n'
               '    if(STATE(int16_t,s450)>750)STATE(int16_t,s450)=750;')
        new = '''    {
        int amount = aragonese_probe_percent("RELOQ_ARAGON_PROBE_AU_PERCENT",100);
        if(STATE(int16_t,s448)>300)
            STATE(int16_t,s448) -= (STATE(int16_t,s448)-300)*amount/100;
        if(STATE(int16_t,s450)>750)
            STATE(int16_t,s450) -= (STATE(int16_t,s450)-750)*amount/100;
    }'''
        text = replace_once(text, old, new)
    else:
        return
    text = replace_once(text, '#include <string.h>',
                        '#include <string.h>\n#include <stdlib.h>\n' + PROBE_PERCENT_HELPER)
    source.write_text(text, encoding='utf8')


def enable_glide(source: Path) -> None:
    """Add F1/F2 controls and a tentative duration probe to private copies."""
    if not source.name.endswith(('c11_eses61.c', 'c30_eses61.c', 'c31_eses61.c')):
        return
    if not source.name.endswith('c11_eses61.c'):
        enable_refinement(source)
    text = source.read_text(encoding='utf8')
    if source.name.endswith('c31_eses61.c'):
        text = replace_once(text, '"RELOQ_ARAGON_PROBE_SH_PERCENT",80',
                            '"RELOQ_ARAGON_PROBE_SH_PERCENT",65')
    elif source.name.endswith('c30_eses61.c'):
        old = '''    {
        int amount = aragonese_probe_percent("RELOQ_ARAGON_PROBE_AU_PERCENT",100);
        if(STATE(int16_t,s448)>300)
            STATE(int16_t,s448) -= (STATE(int16_t,s448)-300)*amount/100;
        if(STATE(int16_t,s450)>750)
            STATE(int16_t,s450) -= (STATE(int16_t,s450)-750)*amount/100;
    }'''
        new = '''    {
        int scope = aragonese_probe_percent("RELOQ_ARAGON_PROBE_GLIDE_SCOPE",0);
        int ctx = aragonese_probe_glide_context(state,STATE_AT(w60,-4));
        int fallback = aragonese_probe_percent("RELOQ_ARAGON_PROBE_AU_PERCENT",100);
        int f1 = scope == 0 ? fallback : (ctx == 2 ?
            aragonese_probe_percent("RELOQ_ARAGON_PROBE_AU_FINAL_F1",0) : ctx == 1 ?
            aragonese_probe_percent("RELOQ_ARAGON_PROBE_AU_MEDIAL_F1",0) : 0);
        int f2 = scope == 0 ? fallback : (ctx == 2 ?
            aragonese_probe_percent("RELOQ_ARAGON_PROBE_AU_FINAL_F2",40) : ctx == 1 ?
            aragonese_probe_percent("RELOQ_ARAGON_PROBE_AU_MEDIAL_F2",40) : 0);
        if(STATE(int16_t,s448)>300)
            STATE(int16_t,s448) -= (STATE(int16_t,s448)-300)*f1/100;
        if(STATE(int16_t,s450)>750)
            STATE(int16_t,s450) -= (STATE(int16_t,s450)-750)*f2/100;
    }'''
        text = replace_once(text, old, new)
        text = replace_once(text, '/* Where each global the rules touch lands in the state,',
                            PROBE_AW_CONTEXT_HELPER + '\n/* Where each global the rules touch lands in the state,')
    else:
        text = replace_once(text, '#include <string.h>',
                            '#include <string.h>\n#include <stdlib.h>\n' + PROBE_PERCENT_HELPER)
        text = replace_once(text, '/* Where each global the rules touch lands in the state,',
                            PROBE_AW_CONTEXT_HELPER + '\n/* Where each global the rules touch lands in the state,')
        begin = text.index('/* span_ph_w_dur, from f61_26.obj */')
        end = text.index('static int32_t evv_span_ph_w_dur', begin)
        section = text[begin:end]
        before = 'L779:;\n    r0 = reloq61_typed_succeed(DELTA_ENTRY_succeed, state);'
        after = '''L779:;
    {
        int ctx = aragonese_probe_glide_context(state,STATE_AT(w60,-4));
        int pct = ctx == 2 ? aragonese_probe_percent("RELOQ_ARAGON_PROBE_AU_FINAL_DURATION",100) :
                  ctx == 1 ? aragonese_probe_percent("RELOQ_ARAGON_PROBE_AU_MEDIAL_DURATION",100) : 100;
        STATE(int32_t,f20) = (int32_t)((int64_t)STATE(int32_t,f20)*pct/100);
    }
    r0 = reloq61_typed_succeed(DELTA_ENTRY_succeed, state);'''
        text = text[:begin] + replace_once(section,before,after) + text[end:]
    source.write_text(text, encoding='utf8')


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
    parser.add_argument('kind', choices=('sh', 'au', 'refine', 'glide'),
                        help='sh, au, refine, or independent glide listening probes')
    parser.add_argument('--dry-run', action='store_true', help='prepare and verify patches without compiling')
    parser.add_argument('--work-dir', type=Path, help='new private directory for the build')
    args = parser.parse_args()
    if args.work_dir:
        work = args.work_dir.resolve()
        work.mkdir(parents=True, exist_ok=False)
    else:
        work = Path(tempfile.mkdtemp(prefix='reloq-aragonese-probe-'))
    patch_files = ([HERE / 'native-sh-probe.patch', HERE / 'native-au-probe.patch']
                   if args.kind in ('refine', 'glide') else [HERE / f'native-{args.kind}-probe.patch'])
    modified = []
    for patch_file in patch_files:
        for relative, patch_text in split_patches(patch_file.read_text(encoding='utf8')):
            original = ROOT / relative
            destination = work / original.name
            subprocess.run(['patch', '--silent', '-o', str(destination), str(original)],
                           input=patch_text, text=True, check=True)
            if args.kind == 'refine': enable_refinement(destination)
            if args.kind == 'glide': enable_glide(destination)
            modified.append(destination)
    if args.kind == 'glide':
        original = PROVIDER / 'lang/eses61/delta_rules_c11_eses61.c'
        destination = work / original.name
        shutil.copyfile(original,destination)
        enable_glide(destination)
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
        print('Compiling', source.name, flush=True)
        obj = work / (source.stem + '.o')
        subprocess.run(['cc', *flags, *['-I' + p for p in dirs], '-c', str(source), '-o', str(obj)],
                       cwd=PROVIDER, check=True)
        compiled.append(obj)
    archive = work / 'libreloq61.a'
    shutil.copyfile(parent, archive)
    subprocess.run(['gcc-ar', 'r', str(archive), *map(str, compiled)], check=True)
    library = work / 'libreloq_eci.so'
    print('Linking experimental library; this can take several minutes', flush=True)
    subprocess.run(['cc', '-O3', '-flto=4', '-fno-strict-aliasing', '-ffp-contract=off', '-shared',
                    *map(str, objects), str(archive), '-pthread', '-Wl,--gc-sections,--no-undefined',
                    '-lm', '-o', str(library)], check=True, env={**os.environ, 'TMPDIR': str(work)})
    print('Experimental library:', library)


if __name__ == '__main__':
    main()
