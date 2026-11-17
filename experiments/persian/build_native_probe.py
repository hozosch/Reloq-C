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
from concurrent.futures import ThreadPoolExecutor
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
PROVIDER = ROOT / 'vendor' / 'openevv61'
sys.path.insert(0, str(ROOT / 'experiments' / 'aragonese'))
from build_native_probe import host_objects

Q_HELPER = r'''
#include <stdio.h>
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
    return value && value[0] >= '1' && value[0] <= '7' && !value[1]
        ? value[0] - '0' : 0;
}
/* A marked Finnish /k/ denotes the separately selected Persian uvular
   phone in the disposable mode 7. Ordinary /k/ is never selected. */
static int persian_probe_marked_stop(void *state, int32_t position, unsigned marker)
{
    delta_state *d=state;
    if (!d || !position) return 0;
    delta_node *point=vmovel(d,EVV_AT(delta_node *,position),2);
    if (!point) return 0;
    int32_t node=vgetsc(d,1,0,EVV_REF(point),2);
    if (!node || (*EVV_AT(int32_t *,node)&2)) return 0;
    void *fields=TFLDS(EVV_AT(void *,node));
    return *(const unsigned char *)delta_record_field(&vstmtbl[2],0,fields)==3 &&
           *(const unsigned char *)delta_record_field(&vstmtbl[2],12,fields)==marker;
}
static int16_t persian_probe_lower(int16_t value, int offset, int minimum)
{
    if (value <= 0) return value; /* -1/-2 are native inheritance markers. */
    return value - offset < minimum ? minimum : value - offset;
}
/* Shape a special, non-velar sound from its neighbouring vowel instead of
   leaving the ordinary Finnish K resonance on its acoustic carrier. */
static int persian_probe_neighbour_vowel(void *state,int32_t position,int backwards,
                                        int *f1,int *f2,int *f3)
{
    delta_state *d=state;
    if (!d || !position) return 0;
    void *point=backwards ? (void *)vmovel(d,EVV_AT(delta_node *,position),2) :
                           (void *)vmover(d,EVV_AT(int32_t *,position),2);
    int32_t node=point ? vgetsc(d,backwards,0,EVV_REF(point),2) : 0;
    if (!node || (*EVV_AT(int32_t *,node)&2)) return 0;
    unsigned phone=*(const unsigned char *)delta_record_field(&vstmtbl[2],0,TFLDS(EVV_AT(void *,node)));
    switch (phone) {
    case 31: *f1=680; *f2=1500; *f3=2550; break; /* Finnish A /ae/ */
    case 34: *f1=656; *f2=1000; *f3=2620; break; /* selected back a */
    case 29: *f1=450; *f2=2000; *f3=2700; break;
    case 27: *f1=250; *f2=2170; *f3=2900; break;
    case 33: *f1=440; *f2=840;  *f3=2400; break;
    case 32: *f1=280; *f2=700;  *f3=2150; break;
    default: return 0;
    }
    return (int)phone;
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

FRONT_A_HELPER = r'''
/* Inspect an adjacent Finnish phone without changing the native scanner. */
static unsigned persian_probe_front_a_neighbour(void *state,int32_t position,int backwards)
{
    delta_state *d=state;
    if(!d || !position)return 0;
    void *point=backwards ? (void *)vmovel(d,EVV_AT(delta_node *,position),2) :
                            (void *)vmover(d,EVV_AT(int32_t *,position),2);
    int32_t node=point?vgetsc(d,backwards,0,EVV_REF(point),2):0;
    if(!node || (*EVV_AT(int32_t *,node)&2))return 0;
    return *(const unsigned char *)delta_record_field(&vstmtbl[2],0,TFLDS(EVV_AT(void *,node)));
}
'''

FRONT_A_PATCH = r'''
    /* Match the vowel nucleus in the actual British "sad" reference. Keep
       the Finnish/Persian duration, stress and the neighbouring consonants;
       their native transitions still enter and leave the replacement core. */
    {
        const char *british=getenv("RELOQ_FA_BRITISH_FRONT_A");
        if (british && british[0]=='1' && !british[1]) {
            int16_t female=0;
            memcpy(&female,(const unsigned char *)state+DG_BASE+3092+12,sizeof female);
            STATE(int16_t,s448)=675;  STATE(int16_t,s449)=675;
            STATE(int16_t,s450)=1475; STATE(int16_t,s451)=1475;
            STATE(int16_t,s452)=2350; STATE(int16_t,s453)=2350;
            if (female) {
                /* The female Finnish source's tilt is one unit higher than
                   British at matched voice parameters. Keep the voice's
                   own baseline, but use the British target for this vowel. */
                int16_t tilt=STATE(int16_t,s444);
                STATE(int16_t,s405)=tilt>0 ? tilt-1 : tilt;
                STATE(int16_t,s436)=1;
            } else {
                STATE(int16_t,s405)=0;
                STATE(int16_t,s436)=0;
            }
        } else {
        /* Keep the earlier private alternatives for comparisons with the
           old Finnish-shaped front vowel outside the selected FA scope. */
        const char *setting=getenv("RELOQ_FA_FRONT_A_ACH");
        if(setting && setting[0]=='1' && !setting[1] &&
           persian_probe_front_a_neighbour(state,STATE(int32_t,w60),1)!=18 &&
           persian_probe_front_a_neighbour(state,STATE(int32_t,w61),0)==18) {
            if(STATE(int16_t,s448)>0)STATE(int16_t,s448)+=100;
            if(STATE(int16_t,s449)>0)STATE(int16_t,s449)+=100;
        }
        /* Retain dev121's formant centres and F1/F2. Its third resonance
           is considerably narrower than the US reference; reduce that
           narrowing cautiously without forcing the English vowel identity. */
        const char *f3=getenv("RELOQ_FA_FRONT_A_F3");
        if(f3 && f3[0]=='1' && !f3[1]) {
            int16_t female=0;
            memcpy(&female,(const unsigned char *)state+DG_BASE+3092+12,sizeof female);
            int delta=female ? -150 : 50;
            if(STATE(int16_t,s452)>150)STATE(int16_t,s452)+=delta;
            if(STATE(int16_t,s453)>150)STATE(int16_t,s453)+=delta;
            if(STATE(int16_t,s399)>0)
                STATE(int16_t,s399)=(int16_t)((5*(int)STATE(int16_t,s399)+3)/6);
            if(!female && STATE(int16_t,s401)>260)STATE(int16_t,s401)=260;
        }
        }
    }
'''

A_PATCH = r'''
    /* Preserve coarticulation and the native inheritance markers.  This is
       an approximation of the eSpeak 0_3 nucleus, not an imported spectrum. */
    const char *fa_a_setting = getenv("RELOQ_FA_PROBE_A");
    if (fa_a_setting && fa_a_setting[0]=='1' && !fa_a_setting[1]) {
        if (STATE(int16_t,s448)>0) STATE(int16_t,s448)+=31;
        if (STATE(int16_t,s449)>0) STATE(int16_t,s449)+=31;
        if (STATE(int16_t,s450)>0) STATE(int16_t,s450)-=50;
        if (STATE(int16_t,s451)>0) STATE(int16_t,s451)-=50;
        if (STATE(int16_t,s452)>0) STATE(int16_t,s452)+=20;
        if (STATE(int16_t,s453)>0) STATE(int16_t,s453)+=20;
    }
    else if (fa_a_setting && fa_a_setting[0]=='2' && !fa_a_setting[1]) {
        /* 40% US father /a/, 60% US off /c/: F1/F2/F3 720/1110/2422,
           F4 3600, B1 168. Adjust from Finnish defaults while retaining
           the native coarticulation offsets and inherited-value markers. */
        if (STATE(int16_t,s448)>0) STATE(int16_t,s448)+=95;
        if (STATE(int16_t,s449)>0) STATE(int16_t,s449)+=95;
        if (STATE(int16_t,s450)>0) STATE(int16_t,s450)+=60;
        if (STATE(int16_t,s451)>0) STATE(int16_t,s451)+=60;
        if (STATE(int16_t,s452)>0) STATE(int16_t,s452)-=178;
        if (STATE(int16_t,s453)>0) STATE(int16_t,s453)-=178;
        if (STATE(int16_t,s454)>0) STATE(int16_t,s454)+=100;
        if (STATE(int16_t,s455)>0) STATE(int16_t,s455)+=100;
        if (STATE(int16_t,s397)>0) STATE(int16_t,s397)+=48;
    }
    else if (fa_a_setting && !fa_a_setting[1] &&
             (fa_a_setting[0]=='3' || fa_a_setting[0]=='4')) {
        /* 20/80 father/off and pure off, respectively. Keep Finnish
           contextual offsets and inherited-value markers, as above. */
        int blended=fa_a_setting[0]=='3';
        int f1=blended ? 85 : 75, f2=blended ? 30 : 0;
        int f3=blended ? 184 : 190, b1=blended ? 64 : 80;
        if (STATE(int16_t,s448)>0) STATE(int16_t,s448)+=f1;
        if (STATE(int16_t,s449)>0) STATE(int16_t,s449)+=f1;
        if (STATE(int16_t,s450)>0) STATE(int16_t,s450)+=f2;
        if (STATE(int16_t,s451)>0) STATE(int16_t,s451)+=f2;
        if (STATE(int16_t,s452)>0) STATE(int16_t,s452)-=f3;
        if (STATE(int16_t,s453)>0) STATE(int16_t,s453)-=f3;
        if (STATE(int16_t,s454)>0) STATE(int16_t,s454)+=100;
        if (STATE(int16_t,s455)>0) STATE(int16_t,s455)+=100;
        if (STATE(int16_t,s397)>0) STATE(int16_t,s397)+=b1;
    }
    else if (fa_a_setting && !fa_a_setting[1] &&
             (fa_a_setting[0]=='5' || fa_a_setting[0]=='6')) {
        /* Finnish /a/ mixed with the US off /c/ targets: 50/50 or 25/75.
           Preserve native offsets and inherited-value markers. */
        int balanced=fa_a_setting[0]=='5';
        int f1=balanced ? 38 : 56, f3=balanced ? 95 : 142;
        int f4=balanced ? 50 : 75, b1=balanced ? 40 : 60;
        if (STATE(int16_t,s448)>0) STATE(int16_t,s448)+=f1;
        if (STATE(int16_t,s449)>0) STATE(int16_t,s449)+=f1;
        if (STATE(int16_t,s452)>0) STATE(int16_t,s452)-=f3;
        if (STATE(int16_t,s453)>0) STATE(int16_t,s453)-=f3;
        if (STATE(int16_t,s454)>0) STATE(int16_t,s454)+=f4;
        if (STATE(int16_t,s455)>0) STATE(int16_t,s455)+=f4;
        if (STATE(int16_t,s397)>0) STATE(int16_t,s397)+=b1;
    }
    else if (fa_a_setting && fa_a_setting[0]=='7' && !fa_a_setting[1]) {
        /* 0_3 middle-frame peaks 656/1000/2620/3040 Hz. Unlike the
           earlier A=1 probe, also try the fourth peak. These are targets
           for a different synthesizer, not a copied eSpeak spectrum. */
        if (STATE(int16_t,s448)>0) STATE(int16_t,s448)+=31;
        if (STATE(int16_t,s449)>0) STATE(int16_t,s449)+=31;
        if (STATE(int16_t,s450)>0) STATE(int16_t,s450)-=50;
        if (STATE(int16_t,s451)>0) STATE(int16_t,s451)-=50;
        if (STATE(int16_t,s452)>0) STATE(int16_t,s452)+=20;
        if (STATE(int16_t,s453)>0) STATE(int16_t,s453)+=20;
        if (STATE(int16_t,s454)>0) STATE(int16_t,s454)-=460;
        if (STATE(int16_t,s455)>0) STATE(int16_t,s455)-=460;
    }
    /* Optional listening experiment: move the centre of F1/F2/F3 towards
       a measured reference while preserving the native endpoint difference
       and negative inheritance markers. The values are supplied only after
       tracing the accepted Salam example; this does not assume that eSpeak
       and this Klatt engine have interchangeable spectra. */
    {
        const char *anchor=getenv("RELOQ_FA_PROBE_A_TARGET");
        int targets[3];
        if (fa_a_setting && fa_a_setting[0]=='1' && !fa_a_setting[1] && anchor &&
            strcmp(anchor,"salam-fixed")==0) {
            /* Hold the accepted Salam nucleus itself rather than merely its
               average. Adjacent phones retain their own native trajectories. */
            STATE(int16_t,s448)=656;  STATE(int16_t,s449)=656;
            STATE(int16_t,s450)=1000; STATE(int16_t,s451)=1000;
            STATE(int16_t,s452)=2620; STATE(int16_t,s453)=2620;
            STATE(int16_t,s454)=3500; STATE(int16_t,s455)=3500;
            STATE(int16_t,s456)=3900; STATE(int16_t,s457)=-1;
            STATE(int16_t,s397)=120;  STATE(int16_t,s398)=100;
            STATE(int16_t,s399)=150;  STATE(int16_t,s400)=330;
            STATE(int16_t,s401)=260;
            const char *nasal=getenv("RELOQ_FA_PROBE_A_NASAL");
            if (nasal && !nasal[1] && (nasal[0]=='1' || nasal[0]=='2')) {
                /* Keep the confirmed A resonances and adjust only the
                   nasal pole/zero separation. Two mild listening options. */
                STATE(int16_t,s406)=200;
                STATE(int16_t,s407)=nasal[0]=='1' ? 225 : 250;
            }
        } else if (fa_a_setting && fa_a_setting[0]=='1' && !fa_a_setting[1] && anchor &&
            sscanf(anchor,"%d,%d,%d",&targets[0],&targets[1],&targets[2])==3 &&
            targets[0]>=200 && targets[0]<=1200 &&
            targets[1]>=500 && targets[1]<=2500 &&
            targets[2]>=1500 && targets[2]<=4000) {
            int starts[3]={STATE(int16_t,s448),STATE(int16_t,s450),STATE(int16_t,s452)};
            int ends[3]={STATE(int16_t,s449),STATE(int16_t,s451),STATE(int16_t,s453)};
            int delta[3]={0,0,0};
            for (int i=0;i<3;i++) {
                int centre=starts[i]>0 && ends[i]>0 ? (starts[i]+ends[i]+1)/2 :
                           starts[i]>0 ? starts[i] : ends[i];
                if (centre>0) delta[i]=targets[i]-centre;
            }
            if (starts[0]>0) STATE(int16_t,s448)=starts[0]+delta[0];
            if (ends[0]>0) STATE(int16_t,s449)=ends[0]+delta[0];
            if (starts[1]>0) STATE(int16_t,s450)=starts[1]+delta[1];
            if (ends[1]>0) STATE(int16_t,s451)=ends[1]+delta[1];
            if (starts[2]>0) STATE(int16_t,s452)=starts[2]+delta[2];
            if (ends[2]>0) STATE(int16_t,s453)=ends[2]+delta[2];
        }
        if (getenv("RELOQ_FA_TRACE_A"))
            fprintf(stderr,"FA_A F=%d,%d,%d,%d,%d R=%d,%d,%d,%d,%d B=%d,%d,%d,%d,%d\n",
                STATE(int16_t,s448),STATE(int16_t,s450),STATE(int16_t,s452),
                STATE(int16_t,s454),STATE(int16_t,s456),
                STATE(int16_t,s449),STATE(int16_t,s451),STATE(int16_t,s453),
                STATE(int16_t,s455),STATE(int16_t,s457),
                STATE(int16_t,s397),STATE(int16_t,s398),STATE(int16_t,s399),
                STATE(int16_t,s400),STATE(int16_t,s401));
    }
'''

L_PATCH = r'''
    /* Test German's brighter /l/ targets in the Finnish trajectory. Keep
       contextual offsets and negative inherited-value sentinels intact. */
    const char *fa_l_setting=getenv("RELOQ_FA_PROBE_L");
    if (fa_l_setting && fa_l_setting[0]=='1' && !fa_l_setting[1]) {
        if (STATE(int16_t,s450)>0) STATE(int16_t,s450)+=400;
        if (STATE(int16_t,s451)>0) STATE(int16_t,s451)+=400;
        if (STATE(int16_t,s452)>0) STATE(int16_t,s452)+=400;
        if (STATE(int16_t,s453)>0) STATE(int16_t,s453)+=400;
        if (STATE(int16_t,s454)>0) STATE(int16_t,s454)+=450;
        if (STATE(int16_t,s455)>0) STATE(int16_t,s455)+=450;
    }
'''

GERMAN_L_TRACE = r'''
    if (getenv("RELOQ_FA_TRACE_L")) {
        fprintf(stderr, "DE_L F=%d,%d,%d,%d,%d R=%d,%d,%d,%d,%d "
                "B=%d,%d,%d,%d,%d AV=%d A=%d\n",
            STATE(int16_t,s449),STATE(int16_t,s451),STATE(int16_t,s453),
            STATE(int16_t,s455),STATE(int16_t,s457),
            STATE(int16_t,s450),STATE(int16_t,s452),STATE(int16_t,s454),
            STATE(int16_t,s456),STATE(int16_t,s458),
            STATE(int16_t,s398),STATE(int16_t,s399),STATE(int16_t,s400),
            STATE(int16_t,s401),STATE(int16_t,s402),
            STATE(int16_t,s403),STATE(int16_t,s422));
    }
'''

FINNISH_L_TRACE = r'''
    if (getenv("RELOQ_FA_TRACE_L")) {
        fprintf(stderr, "FA_L F=%d,%d,%d,%d,%d R=%d,%d,%d,%d,%d "
                "B=%d,%d,%d,%d,%d AV=%d A=%d\n",
            STATE(int16_t,s448),STATE(int16_t,s450),STATE(int16_t,s452),
            STATE(int16_t,s454),STATE(int16_t,s456),
            STATE(int16_t,s449),STATE(int16_t,s451),STATE(int16_t,s453),
            STATE(int16_t,s455),STATE(int16_t,s457),
            STATE(int16_t,s397),STATE(int16_t,s398),STATE(int16_t,s399),
            STATE(int16_t,s400),STATE(int16_t,s401),
            STATE(int16_t,s402),STATE(int16_t,s421));
    }
'''

FINNISH_L_CONTEXT = r'''
/* The Finnish phone following this L, without moving the rule's scan. */
static unsigned persian_probe_l_next_phone(void *state)
{
    delta_state *d=state;
    int32_t at=STATE(int32_t,w63);
    if (!at) return 0;
    int32_t *point=vmover(d,EVV_AT(int32_t *,at),2);
    if (!point) return 0;
    int32_t node=vgetsc(d,0,0,EVV_REF(point),2);
    if (!node || (*EVV_AT(int32_t *,node)&2)) return 0;
    return *(const unsigned char *)delta_record_field(
        &vstmtbl[2],0,TFLDS(EVV_AT(void *,node)));
}
'''


FINNISH_L_EDGE_TRACE = r'''
/* Diagnostic only: observe the scanner and word fence, leaving all state as
   found. The actual combined German/French L rule is not selected here. */
static void persian_probe_trace_l_edges(void *state,int32_t ltok,int32_t rtok)
{
    if (!getenv("RELOQ_FA_TRACE_L_EDGES")) return;
    delta_state *d=state;
    if (!d || !d->vars || d->nstmts<5 || d->nstmts>256 || !ltok || !rtok) return;
    delta_vars *v=EVV_AT(delta_vars *,d->vars);
    delta_tpos left=d->lpta, right=d->rpta;
    unsigned char scan[8],index[256],marks[256],chars[256];
    int8_t compared=v->compared_equal, fences=v->fence_count, testing=v->testing;
    memcpy(scan,&v->scan_ptr,sizeof scan);
    memcpy(index,EVV_AT(const void *,d->fence_index),d->nstmts);
    memcpy(marks,EVV_AT(const void *,d->fence_marks),d->nstmts);
    memcpy(chars,EVV_AT(const void *,d->fence_chars),d->nstmts);
    const uint8_t word_fence[]={4};
    int32_t positions[]={ltok,rtok};
    for (int p=0;p<2;p++) for (int rev=0;rev<2;rev++) {
        d->lpta=left; d->rpta=right; memcpy(&v->scan_ptr,scan,sizeof scan);
        v->compared_equal=compared; v->fence_count=fences; v->testing=0;
        memcpy(EVV_AT(void *,d->fence_index),index,d->nstmts);
        memcpy(EVV_AT(void *,d->fence_marks),marks,d->nstmts);
        memcpy(EVV_AT(void *,d->fence_chars),chars,d->nstmts);
        fence(d,1,word_fence);
        memset(EVV_AT(void *,d->fence_marks),0,d->nstmts);
        lpta_loadp(d,EVV_AT(const delta_token *,positions[p]));
        int rc=rev ? setscan_r(d,2) : setscan_l(d,2);
        int is_l=-1,step1=-1,step2=-1;
        if (!rc) {
            is_l=!testFldeq(d,2,0,21);
            step1=advance_tok(d);
            if (!step1) step2=advance_tok(d);
        }
        fprintf(stderr,"FA_EDGE p=%d dir=%d rc=%d L=%d step1=%d step2=%d\n",
                p,rev,rc,is_l,step1,step2);
    }
    d->lpta=left; d->rpta=right; memcpy(&v->scan_ptr,scan,sizeof scan);
    v->compared_equal=compared; v->fence_count=fences; v->testing=testing;
    memcpy(EVV_AT(void *,d->fence_index),index,d->nstmts);
    memcpy(EVV_AT(void *,d->fence_marks),marks,d->nstmts);
    memcpy(EVV_AT(void *,d->fence_chars),chars,d->nstmts);
}
'''


FINNISH_L_WORD_FINAL = r'''
/* Determine whether this Finnish phone is the last phone within its word.
   The native scanner treats field 4 as a word fence. Save its registers and
   fence state so the subsequent real L rule sees exactly its normal state. */
static int persian_probe_l_word_final(void *state,int32_t token_ref)
{
    delta_state *d=state;
    if (!d || !d->vars || d->nstmts<5 || d->nstmts>256 || !token_ref) return 0;
    delta_vars *v=EVV_AT(delta_vars *,d->vars);
    delta_tpos left=d->lpta,right=d->rpta;
    unsigned char scan[8],index[256],marks[256],chars[256];
    int8_t compared=v->compared_equal,fences=v->fence_count,testing=v->testing;
    memcpy(scan,&v->scan_ptr,sizeof scan);
    memcpy(index,EVV_AT(const void *,d->fence_index),d->nstmts);
    memcpy(marks,EVV_AT(const void *,d->fence_marks),d->nstmts);
    memcpy(chars,EVV_AT(const void *,d->fence_chars),d->nstmts);
    v->testing=0;
    const uint8_t word_fence[]={4};
    fence(d,1,word_fence);
    memset(EVV_AT(void *,d->fence_marks),0,d->nstmts);
    lpta_loadp(d,EVV_AT(const delta_token *,token_ref));
    int final=!setscan_r(d,2) && !testFldeq(d,2,0,21) &&
              !advance_tok(d) && advance_tok(d);
    d->lpta=left; d->rpta=right; memcpy(&v->scan_ptr,scan,sizeof scan);
    v->compared_equal=compared; v->fence_count=fences; v->testing=testing;
    memcpy(EVV_AT(void *,d->fence_index),index,d->nstmts);
    memcpy(EVV_AT(void *,d->fence_marks),marks,d->nstmts);
    memcpy(EVV_AT(void *,d->fence_chars),chars,d->nstmts);
    return final;
}
'''

FRENCH_L_ADAPTER = r'''
/* Translate the French phone record's feature tests to Finnish records.
   Phones unavailable in this private probe simply cannot match. */
static int32_t persian_probe_fr_field(void *state, uint8_t st, uint8_t field, uint8_t value)
{
    if (st!=2) return reloq61_typed_testFldeq(DELTA_ENTRY_testFldeq,state,st,field,value);
    static const uint8_t names[]={
        0,4,1,5,2,3,6,255,255,8,7,10,9,12,13,14,11,15,16,17,
        255,255,255,19,21,22,23,255,34,34,29,31,27,24,28,25,33,
        33,32,26,30,30,255,255,255,255,255,18,36
    };
    uint8_t tofield=field, target=value;
    if (field==0) {
        if (value>=sizeof(names) || names[value]==255) return 1;
        /* Finnish G represents /x/ only when the private x probe is active. */
        if (value==47) {
            const char *x=getenv("RELOQ_FA_PROBE_X");
            if (!x || x[0]!='1' || x[1]) return 1;
        }
        target=names[value];
    } else if (field==4) {
        static const uint8_t manner[]={1,2,255,3,4,5,6,7,0};
        if (value>=sizeof(manner) || manner[value]==255) return 1;
        target=manner[value];
    } else if (field==5) {
        static const uint8_t place[]={1,2,1,2,3,4,255,0};
        if (value>=sizeof(place) || place[value]==255) return 1;
        target=place[value];
    } else if (field==6) {
        /* Finnish ä is the Persian short front vowel, corresponding here
           to French /E/ rather than French low /a/. */
        int front_a=reloq61_typed_testFldeq(DELTA_ENTRY_testFldeq,state,2,0,31)==0;
        if (front_a && value==1) return 0;
        if (front_a && value==2) return 1;
        static const uint8_t height[]={5,6,7,0};
        if (value>=sizeof(height)) return 1;
        tofield=5; target=height[value];
    } else if (field==7) {
        /* The selected back A maps to French /a/ for L coarticulation. */
        int back_a=reloq61_typed_testFldeq(DELTA_ENTRY_testFldeq,state,2,0,34)==0;
        if (back_a && value==0) return 0;
        if (back_a && (value==1 || value==2)) return 1;
        static const uint8_t back[]={1,255,2,0};
        if (value>=sizeof(back) || back[value]==255) return 1;
        tofield=6; target=back[value];
    } else if (field==8) {
        if (value>1) return 1;
        tofield=11; target=value; /* French tense/lax ~ Finnish long/short */
    } else if (field==9) {
        if (value==0 || value>2) return 1;
        tofield=10; target=value-1;
    } else if (field==10) {
        /* The mapped Persian vowel inventory here has no nasal vowels. */
        if (value==1) return 1;
        if (value==2) return reloq61_typed_testFldeq(DELTA_ENTRY_testFldeq,state,2,1,1);
        return 1;
    } else if (field==12) {
        tofield=8;
    } else if (field==13) {
        if (value==1) { tofield=9; target=3; }
        else return 1;
    } else if (field>10) {
        return 1;
    }
    return reloq61_typed_testFldeq(DELTA_ENTRY_testFldeq,state,st,tofield,target);
}
'''


def phone_symbol_data(language: str) -> tuple[list[str], list[int]]:
    """Resolve this build's symbols to their first byte and possible phone name."""
    root = PROVIDER / 'lang' / language
    link = (root / f'delta_link_{language}.c').read_text(encoding='utf8')
    names = dict(re.findall(r'static const char (s\d+)\[\] = "([^"]*)";', link))
    table = re.search(r'static const char \*const v2_0\[\] = \{([^}]+)', link)
    if not table:
        raise RuntimeError('Cannot find phone table for ' + language)
    phones = [names[key] for key in re.findall(r's\d+', table.group(1))]
    authored = (root / f'delta_authored_{language}.c').read_text(encoding='utf8')
    authored = authored.split(f'uint8_t {language}_evv_authored_data[] = {{', 1)[1].split('};', 1)[0]
    authored = re.sub(r'/\*.*?\*/', '', authored, flags=re.S)
    data = bytes(int(value, 0) for value in
                 re.findall(r'(?<![A-Za-z_])(?:0[xX][0-9a-fA-F]+|[0-9]+)', authored))
    table = (root / f'delta_rules_{language}.c').read_text(encoding='utf8')
    table = table.split(f'{language}_delta_rule_sym[] = {{', 1)[1].split('};', 1)[0]
    offsets = [int(value) for value in
               re.findall(rf'{language}_evv_authored_data \+ (\d+)', table)]
    values = [data[offset] for offset in offsets]
    return [phones[value] if value < len(phones) else '' for value in values], values


def prepare_german_l(work: Path) -> list[Path]:
    """Adapt the German L rule's real control flow to Finnish phone fields.

    This is only a disposable listening experiment. The rule's tests name
    Finnish phone symbols and remap German vowel features; Finnish scheduling
    and defaults still differ, so this is not a bit-identical German voice.
    """
    destination = prepare('fifi', 'delta_rules_c02_fifi.c', 'fin_ph_l',
                          'L24070', L_PATCH, '', work)
    text = destination.read_text(encoding='utf8')
    german = (PROVIDER / 'lang' / 'dede61' / 'delta_rules_c05_dede61.c').read_text(encoding='utf8')
    begin = german.index('int32_t reloq_dede61_ger_ph_l(')
    end = german.index('\nstatic int32_t evv_ger_ph_l(', begin)
    rule = german[begin:end].replace('int32_t reloq_dede61_ger_ph_l(',
                                     'static int32_t persian_probe_german_l(', 1)
    # The German field names index a different state layout. These mappings
    # are derived from both languages' insert_acoustic_vals calls. Symbol
    # IDs refer to phone names, except German O (/ɔ/) maps to Finnish o.
    field_map = {403: 402, 422: 421, 449: 448, 451: 450, 453: 452, 455: 454, 457: 456}
    symbol_map = {1553: 724, 1557: 726, 1573: 743, 1576: 747, 1578: 754,
                  1579: 748, 1581: 750, 1582: 747, 1584: 746, 1587: 742}
    german_names, german_bytes = phone_symbol_data('dede61')
    finnish_names, finnish_bytes = phone_symbol_data('fifi')
    for source, target in symbol_map.items():
        if source == 1553:  # the two null fences, not phone symbols
            if german_bytes[source] != 0 or finnish_bytes[target] != 0:
                raise RuntimeError('German L null fence changed')
            continue
        if source == 1557:  # test_synch's word-boundary type, not phone d
            if german_bytes[source] != 4 or finnish_bytes[target] != 4:
                raise RuntimeError('German L word-boundary type changed')
            continue
        name = german_names[source]
        expected = 'o' if name == 'O' else name
        if not name or finnish_names[target] != expected:
            raise RuntimeError(f'German L phone mapping changed: {source} -> {target}')
    if finnish_bytes[752] != 34 or finnish_names[752] != 'a':
        raise RuntimeError('Finnish a phone mapping changed')
    if finnish_bytes[766] != 31 or finnish_names[766] != 'A':
        raise RuntimeError('Finnish ä phone mapping changed')
    used_fields = {int(x) for x in re.findall(r'STATE\(int16_t, s(\d+)\)', rule)}
    used_symbols = {int(x) for x in re.findall(r'delta_sym_ref\[(\d+)\]', rule)}
    if used_fields != set(field_map) or used_symbols != set(symbol_map):
        raise RuntimeError('German L state or symbol layout changed; review mappings')
    rule = re.sub(r'STATE\(int16_t, s(\d+)\)',
                  lambda m: f'STATE(int16_t, s{field_map[int(m.group(1))]})', rule)
    rule = re.sub(r'delta_sym_ref\[(\d+)\]',
                  lambda m: f'delta_sym_ref[{symbol_map[int(m.group(1))]}]', rule)
    # German phone field 7/back=3 -> Finnish field 6/back=2;
    # German field 6/low=3 -> Finnish field 5/low=7.
    back = 'L1941:;\n    reloq_local_220.i32 = (2);\n    reloq_local_216.i32 = (7);\n    reloq_local_212.i32 = (3);'
    low = 'L2064:;\n    reloq_local_220.i32 = (2);\n    reloq_local_216.i32 = (6);\n    reloq_local_212.i32 = (3);'
    if rule.count(back) != 1 or rule.count(low) != 1:
        raise RuntimeError('Cannot locate the German L vowel feature tests')
    rule = rule.replace(back, back.replace('(7);', '(6);').replace('(3);', '(2);'), 1)
    rule = rule.replace(low, low.replace('(6);', '(5);').replace('(3);', '(7);'), 1)
    # Finnish /a/ is marked back whereas the analogous German /a/ selects
    # the low-vowel branch. Finnish /A/ (ä) is low, but the analogous German
    # /E/ selects the default/front branch. Match the sound, not the name of
    # the two languages' independently defined phonetic feature values.
    back_test = '    if (reloq_local_208.i32 == 0) goto L2014;'
    low_test = '    if (reloq_local_208.i32 == 0) goto L2137;'
    if rule.count(back_test) != 1 or rule.count(low_test) != 1:
        raise RuntimeError('Cannot locate the German L feature-test branches')
    rule = rule.replace(back_test,
                        '    if (persian_probe_l_next_phone(state)==34) reloq_local_208.i32=1;\n' + back_test, 1)
    rule = rule.replace(low_test,
                        '    if (persian_probe_l_next_phone(state)==31) reloq_local_208.i32=1;\n' + low_test, 1)
    label = 'L2680:;\n    r0 = reloq61_typed_succeed'
    if rule.count(label) != 1:
        raise RuntimeError('Cannot locate German L exit')
    rule = rule.replace(label, 'L2680:;\n' + FINNISH_L_TRACE +
                        '    r0 = reloq61_typed_succeed', 1)
    marker = 'int32_t reloq_fifi_fin_ph_l(void *state, int32_t p0)\n{\n'
    if text.count(marker) != 1:
        raise RuntimeError('Cannot locate Finnish L rule')
    redirect = r'''    persian_probe_trace_l_edges(state,STATE_AT(w60,-4),STATE_AT(w61,-4));
    const char *fa_l=getenv("RELOQ_FA_PROBE_L");
    if (fa_l && fa_l[0]=='2' && !fa_l[1])
        return persian_probe_german_l(state,p0);
'''
    text = text.replace('#include <stdlib.h>\n', '#include <stdlib.h>\n#include <stdio.h>\n', 1)
    text = text.replace(marker, FINNISH_L_CONTEXT + '\n' + FINNISH_L_EDGE_TRACE + '\n' + FINNISH_L_WORD_FINAL + '\n' + rule + '\n' + marker + redirect, 1)
    destination.write_text(text, encoding='utf8')
    native = prepare('dede61', 'delta_rules_c05_dede61.c', 'ger_ph_l',
                     'L2680', GERMAN_L_TRACE, '#include <stdio.h>\n', work)
    return [destination, native]


def prepare_french_l(work: Path) -> list[Path]:
    """Adapt the French lateral and its transition rule to Finnish phones."""
    destination = work / 'delta_rules_c02_fifi.c'
    text = destination.read_text(encoding='utf8')

    def extract(filename: str, name: str, replacement: str) -> str:
        src = (PROVIDER / 'lang' / 'frfr61' / filename).read_text(encoding='utf8')
        start = src.index('int32_t reloq_frfr61_' + name + '(')
        end = src.index('\nstatic int32_t evv_' + name + '(', start)
        return src[start:end].replace('int32_t reloq_frfr61_' + name + '(',
                                      'static int32_t ' + replacement + '(', 1)

    french_names, french_bytes = phone_symbol_data('frfr61')
    finnish_names, finnish_bytes = phone_symbol_data('fifi')
    absent = next((i for i, value in enumerate(finnish_bytes) if value > 36), None)
    if absent is None:
        raise RuntimeError('No impossible Finnish phone symbol found')
    # 2124 is a word fence, not the French /t/ phone; 2138 is /ɥ/,
    # unavailable in Finnish; /x/ maps to the private Persian G slot.
    symbols = {2065: 724, 2121: 724, 2122: 725, 2124: 726, 2134: 741,
               2135: 740, 2138: absent, 2143: 745, 2145: 739, 2160: 728}
    for source, target in symbols.items():
        if source in (2065, 2121):
            if french_bytes[source] != 0 or finnish_bytes[target] != 0:
                raise RuntimeError('French L null fence changed')
        elif source == 2124:
            if french_bytes[source] != 4 or finnish_bytes[target] != 4:
                raise RuntimeError('French L word fence changed')
        elif source == 2138:
            if french_names[source] != 'H' or finnish_bytes[target] <= 36:
                raise RuntimeError('French /ɥ/ fallback changed')
        else:
            if source == 2145 and french_names[source] != 'x':
                raise RuntimeError('French /x/ phone mapping changed')
            name = 'G' if source == 2145 else french_names[source]
            if finnish_names[target] != name:
                raise RuntimeError(f'French L phone mapping changed: {source} -> {target}')
    if (finnish_names[752], finnish_bytes[752], finnish_names[766], finnish_bytes[766]) != ('a', 34, 'A', 31):
        raise RuntimeError('Finnish A/ä phone mapping changed')

    average = extract('delta_rules_c17_frfr61.c', 'average', 'persian_probe_fr_average')
    subval = extract('delta_rules_c04_frfr61.c', 'subval', 'persian_probe_fr_subval')
    lateral = extract('delta_rules_c30_frfr61.c', 'fren_lat_Fv', 'persian_probe_fr_lateral')
    rule = extract('delta_rules_c08_frfr61.c', 'fren_ph_l', 'persian_probe_fr_l')
    # French special /r/ voice-setting logic depends on globals absent from
    # Finnish. Keep its immediate L target but omit that language-specific
    # postprocessing rather than reading unrelated Finnish state cells.
    fr_only = ('    r1 = ((int32_t)STATE(uint16_t, s326));\n'
               '    AT(int32_t, -28) = (r1);\n'
               '    if (AT(int16_t, -28) == (int16_t)(STATE(int16_t, s319))) goto L3601;\n'
               '    goto L4217;')
    if rule.count(fr_only) != 1:
        raise RuntimeError('Cannot locate French-specific R voice branch')
    rule = rule.replace(fr_only, '    goto L4217;', 1)
    blocks = [average, subval, lateral, rule]
    used = {int(x) for block in blocks for x in re.findall(r'delta_sym_ref\[(\d+)\]', block)}
    if used != set(symbols):
        raise RuntimeError('French L symbol layout changed; review mappings')
    mapped = []
    for block in blocks:
        block = re.sub(r'delta_sym_ref\[(\d+)\]',
                       lambda m: f'delta_sym_ref[{symbols[int(m.group(1))]}]', block)
        block = re.sub(r'STATE\((u?int16_t), s(\d+)\)',
                       lambda m: f'STATE({m.group(1)}, s{int(m.group(2))-1})', block)
        block = re.sub(r'STATE_AT\(s(\d+),',
                       lambda m: f'STATE_AT(s{int(m.group(1))-1},', block)
        block = block.replace('reloq_call_frfr61_average(', 'persian_probe_fr_average(')
        block = block.replace('reloq_call_frfr61_subval(', 'persian_probe_fr_subval(')
        block = block.replace('reloq_call_frfr61_fren_lat_Fv(', 'persian_probe_fr_lateral(')
        block = block.replace('reloq61_typed_testFldeq(DELTA_ENTRY_testFldeq, state,',
                              'persian_probe_fr_field(state,')
        mapped.append(block)
    label = 'L4217:;\n    r0 = reloq61_typed_succeed'
    if mapped[-1].count(label) != 1:
        raise RuntimeError('Cannot locate French L exit')
    mapped[-1] = mapped[-1].replace(label, 'L4217:;\n' + FINNISH_L_TRACE +
                                    '    r0 = reloq61_typed_succeed', 1)
    marker = 'int32_t reloq_fifi_fin_ph_l(void *state, int32_t p0)\n{\n'
    if text.count(marker) != 1:
        raise RuntimeError('Cannot locate Finnish L rule')
    text = text.replace(marker, FRENCH_L_ADAPTER + '\n' + '\n'.join(mapped) + '\n' + marker, 1)
    old = '    if (fa_l && fa_l[0]==\'2\' && !fa_l[1])\n        return persian_probe_german_l(state,p0);\n'
    if text.count(old) != 1:
        raise RuntimeError('Cannot locate the German L probe redirect')
    text = text.replace(old, old + '    if (fa_l && fa_l[0]==\'3\' && !fa_l[1])\n'
                        '        return persian_probe_fr_l(state,p0);\n'
                        '    if (fa_l && fa_l[0]==\'4\' && !fa_l[1])\n'
                        '        return persian_probe_l_word_final(state,STATE_AT(w60,-4)) ?\n'
                        '            persian_probe_fr_l(state,p0) : persian_probe_german_l(state,p0);\n', 1)
    destination.write_text(text, encoding='utf8')
    native = prepare('frfr61', 'delta_rules_c08_frfr61.c', 'fren_ph_l', 'L4217',
                     GERMAN_L_TRACE.replace('DE_L', 'FR_L'), '#include <stdio.h>\n', work)
    return [native]

Q_PATCH = r'''
    {
        int variant = persian_probe_setting("RELOQ_FA_PROBE_Q");
        int selected=variant==6 ||
            (variant==7 ? persian_probe_marked_stop(state,STATE(int32_t,w61),3) :
                          persian_probe_source_q(state,STATE(int32_t,w61)));
        if (variant && selected) {
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
                /* Finnish treats adjacent q+k as a geminate /k/ and can
                   clear the first release's AF. The private uvular probe
                   still needs its own frication before the second stop. */
                if (STATE(int16_t,s430)<=0) STATE(int16_t,s430)=60;
                if (variant>=5 && STATE(int16_t,s430)>3)
                    STATE(int16_t,s430)-=3;
            }
        }
        const char *glottal=getenv("RELOQ_FA_PROBE_GLOTTAL");
        if (glottal && (glottal[0]=='1' || glottal[0]=='2') && !glottal[1] &&
            (persian_probe_marked_stop(state,STATE(int32_t,w61),4) ||
             persian_probe_marked_stop(state,STATE(int32_t,w61),5))) {
            /* A separate closure with no Finnish K burst or H aspiration.
               The deliberately quiet release is an initial listening
               approximation, not yet a validated Persian glottal stop. */
            STATE(int16_t,s430)=0;
            STATE(int16_t,s404)=0;
            STATE(int16_t,s437)=0; STATE(int16_t,s438)=0;
            STATE(int16_t,s439)=0; STATE(int16_t,s440)=0;
            STATE(int16_t,s441)=0; STATE(int16_t,s442)=0;
            const char *end_h=getenv("RELOQ_FA_PROBE_END_H");
            int glot_tract=glottal[0]=='2' && persian_probe_marked_stop(state,STATE(int32_t,w61),4);
            int h_tract=end_h && end_h[0]=='1' && !end_h[1] &&
                        persian_probe_marked_stop(state,STATE(int32_t,w61),5);
            int f1,f2,f3;
            if ((glot_tract || h_tract) &&
                persian_probe_neighbour_vowel(state,h_tract ? STATE(int32_t,w60) : STATE(int32_t,w61),
                                               h_tract,&f1,&f2,&f3)) {
                STATE(int16_t,s448)=f1; STATE(int16_t,s449)=f1;
                STATE(int16_t,s450)=f2; STATE(int16_t,s451)=f2;
                STATE(int16_t,s452)=f3; STATE(int16_t,s453)=f3;
            }
        }
        const char *k_shape=getenv("RELOQ_FA_PROBE_K_SHAPE");
        if (k_shape && !k_shape[1] && k_shape[0]>='1' && k_shape[0]<='5' && !selected &&
            !persian_probe_marked_stop(state,STATE(int32_t,w61),4) &&
            !persian_probe_marked_stop(state,STATE(int32_t,w61),5)) {
            int f1,f2,f3;
            if (persian_probe_neighbour_vowel(state,STATE(int32_t,w61),0,&f1,&f2,&f3)==34) {
                if (k_shape[0]<='2') {
                    /* Previous German-band and midpoint comparison. */
                    STATE(int16_t,s440)=k_shape[0]=='1' ? 0 : 20;
                } else {
                    /* User likes the Finnish K before front /ae/. Copy that
                       consonant's measured noise bands before back /a/;
                       keep the selected 60% H and the vowel's own targets. */
                    STATE(int16_t,s437)=0;  STATE(int16_t,s438)=65;
                    STATE(int16_t,s439)=50; STATE(int16_t,s440)=0;
                    STATE(int16_t,s441)=0;  STATE(int16_t,s442)=0;
                    if (k_shape[0]>='4') {
                        int full=k_shape[0]=='5';
                        /* Try the /ae/ K release-side resonances either
                           halfway or in full. Preserve native sentinels. */
                        int targets[3]={300,2200,2600};
                        int16_t *actual[3]={&STATE(int16_t,s449),&STATE(int16_t,s451),
                                            &STATE(int16_t,s453)};
                        for (int i=0;i<3;i++) if (*actual[i]>0)
                            *actual[i]=full ? targets[i] : (*actual[i]+targets[i]+1)/2;
                    }
                }
            }
        }
        const char *k_noise=getenv("RELOQ_FA_PROBE_K_NOISE");
        if (k_noise && *k_noise && !selected &&
            !persian_probe_marked_stop(state,STATE(int32_t,w61),4) &&
            !persian_probe_marked_stop(state,STATE(int32_t,w61),5)) {
            int values[7],end=0,valid=1;
            if (sscanf(k_noise,"%d,%d,%d,%d,%d,%d,%d%n",values,values+1,values+2,
                       values+3,values+4,values+5,values+6,&end)==7 && !k_noise[end]) {
                for (int i=0;i<7;i++) if (values[i]<0 || values[i]>90) valid=0;
                if (valid) {
                    STATE(int16_t,s430)=values[0]; STATE(int16_t,s437)=values[1];
                    STATE(int16_t,s438)=values[2]; STATE(int16_t,s439)=values[3];
                    STATE(int16_t,s440)=values[4]; STATE(int16_t,s441)=values[5];
                    STATE(int16_t,s442)=values[6];
                }
            }
        }
        if (getenv("RELOQ_FA_TRACE_K"))
            fprintf(stderr,"FA_K selected=%d F=%d,%d,%d R=%d,%d,%d AF=%d BANDS=%d,%d,%d,%d,%d,%d AH=%d\n",
                variant && selected,STATE(int16_t,s448),STATE(int16_t,s450),STATE(int16_t,s452),
                STATE(int16_t,s449),STATE(int16_t,s451),STATE(int16_t,s453),STATE(int16_t,s430),
                STATE(int16_t,s437),STATE(int16_t,s438),STATE(int16_t,s439),
                STATE(int16_t,s440),STATE(int16_t,s441),STATE(int16_t,s442),STATE(int16_t,s420));
    }
'''

GERMAN_K_TRACE = r'''
    if (getenv("RELOQ_FA_TRACE_K"))
        fprintf(stderr,"DE_K F=%d,%d,%d R=%d,%d,%d AF=%d BANDS=%d,%d,%d,%d,%d,%d AH=%d\n",
            STATE(int16_t,s449),STATE(int16_t,s451),STATE(int16_t,s453),
            STATE(int16_t,s450),STATE(int16_t,s452),STATE(int16_t,s454),STATE(int16_t,s431),
            STATE(int16_t,s438),STATE(int16_t,s439),STATE(int16_t,s440),
            STATE(int16_t,s441),STATE(int16_t,s442),STATE(int16_t,s443),STATE(int16_t,s421));
'''

X_HELPER = r'''
/* Private only: a G phoneme is a temporary test slot for Persian /x/.
   In ordinary Finnish G remains its original nasal phoneme. */
_Thread_local int persian_probe_x_active;
'''

X_REDIRECT = r'''
    const char *fa_x = getenv("RELOQ_FA_PROBE_X");
    if (fa_x && fa_x[0]=='1' && !fa_x[1]) {
        ++persian_probe_x_active;
        int32_t result = reloq_fifi_fin_ph_S(state,p0);
        --persian_probe_x_active;
        return result;
    }
'''

X_FRICTION = r'''
    if (persian_probe_x_active) {
        /* German /x/ uses AF=55 with band levels 70/0/55/0/0. The
           corresponding Finnish AF and band cells are s404 and s409..413. */
        STATE(int16_t,s404)=55;
        STATE(int16_t,s409)=70; STATE(int16_t,s410)=0;
        STATE(int16_t,s411)=55; STATE(int16_t,s412)=0;
        STATE(int16_t,s413)=0;
        STATE(int16_t,s448)=300; STATE(int16_t,s449)=300;
        STATE(int16_t,s450)=1350; STATE(int16_t,s451)=1350;
        STATE(int16_t,s452)=1400; STATE(int16_t,s453)=2200;
    } else {
        const char *mode=getenv("RELOQ_FA_US_ARTICULATION");
        if (mode && mode[0]=='1' && !mode[1]) {
            /* The SH rule can backtrack and rebuild its targets multiple
               times, so select the US-shaped bands at each actual target
               construction, not only after its dispatch returns. */
            STATE(int16_t,s408)=0; STATE(int16_t,s409)=0;
            STATE(int16_t,s410)=62; STATE(int16_t,s411)=59;
            STATE(int16_t,s412)=0; STATE(int16_t,s413)=0;
        }
    }
'''

Q_CLUSTER_HELPER = r'''
/* Private listening build: schedule the selected q's release when the
   ordinary Finnish stop+stop path would suppress it. Source spelling is a
   probe switch, not a replacement for a Persian phoneme identity. */
static int persian_probe_before_stop(void *state)
{
    const char *setting=getenv("RELOQ_FA_PROBE_Q");
    if (!setting || setting[1] ||
        !((setting[0]>='3' && setting[0]<='5') || setting[0]=='7')) return 0;
    delta_state *d=state;
    int32_t position=STATE(int32_t,w61);
    if (!position) return 0;
    if (setting[0]=='7') {
        delta_node *left=vmovel(d,EVV_AT(delta_node *,position),2);
        int32_t current=left ? vgetsc(d,1,0,EVV_REF(left),2) : 0;
        if (!current || (*EVV_AT(int32_t *,current)&2)) return 0;
        void *fields=TFLDS(EVV_AT(void *,current));
        if (*(const unsigned char *)delta_record_field(&vstmtbl[2],0,fields)!=3 ||
            *(const unsigned char *)delta_record_field(&vstmtbl[2],12,fields)!=3) return 0;
        int32_t *right=vmover(d,EVV_AT(int32_t *,position),2);
        int32_t next=right ? vgetsc(d,0,0,EVV_REF(right),2) : 0;
        if (!next || (*EVV_AT(int32_t *,next)&2)) return 0;
        unsigned name=*(const unsigned char *)delta_record_field(
            &vstmtbl[2],0,TFLDS(EVV_AT(void *,next)));
        return name>=1 && name<=3;
    }
    delta_node *point=vmovel(d,EVV_AT(delta_node *,position),1);
    if (!point) return 0;
    int32_t node=vgetsc(d,1,0,EVV_REF(point),1);
    if (!node || (*EVV_AT(int32_t *,node)&2)) return 0;
    unsigned key=*(const unsigned char *)delta_record_field(
        &vstmtbl[1],0,TFLDS(EVV_AT(void *,node)));
    if (key!=25 && key!=45) return 0; /* q/Q */
    int32_t after=*EVV_AT(int32_t *,node+4)&~3;
    if (!after) return 0;
    int32_t *nextpoint=vmover(d,EVV_AT(int32_t *,after),1);
    if (!nextpoint) return 0;
    node=vgetsc(d,0,0,EVV_REF(nextpoint),1);
    if (!node || (*EVV_AT(int32_t *,node)&2)) return 0;
    key=*(const unsigned char *)delta_record_field(
        &vstmtbl[1],0,TFLDS(EVV_AT(void *,node)));
    /* Finnish spelling projection after case normalization: k, p, t. */
    return key==20 || key==24 || key==28;
}

static void persian_probe_cluster_release(void *state)
{
    if (!persian_probe_before_stop(state)) return;
    const char *setting=getenv("RELOQ_FA_PROBE_Q");
    int32_t end=STATE(int32_t,f11);
    int32_t duration=setting[0]=='3' ? 100000 : 250000;
    int32_t start=end>duration ? end-duration : 0;
    int32_t onset=start+50000; /* 5 ms ramp in native 1/10000-ms units. */
    if (end<=0 || onset>=end) return;
    /* Put targets in monotonic stream order before the native cluster rule.
       It supplies the following stop and its own boundary targets. */
    reloq61_submit_interval(state,STATE(int16_t,s66),0,start,start);
    reloq61_submit_interval(state,STATE(int16_t,s67),STATE(int16_t,s437),onset,end);
    reloq61_submit_interval(state,STATE(int16_t,s68),STATE(int16_t,s438),onset,end);
    reloq61_submit_interval(state,STATE(int16_t,s69),STATE(int16_t,s439),onset,end);
    reloq61_submit_interval(state,STATE(int16_t,s70),STATE(int16_t,s440),onset,end);
    reloq61_submit_interval(state,STATE(int16_t,s71),STATE(int16_t,s441),onset,end);
    reloq61_submit_interval(state,STATE(int16_t,s72),STATE(int16_t,s442),onset,end);
    reloq61_submit_interval(state,STATE(int16_t,s66),STATE(int16_t,s430),onset,end);
}

static int32_t persian_probe_phone(delta_state *d, int32_t position, int back)
{
    if (!position) return 0;
    void *point=back ? (void *)vmovel(d,EVV_AT(delta_node *,position),2)
                     : (void *)vmover(d,EVV_AT(int32_t *,position),2);
    if (!point) return 0;
    int32_t node=vgetsc(d,back,0,EVV_REF(point),2);
    return node && !(*EVV_AT(int32_t *,node)&2) ? node : 0;
}

static unsigned persian_probe_phone_field(int32_t node, int field)
{
    return *(const unsigned char *)delta_record_field(
        &vstmtbl[2],field,TFLDS(EVV_AT(void *,node)));
}

static int persian_probe_is_q(void *state, int32_t position)
{
    delta_state *d=state;
    if (!position) return 0;
    delta_node *point=vmovel(d,EVV_AT(delta_node *,position),1);
    if (!point) return 0;
    int32_t node=vgetsc(d,1,0,EVV_REF(point),1);
    if (!node || (*EVV_AT(int32_t *,node)&2)) return 0;
    unsigned key=*(const unsigned char *)delta_record_field(
        &vstmtbl[1],0,TFLDS(EVV_AT(void *,node)));
    return key==25 || key==45;
}

static void persian_probe_stop_aspiration(void *state)
{
    const char *setting=getenv("RELOQ_FA_PROBE_STOPS");
    if (!setting || setting[0]!='1' || setting[1]) return;
    delta_state *d=state;
    int32_t begin=STATE(int32_t,w60),end=STATE(int32_t,w61);
    int32_t current=persian_probe_phone(d,end,1);
    int32_t next=persian_probe_phone(d,end,0);
    int32_t previous=persian_probe_phone(d,begin,1);
    if (!current || !next || persian_probe_phone_field(next,1)!=1 ||
        (previous && persian_probe_phone_field(previous,1)!=1)) return;
    unsigned phone=persian_probe_phone_field(current,0);
    if (phone<1 || phone>3 || (phone==3 && persian_probe_is_q(state,end))) return;
    int32_t start=STATE(int32_t,f11);
    if (start<0 || start>INT32_MAX-400000) return;
    STATE(int32_t,f11)=start+400000;
    /* A first approximation to the German /p t k/ AH levels is transferred
       into the Finnish stream. Only initial and intervocalic positions are
       targeted; the 40-ms extension is a listening hypothesis, not a
       language-independent voice-onset-time rule. */
    int16_t level=phone==1 ? 49 : phone==2 ? 47 : 45;
    reloq61_submit_interval(state,STATE(int16_t,s65),0,start,start);
    reloq61_submit_interval(state,STATE(int16_t,s65),level,start+50000,start+400000);
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


def prepare_x(work: Path) -> list[Path]:
    """Reserve Finnish G for a German-shaped /x/ in a throwaway build."""
    outputs = []
    include = '#include "reloq61_rule_calls_fifi.h"\n'
    for name, rule in (('delta_rules_c30_fifi.c', 'fin_ph_G'),
                       ('delta_rules_c26_fifi.c', 'fin_ph_S')):
        text = (PROVIDER / 'lang' / 'fifi' / name).read_text(encoding='utf8')
        start = text.index('int32_t reloq_fifi_' + rule + '(')
        end = text.index('\nstatic int32_t evv_' + rule + '(', start)
        block = text[start:end]
        if rule == 'fin_ph_G':
            marker = 'int32_t reloq_fifi_fin_ph_G(void *state, int32_t p0)\n{\n'
            insertion = marker + X_REDIRECT
            helper = X_HELPER
        else:
            marker = '    STATE(int16_t, s410) = (62);\n    STATE(int16_t, s411) = (62);\n'
            insertion = marker + X_FRICTION
            helper = 'extern _Thread_local int persian_probe_x_active;\n'
        if block.count(marker) != 1 or text.count(include) != 1:
            raise RuntimeError('Cannot locate the unique X probe insertion in ' + name)
        block = block.replace(marker, insertion, 1)
        text = (text[:start] + block + text[end:]).replace(
            include, include + '#include <stdlib.h>\n' + helper, 1)
        destination = work / name
        destination.write_text(text, encoding='utf8')
        outputs.append(destination)
    return outputs


def prepare_q_cluster(work: Path) -> Path:
    name = 'delta_rules_c28_fifi.c'
    text = (PROVIDER / 'lang' / 'fifi' / name).read_text(encoding='utf8')
    include = '#include "reloq61_rule_calls_fifi.h"\n'
    marker = 'int32_t reloq_fifi_do_non_son_amps(void *state, int32_t p0)\n{\n'
    if text.count(include) != 1 or text.count(marker) != 1:
        raise RuntimeError('Cannot locate unique Q cluster probe insertion')
    text = text.replace(include, include + '#include <stdlib.h>\n'
                        '#include "reloq61_acoustic_output.h"\n', 1)
    text = text.replace(marker, Q_CLUSTER_HELPER + '\n' + marker +
                        '    persian_probe_cluster_release(state);\n'
                        '    persian_probe_stop_aspiration(state);\n', 1)
    # Word records carry their ordinal in their first half-word. Consult the
    # existing word projection at the glottalization candidate's location:
    # keep the original first-word onset, but omit automatic Finnish marking
    # on later words. If the owner cannot be read, preserve the native rule.
    helper = r'''
#include <stdio.h>
#include <stdlib.h>
#ifdef RELOQ_FA_SCOPED_ONSETS
extern int reloq_fa_scoped_native_onset(int);
#endif
static int persian_glottal_word_number(void *state,int32_t token_ref)
{
    delta_state *d=state;
    if (!d || !d->vars || !token_ref) return 0;
    int32_t at=0;
    memcpy(&at,EVV_AT(const unsigned char *,token_ref)+4,4);
    if (!at) return 0;
    int32_t begin=vgetsc(d,1,1,at,4);
    if (!begin) return 0;
    int32_t position=EVV_REF(vmover(d,EVV_AT(int32_t *,begin),4));
    if (!position) return 0;
    int32_t word=vgetsc(d,0,0,position,4);
    if (!word || (*EVV_AT(int32_t *,word)&2)) return 0;
    int16_t number=0;
    memcpy(&number,TFLDS(EVV_AT(void *,word)),sizeof number);
    if (getenv("RELOQ_FA_TRACE_GLOTTAL"))
        fprintf(stderr,"FA_GLOTTAL_WORD number=%d\n",(int)number);
    return number>0 ? number : 0;
}
'''
    text = text.replace(include, include + helper, 1)
    # The explicit Persian w/ʔ carrier is marked later by
    # persian_auto_h_insert and remains independent of this decision.
    call = ('    r0 = reloq_call_fifi_glottalize_vow(state, (int32_t)(FIELD(0)), '
            '(int32_t)((int32_t)AT(int32_t, -40)));\n')
    if text.count(call) != 1:
        raise RuntimeError('Cannot locate automatic Finnish glottalization')
    text = text.replace(call, r'''    {
        const char *fa_no_auto_glottal=getenv("RELOQ_FA_PROBE_NO_AUTO_GLOTTAL");
        int word=fa_no_auto_glottal && fa_no_auto_glottal[0]=='1' && !fa_no_auto_glottal[1]
            ? persian_glottal_word_number(state,(int32_t)AT(int32_t,-40)) : 0;
        int marked=0;
#ifdef RELOQ_FA_SCOPED_ONSETS
        if (word>1) marked=reloq_fa_scoped_native_onset(word);
#endif
        if (!fa_no_auto_glottal || fa_no_auto_glottal[0]!='1' || fa_no_auto_glottal[1] || word<=1 || marked)
            r0 = reloq_call_fifi_glottalize_vow(state, (int32_t)(FIELD(0)), (int32_t)((int32_t)AT(int32_t, -40)));
        else
            r0 = 0;
    }
''', 1)
    destination = work / name
    destination.write_text(text, encoding='utf8')
    return destination


def prepare_q_phone(work: Path) -> Path:
    """Reserve R and w as marked stops only in the isolated probes."""
    name = 'delta_rules_c23_fifi.c'
    text = (PROVIDER / 'lang' / 'fifi' / name).read_text(encoding='utf8')
    include = '#include "reloq61_rule_calls_fifi.h"\n'
    begin = text.index('int32_t reloq_fifi_fin_cons_durs(')
    end = text.index('\nstatic int32_t evv_fin_cons_durs(', begin)
    section = text[begin:end]
    marker = 'L485:;\n    STATE(int16_t, s458)'
    if text.count(include) != 1 or section.count(marker) != 1:
        raise RuntimeError('Cannot locate the isolated Q phone conversion')
    helper = r'''
/* The mapper uses Finnish R for Persian q and, only in the glottal probe,
   Finnish w for the glottal stop. Turn each into a marked stop before
   duration/acoustic dispatch. These private markers are disabled by default. */
static void persian_probe_convert_q_phone(void *state,int32_t position)
{
    const char *setting=getenv("RELOQ_FA_PROBE_Q");
    const char *glottal=getenv("RELOQ_FA_PROBE_GLOTTAL");
    int use_q=setting && setting[0]=='7' && !setting[1];
    int use_glottal=glottal && (glottal[0]=='1' || glottal[0]=='2') && !glottal[1];
    if ((!use_q && !use_glottal) || !position) return;
    delta_state *d=state;
    int32_t *point=vmover(d,EVV_AT(int32_t *,position),2);
    int32_t node=point ? vgetsc(d,0,0,EVV_REF(point),2) : 0;
    if (!node || (*EVV_AT(int32_t *,node)&2)) return;
    void *fields=TFLDS(EVV_AT(void *,node));
    unsigned name=*(const unsigned char *)delta_record_field(&vstmtbl[2],0,fields);
    if (!((name==20 && use_q) || (name==23 && use_glottal))) return;
    unsigned after=*(const unsigned char *)delta_record_field(&vstmtbl[2],9,fields);
    /* h+w marks a word-final H carrier. Bare w is the glottal marker. */
    const unsigned char k=3,mark=name==20 ? 3 : after==1 ? 5 : 4;
    memcpy(fields,vstmtbl[2].variants+k*vstmtbl[2].stride,vstmtbl[2].varlen);
    vstmtbl[2].put[0](fields,&k);
    vstmtbl[2].put[12](fields,&mark);
    EVV_AT(delta_owner *,d->owner)->changed=1;
}
static unsigned persian_probe_special_stop(void *state,int32_t position)
{
    const char *enabled=getenv("RELOQ_FA_PROBE_GLOTTAL");
    if (!enabled || (enabled[0]!='1' && enabled[0]!='2') || enabled[1] || !position) return 0;
    delta_state *d=state;
    int32_t *point=vmover(d,EVV_AT(int32_t *,position),2);
    int32_t node=point ? vgetsc(d,0,0,EVV_REF(point),2) : 0;
    if (!node || (*EVV_AT(int32_t *,node)&2)) return 0;
    void *fields=TFLDS(EVV_AT(void *,node));
    if (*(const unsigned char *)delta_record_field(&vstmtbl[2],0,fields)!=3) return 0;
    unsigned marker=*(const unsigned char *)delta_record_field(&vstmtbl[2],12,fields);
    return marker==4 || marker==5 ? marker : 0;
}
static unsigned persian_probe_cons_name(void *state,int32_t position)
{
    delta_state *d=state;
    if (!d || !position) return 0;
    int32_t *point=vmover(d,EVV_AT(int32_t *,position),2);
    int32_t node=point ? vgetsc(d,0,0,EVV_REF(point),2) : 0;
    if (!node || (*EVV_AT(int32_t *,node)&2)) return 0;
    return *(const unsigned char *)delta_record_field(&vstmtbl[2],0,TFLDS(EVV_AT(void *,node)));
}
static unsigned persian_probe_previous_cons(void *state,int32_t position)
{
    delta_state *d=state;
    if (!d || !position) return 0;
    delta_node *point=vmovel(d,EVV_AT(delta_node *,position),2);
    int32_t node=point ? vgetsc(d,1,0,EVV_REF(point),2) : 0;
    if (!node || (*EVV_AT(int32_t *,node)&2)) return 0;
    return *(const unsigned char *)delta_record_field(&vstmtbl[2],0,TFLDS(EVV_AT(void *,node)));
}
static unsigned persian_probe_current_k_marker(void *state,int32_t position)
{
    delta_state *d=state;
    if (!d || !position) return 0;
    int32_t *point=vmover(d,EVV_AT(int32_t *,position),2);
    int32_t node=point ? vgetsc(d,0,0,EVV_REF(point),2) : 0;
    if (!node || (*EVV_AT(int32_t *,node)&2)) return 0;
    void *fields=TFLDS(EVV_AT(void *,node));
    return *(const unsigned char *)delta_record_field(&vstmtbl[2],0,fields)==3
        ? *(const unsigned char *)delta_record_field(&vstmtbl[2],12,fields) : 0;
}
#ifdef RELOQ_FA_SCOPED_ONSETS
extern int reloq_fa_scoped_has_geminates(void);
extern int reloq_fa_scoped_native_geminate(unsigned,unsigned);

/* Find this J's native word and J ordinal without changing the provider's
   scan registers or fences. Native J is represented by consecutive D, ZH
   phone records. Return no match when that projection cannot be verified. */
static int persian_probe_selected_j(void *state,int32_t position)
{
    delta_state *d=state;
    if(!d || !d->vars || !position || !reloq_fa_scoped_has_geminates())return 0;
    int32_t begin=vgetsc(d,1,1,position,4);
    if(!begin)return 0;
    int32_t word_pos=EVV_REF(vmover(d,EVV_AT(int32_t *,begin),4));
    int32_t word_node=word_pos?vgetsc(d,0,0,word_pos,4):0;
    if(!word_node || (*EVV_AT(int32_t *,word_node)&2))return 0;
    int16_t number=0;
    memcpy(&number,TFLDS(EVV_AT(void *,word_node)),sizeof number);
    if(number<=0)return 0;
    int32_t target_pos=EVV_REF(vmover(d,EVV_AT(int32_t *,position),2));
    int32_t target=target_pos?vgetsc(d,0,0,target_pos,2):0;
    int32_t at=EVV_REF(vmover(d,EVV_AT(int32_t *,begin),2));
    unsigned ordinal=0;
    for(unsigned guard=0;at && guard<4096;++guard) {
        int32_t node=vgetsc(d,0,0,at,2);
        if(!node || (*EVV_AT(int32_t *,node)&2))return 0;
        unsigned name=*(const unsigned char *)delta_record_field(&vstmtbl[2],0,TFLDS(EVV_AT(void *,node)));
        int32_t next=EVV_AT(int32_t *,node)[1]&~3;
        if(!next || next==at)return 0;
        next=EVV_REF(vmover(d,EVV_AT(int32_t *,next),2));
        int32_t after=next?vgetsc(d,0,0,next,2):0;
        unsigned next_name=after && !(*EVV_AT(int32_t *,after)&2)
            ? *(const unsigned char *)delta_record_field(&vstmtbl[2],0,TFLDS(EVV_AT(void *,after))):0;
        if(name==5 && next_name==12)++ordinal;
        if(node==target)
            return name==5 && next_name==12 &&
                   reloq_fa_scoped_native_geminate((unsigned)number,ordinal);
        at=next;
    }
    return 0;
}
#endif
'''
    helper += FINNISH_L_WORD_FINAL.replace('persian_probe_l_word_final', 'persian_probe_d_word_final').replace(
        'testFldeq(d,2,0,21)', 'testFldeq(d,2,0,5)')
    helper += FINNISH_L_WORD_FINAL.replace(
        'persian_probe_l_word_final(void *state,int32_t token_ref)',
        'persian_probe_cons_word_final(void *state,int32_t token_ref,unsigned phone)'
    ).replace('testFldeq(d,2,0,21)', 'testFldeq(d,2,0,phone)')
    section = section.replace(marker, 'L485:;\n    persian_probe_convert_q_phone(state,STATE(int32_t,w60));\n'
                              '    STATE(int16_t, s458)', 1)
    duration = 'L2614:;\n    r0 = reloq_call_fifi_insert_dur'
    if section.count(duration) != 1:
        raise RuntimeError('Cannot locate final H carrier duration')
    section = section.replace(duration, r'''L2614:;
    unsigned fa_special_stop=persian_probe_special_stop(state,STATE(int32_t,w60));
    if (fa_special_stop) {
        /* eSpeak 1.53 reports roughly 20 ms for the glottal phone in
           saat/soal/moallem. A final-H carrier needs only 5 ms after its
           native H transition. Both are unvoiced, burst-free approximations. */
        const char *glottal=getenv("RELOQ_FA_PROBE_GLOTTAL");
        STATE(int32_t,f20)=fa_special_stop==5 || (glottal && glottal[0]=='2') ? 50000 : 200000;
        STATE(int16_t,s458)=0;
    }
    {
        const char *mode=getenv("RELOQ_FA_PROBE_D");
        int trace=getenv("RELOQ_FA_TRACE_DB")!=NULL;
        if (trace || (mode && (mode[0]=='1' || mode[0]=='2') && !mode[1])) {
            int32_t at=STATE(int32_t,w60), after=STATE(int32_t,w61);
            unsigned current=persian_probe_cons_name(state,at);
            unsigned next=persian_probe_cons_name(state,after);
            int within_word=current==5 && next==4 && !persian_probe_d_word_final(state,at);
            if (trace && current==5)
                fprintf(stderr,"FA_D next=%u within=%d duration=%d\n",next,within_word,(int)STATE(int32_t,f20));
            /* Hearing candidates for a D before B inside one word. Retain
               native voicing, release, stress and the next consonant. */
            if (within_word && mode && (mode[0]=='1' || mode[0]=='2') && !mode[1] && STATE(int32_t,f20)>0) {
                int factor=mode[0]=='1' ? 150 : 200;
                int64_t scaled=((int64_t)STATE(int32_t,f20)*factor+50)/100;
                STATE(int32_t,f20)=scaled>INT32_MAX ? INT32_MAX : (int32_t)scaled;
            }
        }
    }
    {
        /* Offline-only hearing candidates: the native J is expanded into
           one D followed by voiced ZH. Lengthen those native components
           together instead of inserting a second, separate D. */
        const char *mode=getenv("RELOQ_FA_GEMINATE_J");
        int offline=mode && (mode[0]=='1' || mode[0]=='2' || mode[0]=='3') && !mode[1];
#ifdef RELOQ_FA_SCOPED_ONSETS
        int scoped=persian_probe_selected_j(state,STATE(int32_t,w60));
#else
        int scoped=0;
#endif
        if (offline || scoped) {
            int32_t at=STATE(int32_t,w60), after=STATE(int32_t,w61);
            unsigned current=persian_probe_cons_name(state,at);
            unsigned next=persian_probe_cons_name(state,after);
            unsigned previous=persian_probe_previous_cons(state,at);
            int factor=100;
            if (scoped && current==5 && next==12) factor=200;
            else if (offline && current==5 && next==12) factor=mode[0]=='2' ? 150 : 200;
            else if (offline && current==12 && previous==5 && mode[0]!='1') factor=150;
            if (getenv("RELOQ_FA_TRACE_GEM") && factor!=100)
                fprintf(stderr,"FA_GEM phone=%u before=%u after=%u factor=%d duration=%d\n",
                        current,previous,next,factor,(int)STATE(int32_t,f20));
            if (factor!=100 && STATE(int32_t,f20)>0) {
                int64_t scaled=((int64_t)STATE(int32_t,f20)*factor+50)/100;
                STATE(int32_t,f20)=scaled>INT32_MAX ? INT32_MAX : (int32_t)scaled;
            }
        }
    }
    r0 = reloq_call_fifi_insert_dur''', 1)
    for phone in 'ptkbdgfvszSZmn':
        call = f'    r0 = reloq_call_fifi_fin_ph_{phone}_dur(state, (int32_t)(FIELD(0)));\n'
        if section.count(call) != 1:
            raise RuntimeError('Cannot locate Finnish stop-duration dispatch: ' + phone)
        special = (' && persian_probe_current_k_marker(state,STATE(int32_t,w60))<3'
                   if phone == 'k' else '')
        key = ('RELOQ_FA_US_STOPS' if phone in 'ptkbdg' else
               'RELOQ_FA_US_NASALS' if phone in 'mn' else 'RELOQ_FA_US_FRICATIVES')
        section = section.replace(call, '    {\n' + f'        const char *us=getenv("{key}");\n' +
            f"        if (us && us[0]=='1' && !us[1]{special})\n" +
            f'            r0=persian_us_eng_ph_{phone}_dur(state,(int32_t)(FIELD(0)));\n' +
            '        else\n' + call + '    }\n', 1)
    prototypes = ''.join(f'extern int32_t persian_us_eng_ph_{phone}_dur(void *,int32_t);\n'
                         for phone in 'ptkbdgfvszSZmn')
    text = text[:begin] + prototypes + helper + '\n' + section + text[end:]
    text = text.replace(include, include + '#include <stdlib.h>\n#include <stdio.h>\n#include <stdint.h>\n', 1)
    destination = work / name
    destination.write_text(text, encoding='utf8')
    return destination


def prepare_finnish_h(work: Path) -> Path:
    """Shorten a real Finnish /h/ after /p t k/ in the private comparison."""
    name = 'delta_rules_c12_fifi.c'
    text = (PROVIDER / 'lang' / 'fifi' / name).read_text(encoding='utf8')
    include = '#include "reloq61_rule_calls_fifi.h"\n'
    helper = r'''
#include <stdlib.h>
static int32_t persian_probe_h_phone(delta_state *d,int32_t at,int back)
{
    if (!at) return 0;
    void *point=back ? (void *)vmovel(d,EVV_AT(delta_node *,at),2)
                     : (void *)vmover(d,EVV_AT(int32_t *,at),2);
    if (!point) return 0;
    int32_t node=vgetsc(d,back,0,EVV_REF(point),2);
    return node && !(*EVV_AT(int32_t *,node)&2) ? node : 0;
}
static int persian_probe_short_h(void *state,int32_t at)
{
    const char *setting=getenv("RELOQ_FA_PROBE_H");
    if (!setting || (setting[0]!='1' && setting[0]!='2' && setting[0]!='3' &&
                     setting[0]!='4' && setting[0]!='5') || setting[1]) return 0;
    delta_state *d=state;
    int32_t before=persian_probe_h_phone(d,at,1);
    int32_t after=persian_probe_h_phone(d,at,0);
    if (!before || !after) return 0;
    void *a=TFLDS(EVV_AT(void *,before)),*b=TFLDS(EVV_AT(void *,after));
    unsigned name=*(const unsigned char *)delta_record_field(&vstmtbl[2],0,a);
    unsigned cls=*(const unsigned char *)delta_record_field(&vstmtbl[2],1,b);
    return name>=1 && name<=3 && cls==1; /* /p t k/ before a vowel */
}
'''
    start = text.index('int32_t reloq_fifi_get_h_dur(')
    end = text.index('\nstatic int32_t evv_get_h_dur(', start)
    block = text[start:end]
    marker = 'L1996:;\n    r0 = reloq61_typed_succeed'
    if block.count(marker) != 1 or text.count(include) != 1:
        raise RuntimeError('Cannot locate unique Finnish H duration insertion')
    block = block.replace(marker, r'''L1996:;
    if (persian_probe_short_h(state,AT(int32_t,-32))) {
        int16_t *duration=(int16_t *)(REG_P(p3)+2);
        const char *setting=getenv("RELOQ_FA_PROBE_H");
        if (*duration>0) {
            if (setting[0]=='4') *duration=(int16_t)(((int)*duration+1)/2);
            else if (setting[0]=='5') *duration=(int16_t)(((int)*duration*3+2)/5);
            else if (setting[0]=='3') *duration=(int16_t)(((int)*duration*7+5)/10);
            else *duration=(int16_t)(((int)*duration*4+2)/5);
        }
    }
    r0 = reloq61_typed_succeed''', 1)
    text = (text[:start] + block + text[end:]).replace(include, include + helper, 1)
    destination = work / name
    destination.write_text(text, encoding='utf8')
    return destination


def prepare_auto_finnish_h(work: Path) -> Path:
    """Insert Finnish H as a native transition feature in a private probe."""
    name = 'delta_rules_c16_fifi.c'
    text = (PROVIDER / 'lang' / 'fifi' / name).read_text(encoding='utf8')
    include = '#include "reloq61_rule_calls_fifi.h"\n'
    helper = r'''
#include <stdlib.h>
#include <stdio.h>
static int32_t persian_auto_h_phone(delta_state *d,int32_t at,int back)
{
    if (!at) return 0;
    void *point=back ? (void *)vmovel(d,EVV_AT(delta_node *,at),2)
                     : (void *)vmover(d,EVV_AT(int32_t *,at),2);
    if (!point) return 0;
    int32_t node=vgetsc(d,back,0,EVV_REF(point),2);
    return node && !(*EVV_AT(int32_t *,node)&2) ? node : 0;
}
static unsigned persian_auto_h_field(int32_t node,int field)
{
    return *(const unsigned char *)delta_record_field(
        &vstmtbl[2],field,TFLDS(EVV_AT(void *,node)));
}
static int persian_auto_h_is_q(delta_state *d,int32_t at)
{
    if (!at) return 0;
    delta_node *point=vmovel(d,EVV_AT(delta_node *,at),1);
    if (!point) return 0;
    int32_t node=vgetsc(d,1,0,EVV_REF(point),1);
    if (!node || (*EVV_AT(int32_t *,node)&2)) return 0;
    unsigned name=*(const unsigned char *)delta_record_field(
        &vstmtbl[1],0,TFLDS(EVV_AT(void *,node)));
    return name==25 || name==45;
}
/* Use the native word fence; a consonant from the preceding word must not
   suppress aspiration of this word's initial stop. Restore scanner state. */
static int persian_auto_h_word_start(void *state,int32_t token_ref)
{
    delta_state *d=state;
    if (!d || !d->vars || d->nstmts<5 || d->nstmts>256 || !token_ref) return 0;
    delta_vars *v=EVV_AT(delta_vars *,d->vars);
    delta_tpos left=d->lpta,right=d->rpta;
    unsigned char scan[8],index[256],marks[256],chars[256];
    int8_t compared=v->compared_equal,fences=v->fence_count,testing=v->testing;
    memcpy(scan,&v->scan_ptr,sizeof scan);
    memcpy(index,EVV_AT(const void *,d->fence_index),d->nstmts);
    memcpy(marks,EVV_AT(const void *,d->fence_marks),d->nstmts);
    memcpy(chars,EVV_AT(const void *,d->fence_chars),d->nstmts);
    v->testing=0;
    const uint8_t word_fence[]={4};
    fence(d,1,word_fence);
    memset(EVV_AT(void *,d->fence_marks),0,d->nstmts);
    lpta_loadp(d,EVV_AT(const delta_token *,token_ref));
    /* The first scan finds the word fence; advancing fails exactly when
       there is no preceding phone inside this word. */
    int initial=!setscan_l(d,2) && advance_tok(d);
    d->lpta=left; d->rpta=right; memcpy(&v->scan_ptr,scan,sizeof scan);
    v->compared_equal=compared; v->fence_count=fences; v->testing=testing;
    memcpy(EVV_AT(void *,d->fence_index),index,d->nstmts);
    memcpy(EVV_AT(void *,d->fence_marks),marks,d->nstmts);
    memcpy(EVV_AT(void *,d->fence_chars),chars,d->nstmts);
    return initial;
}
static void persian_auto_h_insert(void *state)
{
    const char *setting=getenv("RELOQ_FA_PROBE_H");
    const char *glottal=getenv("RELOQ_FA_PROBE_GLOTTAL");
    int h_enabled=setting && !setting[1] && (setting[0]=='2' || setting[0]=='3' ||
                                             setting[0]=='4' || setting[0]=='5');
    int glot_enabled=glottal && glottal[0]=='2' && !glottal[1];
    if (!h_enabled && !glot_enabled) return;
    delta_state *d=state;
    int32_t begin=STATE(int32_t,w60),end=STATE(int32_t,w61);
    int32_t previous=persian_auto_h_phone(d,begin,1);
    int32_t stop=persian_auto_h_phone(d,end,1);
    int32_t vowel=persian_auto_h_phone(d,STATE(int32_t,w63),0);
    if (!stop || !vowel || persian_auto_h_field(vowel,1)!=1) return;
    unsigned phone=persian_auto_h_field(stop,0);
    unsigned marker=persian_auto_h_field(stop,12);
    /* Transition duration runs before the private w carrier is converted to
       a marked stop. On paths that already converted it, accept marker 4. */
    int glot_carrier=(phone==23 && persian_auto_h_field(stop,9)==0) ||
                     (phone==3 && marker==4);
    const char *glot_onset=getenv("RELOQ_FA_PROBE_GLOT_ONSET");
    int use_onset=!glot_onset || (glot_onset[0]=='1' && !glot_onset[1]);
    if (glot_enabled && glot_carrier && use_onset) {
        /* Let Finnish's native glottal onset shape the source at the next
           vowel, instead of approximating it only with a silent K closure. */
        void *fields=TFLDS(EVV_AT(void *,vowel));
        if (*(const unsigned char *)delta_record_field(&vstmtbl[2],9,fields)==0) {
            const unsigned char after_glottal=3;
            vstmtbl[2].put[9](fields,&after_glottal);
            EVV_AT(delta_owner *,d->owner)->changed=1;
            if (getenv("RELOQ_FA_TRACE_H_START"))
                fprintf(stderr,"FA_GLOTTAL assigned native vowel onset\n");
        }
    }
    if (!h_enabled) return;
    int initial=persian_auto_h_word_start(state,STATE_AT(w60,-4));
    if (getenv("RELOQ_FA_TRACE_H_START"))
        fprintf(stderr,"FA_H phone=%u prev=%u initial=%d\n",phone,
                previous ? persian_auto_h_field(previous,0) : 0,initial);
    /* Onsets after voiced consonants and after a preceding stop need the
       same selected Finnish H impulse as onsets after vowels and nasals.
       The native pause phone (36), including at bracket boundaries, can
       precede a word without presenting a normal word fence. Some Persian
       descriptions report deaspiration after voiceless fricatives; keep
       those clusters on their existing path. Uvular and glottal carriers
       are checked separately below and must never acquire stop aspiration. */
    unsigned previous_name=previous ? persian_auto_h_field(previous,0) : 0;
    int after_voiced=previous_name==4 || previous_name==5 || previous_name==6 ||
                     previous_name==8 || previous_name==10 || previous_name==12 ||
                     previous_name==16 || previous_name==17 || previous_name==19 ||
                     previous_name==21 || previous_name==22;
    int after_stop=previous_name>=1 && previous_name<=3;
    int after_pause=previous_name==36;
    if (previous && persian_auto_h_field(previous,1)!=1 && !initial &&
        !after_voiced && !after_stop && !after_pause) return;
    const char *qsetting=getenv("RELOQ_FA_PROBE_Q");
    int forced_q=qsetting && qsetting[0]=='6' && !qsetting[1];
    int marked_q=qsetting && qsetting[0]=='7' && !qsetting[1] &&
                 persian_auto_h_field(stop,12)==3;
    int marked_glottal=glottal && (glottal[0]=='1' || glottal[0]=='2') && !glottal[1] &&
                       (marker==4 || marker==5);
    if (phone<1 || phone>3 || (phone==3 && (forced_q || marked_q || marked_glottal || persian_auto_h_is_q(d,end)))) return;
    void *fields=TFLDS(EVV_AT(void *,vowel));
    if (*(const unsigned char *)delta_record_field(&vstmtbl[2],9,fields)!=0) return;
    const unsigned char after_h=1;
    vstmtbl[2].put[9](fields,&after_h);
    EVV_AT(delta_owner *,d->owner)->changed=1;
}
'''
    marker = 'int32_t reloq_fifi_get_fin_trans_dur(void *state, int32_t p0, int32_t p1)\n{\n'
    if text.count(include) != 1 or text.count(marker) != 1:
        raise RuntimeError('Cannot locate Finnish H transition insertion')
    text = text.replace(include, include + '#include <stdlib.h>\n', 1)
    text = text.replace(marker, helper + '\n' + marker + '    persian_auto_h_insert(state);\n', 1)
    stop_call = ('    r0 = reloq_call_fifi_get_fin_stop_trans_dur(state, (int32_t)(FIELD(0)), '
                 '(int32_t)(RELOQ61_WORKSPACE_REF(frame, reloq_workspace_ref, 192)));\n')
    if text.count(stop_call) != 1:
        raise RuntimeError('Cannot locate the Finnish stop-transition duration')
    text = text.replace(include, include +
                        'extern int32_t persian_us_get_eng_stop_trans_dur(void *,int32_t,int32_t);\n'
                        'extern int32_t persian_us_get_eng_fric_trans_dur(void *,int32_t,int32_t);\n'
                        'extern int32_t persian_us_get_eng_nas_trans_dur(void *,int32_t,int32_t);\n'
                        'extern int32_t persian_us_eng_trans_features(void *,int32_t);\n', 1)
    trans_call = '    r0 = reloq_call_fifi_fin_trans_features(state, (int32_t)(FIELD(0)));\n'
    if text.count(trans_call) != 1:
        raise RuntimeError('Cannot locate Finnish transition feature selection')
    text = text.replace(trans_call, r'''    {
        const char *us=getenv("RELOQ_FA_US_STOPS");
        delta_state *d=state;
        int32_t previous=persian_auto_h_phone(d,STATE(int32_t,w61),1);
        int32_t following=persian_auto_h_phone(d,STATE(int32_t,w63),0);
        unsigned phone=previous ? persian_auto_h_field(previous,0) : 0;
        unsigned mark=previous ? persian_auto_h_field(previous,12) : 0;
        int ordinary=phone==1 || phone==2 || (phone>=4 && phone<=6) || (phone==3 && mark<3 &&
                         !persian_auto_h_is_q(d,STATE(int32_t,w61)));
        int vowel=following && persian_auto_h_field(following,1)==1;
        if (us && us[0]=='1' && !us[1] && ordinary && vowel)
            r0=persian_us_eng_trans_features(state,(int32_t)(FIELD(0)));
        else
''' + trans_call + '    }\n', 1)
    text = text.replace(stop_call, r'''    {
        const char *us=getenv("RELOQ_FA_US_STOPS");
        delta_state *d=state;
        int32_t current=persian_auto_h_phone(d,STATE(int32_t,w61),1);
        unsigned phone=current ? persian_auto_h_field(current,0) : 0;
        unsigned mark=current ? persian_auto_h_field(current,12) : 0;
        int ordinary=phone==1 || phone==2 || (phone>=4 && phone<=6) || (phone==3 && mark<3 &&
                         !persian_auto_h_is_q(d,STATE(int32_t,w61)));
        if (us && us[0]=='1' && !us[1] && ordinary)
            r0=persian_us_get_eng_stop_trans_dur(state,(int32_t)(FIELD(0)),
                      (int32_t)(RELOQ61_WORKSPACE_REF(frame,reloq_workspace_ref,192)));
        else
''' + stop_call + '    }\n', 1)
    fric_call = ('    r0 = reloq_call_fifi_get_fin_fric_trans_dur(state, (int32_t)(FIELD(0)), '
                 '(int32_t)(RELOQ61_WORKSPACE_REF(frame, reloq_workspace_ref, 192)));\n')
    if text.count(fric_call) != 1:
        raise RuntimeError('Cannot locate the Finnish fricative transition duration')
    text = text.replace(fric_call, r'''    {
        const char *us=getenv("RELOQ_FA_US_FRICATIVES");
        delta_state *d=state;
        int32_t current=persian_auto_h_phone(d,STATE(int32_t,w61),1);
        unsigned phone=current ? persian_auto_h_field(current,0) : 0;
        if (us && us[0]=='1' && !us[1] && phone>=7 && phone<=12)
            r0=persian_us_get_eng_fric_trans_dur(state,(int32_t)(FIELD(0)),
                      (int32_t)(RELOQ61_WORKSPACE_REF(frame,reloq_workspace_ref,192)));
        else
''' + fric_call + '    }\n', 1)
    nas_call = ('    r0 = reloq_call_fifi_get_fin_nas_trans_dur(state, (int32_t)(FIELD(0)), '
                '(int32_t)(RELOQ61_WORKSPACE_REF(frame, reloq_workspace_ref, 192)));\n')
    if text.count(nas_call) != 1:
        raise RuntimeError('Cannot locate the Finnish nasal transition duration')
    text = text.replace(nas_call, r'''    {
        const char *us=getenv("RELOQ_FA_US_NASALS");
        delta_state *d=state;
        int32_t current=persian_auto_h_phone(d,STATE(int32_t,w61),1);
        unsigned phone=current ? persian_auto_h_field(current,0) : 0;
        if (us && us[0]=='1' && !us[1] && (phone==16 || phone==17))
            r0=persian_us_get_eng_nas_trans_dur(state,(int32_t)(FIELD(0)),
                      (int32_t)(RELOQ61_WORKSPACE_REF(frame,reloq_workspace_ref,192)));
        else
''' + nas_call + '    }\n', 1)
    begin = text.index('int32_t reloq_fifi_do_fin_h_amps(')
    end = text.index('\nstatic int32_t evv_do_fin_h_amps(', begin)
    section = text[begin:end]
    exit_point = 'L12794:;\n    r0 = reloq61_typed_succeed'
    if section.count(exit_point) != 1:
        raise RuntimeError('Cannot locate the final H amplitude')
    section = section.replace(exit_point, r'''L12794:;
    {
        const char *level=getenv("RELOQ_FA_PROBE_END_H_DB");
        delta_state *d=state;
        int32_t current=persian_auto_h_phone(d,STATE(int32_t,w61),1);
        int32_t next=persian_auto_h_phone(d,STATE(int32_t,w61),0);
        unsigned phone=current ? persian_auto_h_field(current,0) : 0;
        unsigned nextphone=next ? persian_auto_h_field(next,0) : 0;
        unsigned nextmark=next ? persian_auto_h_field(next,12) : 0;
        unsigned nextafter=next ? persian_auto_h_field(next,9) : 0;
        int carrier=(nextphone==3 && nextmark==5) ||
                    (nextphone==23 && nextafter==1);
        if (getenv("RELOQ_FA_TRACE_H_LEVEL"))
            fprintf(stderr,"FA_HAMP phone=%u next=%u mark=%u after=%u carrier=%d level=%d\n",
                    phone,nextphone,nextmark,nextafter,carrier,STATE(int16_t,s420));
        /* A marked word-final H after front /ae/: reduce its native H source,
           without lowering the whole word or the aspirated P/T/K transitions. */
        char *end=0;
        long reduction=level && *level ? strtol(level,&end,10) : 0;
        if (end && !*end && reduction>0 && reduction<=18 &&
            phone==31 && carrier && STATE(int16_t,s420)>0) {
            int value=STATE(int16_t,s420)-(int)reduction;
            STATE(int16_t,s420)=value>0 ? value : 0;
        }
    }
    r0 = reloq61_typed_succeed''', 1)
    text = text[:begin] + section + text[end:]
    destination = work / name
    destination.write_text(text, encoding='utf8')
    return destination


def prepare_question_rise(work: Path) -> list[Path]:
    """Transfer Spanish question tone choices into the private Finnish path."""
    outputs = []
    include = '#include "reloq61_rule_calls_fifi.h"\n'
    helper = r'''
#include <stdlib.h>
extern int reloq61_english_is_question(void *, int32_t);
static int persian_probe_question(void *state, int32_t phrase)
{
    const char *setting=getenv("RELOQ_FA_PROBE_RISE");
    return setting && (setting[0]=='1' || setting[0]=='2') && !setting[1] &&
        reloq61_english_is_question(state,phrase);
}
static int persian_probe_spanish_rise(void)
{
    const char *setting=getenv("RELOQ_FA_PROBE_RISE");
    return setting && setting[0]=='2' && !setting[1];
}
'''
    name = 'delta_rules_c29_fifi.c'
    text = (PROVIDER / 'lang' / 'fifi' / name).read_text(encoding='utf8')
    start = text.index('int32_t reloq_fifi_init_phrase_inton_vars(')
    end = text.index('\nstatic int32_t evv_init_phrase_inton_vars(', start)
    block = text[start:end]
    marker = '    r0 = reloq_call_fifi_init_nuclwrd_vars(state, (int32_t)(FIELD(0)));\n'
    if block.count(marker) != 1 or text.count(include) != 1:
        raise RuntimeError('Cannot locate unique question tone insertion')
    block = block.replace(marker, marker + r'''
    /* The Spanish question choice is low nuclear accent, followed by high
       phrase and boundary tones. Keep all Finnish non-question choices. */
    if (persian_probe_question(state,STATE(int32_t,w52))) {
        STATE(int16_t,s271)=STATE(int16_t,s281);
        STATE(int16_t,s272)=STATE(int16_t,s280);
        STATE(int16_t,s273)=STATE(int16_t,s280);
    }
''', 1)
    text = (text[:start] + block + text[end:]).replace(include, include + helper, 1)
    destination = work / name
    destination.write_text(text, encoding='utf8')
    outputs.append(destination)

    # This copy already carries the independent Q patch. Extend it instead
    # of replacing it from the original provider source.
    name = 'delta_rules_c22_fifi.c'
    destination = work / name
    text = destination.read_text(encoding='utf8')
    start = text.index('int32_t reloq_fifi_fin_phrase_tone(')
    end = text.index('\nstatic int32_t evv_fin_phrase_tone(', start)
    block = text[start:end]
    phrase = 'L1809:;\n    r0 = reloq_call_fifi_insert_f0'
    if block.count(phrase) != 1 or text.count(include) != 1:
        raise RuntimeError('Cannot locate unique question phrase tone insertion')
    block = block.replace(phrase, r'''L1809:;
    if (persian_probe_spanish_rise() &&
        persian_probe_question(state,STATE(int32_t,w52)) &&
        STATE(int16_t,s280)==STATE(int16_t,s272)) {
        /* Spanish's high phrase target occurs earlier and substantially
           higher than Finnish's. In the reference phrase its targets are
           170 at 70%, followed by 190 at 99%. */
        AT(int16_t,-42)=70;
        int target=AT(int16_t,-50)+70;
        if (target>INT16_MAX) target=INT16_MAX;
        AT(int16_t,-50)=(int16_t)target;
    }
    r0 = reloq_call_fifi_insert_f0''', 1)
    text = (text[:start] + block + text[end:]).replace(include, include + helper, 1)
    destination.write_text(text, encoding='utf8')

    name = 'delta_rules_c01_fifi.c'
    text = (PROVIDER / 'lang' / 'fifi' / name).read_text(encoding='utf8')
    start = text.index('int32_t reloq_fifi_fin_boundary_tone(')
    end = text.index('\nstatic int32_t evv_fin_boundary_tone(', start)
    block = text[start:end]
    declaration = '    int32_t r0 = 0, r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0, r6 = 0, r7 = 0;\n'
    read = '    r0 = reloq61_typed_get_parm(DELTA_ENTRY_get_parm, state, (delta_loc *)(base + (-36)), EVV_AT(delta_loc *, (int32_t)PARAM(int32_t, 1)), (int16_t)(-4));\n'
    boundary = 'L478:;\n    r0 = reloq_call_fifi_insert_f0'
    if any(block.count(marker) != 1 for marker in (declaration, read, boundary)) or text.count(include) != 1:
        raise RuntimeError('Cannot locate unique question boundary insertion')
    block = block.replace(declaration, declaration + '    int16_t persian_boundary_base = 0;\n', 1)
    block = block.replace(read, read + '    persian_boundary_base = AT(int16_t, -34);\n', 1)
    block = block.replace(boundary, r'''L478:;
    if (persian_probe_question(state,STATE(int32_t,w52)) &&
        STATE(int16_t,s280)==STATE(int16_t,s273)) {
        /* Keep the earlier probe's 10/30 offsets for listening comparison.
           The stronger probe follows the Spanish 10/20 boundary offsets. */
        int increase=STATE(int16_t,s274)>=2 ?
            (persian_probe_spanish_rise() ? 20 : 30) : 10;
        int target=persian_boundary_base+increase;
        if (target>INT16_MAX) target=INT16_MAX;
        AT(int16_t,-34)=(int16_t)target;
    }
    r0 = reloq_call_fifi_insert_f0''', 1)
    text = (text[:start] + block + text[end:]).replace(include, include + helper, 1)
    destination = work / name
    destination.write_text(text, encoding='utf8')
    outputs.append(destination)
    return outputs



def prepare_vowel_start_duration(work: Path) -> None:
    """Match Finnish E and back A starting quantity to front A in the probe."""
    destination = work / 'delta_rules_c29_fifi.c'
    text = destination.read_text(encoding='utf8')
    begin = text.index('int32_t reloq_fifi_assign_stanfin_start_dur(')
    end = text.index('\nstatic int32_t evv_assign_stanfin_start_dur(', begin)
    section = text[begin:end]
    marker = 'L3292:;\n    STATE(int32_t, f33) = ((int32_t)reloq_local_200.i32);\n'
    if section.count(marker) != 1:
        raise RuntimeError('Cannot locate the Finnish starting vowel duration')
    section = section.replace(marker, marker + r'''
    {
        const char *mode=getenv("RELOQ_FA_PROBE_VOWEL");
        if (mode && mode[0]=='1' && !mode[1]) {
            delta_state *d=state;
            int32_t at=STATE(int32_t,w26),node=0;
            if (at) {
                int32_t *point=vmover(d,EVV_AT(int32_t *,at),2);
                if (point) node=vgetsc(d,0,0,EVV_REF(point),2);
            }
            if (node && !(*EVV_AT(int32_t *,node)&2)) {
                void *fields=TFLDS(EVV_AT(void *,node));
                unsigned phone=*(const unsigned char *)delta_record_field(&vstmtbl[2],0,fields);
                unsigned length=*(const unsigned char *)delta_record_field(&vstmtbl[2],11,fields);
                /* eSpeak assigns 160 to both /e/ and short /a/, and 200 to
                   back /A/. Equalize the Finnish starting values first; a
                   separate 125% back-A adjustment applies later. Preserve
                   the native rules for special diphthong contexts and every
                   later stress/phrase adjustment. Explicitly long Finnish
                   back A remains untouched for controlled comparisons. */
                if (phone==29 && length==1 && STATE(int32_t,f33)==850000)
                    STATE(int32_t,f33)=1000000;
                else if (phone==29 && length==0 && STATE(int32_t,f33)==1950000)
                    STATE(int32_t,f33)=2100000;
                else if (phone==34 && length==1 && STATE(int32_t,f33)==950000)
                    STATE(int32_t,f33)=1000000;
            }
        }
    }
''', 1)
    destination.write_text(text[:begin] + section + text[end:], encoding='utf8')


FINNISH_S_TRACE = r'''
    if (getenv("RELOQ_FA_TRACE_S")) {
        fprintf(stderr,"FA_S F=%d,%d,%d,%d,%d R=%d,%d,%d,%d,%d "
                "AF=%d BANDS=%d,%d,%d,%d,%d\n",
            STATE(int16_t,s448),STATE(int16_t,s450),STATE(int16_t,s452),
            STATE(int16_t,s454),STATE(int16_t,s456),
            STATE(int16_t,s449),STATE(int16_t,s451),STATE(int16_t,s453),
            STATE(int16_t,s455),STATE(int16_t,s457),STATE(int16_t,s404),
            STATE(int16_t,s409),STATE(int16_t,s410),STATE(int16_t,s411),
            STATE(int16_t,s412),STATE(int16_t,s413));
    }
'''


def prepare_s_noise(work: Path) -> None:
    """Select Persian S noise targets before Finnish schedules the noise."""
    destination = work / 'delta_rules_c13_fifi.c'
    text = destination.read_text(encoding='utf8')
    begin = text.index('int32_t reloq_fifi_fin_ph_s(')
    end = text.index('\nstatic int32_t evv_fin_ph_s(', begin)
    section = text[begin:end]
    marker = '    STATE(int16_t, s404) = ((int32_t)AT(int16_t, -16));\n'
    if section.count(marker) != 1:
        raise RuntimeError('Cannot locate the S noise target before scheduling')
    section = section.replace(marker, marker + r'''
    /* Only source and spectral noise amplitudes: keep Finnish formants and
       transitions. Mode 1 is the earlier Finnish "viisi" reference; mode 2
       uses the US English S amplitudes before normal noise modulation.
       Mode 3 also selects its dominant fifth noise-resonance target. */
    {
        const char *s=getenv("RELOQ_FA_PROBE_S");
        if (s && s[0]=='1' && !s[1]) {
            STATE(int16_t,s404)=52;
            STATE(int16_t,s409)=30;
            STATE(int16_t,s410)=50;
            STATE(int16_t,s411)=58;
            STATE(int16_t,s412)=63;
            STATE(int16_t,s413)=0;
        } else if (s && (s[0]=='2' || s[0]=='3') && !s[1]) {
            STATE(int16_t,s404)=65;
            STATE(int16_t,s409)=0;
            STATE(int16_t,s410)=0;
            STATE(int16_t,s411)=0;
            STATE(int16_t,s412)=50;
            STATE(int16_t,s413)=0;
            if (s[0]=='3') {
                STATE(int16_t,s456)=4300;
                STATE(int16_t,s457)=4300;
                const char *us=getenv("RELOQ_FA_US_ARTICULATION");
                if (us && us[0]=='1' && !us[1]) {
                    STATE(int16_t,s454)=4000;
                    STATE(int16_t,s455)=4000;
                }
            }
        }
    }
''', 1)
    destination.write_text(text[:begin] + section + text[end:], encoding='utf8')


def prepare_z_noise(work: Path) -> None:
    """Select voiced-sibilant noise without changing voicing or formants."""
    destination = work / 'delta_rules_c16_fifi.c'
    text = destination.read_text(encoding='utf8')
    begin = text.index('int32_t reloq_fifi_fin_ph_z(')
    end = text.index('\nstatic int32_t evv_fin_ph_z(', begin)
    section = text[begin:end]
    marker = '    STATE(int16_t, s404) = (40);\n'
    if section.count(marker) != 1:
        raise RuntimeError('Cannot locate the voiced S noise target')
    section = section.replace(marker, marker + r'''
    {
        const char *z=getenv("RELOQ_FA_PROBE_Z");
        /* Mode 3 retains the selected 49 source level and the noise bands
           measured before i, independently of the neighboring vowels. */
        if (z && z[0]=='1' && !z[1]) STATE(int16_t,s404)=46;
        else if (z && z[0]=='2' && !z[1]) STATE(int16_t,s404)=49;
        else if (z && z[0]=='3' && !z[1]) {
            STATE(int16_t,s404)=49;
            STATE(int16_t,s409)=30;
            STATE(int16_t,s410)=50;
            STATE(int16_t,s411)=58;
            STATE(int16_t,s412)=63;
            STATE(int16_t,s413)=0;
            const char *us=getenv("RELOQ_FA_US_ARTICULATION");
            if (us && us[0]=='1' && !us[1]) {
                /* The US voiced sibilant uses a different noise spectrum
                   from the earlier Finnish S-derived listening candidate. */
                STATE(int16_t,s404)=65;
                STATE(int16_t,s408)=0; STATE(int16_t,s409)=0;
                STATE(int16_t,s410)=0; STATE(int16_t,s411)=50;
                STATE(int16_t,s412)=50; STATE(int16_t,s413)=0;
                STATE(int16_t,s454)=4000; STATE(int16_t,s455)=4000;
                STATE(int16_t,s456)=4300; STATE(int16_t,s457)=4300;
            }
        }
    }
''', 1)
    voiced = '    STATE(int16_t, s402) = (45);\n'
    if section.count(voiced) != 1:
        raise RuntimeError('Cannot locate voiced S voicing target')
    section = section.replace(voiced, voiced + r'''    {
        const char *us=getenv("RELOQ_FA_US_ARTICULATION");
        if (us && us[0]=='1' && !us[1]) STATE(int16_t,s402)=33;
    }
''', 1)
    destination.write_text(text[:begin] + section + text[end:], encoding='utf8')


def prepare_vowel_duration_probe(work: Path) -> Path:
    """Trace Finnish timing and try eSpeak's relative A base in this probe."""
    name = 'delta_rules_c08_fifi.c'
    text = (PROVIDER / 'lang' / 'fifi' / name).read_text(encoding='utf8')
    begin = text.index('int32_t reloq_fifi_distribute_nucdur(')
    end = text.index('\nstatic int32_t evv_distribute_nucdur(', begin)
    section = text[begin:end]
    marker = '    STATE(int32_t, f33) = ((int32_t)AT(int32_t, -12));'
    if section.count(marker) != 1:
        raise RuntimeError('Cannot locate Finnish nucleus duration')
    section = section.replace(marker, marker + r'''
    {
        const char *mode=getenv("RELOQ_FA_PROBE_VOWEL");
        int enabled=mode && mode[0]=='1' && !mode[1];
        const char *timing=getenv("RELOQ_FA_PROBE_TIMING");
        int short_unstressed=timing && timing[0]=='1' && !timing[1];
        const char *us=getenv("RELOQ_FA_US_ARTICULATION");
        int us_timing=us && us[0]=='1' && !us[1];
        int tracing=getenv("RELOQ_FA_TRACE_NUC")!=NULL;
        const char *end_mode=getenv("RELOQ_FA_PROBE_FINAL");
        int medium=end_mode && end_mode[0]=='1' && !end_mode[1];
        if (enabled || tracing || medium || short_unstressed || us_timing) {
            delta_state *d=state;
            int32_t at=STATE(int32_t,w26),node=0;
            if (at) {
                int32_t *point=vmover(d,EVV_AT(int32_t *,at),2);
                if (point) node=vgetsc(d,0,0,EVV_REF(point),2);
            }
            int phone=-1,stress=-1,length=-1,cls=-1;
            if (node && !(*EVV_AT(int32_t *,node)&2)) {
                void *fields=TFLDS(EVV_AT(void *,node));
                phone=*(const unsigned char *)delta_record_field(&vstmtbl[2],0,fields);
                cls=*(const unsigned char *)delta_record_field(&vstmtbl[2],1,fields);
                int16_t stress16=0;
                memcpy(&stress16,delta_record_field(&vstmtbl[2],7,fields),sizeof stress16);
                stress=stress16;
                length=*(const unsigned char *)delta_record_field(&vstmtbl[2],11,fields);
            }
            int32_t before=STATE(int32_t,f33);
            /* eSpeak's Farsi source uses 200 vs 160 nominal length units
               for back A vs front a, before its own context adjustments.
               This optional 125% Finnish-nucleus approximation is a hearing
               candidate, not an empirically fixed Persian duration law.
               Explicit Finnish long-A test tags retain their original timing. */
            if (enabled && phone==34 && cls==1 && length==1 && before>0) {
                int64_t scaled=((int64_t)before*5+2)/4;
                STATE(int32_t,f33)=scaled>INT32_MAX ? INT32_MAX : (int32_t)scaled;
            }
            /* A private listening candidate for E/A at a real word end.
               Factors interpolate the previously compared native short and
               long examples while retaining Finnish stress/phrase timing.
               A before /n/ in Iran and E inside sefr must remain unchanged. */
            int final=medium && cls==1 && (phone==29 || phone==34) &&
                      persian_probe_vowel_word_final(state,STATE_AT(w26,-4),phone);
            if (final && STATE(int32_t,f33)>0 && (length==0 || length==1)) {
                int factor=phone==29 ? (length==0 ? 734 : 1566)
                                     : (length==0 ? 820 : 1280);
                int64_t scaled=((int64_t)STATE(int32_t,f33)*factor+500)/1000;
                STATE(int32_t,f33)=scaled>INT32_MAX ? INT32_MAX : (int32_t)scaled;
            }
            /* Listening candidate, not a universal Persian duration rule:
               shorten only short, unstressed, word-internal vowels by 10%.
               Leave accepted word-final vowels, long markers, stressed vowels
               and every consonant's own timing untouched. */
            if (short_unstressed && cls==1 && stress==0 && length==1 &&
                (phone==27 || phone==29 || phone==31 || phone==32 || phone==33 || phone==34) &&
                STATE(int32_t,f33)>0 &&
                !persian_probe_vowel_word_final(state,STATE_AT(w26,-4),(unsigned)phone)) {
                int64_t scaled=((int64_t)STATE(int32_t,f33)*9+5)/10;
                STATE(int32_t,f33)=scaled>INT32_MAX ? INT32_MAX : (int32_t)scaled;
            }
            int short_yek=0;
            if (us_timing && node && cls==1 && STATE(int32_t,f33)>0 &&
                !persian_probe_vowel_word_final(state,STATE_AT(w26,-4),(unsigned)phone)) {
                int32_t next_ref=EVV_AT(int32_t *,node)[1]&~3;
                int32_t *point=next_ref ? vmover(d,EVV_AT(int32_t *,next_ref),2) : NULL;
                int32_t next=point ? vgetsc(d,0,0,EVV_REF(point),2) : 0;
                unsigned next_phone=0,next_marker=0;
                if (next && !(*EVV_AT(int32_t *,next)&2)) {
                    void *fields=TFLDS(EVV_AT(void *,next));
                    next_phone=*(const unsigned char *)delta_record_field(&vstmtbl[2],0,fields);
                    next_marker=*(const unsigned char *)delta_record_field(&vstmtbl[2],12,fields);
                }
                int ordinary=next_phone==1 || next_phone==2 || next_phone==7 ||
                    next_phone==9 || next_phone==11 ||
                    (next_phone==3 && next_marker!=3 && next_marker!=4 && next_marker!=5);
                /* The short vowel in yek (one), followed by an ordinary
                   word-final /k/, sounded too long. Do not change its stop
                   or the same vowel when a syllable follows, as in yeki. */
                if (phone==29 && length==1 && ordinary && next_phone==3 && at) {
                    delta_node *previous_point=vmovel(d,EVV_AT(delta_node *,at),2);
                    int32_t previous=previous_point ?
                        vgetsc(d,1,0,EVV_REF(previous_point),2) : 0;
                    unsigned previous_phone=previous && !(*EVV_AT(int32_t *,previous)&2) ?
                        *(const unsigned char *)delta_record_field(&vstmtbl[2],0,
                            TFLDS(EVV_AT(void *,previous))) : 0;
                    short_yek=previous_phone==22 &&
                        persian_probe_next_phone_word_final(state,next_ref,3);
                }
                int factor=100;
                if (ordinary && length==1 && phone==31) factor=130;
                else if (ordinary && length==1 && phone==29) factor=118;
                if (factor!=100) {
                    int64_t scaled=((int64_t)STATE(int32_t,f33)*factor+50)/100;
                    STATE(int32_t,f33)=scaled>INT32_MAX ? INT32_MAX : (int32_t)scaled;
                }
            }
            /* Keep most of the listener-approved duration extension, but
               bound its absolute contribution so longer vowels are less
               mechanically stretched. Give the yek context a smaller boost.
               These are listening candidates, not a Persian timing law. */
            if (us_timing && node && cls==1 && STATE(int32_t,f33)>0) {
                int64_t duration=STATE(int32_t,f33);
                int64_t extra=short_yek ? (duration+10)/20 : (duration+2)/5;
                if (extra>150000) extra=150000;
                int64_t scaled=duration+extra;
                STATE(int32_t,f33)=scaled>INT32_MAX ? INT32_MAX : (int32_t)scaled;
            }
            if (tracing)
                fprintf(stderr,"FA_NUC phone=%d class=%d stress=%d length=%d before=%d duration=%d final=%d yek=%d\n",
                        phone,cls,stress,length,(int)before,(int)STATE(int32_t,f33),final,short_yek);
        }
    }
''', 1)
    helper = FINNISH_L_WORD_FINAL.replace(
        'persian_probe_l_word_final(void *state,int32_t token_ref)',
        'persian_probe_vowel_word_final(void *state,int32_t token_ref,unsigned phone)'
    ).replace('testFldeq(d,2,0,21)', 'testFldeq(d,2,0,phone)')
    # next_ref is the scanner node value, not the address of a delta_token.
    # Construct an actual token so lpta_loadp reads the intended /k/ node.
    next_helper = helper.replace(
        'persian_probe_vowel_word_final(void *state,int32_t token_ref,unsigned phone)',
        'persian_probe_next_phone_word_final(void *state,int32_t token_ref,unsigned phone)'
    ).replace('    lpta_loadp(d,EVV_AT(const delta_token *,token_ref));',
              '    const delta_token token={0,token_ref};\n    lpta_loadp(d,&token);')
    text = text[:begin] + helper + '\n' + next_helper + '\n' + section + text[end:]
    include = '#include "reloq61_rule_calls_fifi.h"\n'
    if text.count(include) != 1:
        raise RuntimeError('Cannot locate duration probe includes')
    text = text.replace(include, include + '#include <stdlib.h>\n#include <stdio.h>\n#include <stdint.h>\n', 1)
    dest = work / name
    dest.write_text(text, encoding='utf8')
    return dest



def prepare_e_context_probe(work: Path) -> Path:
    """Trace and optionally stabilize Finnish E in the disposable library."""
    destination = work / 'delta_rules_c17_fifi.c'
    text = destination.read_text(encoding='utf8')
    marker = 'int32_t reloq_fifi_stanfin_ph_e(void *state, int32_t p0)\n{\n'
    helper = r'''
#include <stdlib.h>
/* Private diagnostic: inspect the preceding/following phoneme without
   modifying Finnish's vowel timing or any ordinary language build. */
static unsigned persian_probe_e_neighbour(void *state,int32_t position,int backwards)
{
    delta_state *d=state;
    if(!d||!position)return 0;
    void *point=backwards ? (void *)vmovel(d,EVV_AT(delta_node *,position),2) :
                            (void *)vmover(d,EVV_AT(int32_t *,position),2);
    int32_t node=point ? vgetsc(d,backwards,0,EVV_REF(point),2) : 0;
    if(!node||(*EVV_AT(int32_t *,node)&2))return 0;
    return *(const unsigned char *)delta_record_field(&vstmtbl[2],0,TFLDS(EVV_AT(void *,node)));
}
'''
    if text.count(marker) != 1:
        raise RuntimeError('Cannot find Finnish E rule')
    text = text.replace(marker, helper + '\n' + marker, 1)
    begin = text.index(marker)
    end = text.index('\nstatic int32_t evv_stanfin_ph_e(', begin)
    block = text[begin:end]
    exit_point = 'L8178:;\n    r0 = reloq61_typed_succeed'
    if block.count(exit_point) != 1:
        raise RuntimeError('Cannot find Finnish E rule exit')
    block = block.replace(exit_point, r'''L8178:;
    {
        const char *mode=getenv("RELOQ_FA_E_MODE"), *context=getenv("RELOQ_FA_E_CONTEXT");
        const char *target=getenv("RELOQ_FA_E_TARGET");
        unsigned before=persian_probe_e_neighbour(state,STATE(int32_t,w60),1);
        unsigned after=persian_probe_e_neighbour(state,STATE(int32_t,w61),0);
        if(getenv("RELOQ_FA_TRACE_E"))
            fprintf(stderr,"FA_E prev=%u next=%u F=%d,%d,%d,%d,%d R=%d,%d,%d,%d,%d B=%d,%d,%d,%d,%d\n",
                before,after,STATE(int16_t,s448),STATE(int16_t,s450),STATE(int16_t,s452),
                STATE(int16_t,s454),STATE(int16_t,s456),STATE(int16_t,s449),STATE(int16_t,s451),
                STATE(int16_t,s453),STATE(int16_t,s455),STATE(int16_t,s457),
                STATE(int16_t,s397),STATE(int16_t,s398),STATE(int16_t,s399),
                STATE(int16_t,s400),STATE(int16_t,s401));
        int f1,f2,f3,end=0,neighbour=context ? atoi(context) : 0;
        int selected=neighbour==0 || (neighbour>0 && neighbour<=255 &&
                     (before==(unsigned)neighbour || after==(unsigned)neighbour));
        if(mode && !mode[1] && (mode[0]=='1' || mode[0]=='2') && selected && target &&
           sscanf(target,"%d,%d,%d%n",&f1,&f2,&f3,&end)==3 && !target[end] &&
           f1>=200 && f1<=1200 && f2>=500 && f2<=3000 && f3>=1500 && f3<=4000) {
            int values[3]={f1,f2,f3};
            int16_t *ends[6]={&STATE(int16_t,s448),&STATE(int16_t,s449),
                              &STATE(int16_t,s450),&STATE(int16_t,s451),
                              &STATE(int16_t,s452),&STATE(int16_t,s453)};
            for(int i=0;i<6;i++) {
                int goal=values[i/2],current=*ends[i];
                if(mode[0]=='1') *ends[i]=(int16_t)goal;
                else if(current>0) *ends[i]=(int16_t)((current+goal+1)/2);
            }
        }
    }
    r0 = reloq61_typed_succeed''', 1)
    destination.write_text(text[:begin] + block + text[end:], encoding='utf8')
    return destination


def us_consonant_rules() -> str:
    """Adapt US stop, fricative and nasal rules to Persian phone records.

    Keep the donor control flow and source targets.  English's phone and
    feature enums differ from Finnish's even when their names match, so
    compare against the local record by name instead of copying numeric IDs.
    """
    root = PROVIDER / 'lang' / 'enus61'
    source = (root / 'delta_authored_enus61.c').read_text(encoding='utf8')
    source = source.split('uint8_t enus61_evv_authored_data[] = {', 1)[1].split('};', 1)[0]
    source = re.sub(r'/\*.*?\*/', '', source, flags=re.S)
    raw = bytes(int(value, 0) for value in
                re.findall(r'(?<![A-Za-z_])(?:0[xX][0-9a-fA-F]+|[0-9]+)', source))
    table = (root / 'delta_rules_enus61.c').read_text(encoding='utf8')
    table = table.split('enus61_delta_rule_sym[] = {', 1)[1].split('};', 1)[0]
    offsets = [int(value) for value in
               re.findall(r'enus61_evv_authored_data \+ (\d+)', table)]
    indices, width = tuple(range(2106, 2247)) + tuple(range(4007, 4029)), 32
    index_map = {value: index for index, value in enumerate(indices)}
    if len(offsets) <= max(indices) or any(offsets[i] + width > len(raw) for i in indices):
        raise RuntimeError('US stop symbol storage changed')
    constants = ',\n'.join('    {' + ','.join(str(x) for x in raw[offsets[i]:offsets[i]+width]) + '}'
                           for i in indices)
    helper = r'''
#include <stdatomic.h>
extern delta_stmt enus61_vstmtbl[];
extern int32_t reloq_fifi_do_voice_bar(void *,int32_t);
static const unsigned char persian_us_constants[][32]={
__CONSTANTS__
};
static _Atomic int32_t persian_us_constants_ref;
static int32_t persian_us_symbols(void)
{
    int32_t result=atomic_load_explicit(&persian_us_constants_ref,memory_order_acquire);
    if (result) return result;
    void *copy=evv_arena_alloc(sizeof persian_us_constants);
    if (!copy) return 0;
    memcpy(copy,persian_us_constants,sizeof persian_us_constants);
    int32_t made=EVV_REF(copy), expected=0;
    if (!atomic_compare_exchange_strong_explicit(&persian_us_constants_ref,&expected,made,
                                                  memory_order_release,memory_order_acquire)) {
        evv_arena_free(copy);
        return expected;
    }
    return made;
}
/* Match field names and their enum values, rather than interpreting an
   English numeric feature as though it belonged to a Finnish phone. */
static int persian_us_local_field(unsigned st,unsigned field)
{
    if (st>8 || field>=(unsigned)enus61_vstmtbl[st].nfields) return -1;
    const char *name=enus61_vstmtbl[st].fields[field].name;
    if (!name) return -1;
    for (int i=0;i<vstmtbl[st].nfields;i++)
        if (vstmtbl[st].fields[i].name && !strcmp(name,vstmtbl[st].fields[i].name)) return i;
    return -1;
}
static int persian_us_value(unsigned st,unsigned field,unsigned value,int target)
{
    const delta_fielddesc *src=&enus61_vstmtbl[st].fields[field];
    const delta_fielddesc *dst=&vstmtbl[st].fields[target];
    if (!src->values || !dst->values) return (int)value;
    if (value>=(unsigned)src->nvalues) return -1;
    const char *name=((const char *const *)src->values)[value];
    if (!name) return -1;
    if (st==2 && field==0) {
        if (!strcmp(name,"y")) name="j";
        else if (!strcmp(name,"E")) name="e";
        else if (!strcmp(name,"c")) name="o";
        else if (!strcmp(name,"G")) return -1; /* Persian G is /x/. */
    }
    if (st==2 && !strcmp(src->name,"diaph_ghost")) {
        if (!strcmp(name,"+t")) name="+C";
        else if (!strcmp(name,"+d")) name="+J";
    }
    for (int i=0;i<dst->nvalues;i++) {
        const char *candidate=((const char *const *)dst->values)[i];
        if (candidate && !strcmp(name,candidate)) return i;
    }
    return -1;
}
static int32_t persian_us_field(void *state,uint8_t st,uint8_t field,uint8_t value)
{
    if (st>8 || field>=(unsigned)enus61_vstmtbl[st].nfields) return 1;
    int local=persian_us_local_field(st,field);
    if (local<0) {
        /* Persian's records have no English contraction/origin/flap flags. */
        const char *name=enus61_vstmtbl[st].fields[field].name;
        return value==0 && name && (!strcmp(name,"contrac") || !strcmp(name,"origin") ||
                                   !strcmp(name,"number") || !strcmp(name,"pl_type")) ? 0 : 1;
    }
    int mapped=persian_us_value(st,field,value,local);
    return mapped<0 ? 1 : reloq61_typed_testFldeq(DELTA_ENTRY_testFldeq,state,st,(uint8_t)local,(uint8_t)mapped);
}
static int32_t persian_us_string(void *state,uint8_t st,uint8_t count,const uint8_t *source)
{
    if (st>8 || !source || count>32) return 1;
    uint8_t mapped[32];
    int local=persian_us_local_field(st,0);
    if (local!=0) return 1;
    for (unsigned i=0;i<count;i++) {
        int value=persian_us_value(st,0,source[i],0);
        if (value<0 || value>255) return 1;
        mapped[i]=(uint8_t)value;
    }
    /* The short temporary is ordinary C storage; do not turn it into an
       arena reference just to perform the native string comparison. */
    return test_string_s(state,st,count,mapped);
}
static int32_t persian_us_npush_field(void *state,uint8_t st,uint8_t field)
{
    int local=persian_us_local_field(st,field);
    return local<0 ? 1 : reloq61_typed_npush_fld(DELTA_ENTRY_npush_fld,state,st,(uint8_t)local);
}
static int32_t persian_us_mark(void *state,uint8_t st,uint8_t field,uint8_t value,uint8_t where)
{
    int local=persian_us_local_field(st,field);
    int mapped=local<0 ? -1 : persian_us_value(st,field,value,local);
    return mapped<0 ? 1 : reloq61_typed_mark_s(DELTA_ENTRY_mark_s,state,st,
                                                (uint8_t)local,(uint8_t)mapped,where);
}
'''.replace('__CONSTANTS__', constants)
    helpers = ('eng_lab_Fv', 'eng_alv_Fv', 'eng_vel_Fv', 'eng_pal_Fv')
    nasal_formants = ('ga_ph_m', 'ga_ph_n')
    acoustic = tuple('eng_ph_' + phone for phone in 'ptkbdgfvszSZmn')
    durations = tuple('eng_ph_' + phone + '_dur' for phone in 'ptkbdgfvszSZmn')
    exported = durations + ('get_eng_stop_trans_dur', 'get_eng_fric_trans_dur',
                            'get_eng_nas_trans_dur', 'eng_trans_features')
    names = helpers + nasal_formants + acoustic + exported
    pieces = []
    for name in names:
        matches = []
        for path in root.glob('delta_rules_c*.c'):
            text = path.read_text(encoding='utf8')
            start = text.find('int32_t reloq_enus61_' + name + '(')
            if start >= 0:
                end = text.index('\nstatic int32_t evv_' + name + '(', start)
                matches.append(text[start:end])
        if len(matches) != 1:
            raise RuntimeError('Cannot locate US stop donor ' + name)
        visibility = '' if name in exported else 'static '
        block = matches[0].replace('int32_t reloq_enus61_' + name + '(',
                                   visibility + 'int32_t persian_us_' + name + '(', 1)
        used = {int(x) for x in re.findall(r'delta_sym_ref\[(\d+)\]', block)}
        if not used or not used <= index_map.keys():
            raise RuntimeError('US stop symbol range changed: ' + name)
        block = re.sub(r'delta_sym_ref\[(\d+)\]',
                       lambda m: f'(persian_us_refs + {index_map[int(m.group(1))]*width})', block)
        marker = '    unsigned char *frame = evv_frame_push('
        if block.count(marker) != 1:
            raise RuntimeError('Cannot locate US stop frame ' + name)
        block = block.replace(marker,
                              '    const int32_t persian_us_refs=persian_us_symbols();\n'
                              '    if (!persian_us_refs) return 94;\n' + marker, 1)
        block = block.replace('reloq61_typed_testFldeq(DELTA_ENTRY_testFldeq, state,',
                              'persian_us_field(state,')
        block = block.replace('reloq61_typed_test_string_s(DELTA_ENTRY_test_string_s, state,',
                              'persian_us_string(state,')
        block = block.replace('reloq61_typed_npush_fld(DELTA_ENTRY_npush_fld, state,',
                              'persian_us_npush_field(state,')
        block = block.replace('reloq61_typed_mark_s(DELTA_ENTRY_mark_s, state,',
                              'persian_us_mark(state,')
        if name == 'get_eng_nas_trans_dur':
            # English has two language-specific state values for this one
            # contextual branch. Finnish/Persian has neither; choose the
            # donor's ordinary alternative rather than reading unrelated
            # bytes from the Finnish state layout.
            absent = ('    r1 = ((int32_t)STATE(uint16_t, s322));\n'
                      '    AT(int32_t, -20) = (r1);\n'
                      '    if (AT(int16_t, -20) == (int16_t)(STATE(int16_t, s318))) goto L1991;\n'
                      '    goto L3865;\n')
            if block.count(absent) != 1:
                raise RuntimeError('Cannot isolate the English-only nasal context')
            block = block.replace(absent, '    goto L3865;\n', 1)
        for helper_name in helpers:
            block = block.replace('reloq_call_enus61_' + helper_name + '(',
                                  'persian_us_' + helper_name + '(')
        # These three f61_06 helpers have equivalent Finnish implementations;
        # use their local scheduler and feature enums for the shared streams.
        for helper_name in ('modulate_noise', 'modulate_nas_AV', 'final_voiced_release'):
            block = block.replace('reloq_call_enus61_' + helper_name + '(',
                                  'reloq_call_fifi_' + helper_name + '(')
        block = block.replace('reloq_call_enus61_do_voice_bar(', 'reloq_fifi_do_voice_bar(')
        if 'reloq_call_enus61_' in block:
            raise RuntimeError('Unmapped US stop dependency: ' + name)
        pieces.append(block)
    return helper + '\n' + '\n'.join(pieces)


def prepare_us_articulation(work: Path) -> None:
    """Adapt Persian vowel resonances and ordinary US-like consonants."""
    vowel_file = work / 'delta_rules_c08_fifi.c'
    text = vowel_file.read_text(encoding='utf8')
    # The British sad rule sets F1-F3 but inherits the voice's higher
    # formants and bandwidths. Save those defaults before Finnish's /A/
    # rule can overwrite them, then restore them for the Persian candidate.
    # This also preserves the way the donor adapts to gender and head size.
    begin = text.index('int32_t reloq_fifi_stanfin_vals(')
    end = text.index('\nstatic int32_t evv_stanfin_vals(', begin)
    section = text[begin:end]
    front_call = '    r0 = reloq_call_fifi_stanfin_ph_A(state, (int32_t)(FIELD(0)));\n'
    if section.count(front_call) != 1:
        raise RuntimeError('Cannot locate Persian front-vowel dispatcher')
    section = section.replace(front_call, r'''    {
        const char *british=getenv("RELOQ_FA_BRITISH_FRONT_A");
        int selected=british && british[0]=='1' && !british[1];
        int16_t b1=STATE(int16_t,s397),b2=STATE(int16_t,s398),b3=STATE(int16_t,s399);
        int16_t b4=STATE(int16_t,s400),b5=STATE(int16_t,s401);
        int16_t f4=STATE(int16_t,s454),f4_end=STATE(int16_t,s455);
        int16_t f5=STATE(int16_t,s456),f5_end=STATE(int16_t,s457);
        r0 = reloq_call_fifi_stanfin_ph_A(state, (int32_t)(FIELD(0)));
        if (selected && !r0) {
            STATE(int16_t,s397)=b1; STATE(int16_t,s398)=b2;
            STATE(int16_t,s399)=b3; STATE(int16_t,s400)=b4; STATE(int16_t,s401)=b5;
            STATE(int16_t,s454)=f4; STATE(int16_t,s455)=f4_end;
            STATE(int16_t,s456)=f5; STATE(int16_t,s457)=f5_end;
        }
    }
''', 1)
    text = text[:begin] + section + text[end:]
    # Apply this only to the accepted long back-A rule. B1/B2 are bandwidths,
    # not formant frequencies: a
    # modest increase broadens/damps the two resonances without shifting their
    # centres. Keep non-positive native inheritance markers unchanged.
    for phone in ('a',):
        call = f'    r0 = reloq_call_fifi_stanfin_ph_{phone}(state, (int32_t)(FIELD(0)));\n'
        # A few rules can call a vowel outside the normal vowel dispatcher.
        # Restrict this insertion to the dispatch function itself.
        begin = text.index('int32_t reloq_fifi_stanfin_vals(')
        end = text.index('\nstatic int32_t evv_stanfin_vals(', begin)
        section = text[begin:end]
        if section.count(call) != 1:
            raise RuntimeError(f'Cannot locate Persian vowel dispatcher for {phone}')
        section = section.replace(call, call + r'''    {
        const char *fa_damp=getenv("RELOQ_FA_VOWEL_DAMPING");
        if (!r0 && fa_damp && fa_damp[0]=='1' && !fa_damp[1]) {
            if (STATE(int16_t,s397)>0 && STATE(int16_t,s397)<3000)
                STATE(int16_t,s397)=(int16_t)((5*STATE(int16_t,s397)+2)/4);
            if (STATE(int16_t,s398)>0 && STATE(int16_t,s398)<3000)
                STATE(int16_t,s398)=(int16_t)((5*STATE(int16_t,s398)+2)/4);
        }
    }
''', 1)
        text = text[:begin] + section + text[end:]
    # Apply after the phoneme-specific rule so all of its ordinary contextual
    # endpoints and earlier accepted back-A bandwidths remain in force.
    # These are listening candidates from matched-F0 spectral comparisons,
    # not a global EQ or a substitution of English vowel identities.
    vowel_helper = r'''
static void persian_open_vowel(void *state,int phone)
{
    const char *us=getenv("RELOQ_FA_US_ARTICULATION");
    if (!us || us[0]!='1' || us[1]) return;
    int16_t female=0;
    memcpy(&female,(const unsigned char *)state+DG_BASE+3092+12,sizeof female);
    int f2=0,f3=0,f4=0,f5=0,b1=0,b3=0,b5=0;
    /* Use the /o/ nucleus and source targets from "two" in every context.
       Do not replace inheritance sentinels: the acoustic engine still
       interpolates into and out of neighbouring consonants. Other vowels
       retain their own centres, and /u/ keeps its restored dev121 centres. */
    if (phone=='a') { f3=female ? -100 : 0; f4=200; b3=100; }
    else if (phone=='e') { f3=female ? -80 : 0; f4=50; b1=35; b3=125; }
    else if (phone=='i') { f3=female ? -80 : 0; f4=50; b3=100; }
    else if (phone=='o') {
        if (STATE(int16_t,s448)>0) STATE(int16_t,s448)=440;
        if (STATE(int16_t,s449)>0) STATE(int16_t,s449)=440;
        if (STATE(int16_t,s450)>0) STATE(int16_t,s450)=900;
        if (STATE(int16_t,s451)>0) STATE(int16_t,s451)=900;
        if (STATE(int16_t,s452)>0) STATE(int16_t,s452)=2400;
        if (STATE(int16_t,s453)>0) STATE(int16_t,s453)=2400;
        if (STATE(int16_t,s454)>0) STATE(int16_t,s454)=3500;
        if (STATE(int16_t,s455)>0) STATE(int16_t,s455)=3500;
        b3=120;
        STATE(int16_t,s405)=0; /* TL target from "two" */
        STATE(int16_t,s436)=0; /* emit the vowel's own TL target */
    }
    else if (phone=='u') { f4=200; b3=120; b5=260; }
    if (b1 && STATE(int16_t,s397)>0 && STATE(int16_t,s397)<3000)
        STATE(int16_t,s397)+=b1;
    if (STATE(int16_t,s450)>0) STATE(int16_t,s450)+=f2;
    if (STATE(int16_t,s451)>0) STATE(int16_t,s451)+=f2;
    if (STATE(int16_t,s452)>150) STATE(int16_t,s452)+=f3;
    if (STATE(int16_t,s453)>150) STATE(int16_t,s453)+=f3;
    if (STATE(int16_t,s454)>0) STATE(int16_t,s454)+=f4;
    if (STATE(int16_t,s455)>0) STATE(int16_t,s455)+=f4;
    if (STATE(int16_t,s456)>0) STATE(int16_t,s456)+=f5;
    if (STATE(int16_t,s457)>0) STATE(int16_t,s457)+=f5;
    if (b3 && STATE(int16_t,s399)>0) STATE(int16_t,s399)=b3;
    if (b5 && STATE(int16_t,s401)>0) STATE(int16_t,s401)=b5;
}
'''
    dispatcher = 'int32_t reloq_fifi_stanfin_vals(void *state, int32_t p0)\n{\n'
    if text.count(dispatcher) != 1:
        raise RuntimeError('Cannot locate the Persian vowel dispatcher')
    text = text.replace(dispatcher, vowel_helper + '\n' + dispatcher, 1)
    begin = text.index(dispatcher)
    end = text.index('\nstatic int32_t evv_stanfin_vals(', begin)
    section = text[begin:end]
    for phone in ('a', 'e', 'i', 'o', 'u'):
        call = f'    r0 = reloq_call_fifi_stanfin_ph_{phone}(state, (int32_t)(FIELD(0)));\n'
        if section.count(call) != 1:
            raise RuntimeError(f'Cannot locate Persian vowel dispatcher for {phone}')
        section = section.replace(call, call +
            f"    if (!r0) persian_open_vowel(state,'{phone}');\n", 1)
    text = text[:begin] + section + text[end:]

    # /v/ is constructed repeatedly during native backtracking. Its noise
    # and bandwidths must be set inside the actual rule rather than merely
    # after the dispatch call has returned.
    begin = text.index('int32_t reloq_fifi_fin_ph_v(')
    end = text.index('\nstatic int32_t evv_fin_ph_v(', begin)
    section = text[begin:end]
    marker = 'L564:;\n    STATE(int16_t, s402) = (36);\n'
    if section.count(marker) != 1:
        raise RuntimeError('Cannot locate Persian voiced labial fricative')
    section = section.replace(marker, marker + r'''    {
        const char *us=getenv("RELOQ_FA_US_ARTICULATION");
        if (us && us[0]=='1' && !us[1]) {
            STATE(int16_t,s404)=60;
            STATE(int16_t,s413)=60;
            STATE(int16_t,s398)=95; STATE(int16_t,s399)=125;
            STATE(int16_t,s402)=33;
        }
    }
''', 1)
    text = text[:begin] + section + text[end:]
    vowel_file.write_text(text, encoding='utf8')

    consonant_file = work / 'delta_rules_c02_fifi.c'
    text = consonant_file.read_text(encoding='utf8')
    include = '#include "reloq61_rule_calls_fifi.h"\n'
    helper = r'''
/* Current K can also carry Persian uvular or glottal phones. Do not modify
   those carriers while adapting the ordinary velar stop. */
static int persian_articulation_special_k(void *state)
{
    delta_state *d=state;
    int32_t position=STATE(int32_t,w61);
    if (!d || !position) return 1;
    delta_node *point=vmovel(d,EVV_AT(delta_node *,position),2);
    int32_t node=point ? vgetsc(d,1,0,EVV_REF(point),2) : 0;
    if (!node || (*EVV_AT(int32_t *,node)&2)) return 1;
    void *fields=TFLDS(EVV_AT(void *,node));
    unsigned phone=*(const unsigned char *)delta_record_field(&vstmtbl[2],0,fields);
    unsigned marker=*(const unsigned char *)delta_record_field(&vstmtbl[2],12,fields);
    return phone!=3 || marker==3 || marker==4 || marker==5;
}
static unsigned persian_articulation_following(void *state)
{
    delta_state *d=state;
    int32_t position=STATE(int32_t,w61);
    if (!d || !position) return 0;
    void *point=(void *)vmover(d,EVV_AT(int32_t *,position),2);
    int32_t node=point ? vgetsc(d,0,0,EVV_REF(point),2) : 0;
    if (!node || (*EVV_AT(int32_t *,node)&2)) return 0;
    return *(const unsigned char *)delta_record_field(&vstmtbl[2],0,TFLDS(EVV_AT(void *,node)));
}
static int16_t persian_articulation_toward(int16_t value,int target)
{
    /* Preserve native inheritance markers and two thirds of each contextual
       target; this is an auditable listening candidate, not a rule that all
       Persian allophones equal the English allophones. */
    return value>0 ? (int16_t)((2*(int)value+target+1)/3) : value;
}
static void persian_articulation_nasal(void *state,int second_formant)
{
    const char *mode=getenv("RELOQ_FA_US_ARTICULATION");
    if (!mode || mode[0]!='1' || mode[1]) return;
    /* US ga_ph_m/n: F1 300, F2 1200/1600, B2 200 and nasal pole/zero
       200/320. Blend only explicit positive values so Finnish's contextual
       endpoints and inheritance remain meaningful. The separate back-A
       nasal experiment is not enabled by this nasal-consonant adjustment. */
    STATE(int16_t,s448)=persian_articulation_toward(STATE(int16_t,s448),300);
    STATE(int16_t,s449)=persian_articulation_toward(STATE(int16_t,s449),300);
    STATE(int16_t,s450)=persian_articulation_toward(STATE(int16_t,s450),second_formant);
    STATE(int16_t,s451)=persian_articulation_toward(STATE(int16_t,s451),second_formant);
    STATE(int16_t,s398)=persian_articulation_toward(STATE(int16_t,s398),200);
    STATE(int16_t,s406)=persian_articulation_toward(STATE(int16_t,s406),200);
    STATE(int16_t,s407)=persian_articulation_toward(STATE(int16_t,s407),320);
}
'''
    if text.count(include) != 1:
        raise RuntimeError('Cannot locate consonant dispatcher includes')
    text = text.replace(include, include + '#include <stdlib.h>\n', 1)
    marker = 'int32_t reloq_fifi_fin_con_vals(void *state, int32_t p0)\n{\n'
    if text.count(marker) != 1:
        raise RuntimeError('Cannot locate consonant dispatcher')
    # STATE offsets are declared after the includes in generated sources.
    text = text.replace(marker, us_consonant_rules() + '\n' + helper + '\n' + marker, 1)
    for phone in 'ptkbdgfvszSZ':
        call = f'    r0 = reloq_call_fifi_fin_ph_{phone}(state, (int32_t)(FIELD(0)));\n'
        if text.count(call) != 1:
            raise RuntimeError(f'Cannot locate Persian consonant dispatcher for {phone}')
        condition = (' && !persian_articulation_special_k(state)' if phone == 'k' else '')
        key = 'RELOQ_FA_US_STOPS' if phone in 'ptkbdg' else 'RELOQ_FA_US_FRICATIVES'
        replacement = ('    {\n' + f'        const char *us=getenv("{key}");\n' +
                       f"        if (us && us[0]=='1' && !us[1]{condition})\n" +
                       f'            r0=persian_us_eng_ph_{phone}(state,(int32_t)(FIELD(0)));\n' +
                       '        else\n' + call + '    }\n')
        text = text.replace(call, replacement, 1)
    for phone, target in (('m', 1200), ('n', 1600)):
        call = f'    r0 = reloq_call_fifi_fin_ph_{phone}(state, (int32_t)(FIELD(0)));\n'
        if text.count(call) != 1:
            raise RuntimeError(f'Cannot locate Persian nasal dispatcher for {phone}')
        replacement = r'''    {
        const char *us=getenv("RELOQ_FA_US_NASALS");
        if (us && us[0]=='1' && !us[1]) {
''' + f'            r0=persian_us_ga_ph_{phone}(state,(int32_t)(FIELD(0)));\n' + \
            f'            if (!r0) r0=persian_us_eng_ph_{phone}(state,(int32_t)(FIELD(0)));\n' + \
            (r'''            if (!r0) {
                /* The Finnish scheduler leaves the nasal F1 too high in
                   these contexts. US /m/ before non-high vowels instead
                   keeps its original higher F1. */
                unsigned next=persian_articulation_following(state);
                int shift=(next==29 || next==31 || next==33 || next==34) ? 0 : 50;
                if (STATE(int16_t,s448)>150) STATE(int16_t,s448)-=shift;
                if (STATE(int16_t,s449)>150) STATE(int16_t,s449)-=shift;
            }
''' if phone == 'm' else r'''            if (!r0) {
                if (STATE(int16_t,s448)>150) STATE(int16_t,s448)-=50;
                if (STATE(int16_t,s449)>150) STATE(int16_t,s449)-=50;
            }
''') + \
            '        } else {\n' + call + \
            f'            if (!r0) persian_articulation_nasal(state,{target});\n' + \
            '        }\n    }\n'
        text = text.replace(call, replacement, 1)
    consonant_file.write_text(text, encoding='utf8')


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
        prepare('fifi', 'delta_rules_c17_fifi.c', 'stanfin_ph_a', 'L13086', A_PATCH, '#include <stdio.h>\n', work),
        prepare('fifi', 'delta_rules_c15_fifi.c', 'stanfin_ph_A', 'L13209', FRONT_A_PATCH, FRONT_A_HELPER, work),
        prepare('fifi', 'delta_rules_c22_fifi.c', 'fin_ph_k', 'L5915', Q_PATCH, Q_HELPER, work),
        prepare('enus61', 'delta_rules_c06_enus61.c', 'ga_ph_a', 'L3010', US_A_PATCH, '', work),
        prepare('dede61', 'delta_rules_c07_dede61.c', 'ger_ph_k', 'L22634',
                GERMAN_K_TRACE, '#include <stdio.h>\n', work),
    ]
    prepare_e_context_probe(work)
    modified.extend(prepare_german_l(work))
    modified.extend(prepare_french_l(work))
    modified.extend(prepare_x(work))
    modified.append(prepare_q_cluster(work))
    modified.append(prepare_q_phone(work))
    modified.extend(prepare_question_rise(work))
    prepare_vowel_start_duration(work)
    modified.append(prepare('fifi', 'delta_rules_c13_fifi.c', 'fin_ph_s',
                            'L1679', FINNISH_S_TRACE, '#include <stdio.h>\n', work))
    prepare_s_noise(work)
    modified.append(prepare_finnish_h(work))
    modified.append(prepare_auto_finnish_h(work))
    prepare_z_noise(work)
    modified.append(prepare_vowel_duration_probe(work))
    prepare_us_articulation(work)
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
    def compile_one(source: Path) -> Path:
        obj = work / (source.stem + '.o')
        print('Compiling', source.name, flush=True)
        subprocess.run(['cc', *flags, *['-I' + p for p in dirs], '-c', str(source), '-o', str(obj)],
                       cwd=PROVIDER, check=True)
        return obj

    # These disposable objects are independent. Keep archive updates and
    # linking sequential, but avoid compiling every source in series.
    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        compiled = list(pool.map(compile_one, modified))
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
