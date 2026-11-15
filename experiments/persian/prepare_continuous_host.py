"""Prepare a private native host with request-scoped frame capture.

The normal ECI host is not edited. This replaces one host object only when
the experimental Persian continuation library is explicitly built.
"""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def prepare(work: Path) -> Path:
    text = (ROOT / 'src/reloq_rules61.c').read_text(encoding='utf8')
    include = '#include "reloq_rules61.h"\n'
    call = ('        size_t completed=0;\n'
            '        int status=reloq_session_render_frames(s->session,&frame,1,rule_pcm,&p->delivery,&completed);')
    if text.count(include) != 1 or text.count(call) != 1:
        raise RuntimeError('the native frame-render boundary changed')
    text = text.replace(include, include + '#include "continuous_frames.h"\n', 1)
    text = text.replace(call, '''        if(reloq_fa_capture_append(tick,controls,frame.noise.duration_ms,
                (float)(p->settings->volume/100.0*reloq_voice_volume_gain(voice->volume)),
                (int)reloq_german_voice_model_formants(&p->model),
                p->model.head_size,p->model.gender,frame.noise.s_frication,
                frame.noise.limit_frication_extension)) {
            p->result=RELOQ_DE_UTTERANCE_MEMORY;return 1;
        }
        size_t completed=0;
        int status;
        if(reloq_fa_capture_controls_only()) {
            /* The ECI text session uses duration_scale=1. Preserve its exact
               sample and callback clock without rendering discarded audio. */
            long samples=reloq_core_frame_sample_count(frame.noise.duration_ms,1.0f,
                                      reloq_session_sample_rate(s->session)*0.001f);
            status=samples>0 ? RELOQ_SESSION_OK : RELOQ_SESSION_INVALID;
            if(reloq_session_cancelled(s->session))status=RELOQ_SESSION_CANCELLED;
            else if(status==RELOQ_SESSION_OK && reloq_fa_capture_advance_samples((size_t)samples))
                status=RELOQ_SESSION_INVALID;
        } else {
            status=reloq_session_render_frames(s->session,&frame,1,rule_pcm,&p->delivery,&completed);
        }''', 1)
    output = work / 'reloq_rules61_continuous.c'
    output.write_text(text, encoding='utf8')
    return output


def prepare_eci(work: Path) -> Path:
    text = (ROOT / 'src/reloq_eci.c').read_text(encoding='utf8')
    marker = 'int reloq_eci_render_controls(ECIHand handle, const float *controls,'
    if text.count(marker) != 1:
        raise RuntimeError('the native ECI playback implementation changed')
    output = work / 'reloq_eci_continuous.c'
    output.write_text(text + '\n#include "continuous_replay.inc"\n', encoding='utf8')
    return output
