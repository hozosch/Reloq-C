"""Build a disposable Linux library with private F0 trace/override hooks.

The ordinary ECI sources and the existing provider archive are not changed.
This is a research hook for comparing shared intonation, not an NVDA API.
"""
from __future__ import annotations

import argparse
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'experiments/aragonese'))
from build_native_probe import host_objects


def replace_once(text: str, old: str, new: str) -> str:
    if text.count(old) != 1:
        raise RuntimeError(f'missing or ambiguous source insertion: {old[:80]!r}')
    return text.replace(old, new, 1)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--archive', type=Path, required=True)
    parser.add_argument('--work', type=Path, required=True)
    parser.add_argument('--jobs', type=int, default=4)
    args = parser.parse_args()
    if not args.archive.is_file() or not 1 <= args.jobs <= 64:
        parser.error('existing private provider archive and 1–64 jobs required')
    work = args.work.resolve()
    work.mkdir(parents=True, exist_ok=True)
    text = (ROOT / 'src/reloq_rules61.c').read_text(encoding='utf8')
    text = replace_once(text, 'struct rule_plan {\n', '''struct rule_plan {
    /* Private only: request-scoped pitch data in native tenths of Hz. */
    FILE *probe_f0_trace, *probe_frame_trace;
    float *probe_f0;
    size_t probe_f0_count;
''')
    text = replace_once(text, 'static int process_spans(struct rule_plan *,int final);\n', r'''
static int probe_f0_setup(struct rule_plan *s)
{
    const char *path=getenv("RELOQ_FA_F0_OVERRIDE");
    if(path&&*path) {
        FILE *f=fopen(path,"rb");
        if(!f)return -1;
        if(fseek(f,0,SEEK_END)) {fclose(f);return -1;}
        long n=ftell(f);
        if(n<=0||n>4*600000||n%(long)sizeof(float)||fseek(f,0,SEEK_SET)) {fclose(f);return -1;}
        s->probe_f0=malloc((size_t)n);
        if(!s->probe_f0){fclose(f);return -1;}
        if(fread(s->probe_f0,1,(size_t)n,f)!=(size_t)n){fclose(f);return -1;}
        fclose(f);s->probe_f0_count=(size_t)n/sizeof(float);
        for(size_t i=0;i<s->probe_f0_count;++i)
            if(!isfinite(s->probe_f0[i])||s->probe_f0[i]<10||s->probe_f0[i]>240000)return -1;
    }
    path=getenv("RELOQ_FA_F0_TRACE");
    if(path&&*path && !(s->probe_f0_trace=fopen(path,"wb")))return -1;
    path=getenv("RELOQ_FA_FRAME_TRACE");
    if(path&&*path && !(s->probe_frame_trace=fopen(path,"wb")))return -1;
    return 0;
}
static int process_spans(struct rule_plan *,int final);
''')
    text = replace_once(text, '        if(pitch_scale!=1)controls[1]=(float)(controls[1]*pitch_scale);\n', r'''        if(pitch_scale!=1)controls[1]=(float)(controls[1]*pitch_scale);
        if(s->probe_f0_count && tick>=0) {
            size_t offset=(size_t)tick;
            if(offset>=s->probe_f0_count)offset=s->probe_f0_count-1;
            controls[1]=s->probe_f0[offset];
        }
        if(s->probe_f0_trace) {
            struct {int32_t tick;float f0,av,ah;} row={(int32_t)tick,controls[1],controls[2],controls[7]};
            if(fwrite(&row,sizeof row,1,s->probe_f0_trace)!=1){p->result=RELOQ_DE_UTTERANCE_SINK;return 1;}
        }
''')
    text = replace_once(text, '        reloq_german_voice_model_frame(&p->model,controls);\n', r'''        reloq_german_voice_model_frame(&p->model,controls);
        if(s->probe_frame_trace) {
            struct {int32_t tick;float controls[62];} row;
            row.tick=(int32_t)tick;memcpy(row.controls,controls,sizeof row.controls);
            if(fwrite(&row,sizeof row,1,s->probe_frame_trace)!=1){p->result=RELOQ_DE_UTTERANCE_SINK;return 1;}
        }
''')
    text = replace_once(text,
                        '    struct reloq61_mark *volumes=NULL;size_t volume_count=0;size_t *control_sources=NULL;\n',
                        '    struct reloq61_mark *volumes=NULL;size_t volume_count=0;size_t *control_sources=NULL;\n'
                        '    if(probe_f0_setup(&plan))goto done;\n')
    text = replace_once(text, 'done:free(a);free(map);free(marks);',
                        'done:if(plan.probe_f0_trace)fclose(plan.probe_f0_trace);'
                        'if(plan.probe_frame_trace)fclose(plan.probe_frame_trace);'
                        'free(plan.probe_f0);free(a);free(map);free(marks);')
    source = work / 'reloq_rules61_contour_probe.c'
    obj = work / 'reloq_rules61_contour_probe.o'
    replay = work / 'frame_replay_probe.o'
    output = work / 'libreloq_eci.so'
    source.write_text(text, encoding='utf8')
    compile_cmd = ['cc', '-O2', '-std=c11', '-Wall', '-Wextra', '-Werror', '-pedantic',
                   '-ffunction-sections', '-fdata-sections', '-ffp-contract=off',
                   '-fPIC', '-fvisibility=hidden', '-I' + str(ROOT / 'include'),
                   '-I' + str(ROOT / 'src'), '-c', str(source), '-o', str(obj)]
    print('Compiling isolated contour hook', flush=True)
    subprocess.run(compile_cmd, cwd=ROOT, check=True)
    replay_cmd = compile_cmd[:-4] + ['-c', str(ROOT / 'experiments/persian/frame_replay_probe.c'), '-o', str(replay)]
    subprocess.run(replay_cmd, cwd=ROOT, check=True)
    hosts = [p for p in host_objects() if p.name != 'reloq_rules61.o']
    print('Linking isolated contour probe', flush=True)
    subprocess.run(['cc', '-O3', f'-flto={args.jobs}', '-fno-strict-aliasing', '-ffp-contract=off',
                    '-shared', *map(str, hosts), str(obj), str(replay), str(args.archive.resolve()), '-pthread',
                    '-Wl,--gc-sections,--no-undefined', '-lm', '-o', str(output)],
                   cwd=ROOT, check=True)
    print('Ready:', output, flush=True)


if __name__ == '__main__':
    main()
