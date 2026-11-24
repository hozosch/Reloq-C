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
/* Listening variant: a uvular directly before another consonant, as in
   maghrur, is a plain stop without the added post-release frication. Before
   a vowel, a pause or the end of the utterance it keeps its release. */
static int persian_probe_soft_coda_q(void *state, int32_t position)
{
    const char *setting=getenv("RELOQ_FA_SOFT_Q");
    if (!setting || setting[0]!='1' || setting[1]) return 0;
    delta_state *d=state;
    if (!d || !position) return 0;
    int32_t *right=vmover(d,EVV_AT(int32_t *,position),2);
    int32_t next=right ? vgetsc(d,0,0,EVV_REF(right),2) : 0;
    if (!next || (*EVV_AT(int32_t *,next)&2)) return 0;
    void *fields=TFLDS(EVV_AT(void *,next));
    unsigned name=*(const unsigned char *)delta_record_field(&vstmtbl[2],0,fields);
    unsigned cls=*(const unsigned char *)delta_record_field(&vstmtbl[2],1,fields);
    return name && name!=36 && cls!=1;
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
/* The preceding vowel must belong to this word. Restore the native scanner
   completely so no subsequent acoustic rule sees a modified fence. */
static int persian_probe_previous_phone_in_word(void *state,int32_t token_ref)
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
    int previous=!setscan_l(d,2) && !advance_tok(d);
    d->lpta=left; d->rpta=right; memcpy(&v->scan_ptr,scan,sizeof scan);
    v->compared_equal=compared; v->fence_count=fences; v->testing=testing;
    memcpy(EVV_AT(void *,d->fence_index),index,d->nstmts);
    memcpy(EVV_AT(void *,d->fence_marks),marks,d->nstmts);
    memcpy(EVV_AT(void *,d->fence_chars),chars,d->nstmts);
    return previous;
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
static int persian_probe_following_final_h(void *state,int32_t position)
{
    delta_state *d=state;
    if(!d || !position)return 0;
    void *point=vmover(d,EVV_AT(int32_t *,position),2);
    int32_t node=point?vgetsc(d,0,0,EVV_REF(point),2):0;
    if(!node || (*EVV_AT(int32_t *,node)&2))return 0;
    void *fields=TFLDS(EVV_AT(void *,node));
    return *(const unsigned char *)delta_record_field(&vstmtbl[2],0,fields)==3 &&
           *(const unsigned char *)delta_record_field(&vstmtbl[2],12,fields)==5;
}
'''

FRONT_A_PATCH = r'''
    /* Keep the British "sad" vowel as a base, but move the first three
       resonance targets one quarter of the way towards German long /a/ in
       "Tag". Native duration, stress and neighbouring transitions remain. */
    {
        const char *british=getenv("RELOQ_FA_BRITISH_FRONT_A");
        if (british && british[0]=='1' && !british[1]) {
            STATE(int16_t,s448)=702;  STATE(int16_t,s449)=702;
            STATE(int16_t,s450)=1408; STATE(int16_t,s451)=1408;
            STATE(int16_t,s452)=2375; STATE(int16_t,s453)=2375;
            STATE(int16_t,s405)=0;
            STATE(int16_t,s436)=0;
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
            /* A conservative quarter step from the accepted Salam/Iran
               nucleus towards the German /o/ in "oft". Account for the
               different female upper-formant trajectory before the later
               voice-specific adjustment; retain bandwidths and timing. */
            int16_t female=0;
            memcpy(&female,(const unsigned char *)state+DG_BASE+3092+12,sizeof female);
            STATE(int16_t,s448)=634;  STATE(int16_t,s449)=634;
            STATE(int16_t,s450)=975;  STATE(int16_t,s451)=975;
            STATE(int16_t,s452)=female?2615:2590;
            STATE(int16_t,s453)=female?2615:2590;
            STATE(int16_t,s454)=3475; STATE(int16_t,s455)=3475;
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
/* H is encoded as an onset feature on the following vowel. */
static int persian_probe_l_next_has_h(void *state)
{
    delta_state *d=state;
    int32_t at=STATE(int32_t,w63);
    if (!d || !at) return 0;
    int32_t *point=vmover(d,EVV_AT(int32_t *,at),2);
    int32_t node=point?vgetsc(d,0,0,EVV_REF(point),2):0;
    if (!node || (*EVV_AT(int32_t *,node)&2)) return 0;
    void *fields=TFLDS(EVV_AT(void *,node));
    return *(const unsigned char *)delta_record_field(&vstmtbl[2],1,fields)==1 &&
           *(const unsigned char *)delta_record_field(&vstmtbl[2],9,fields)==1;
}
/* The legacy representation attaches a marked, silent carrier to a final
   H. It is not another articulated consonant and must not turn a preceding
   L into a medial L. Accept both stages of the carrier's conversion. */
static int persian_probe_l_next_is_final_h(void *state)
{
    delta_state *d=state;
    int32_t at=STATE(int32_t,w63);
    if (!d || !at) return 0;
    int32_t *point=vmover(d,EVV_AT(int32_t *,at),2);
    int32_t node=point?vgetsc(d,0,0,EVV_REF(point),2):0;
    if (!node || (*EVV_AT(int32_t *,node)&2)) return 0;
    void *fields=TFLDS(EVV_AT(void *,node));
    unsigned name=*(const unsigned char *)delta_record_field(&vstmtbl[2],0,fields);
    unsigned after=*(const unsigned char *)delta_record_field(&vstmtbl[2],9,fields);
    unsigned marker=*(const unsigned char *)delta_record_field(&vstmtbl[2],12,fields);
    return (name==3 && marker==5) || (name==23 && after==1);
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
                        '    if (fa_l && fa_l[0]==\'4\' && !fa_l[1]) {\n'
                        '        int final_h=persian_probe_l_next_is_final_h(state);\n'
                        '        int has_h=final_h || persian_probe_l_next_has_h(state);\n'
                        '        int16_t h_voicing=STATE(int16_t,s421);\n'
                        '        int32_t result=(final_h || persian_probe_l_word_final(state,STATE_AT(w60,-4))) ?\n'
                        '            persian_probe_fr_l(state,p0) : persian_probe_german_l(state,p0);\n'
                        '        /* Do not let the borrowed L overwrite the already planned H. */\n'
                        '        if (has_h && !result) STATE(int16_t,s421)=h_voicing;\n'
                        '        return result;\n'
                        '    }\n', 1)
    destination.write_text(text, encoding='utf8')
    native = prepare('frfr61', 'delta_rules_c08_frfr61.c', 'fren_ph_l', 'L4217',
                     GERMAN_L_TRACE.replace('DE_L', 'FR_L'), '#include <stdio.h>\n', work)
    return [native]


def prepare_french_nasals(work: Path) -> None:
    """Run French /m,n/ acoustic rules on the Persian phone records.

    The French and Finnish phone enums and record layouts differ. Keep the
    French rule flow and map its phone tests to the local records explicitly;
    the separately selected US duration and transition rules stay in charge.
    """
    root = PROVIDER / 'lang' / 'frfr61'
    source = (root / 'delta_authored_frfr61.c').read_text(encoding='utf8')
    source = source.split('uint8_t frfr61_evv_authored_data[] = {', 1)[1].split('};', 1)[0]
    source = re.sub(r'/\*.*?\*/', '', source, flags=re.S)
    raw = bytes(int(value, 0) for value in
                re.findall(r'(?<![A-Za-z_])(?:0[xX][0-9a-fA-F]+|[0-9]+)', source))
    table = (root / 'delta_rules_frfr61.c').read_text(encoding='utf8')
    table = table.split('frfr61_delta_rule_sym[] = {', 1)[1].split('};', 1)[0]
    offsets = [int(value) for value in re.findall(r'frfr61_evv_authored_data \+ (\d+)', table)]
    names = ('addval', 'fren_labial_Fv', 'fren_dental_Fv', 'fren_ph_m', 'fren_ph_n')
    blocks = []
    for name in names:
        matches = []
        for path in root.glob('delta_rules_c*.c'):
            text = path.read_text(encoding='utf8')
            start = text.find('int32_t reloq_frfr61_' + name + '(')
            if start >= 0:
                end = text.index('\nstatic int32_t evv_' + name + '(', start)
                matches.append(text[start:end])
        if len(matches) != 1:
            raise RuntimeError('Cannot locate French nasal donor ' + name)
        blocks.append(matches[0])
    indices = sorted({int(value) for block in blocks
                      for value in re.findall(r'delta_sym_ref\[(\d+)\]', block)})
    width = 32
    if len(offsets) <= max(indices) or any(offsets[i] + width > len(raw) for i in indices):
        raise RuntimeError('French nasal symbol storage changed')
    fr_names, fr_values = phone_symbol_data('frfr61')
    if (fr_names[2139], fr_names[2144], fr_names[2150], fr_values[2121]) != ('n', 'i', 'u', 0):
        raise RuntimeError('French nasal phone symbols changed')
    index_map = {value: index for index, value in enumerate(indices)}
    constants = ',\n'.join('    {' + ','.join(str(x) for x in raw[offsets[i]:offsets[i]+width]) + '}'
                           for i in indices)
    helper = r'''
static const unsigned char persian_fr_nasal_constants[][32]={
__CONSTANTS__
};
static _Atomic int32_t persian_fr_nasal_constants_ref;
static int32_t persian_fr_nasal_symbols(void)
{
    int32_t result=atomic_load_explicit(&persian_fr_nasal_constants_ref,memory_order_acquire);
    if (result) return result;
    void *copy=evv_arena_alloc(sizeof persian_fr_nasal_constants);
    if (!copy) return 0;
    memcpy(copy,persian_fr_nasal_constants,sizeof persian_fr_nasal_constants);
    int32_t made=EVV_REF(copy),expected=0;
    if (!atomic_compare_exchange_strong_explicit(&persian_fr_nasal_constants_ref,&expected,made,
                                                  memory_order_release,memory_order_acquire)) {
        evv_arena_free(copy);
        return expected;
    }
    return made;
}
static int32_t persian_fr_nasal_field(void *state,uint8_t st,uint8_t field,uint8_t value)
{
    if (st==2 && field==11) /* stress */
        return reloq61_typed_testFldeq(DELTA_ENTRY_testFldeq,state,2,7,value);
    return persian_probe_fr_field(state,st,field,value);
}
static int32_t persian_fr_nasal_push(void *state,uint8_t st,uint8_t field)
{
    if (st==2 && field==11)
        return reloq61_typed_npush_fld(DELTA_ENTRY_npush_fld,state,2,7);
    return 1;
}
static int32_t persian_fr_nasal_string(void *state,uint8_t st,uint8_t count,const uint8_t *source)
{
    static const uint8_t phones[]={
        0,4,1,5,2,3,6,255,255,8,7,10,9,12,13,14,11,15,16,17,
        255,255,255,19,21,22,23,255,34,34,29,31,27,24,28,25,33,
        33,32,26,30,30,255,255,255,255,255,18,36
    };
    if (st!=2 || !source || count>32) return 1;
    uint8_t mapped[32];
    for (unsigned i=0;i<count;++i)
        mapped[i]=source[i]<sizeof phones ? phones[source[i]] : 255;
    return test_string_s(state,st,count,mapped);
}
'''.replace('__CONSTANTS__', constants)
    pieces = []
    for name, block in zip(names, blocks):
        block = block.replace('int32_t reloq_frfr61_' + name + '(',
                              'static int32_t persian_fr_' + name + '(', 1)
        if name == 'fren_labial_Fv':
            # The French-only speaker setting has no equivalent in the local
            # voice state; retain the ordinary donor branch.
            extra = ('    r1 = ((int32_t)STATE(uint16_t, s326));\n'
                     '    AT(int32_t, -20) = (r1);\n'
                     '    if (AT(int16_t, -20) == (int16_t)(STATE(int16_t, s319))) goto L11765;\n'
                     '    goto L16771;\n')
            if block.count(extra) != 1:
                raise RuntimeError('French nasal speaker branch changed')
            block = block.replace(extra, '    goto L16771;\n', 1)
        block = re.sub(r'delta_sym_ref\[(\d+)\]',
                       lambda m: f'(persian_fr_refs + {index_map[int(m.group(1))]*width})', block)
        block = re.sub(r'STATE\((u?int(?:16|32)_t), s(\d+)\)',
                       lambda m: f'STATE({m.group(1)}, s{int(m.group(2))-1})', block)
        block = re.sub(r'STATE_AT\(s(\d+),',
                       lambda m: f'STATE_AT(s{int(m.group(1))-1},', block)
        marker = '    unsigned char *frame = evv_frame_push('
        if block.count(marker) != 1:
            raise RuntimeError('Cannot locate French nasal workspace ' + name)
        block = block.replace(marker, '    const int32_t persian_fr_refs=persian_fr_nasal_symbols();\n'
                              '    if (!persian_fr_refs) return 94;\n' + marker, 1)
        for typed, replacement in (('testFldeq', 'field'), ('npush_fld', 'push'),
                                   ('test_string_s', 'string')):
            block = block.replace('reloq61_typed_' + typed + '(DELTA_ENTRY_' + typed + ', state,',
                                  'persian_fr_nasal_' + replacement + '(state,')
        for local in ('addval', 'fren_labial_Fv', 'fren_dental_Fv'):
            block = block.replace('reloq_call_frfr61_' + local + '(', 'persian_fr_' + local + '(')
        for local in ('doAV', 'modulate_nas_AV', 'final_voiced_release'):
            block = block.replace('reloq_call_frfr61_' + local + '(', 'reloq_call_fifi_' + local + '(')
        if 'reloq_call_frfr61_' in block:
            raise RuntimeError('Unmapped French nasal dependency: ' + name)
        pieces.append(block)
    destination = work / 'delta_rules_c02_fifi.c'
    text = destination.read_text(encoding='utf8')
    include = '#include "reloq61_rule_calls_fifi.h"\n'
    if text.count(include) != 1:
        raise RuntimeError('Cannot locate the French nasal declarations')
    text = text.replace(include, include + '#include <stdatomic.h>\n'
                        'static int32_t persian_fr_fren_ph_m(void *,int32_t);\n'
                        'static int32_t persian_fr_fren_ph_n(void *,int32_t);\n', 1)
    marker = 'int32_t reloq_fifi_fin_ph_l(void *state, int32_t p0)\n{\n'
    if text.count(marker) != 1:
        raise RuntimeError('Cannot locate the French nasal insertion point')
    text = text.replace(marker, helper + '\n' + '\n'.join(pieces) + '\n' + marker, 1)
    for phone in 'mn':
        call = f'    r0 = reloq_call_fifi_fin_ph_{phone}(state, (int32_t)(FIELD(0)));\n'
        if text.count(call) != 1:
            raise RuntimeError('Cannot locate Persian nasal dispatch ' + phone)
        if phone == 'm':
            selected_call = '            r0=persian_fr_fren_ph_m(state,(int32_t)(FIELD(0)));\n'
        else:
            selected_call = r'''            {
                /* Keep the French voice source, F0 and lowest resonance.
                   Above it use the contextual US alveolar targets, while
                   preserving the approved US timing and French bass. */
                int16_t *formants[]={&STATE(int16_t,s450),&STATE(int16_t,s451),
                    &STATE(int16_t,s452),&STATE(int16_t,s453),
                    &STATE(int16_t,s454),&STATE(int16_t,s456)};
                int16_t original[6],french[6];
                for(int i=0;i<6;++i)original[i]=*formants[i];
                r0=persian_fr_fren_ph_n(state,(int32_t)(FIELD(0)));
                if(!r0) {
                    for(int i=0;i<6;++i){french[i]=*formants[i];*formants[i]=original[i];}
                    int16_t noise[]={STATE(int16_t,s410),STATE(int16_t,s411),STATE(int16_t,s412),
                        STATE(int16_t,s439),STATE(int16_t,s440),STATE(int16_t,s441)};
                    int16_t scratch=STATE_D(int16_t,c14,2);
                    int32_t english=persian_us_eng_alv_Fv(state,(int32_t)(FIELD(0)));
                    for(int i=0;i<6;++i) {
                        int target=*formants[i],source=french[i];
                        *formants[i]=(int16_t)(!english && target>0 ? target : source);
                    }
                    STATE(int16_t,s410)=noise[0];STATE(int16_t,s411)=noise[1];
                    STATE(int16_t,s412)=noise[2];STATE(int16_t,s439)=noise[3];
                    STATE(int16_t,s440)=noise[4];STATE(int16_t,s441)=noise[5];
                    STATE_D(int16_t,c14,2)=scratch;
                }
            }
'''
        replacement = ('    {\n        const char *selected=getenv("RELOQ_FA_US_NASALS");\n'
                       "        if (selected && selected[0]=='1' && !selected[1])\n" +
                       selected_call + '        else\n' + call + '    }\n')
        text = text.replace(call, replacement, 1)
    destination.write_text(text, encoding='utf8')

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
            if (variant >= 3 && !persian_probe_soft_coda_q(state,STATE(int32_t,w61))) {
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
            int following=(glot_tract || h_tract) ?
                persian_probe_neighbour_vowel(state,h_tract ? STATE(int32_t,w60) : STATE(int32_t,w61),
                                               h_tract,&f1,&f2,&f3) : 0;
            if (following) {
                int p1=0,p2=0,p3=0;
                if (glot_tract && following==29 &&
                    persian_probe_neighbour_vowel(state,STATE(int32_t,w60),1,&p1,&p2,&p3) &&
                    persian_probe_previous_phone_in_word(state,STATE_AT(w60,-4))) {
                    /* Keep the established glottal closure and source onset.
                       Its tract sits midway between the two vowels, so E
                       no longer pulls the preceding vowel all the way to
                       its bright target in a few milliseconds. */
                    f1=(f1+p1+1)/2; f2=(f2+p2+1)/2; f3=(f3+p3+1)/2;
                }
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
        if name == 'delta_rules_c30_fifi.c':
            marker_include = '#include "reloq61_rule_calls_fifi.h"\n'
            helper = r'''
/* Read the current syllable's phone records without changing the scanner.
 * The selected Persian q adds a 25 ms release after duration assignment;
 * report how many such releases precede this syllable's vowel. */
static int persian_early_nucleus_identity(void *state,int32_t begin,int32_t nucleus,
                                          uint32_t *identity)
{
    delta_state *d=state;
    if(!d || !begin || !nucleus || !identity)return 0;
    int32_t at=EVV_REF(vmover(d,EVV_AT(int32_t *,begin),2));
    int32_t end=EVV_REF(vmover(d,EVV_AT(int32_t *,nucleus),2));
    int32_t target=end?vgetsc(d,0,0,end,2):0;
    if(!target || (*EVV_AT(int32_t *,target)&2))return 0;
    unsigned q=0;
    for(unsigned guard=0;at && guard<256;++guard) {
        int32_t node=vgetsc(d,0,0,at,2);
        if(!node || (*EVV_AT(int32_t *,node)&2))return 0;
        void *fields=TFLDS(EVV_AT(void *,node));
        unsigned slot=*(const unsigned char *)delta_record_field(&vstmtbl[2],0,fields);
        if(node==target) {
            if(!slot || slot>255 || q>255)return 0;
            *identity=slot | (q<<8);
            return 1;
        }
        if(slot==3) {
            unsigned marker=*(const unsigned char *)delta_record_field(&vstmtbl[2],12,fields);
            if(marker==3)++q;
        }
        int32_t next=EVV_AT(int32_t *,node)[1]&~3;
        if(!next || next==at)return 0;
        at=EVV_REF(vmover(d,EVV_AT(int32_t *,next),2));
    }
    return 0;
}
'''
            if text.count(marker_include) != 1:
                raise RuntimeError('Cannot locate the early nucleus declarations')
            text = text.replace(marker_include, marker_include + helper, 1)
            marker = '    reloq61_syllable_trace_emit(state,STATE(int32_t,f13),STATE(int32_t,f14));\n'
            if text.count(marker) != 1:
                raise RuntimeError('Cannot locate the pre-intonation duration boundary')
            text = text.replace(marker, marker + r'''    if(reloq61_nucleus_trace_enabled()) {
        /* Candidate linguistic nucleus. This opt-in diagnostic reads the
         * assigned durations without moving the provider's pointer registers.
         * It does not change the normal voice or select a contour. */
        const char *q_mode=getenv("RELOQ_FA_PROBE_Q");
        if(!q_mode || q_mode[0]!='7' || q_mode[1]) {
            /* This mapping belongs to the scoped Persian phone projection,
             * not to ordinary Finnish or other exploratory Q variants. */
            reloq61_nucleus_trace_emit(state,-1,-1);
        } else {
            delta_tpos a={.node=STATE(int32_t,w24),.flags=1};
            delta_tpos b={.node=STATE(int32_t,w26),.flags=1};
            delta_tpos c={.node=STATE(int32_t,w27),.flags=1};
            int32_t before=-1,length=-1;uint32_t identity=0;
            if(a.node && b.node && c.node) {
                before=vdur((delta_state *)state,&a,&b,8);
                length=vdur((delta_state *)state,&b,&c,8);
            }
            int identified=persian_early_nucleus_identity(state,a.node,b.node,&identity);
            int64_t extra=(int64_t)(identity>>8)*250000;
            int64_t from=(int64_t)STATE(int32_t,f13)+(int64_t)before*10000+extra;
            int64_t to=from+(int64_t)length*10000;
            if(!identified || before<0 || length<=0 || from<0 || to>INT32_MAX)
                reloq61_nucleus_trace_emit(state,-1,-1);
            else {
                reloq61_nucleus_identity_trace_emit(identity);
                reloq61_nucleus_trace_emit(state,(int32_t)from,(int32_t)to);
            }
        }
    }
''', 1)
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
                        'extern int32_t persian_us_get_eng_vow_trans_dur(void *,int32_t,int32_t);\n'
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
    vow_call = ('    r0 = reloq_call_fifi_get_fin_vow_trans_dur(state, (int32_t)(FIELD(0)), '
                '(int32_t)(RELOQ61_WORKSPACE_REF(frame, reloq_workspace_ref, 192)));\n')
    if text.count(vow_call) != 1:
        raise RuntimeError('Cannot locate the vowel-transition duration')
    text = text.replace(vow_call, r'''    {
        const char *us=getenv("RELOQ_FA_US_STOPS");
        delta_state *d=state;
        int32_t next=persian_auto_h_phone(d,STATE(int32_t,w63),0);
        unsigned phone=next ? persian_auto_h_field(next,0) : 0;
        /* Preserve the total vowel-plus-transition time while borrowing the
           English vowel-to-D balance. This affects the transition, not the
           stop release or the separate H impulse following it. */
        if (us && us[0]=='1' && !us[1] && phone==5)
            r0=persian_us_get_eng_vow_trans_dur(state,(int32_t)(FIELD(0)),
                      (int32_t)(RELOQ61_WORKSPACE_REF(frame,reloq_workspace_ref,192)));
        else
''' + vow_call + '    }\n', 1)
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


def pre_stressed_hiatus_helper() -> str:
    """Find an unstressed vowel between a vowel and a stressed vowel in one word."""
    medial_helper = FINNISH_L_WORD_FINAL.replace('persian_probe_l_word_final',
                                                 'persian_probe_before_stressed_hiatus')
    original_scan = ('    int final=!setscan_r(d,2) && !testFldeq(d,2,0,21) &&\n'
                     '              !advance_tok(d) && advance_tok(d);')
    if medial_helper.count(original_scan) != 1:
        raise RuntimeError('Cannot locate medial vowel word-fence scan')
    return medial_helper.replace(original_scan, r'''    int left_vowel=!setscan_l(d,2) && !testFldeq(d,2,1,1) && !advance_tok(d);
    memset(EVV_AT(void *,d->fence_marks),0,d->nstmts);
    lpta_loadp(d,EVV_AT(const delta_token *,token_ref));
    int right_vowel=!setscan_r(d,2) && !advance_tok(d) && !testFldeq(d,2,1,1) &&
                    !testFldeq(d,2,7,1) && !advance_tok(d);
    int final=left_vowel && right_vowel;''', 1)


def weak_medial_u_helper() -> str:
    """Find a medial nucleus followed by primary stress in the same word."""
    helper = FINNISH_L_WORD_FINAL.replace('persian_probe_l_word_final',
                                         'persian_probe_weak_medial_u')
    original = ('    int final=!setscan_r(d,2) && !testFldeq(d,2,0,21) &&\n'
                '              !advance_tok(d) && advance_tok(d);')
    if helper.count(original) != 1:
        raise RuntimeError('Cannot locate medial U word-fence scan')
    return helper.replace(original, r'''    int earlier=0,later=0;
    if (!setscan_l(d,2)) {
        for (;;) {
            int vowel=!testFldeq(d,2,1,1);
            if (advance_tok(d)) break;
            if (vowel) { earlier=1; break; }
        }
    }
    if (earlier) {
        memset(EVV_AT(void *,d->fence_marks),0,d->nstmts);
        lpta_loadp(d,EVV_AT(const delta_token *,token_ref));
        if (!setscan_r(d,2)) {
            while (!advance_tok(d)) {
                if (!testFldeq(d,2,1,1) && !testFldeq(d,2,7,1)) {
                    later=1; break;
                }
            }
        }
    }
    int final=earlier && later;''', 1)


def prepare_short_initial_timing(work: Path, first_helper: str, next_helper: str) -> Path:
    """Use short nucleus timing in selected contexts without replacing its sound."""
    name = 'delta_rules_c04_fifi.c'
    text = (PROVIDER / 'lang' / 'fifi' / name).read_text(encoding='utf8')
    marker = 'int32_t reloq_fifi_assign_fin_nuc_durs(void *state, int32_t p0)\n{\n'
    medial_helper = pre_stressed_hiatus_helper()
    # Reuse the word-fenced token scanner, but require at least two further
    # vowel nuclei and an actual later stress. This is a structural timing
    # rule, independent of the original spelling or input mode.
    long_word_helper = first_helper.replace('persian_probe_first_of_multiple_vowels',
                                             'persian_probe_first_of_long_stressed_word')
    scan = 'if (!testFldeq(d,2,1,1)) { later=1; break; }'
    if long_word_helper.count(scan) != 1:
        raise RuntimeError('Cannot locate initial vowel count')
    long_word_helper = long_word_helper.replace('int first=0,later=0;', 'int first=0,later=0,stressed=0;', 1)
    long_word_helper = long_word_helper.replace(scan, r'''if (!testFldeq(d,2,1,1)) {
                    ++later;
                    if (!testFldeq(d,2,7,1) || !testFldeq(d,2,7,2)) stressed=1;
                }''', 1)
    long_word_helper = long_word_helper.replace('int final=first && later;',
        'int final=reloq_fa_short_initial_i_context(first,(unsigned)later,stressed);', 1)
    first_helper = first_helper.replace('int final=first && later;',
        'int final=reloq_fa_short_initial_other_context(first,(unsigned)later);', 1)
    helper = r'''
/* Use the native short-vowel timing for the selected unstressed nuclei.
   Keep the long vowel's acoustic identity by restoring the marker before
   the acoustic scheduler sees the record. */
static int32_t persian_short_initial_begin(void *state)
{
    const char *us=getenv("RELOQ_FA_US_ARTICULATION");
    delta_state *d=state;
    int32_t at=STATE(int32_t,w26),node=0;
    if (!us || us[0]!='1' || us[1] || !d || !at) return 0;
    int32_t *point=vmover(d,EVV_AT(int32_t *,at),2);
    if (point) node=vgetsc(d,0,0,EVV_REF(point),2);
    if (!node || (*EVV_AT(int32_t *,node)&2)) return 0;
    void *fields=TFLDS(EVV_AT(void *,node));
    unsigned phone=*(const unsigned char *)delta_record_field(&vstmtbl[2],0,fields);
    unsigned cls=*(const unsigned char *)delta_record_field(&vstmtbl[2],1,fields);
    unsigned char *length=(unsigned char *)delta_record_field(&vstmtbl[2],11,fields);
    int16_t stress=0;
    memcpy(&stress,delta_record_field(&vstmtbl[2],7,fields),sizeof stress);
    if (cls!=1 || *length!=0 || stress!=0) return 0;
    int selected=(phone==31 || phone==32) &&
        persian_probe_first_of_multiple_vowels(state,STATE_AT(w26,-4));
    if (phone==27)
        selected=persian_probe_before_stressed_hiatus(state,STATE_AT(w26,-4)) ||
            persian_probe_first_of_long_stressed_word(state,STATE_AT(w26,-4));
    if (!selected) return 0;
    if (phone==32) {
        int32_t next_ref=EVV_AT(int32_t *,node)[1]&~3;
        point=next_ref ? vmover(d,EVV_AT(int32_t *,next_ref),2) : NULL;
        int32_t next=point ? vgetsc(d,0,0,EVV_REF(point),2) : 0;
        if (next && !(*EVV_AT(int32_t *,next)&2)) {
            unsigned next_phone=*(const unsigned char *)delta_record_field(
                &vstmtbl[2],0,TFLDS(EVV_AT(void *,next)));
            if (next_phone==2 && !persian_probe_next_phone_word_final(state,next_ref,2)) {
                int32_t after_ref=EVV_AT(int32_t *,next)[1]&~3;
                point=after_ref ? vmover(d,EVV_AT(int32_t *,after_ref),2) : NULL;
                int32_t after=point ? vgetsc(d,0,0,EVV_REF(point),2) : 0;
                if (after && !(*EVV_AT(int32_t *,after)&2)) {
                    unsigned following=*(const unsigned char *)delta_record_field(
                        &vstmtbl[2],0,TFLDS(EVV_AT(void *,after)));
                    if (following>=4 && following<=6) return 0;
                }
            }
        }
    }
    *length=1;
    return node;
}
static void persian_short_initial_end(int32_t node)
{
    if (node && !(*EVV_AT(int32_t *,node)&2)) {
        void *fields=TFLDS(EVV_AT(void *,node));
        *(unsigned char *)delta_record_field(&vstmtbl[2],11,fields)=0;
    }
}

'''
    if text.count(marker) != 1:
        raise RuntimeError('Cannot locate Finnish nucleus timing dispatcher')
    text = text.replace(marker, first_helper + '\n' + next_helper + '\n' + medial_helper + '\n' +
                        long_word_helper + '\n' +
                        '_Thread_local int32_t reloq_fa_adjusted_long_node;\n' + helper + '\n' + marker, 1)
    begin = text.index(marker)
    end = text.index('\nstatic int32_t evv_assign_fin_nuc_durs(', begin)
    section = text[begin:end]
    declaration = '    int argn = 0;\n'
    start = '    r0 = reloq_call_fifi_set_limits(state, (int32_t)(FIELD(0)));\n'
    if section.count(declaration) != 1 or section.count(start) != 1:
        raise RuntimeError('Cannot locate Finnish nucleus timing setup')
    section = section.replace(declaration, declaration +
                              '    int32_t persian_short_node=0;\n'
                              '    int32_t persian_previous_long_node=reloq_fa_adjusted_long_node;\n', 1)
    section = section.replace(start, '    persian_short_node=persian_short_initial_begin(state);\n'
                              '    reloq_fa_adjusted_long_node=persian_short_node;\n' + start, 1)
    for label in ('L212:;\n', 'L231:;\n'):
        if section.count(label) != 1:
            raise RuntimeError('Cannot locate Finnish nucleus timing return')
        section = section.replace(label, label + '    persian_short_initial_end(persian_short_node);\n'
                                  '    reloq_fa_adjusted_long_node=persian_previous_long_node;\n', 1)
    text = text[:begin] + section + text[end:]
    include = '#include "reloq61_rule_calls_fifi.h"\n'
    if text.count(include) != 1:
        raise RuntimeError('Cannot locate Finnish timing includes')
    text = text.replace(include, include + '#include <stdlib.h>\n#include <string.h>\n'
                        '#include "reloq_persian_timing.h"\n', 1)
    dest = work / name
    dest.write_text(text, encoding='utf8')
    return dest


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
            int short_yek=0,short_u_cluster=0;
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
                /* A long /u/ remains long, but its unstressed nucleus may
                   be shortened modestly before /t/ + a voiced stop in the
                   same word (e.g. فوتبال). Do not use the short-U phone. */
                if (phone==32 && length==0 && stress==0 && next_phone==2 &&
                    !persian_probe_next_phone_word_final(state,next_ref,2)) {
                    int32_t after_ref=EVV_AT(int32_t *,next)[1]&~3;
                    int32_t *after_point=after_ref ? vmover(d,EVV_AT(int32_t *,after_ref),2) : NULL;
                    int32_t after=after_point ? vgetsc(d,0,0,EVV_REF(after_point),2) : 0;
                    if(after && !(*EVV_AT(int32_t *,after)&2)) {
                        unsigned following=*(const unsigned char *)delta_record_field(
                            &vstmtbl[2],0,TFLDS(EVV_AT(void *,after)));
                        short_u_cluster=following>=4 && following<=6;
                    }
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
                if(short_u_cluster)scaled=(scaled*9+5)/10;
                STATE(int32_t,f33)=scaled>INT32_MAX ? INT32_MAX : (int32_t)scaled;
            }
            /* The short front /æ/ itself is too long, including outside
               tak. Correct its shared base timing in all contexts instead
               of making a spelling- or stop-specific exception. Preserve
               explicitly long vowels and the vowel's acoustic identity. */
            if (us_timing && node && node!=reloq_fa_adjusted_long_node &&
                cls==1 && phone==31 && length==1 &&
                STATE(int32_t,f33)>0) {
                int64_t scaled=((int64_t)STATE(int32_t,f33)*85+50)/100;
                STATE(int32_t,f33)=scaled>INT32_MAX ? INT32_MAX : (int32_t)scaled;
            }
            /* A weak medial short U must remain audible, without bringing
               back the duration of an imported long vowel. */
            if (us_timing && node && cls==1 && phone==32 && length==1 && stress==0 &&
                STATE(int32_t,f33)>0 &&
                persian_probe_weak_medial_u(state,STATE_AT(w26,-4))) {
                int64_t duration=STATE(int32_t,f33);
                int64_t extra=(3*duration+1)/2;
                if(extra>200000)extra=200000;
                int64_t scaled=duration+extra;
                STATE(int32_t,f33)=scaled>INT32_MAX ? INT32_MAX : (int32_t)scaled;
            }
            /* A phrase-final /i/ or /o/ needs enough voiced time for the
               contour to reach its end. Weak /i/ can also collapse to just
               a few milliseconds before the next word; give it a smaller
               bounded recovery there. Keep isolated vowel-mark names and
               the acoustic vowel targets unchanged. */
            if (us_timing && node && cls==1 && (phone==27 || phone==33) &&
                STATE(int32_t,f33)>0 &&
                persian_probe_vowel_word_final(state,STATE_AT(w26,-4),(unsigned)phone) &&
                !persian_probe_lone_vowel(state,STATE_AT(w26,-4),(unsigned)phone)) {
                int32_t next_ref=EVV_AT(int32_t *,node)[1]&~3;
                int32_t *point=next_ref ? vmover(d,EVV_AT(int32_t *,next_ref),2) : NULL;
                int32_t next=point ? vgetsc(d,0,0,EVV_REF(point),2) : 0;
                unsigned following=next && !(*EVV_AT(int32_t *,next)&2) ?
                    *(const unsigned char *)delta_record_field(&vstmtbl[2],0,
                        TFLDS(EVV_AT(void *,next))) : 0;
                int phrase_final=!next || following==36;
                if (phrase_final || (phone==27 && stress==0)) {
                    int64_t duration=STATE(int32_t,f33);
                    int64_t extra=phrase_final ?
                        (stress==0 ? (3*duration+2)/5 : (duration+2)/5) :
                        (2*duration+2)/5;
                    int64_t limit=phrase_final ? (stress==0 ? 350000 : 250000) : 150000;
                    if(extra>limit)extra=limit;
                    int64_t scaled=duration+extra;
                    STATE(int32_t,f33)=scaled>INT32_MAX ? INT32_MAX : (int32_t)scaled;
                }
            }
            /* Compact a weak /i/ in hiatus as before. A short unstressed
               back /a/ in the same context gets a smaller listening-test
               reduction. Keep both vowel targets and consonants unchanged. */
            if (us_timing && node && cls==1 && (phone==27 || (phone==34 && length==1)) && stress==0 &&
                STATE(int32_t,f33)>0 &&
                persian_probe_before_stressed_hiatus(state,STATE_AT(w26,-4))) {
                unsigned kind=phone==27 ? RELOQ_FA_HIATUS_I : RELOQ_FA_HIATUS_SHORT_BACK_A;
                unsigned factor=reloq_fa_hiatus_timing_percent(kind,length==0,1,1);
                int64_t scaled=((int64_t)STATE(int32_t,f33)*factor+50)/100;
                STATE(int32_t,f33)=scaled>INT32_MAX ? INT32_MAX : (int32_t)scaled;
            }
            /* An isolated kasra is spoken as a short vowel, not as the
               long name used for the letter. The inherited short-E phone
               is unusually long when it is the entire word; adjust only
               that context, leaving short E in words and long E intact. */
            if (us_timing && node && cls==1 && phone==29 && length==1 &&
                STATE(int32_t,f33)>0 &&
                persian_probe_lone_vowel(state,STATE_AT(w26,-4),29)) {
                int64_t scaled=((int64_t)STATE(int32_t,f33)*60+50)/100;
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
    lone_helper = helper.replace('persian_probe_vowel_word_final', 'persian_probe_lone_vowel')
    lone_scan = ('    int final=!setscan_r(d,2) && !testFldeq(d,2,0,phone) &&\n'
                 '              !advance_tok(d) && advance_tok(d);')
    if lone_helper.count(lone_scan) != 1:
        raise RuntimeError('Cannot locate isolated-vowel word-fence scan')
    lone_helper = lone_helper.replace(lone_scan, r'''    int first=!setscan_l(d,2) && advance_tok(d);
    memset(EVV_AT(void *,d->fence_marks),0,d->nstmts);
    lpta_loadp(d,EVV_AT(const delta_token *,token_ref));
    int final=first && !setscan_r(d,2) && !testFldeq(d,2,0,phone) &&
              !advance_tok(d) && advance_tok(d);''', 1)
    # next_ref is the scanner node value, not the address of a delta_token.
    # Construct an actual token so lpta_loadp reads the intended /k/ node.
    next_helper = helper.replace(
        'persian_probe_vowel_word_final(void *state,int32_t token_ref,unsigned phone)',
        'persian_probe_next_phone_word_final(void *state,int32_t token_ref,unsigned phone)'
    ).replace('    lpta_loadp(d,EVV_AT(const delta_token *,token_ref));',
              '    const delta_token token={0,token_ref};\n    lpta_loadp(d,&token);')
    first_helper = FINNISH_L_WORD_FINAL.replace(
        'persian_probe_l_word_final', 'persian_probe_first_of_multiple_vowels')
    original_scan = ('    int final=!setscan_r(d,2) && !testFldeq(d,2,0,21) &&\n'
                     '              !advance_tok(d) && advance_tok(d);')
    if first_helper.count(original_scan) != 1:
        raise RuntimeError('Cannot locate vowel word-fence scan')
    first_helper = first_helper.replace(original_scan, r'''    int first=0,later=0;
    if (!setscan_l(d,2)) {
        first=1;
        for (;;) {
            /* testFldeq inspects the next token, whereas advance_tok also
               enforces the word fence. Check both in that order so a vowel
               from the previous word is never mistaken for this one. */
            int vowel=!testFldeq(d,2,1,1);
            if (advance_tok(d)) break;
            if (vowel) { first=0; break; }
        }
    }
    if (first) {
        memset(EVV_AT(void *,d->fence_marks),0,d->nstmts);
        lpta_loadp(d,EVV_AT(const delta_token *,token_ref));
        if (!setscan_r(d,2)) {
            /* The right register starts on the current vowel. Move past it
               before testing for the next one, or a single-syllable word
               would incorrectly look as if it had a later vowel. */
            while (!advance_tok(d)) {
                if (!testFldeq(d,2,1,1)) { later=1; break; }
            }
        }
    }
    int final=first && later;''', 1)
    prepare_short_initial_timing(work, first_helper, next_helper)
    text = (text[:begin] + helper + '\n' + lone_helper + '\n' + next_helper + '\n' +
            pre_stressed_hiatus_helper() + '\n' + weak_medial_u_helper() + '\n' +
            section + text[end:])
    include = '#include "reloq61_rule_calls_fifi.h"\n'
    if text.count(include) != 1:
        raise RuntimeError('Cannot locate duration probe includes')
    text = text.replace(include, include + '#include <stdlib.h>\n#include <stdio.h>\n#include <stdint.h>\n'
                        '#include "reloq_persian_timing.h"\n'
                        'extern _Thread_local int32_t reloq_fa_adjusted_long_node;\n', 1)
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
        int f[5]={0,0,0,0,0},end=0,neighbour=context ? atoi(context) : 0;
        int count=target ? sscanf(target,"%d,%d,%d,%d,%d%n",f,f+1,f+2,f+3,f+4,&end) : 0;
        if (target && (count!=5 || target[end])) {
            end=0;
            count=sscanf(target,"%d,%d,%d%n",f,f+1,f+2,&end);
        }
        int selected=neighbour==0 || (neighbour>0 && neighbour<=255 &&
                     (before==(unsigned)neighbour || after==(unsigned)neighbour));
        if(mode && mode[0]=='3' && !mode[1] && selected && target &&
           count==5 && !target[end] && f[0]>=200 && f[0]<=1200 &&
           f[1]>=500 && f[1]<=3000 && f[2]>=500 && f[2]<=3000 &&
           f[3]>=1500 && f[3]<=4000 && f[4]>=1500 && f[4]<=4000) {
            /* The earlier "se" E traverses the darker-to-brighter endpoints
               1900->2000 and 2550->2700. Do not flatten that trajectory or
               overwrite voice-specific F4/F5 and their inheritance. */
            STATE(int16_t,s448)=(int16_t)f[0];
            STATE(int16_t,s449)=(int16_t)f[0];
            STATE(int16_t,s450)=(int16_t)f[1];
            STATE(int16_t,s451)=(int16_t)f[2];
            STATE(int16_t,s452)=(int16_t)f[3];
            STATE(int16_t,s453)=(int16_t)f[4];
        } else if(mode && !mode[1] && (mode[0]=='1' || mode[0]=='2') && selected && target &&
           (count==3 || count==5) && !target[end] &&
           f[0]>=200 && f[0]<=1200 && f[1]>=500 && f[1]<=3000 &&
           f[2]>=1500 && f[2]<=4000 &&
           (count==3 || (f[3]>=2500 && f[3]<=5000 && f[4]>=3000 && f[4]<=6000))) {
            int16_t *ends[10]={&STATE(int16_t,s448),&STATE(int16_t,s449),
                              &STATE(int16_t,s450),&STATE(int16_t,s451),
                              &STATE(int16_t,s452),&STATE(int16_t,s453),
                              &STATE(int16_t,s454),&STATE(int16_t,s455),
                              &STATE(int16_t,s456),&STATE(int16_t,s457)};
            for(int i=0;i<2*count;i++) {
                int goal=f[i/2],current=*ends[i];
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
    helpers = ('eng_high_pal_Fv', 'adjust_eng_female_breathiness',
               'eng_lab_Fv', 'eng_alv_Fv', 'eng_vel_Fv', 'eng_pal_Fv')
    nasal_formants = ('ga_ph_m', 'ga_ph_n', 'ga_ph_y')
    acoustic = tuple('eng_ph_' + phone for phone in 'ptkbdgfvszSZmny')
    durations = tuple('eng_ph_' + phone + '_dur' for phone in 'ptkbdgfvszSZmn')
    exported = durations + ('get_eng_stop_trans_dur', 'get_eng_vow_trans_dur', 'get_eng_fric_trans_dur',
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
        if name == 'eng_ph_y':
            # These US-only state cells do not exist in the Persian provider.
            # The dispatcher already calls ga_ph_y (and its high-palatal
            # rule), so the second conditional call is unnecessary here.
            conditional = ('    if ((int16_t)(STATE(int16_t, s319)) == (int16_t)(STATE(int16_t, s318))) goto L116;\n'
                           '    goto L127;\n')
            if block.count(conditional) != 1:
                raise RuntimeError('Cannot isolate the English-only glide context')
            block = block.replace(conditional, '    goto L127;\n', 1)
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



def german_j_rules() -> str:
    """Adapt the German onset /j/ rules to the Persian phone records.

    German and Finnish use different field and enum numbers. The German
    rules retain their decisions and targets, but comparisons are translated
    by name before they inspect a Finnish/Persian record.
    """
    root = PROVIDER / 'lang' / 'dede61'
    source = (root / 'delta_authored_dede61.c').read_text(encoding='utf8')
    source = source.split('uint8_t dede61_evv_authored_data[] = {', 1)[1].split('};', 1)[0]
    source = re.sub(r'/\*.*?\*/', '', source, flags=re.S)
    raw = bytes(int(value, 0) for value in
                re.findall(r'(?<![A-Za-z_])(?:0[xX][0-9a-fA-F]+|[0-9]+)', source))
    table = (root / 'delta_rules_dede61.c').read_text(encoding='utf8')
    table = table.split('dede61_delta_rule_sym[] = {', 1)[1].split('};', 1)[0]
    offsets = [int(value) for value in re.findall(r'dede61_evv_authored_data \+ (\d+)', table)]
    names = ('stanger_ph_j', 'ger_ph_j')
    blocks = []
    for name in names:
        found = []
        for path in root.glob('delta_rules_c*.c'):
            data = path.read_text(encoding='utf8')
            start = data.find('int32_t reloq_dede61_' + name + '(')
            if start >= 0:
                end = data.index('\nstatic int32_t evv_' + name + '(', start)
                found.append(data[start:end])
        if len(found) != 1:
            raise RuntimeError('Cannot locate German J donor ' + name)
        blocks.append(found[0])
    indices = sorted({int(x) for block in blocks for x in re.findall(r'delta_sym_ref\[(\d+)\]', block)})
    width = 32
    if not indices or len(offsets) <= max(indices) or any(offsets[i] + width > len(raw) for i in indices):
        raise RuntimeError('German J symbol storage changed')
    mapping = {index: position for position, index in enumerate(indices)}
    constants = ',\n'.join('    {' + ','.join(str(x) for x in raw[offsets[i]:offsets[i]+width]) + '}'
                           for i in indices)
    helper = r'''
extern delta_stmt dede61_vstmtbl[];
static int persian_de_small_female(void *state)
{
    int16_t female=0,head=50;
    memcpy(&female,(const unsigned char *)state+DG_BASE+3092+12,sizeof female);
    memcpy(&head,(const unsigned char *)state+DG_BASE+3092+14,sizeof head);
    return female && head<30;
}
static const unsigned char persian_de_constants[][32]={
__CONSTANTS__
};
static _Atomic int32_t persian_de_constants_ref;
static int32_t persian_de_symbols(void)
{
    int32_t result=atomic_load_explicit(&persian_de_constants_ref,memory_order_acquire);
    if (result) return result;
    void *copy=evv_arena_alloc(sizeof persian_de_constants);
    if (!copy) return 0;
    memcpy(copy,persian_de_constants,sizeof persian_de_constants);
    int32_t made=EVV_REF(copy),expected=0;
    if (!atomic_compare_exchange_strong_explicit(&persian_de_constants_ref,&expected,made,
                                                  memory_order_release,memory_order_acquire)) {
        evv_arena_free(copy);
        return expected;
    }
    return made;
}
static int persian_de_field_number(unsigned st,unsigned field)
{
    if (st>8 || field>=(unsigned)dede61_vstmtbl[st].nfields) return -1;
    const char *name=dede61_vstmtbl[st].fields[field].name;
    if (!name) return -1;
    for (int i=0;i<vstmtbl[st].nfields;i++)
        if (vstmtbl[st].fields[i].name && !strcmp(name,vstmtbl[st].fields[i].name)) return i;
    return -1;
}
static int persian_de_value(unsigned st,unsigned field,unsigned value,int local)
{
    const delta_fielddesc *src=&dede61_vstmtbl[st].fields[field];
    const delta_fielddesc *dst=&vstmtbl[st].fields[local];
    if (!src->values || !dst->values) return (int)value;
    if (value>=(unsigned)src->nvalues) return -1;
    const char *name=((const char *const *)src->values)[value];
    if (!name) return -1;
    if (st==2 && field==0 && !strcmp(name,"O")) name="o";
    for (int i=0;i<dst->nvalues;i++) {
        const char *candidate=((const char *const *)dst->values)[i];
        if (candidate && !strcmp(name,candidate)) return i;
    }
    return -1;
}
static int32_t persian_de_field(void *state,uint8_t st,uint8_t field,uint8_t value)
{
    int local=persian_de_field_number(st,field);
    int mapped=local<0 ? -1 : persian_de_value(st,field,value,local);
    return mapped<0 ? 1 : reloq61_typed_testFldeq(DELTA_ENTRY_testFldeq,state,st,(uint8_t)local,(uint8_t)mapped);
}
static int32_t persian_de_string(void *state,uint8_t st,uint8_t count,const uint8_t *source)
{
    if (st>8 || !source || count>32 || persian_de_field_number(st,0)!=0) return 1;
    uint8_t mapped[32];
    for (unsigned i=0;i<count;i++) {
        int value=persian_de_value(st,0,source[i],0);
        if (value<0 || value>255) return 1;
        mapped[i]=(uint8_t)value;
    }
    return test_string_s(state,st,count,mapped);
}
'''.replace('__CONSTANTS__', constants)
    fields = {399: 398, 400: 399, 403: 402, 449: 448, 451: 450,
              453: 452, 455: 454, 457: 456}
    pieces = []
    for name, block in zip(names, blocks):
        block = block.replace('int32_t reloq_dede61_' + name + '(',
                              'static int32_t persian_de_' + name + '(', 1)
        used = {int(x) for x in re.findall(r'STATE\(int16_t, s(\d+)\)', block)}
        if not used <= (fields.keys() | {367}):
            raise RuntimeError('German J state layout changed: ' + name)
        # German's s367 is its small-female-voice flag, rather than the
        # unrelated Finnish state cell with the same name. The German voice
        # setup selects it only when the female head size is below 30.
        block = block.replace('STATE(int16_t, s367)', 'persian_de_small_female(state)')
        block = re.sub(r'STATE\(int16_t, s(\d+)\)',
                       lambda m: f'STATE(int16_t, s{fields[int(m.group(1))]})', block)
        block = re.sub(r'delta_sym_ref\[(\d+)\]',
                       lambda m: f'(persian_de_refs + {mapping[int(m.group(1))]*width})', block)
        marker = '    unsigned char *frame = evv_frame_push('
        if block.count(marker) != 1:
            raise RuntimeError('Cannot locate German J frame: ' + name)
        block = block.replace(marker, '    const int32_t persian_de_refs=persian_de_symbols();\n'
                              '    if (!persian_de_refs) return 94;\n' + marker, 1)
        block = block.replace('reloq61_typed_testFldeq(DELTA_ENTRY_testFldeq, state,',
                              'persian_de_field(state,')
        block = block.replace('reloq61_typed_test_string_s(DELTA_ENTRY_test_string_s, state,',
                              'persian_de_string(state,')
        if 'reloq_call_dede61_' in block:
            raise RuntimeError('Unmapped German J dependency: ' + name)
        pieces.append(block)
    return helper + '\n' + '\n'.join(pieces)


def brazilian_r_rules() -> str:
    """Use the actual Portuguese /r/ and duration rules on Persian records.

    Portuguese and Finnish order their word and phone statement types
    differently. Translate statement, field and enum names at the boundary;
    never call a Portuguese rule directly on a Finnish runtime state.
    """
    root = PROVIDER / 'lang' / 'ptbr'
    source = (root / 'delta_authored_ptbr.c').read_text(encoding='utf8')
    source = source.split('uint8_t ptbr_evv_authored_data[] = {', 1)[1].split('};', 1)[0]
    source = re.sub(r'/\*.*?\*/', '', source, flags=re.S)
    raw = bytes(int(value, 0) for value in
                re.findall(r'(?<![A-Za-z_])(?:0[xX][0-9a-fA-F]+|[0-9]+)', source))
    table = (root / 'delta_rules_ptbr.c').read_text(encoding='utf8')
    table = table.split('ptbr_delta_rule_sym[] = {', 1)[1].split('};', 1)[0]
    offsets = [int(value) for value in re.findall(r'ptbr_evv_authored_data \+ (\d+)', table)]
    names = ('add_TL_to_trill', 'por_ph_r', 'por_ph_r_dur')
    blocks = []
    for name in names:
        matches = []
        for path in root.glob('delta_rules_c*.c'):
            data = path.read_text(encoding='utf8')
            start = data.find('int32_t reloq_ptbr_' + name + '(')
            if start >= 0:
                end = data.index('\nstatic int32_t evv_' + name + '(', start)
                matches.append(data[start:end])
        if len(matches) != 1:
            raise RuntimeError('Cannot locate Brazilian R donor ' + name)
        blocks.append(matches[0])
    indices = sorted({int(x) for block in blocks for x in re.findall(r'delta_sym_ref\[(\d+)\]', block)})
    width = 32
    if not indices or len(offsets) <= max(indices) or any(offsets[i] + width > len(raw) for i in indices):
        raise RuntimeError('Brazilian R symbol storage changed')
    index_map = {value: index for index, value in enumerate(indices)}
    constants = ',\n'.join('    {' + ','.join(str(x) for x in raw[offsets[i]:offsets[i]+width]) + '}'
                           for i in indices)
    helper = r'''
extern delta_stmt ptbr_vstmtbl[];
static const unsigned char persian_pt_constants[][32]={
__CONSTANTS__
};
static _Atomic int32_t persian_pt_constants_ref;
static int32_t persian_pt_symbols(void)
{
    int32_t result=atomic_load_explicit(&persian_pt_constants_ref,memory_order_acquire);
    if (result) return result;
    void *copy=evv_arena_alloc(sizeof persian_pt_constants);
    if (!copy) return 0;
    memcpy(copy,persian_pt_constants,sizeof persian_pt_constants);
    int32_t made=EVV_REF(copy),expected=0;
    if (!atomic_compare_exchange_strong_explicit(&persian_pt_constants_ref,&expected,made,
                                                  memory_order_release,memory_order_acquire)) {
        evv_arena_free(copy);
        return expected;
    }
    return made;
}
static unsigned persian_pt_type(unsigned st)
{ return st==2 ? 4 : st==4 ? 2 : st; }
static int persian_pt_field_number(unsigned st,unsigned field)
{
    unsigned local_st=persian_pt_type(st);
    if (st>8 || field>=(unsigned)ptbr_vstmtbl[st].nfields) return -1;
    const char *name=ptbr_vstmtbl[st].fields[field].name;
    if (!name) return -1;
    for (int i=0;i<vstmtbl[local_st].nfields;i++)
        if (vstmtbl[local_st].fields[i].name && !strcmp(name,vstmtbl[local_st].fields[i].name)) return i;
    return -1;
}
static int persian_pt_value(unsigned st,unsigned field,unsigned value,int local)
{
    const delta_fielddesc *src=&ptbr_vstmtbl[st].fields[field];
    const delta_fielddesc *dst=&vstmtbl[persian_pt_type(st)].fields[local];
    if (!src->values || !dst->values) return (int)value;
    if (value>=(unsigned)src->nvalues) return -1;
    const char *name=((const char *const *)src->values)[value];
    if (!name) return -1;
    for (int i=0;i<dst->nvalues;i++) {
        const char *candidate=((const char *const *)dst->values)[i];
        if (candidate && !strcmp(name,candidate)) return i;
    }
    return -1;
}
static int32_t persian_pt_field(void *state,uint8_t st,uint8_t field,uint8_t value)
{
    if (st>8 || field>=(unsigned)ptbr_vstmtbl[st].nfields) return 1;
    int local=persian_pt_field_number(st,field);
    if (local<0) return 1;
    int mapped=persian_pt_value(st,field,value,local);
    return mapped<0 ? 1 : reloq61_typed_testFldeq(DELTA_ENTRY_testFldeq,state,(uint8_t)persian_pt_type(st),(uint8_t)local,(uint8_t)mapped);
}
static int32_t persian_pt_string(void *state,uint8_t st,uint8_t count,const uint8_t *source)
{
    if (st>8 || !source || count>32 || persian_pt_field_number(st,0)!=0) return 1;
    uint8_t mapped[32];
    for (unsigned i=0;i<count;i++) {
        int value=persian_pt_value(st,0,source[i],0);
        if (value<0 || value>255) return 1;
        mapped[i]=(uint8_t)value;
    }
    return test_string_s(state,(uint8_t)persian_pt_type(st),count,mapped);
}
static int32_t persian_pt_synch(void *state,int16_t tag,uint8_t count,const uint8_t *source)
{
    if (!source || count>32) return 1;
    uint8_t mapped[32];
    for (unsigned i=0;i<count;i++) {
        if (source[i]>8) return 1;
        mapped[i]=(uint8_t)persian_pt_type(source[i]);
    }
    return reloq61_typed_test_synch(DELTA_ENTRY_test_synch,state,tag,count,mapped);
}
static int32_t persian_pt_left(void *state,uint8_t st)
{ return reloq61_typed_setscan_l(DELTA_ENTRY_setscan_l,state,(uint8_t)persian_pt_type(st)); }
static int32_t persian_pt_right(void *state,uint8_t st)
{ return reloq61_typed_setscan_r(DELTA_ENTRY_setscan_r,state,(uint8_t)persian_pt_type(st)); }
static int32_t persian_pt_push_field(void *state,uint8_t st,uint8_t field)
{
    int local=persian_pt_field_number(st,field);
    return local<0 ? 1 : reloq61_typed_npush_fld(DELTA_ENTRY_npush_fld,state,(uint8_t)persian_pt_type(st),(uint8_t)local);
}
'''.replace('__CONSTANTS__', constants)
    pieces = []
    for name, original in zip(names, blocks):
        visibility = '' if name == 'por_ph_r_dur' else 'static '
        block = original.replace('int32_t reloq_ptbr_' + name + '(',
                                 visibility + 'int32_t persian_pt_' + name + '(', 1)
        block = re.sub(r'delta_sym_ref\[(\d+)\]',
                       lambda m: f'(persian_pt_refs + {index_map[int(m.group(1))]*width})', block)
        marker = '    unsigned char *frame = evv_frame_push('
        if block.count(marker) != 1:
            raise RuntimeError('Cannot locate Brazilian R workspace: ' + name)
        block = block.replace(marker, '    const int32_t persian_pt_refs=persian_pt_symbols();\n'
                              '    if (!persian_pt_refs) return 94;\n' + marker, 1)
        for original_name, target in (('testFldeq', 'field'), ('test_string_s', 'string'),
                                      ('test_synch', 'synch'), ('setscan_l', 'left'),
                                      ('setscan_r', 'right'), ('npush_fld', 'push_field')):
            block = block.replace('reloq61_typed_' + original_name + '(DELTA_ENTRY_' + original_name + ', state,',
                                  'persian_pt_' + target + '(state,')
        if name in ('por_ph_r', 'por_ph_r_dur'):
            # The dispatcher selects this donor only for a verified Persian
            # word-final R following a vowel. Keep its scan within that word,
            # without adding a pause or touching the sentence pitch plan.
            pattern = (r'    r0 = reloq61_typed_fence\(DELTA_ENTRY_fence, state, '
                       r'\(int8_t\)\(0\), EVV_AT\(const uint8_t \*, \(persian_pt_refs \+ \d+\)\)\);')
            replacement = ('    { const uint8_t word_fence[]={4};\n'
                           '      delta_state *d=state;\n'
                           '      r0=reloq61_typed_fence(DELTA_ENTRY_fence,state,1,word_fence);\n'
                           '      memset(EVV_AT(void *,d->fence_marks),0,d->nstmts); }')
            block, count = re.subn(pattern, replacement, block)
            if count != 1:
                raise RuntimeError('Cannot isolate Brazilian final-R word fence: ' + name)
        if name == 'por_ph_r_dur':
            # The donor normally checks for a phrase-ending sentinel here.
            # The Persian selector has already established the word end.
            start, end = block.index('L207:;'), block.index('L280:;')
            part = block[start:end]
            call = '    AT(int32_t, -48) = (r0);\n'
            if part.count(call) != 1:
                raise RuntimeError('Cannot locate Brazilian final-R duration decision')
            part = part.replace(call, '    AT(int32_t, -48) = (0); /* word final */\n', 1)
            block = block[:start] + part + block[end:]
        if name == 'por_ph_r':
            # At a following word, the donor's sentence-wide synchronization
            # may fail even though the R is word-final. Create the same
            # backtracking checkpoint at the current word boundary for its
            # tract targets and source scheduling.
            for first, last, tag in (('L10493:;', 'L10566:;', 15),
                                     ('L2791:;', 'L2858:;', 32),
                                     ('L3860:;', 'L3972:;', 36)):
                start, end = block.index(first), block.index(last)
                part = block[start:end]
                call = '    AT(int32_t, -48) = (r0);\n'
                if part.count(call) != 1:
                    raise RuntimeError('Cannot locate Brazilian final-R synchronization')
                part = part.replace(call,
                    f'    if (r0) r0=reloq61_typed_test_synch(DELTA_ENTRY_test_synch,state,{tag},0,0);\n' + call, 1)
                block = block[:start] + part + block[end:]
        block = block.replace('reloq_call_ptbr_add_TL_to_trill(', 'persian_pt_add_TL_to_trill(')
        for local in ('doAV', 'doAF', 'addStreamArraySsVal'):
            block = block.replace('reloq_call_ptbr_' + local + '(', 'reloq_call_fifi_' + local + '(')
        if 'reloq_call_ptbr_' in block:
            raise RuntimeError('Unmapped Brazilian R dependency: ' + name)
        pieces.append(block)
    return helper + '\n' + '\n'.join(pieces)


def prepare_uncolored_r_vowels(work: Path) -> None:
    """Keep Persian vowel targets independent of adjacent /r/, /m/ and /n/."""
    files = {'delta_rules_c17_fifi.c': ('a', 'e'),
             'delta_rules_c15_fifi.c': ('A',),
             'delta_rules_c27_fifi.c': ('o',),
             'delta_rules_c19_fifi.c': ('i',),
             'delta_rules_c21_fifi.c': ('u',)}
    helper = r'''
static int32_t persian_uncolored_vowel_field(void *state,uint8_t st,uint8_t field,uint8_t value)
{
    const char *mode=getenv("RELOQ_FA_UNCOLORED_R");
    if (mode && mode[0]=='1' && !mode[1] && st==2 && field==0 && value==19)
        return 1;
    const char *nasal=getenv("RELOQ_FA_US_NASALS");
    if (nasal && nasal[0]=='1' && !nasal[1] && st==2 && field==0 && (value==16 || value==17))
        return 1;
    if (nasal && nasal[0]=='1' && !nasal[1] && st==2 && field==4 && value==4 &&
        (reloq61_typed_testFldeq(DELTA_ENTRY_testFldeq,state,2,0,16)==0 ||
         reloq61_typed_testFldeq(DELTA_ENTRY_testFldeq,state,2,0,17)==0))
        return 1; /* vowel-only test of nasal manner, not the nasal itself */
    if (nasal && nasal[0]=='1' && !nasal[1] && st==2 && field==5 &&
        (reloq61_typed_testFldeq(DELTA_ENTRY_testFldeq,state,2,0,16)==0 ||
         reloq61_typed_testFldeq(DELTA_ENTRY_testFldeq,state,2,0,17)==0))
        return 1; /* do not substitute labial/alveolar vowel allophones */
    return reloq61_typed_testFldeq(DELTA_ENTRY_testFldeq,state,st,field,value);
}
static int32_t persian_uncolored_vowel_string(void *state,uint8_t st,uint8_t count,const uint8_t *source)
{
    const char *mode=getenv("RELOQ_FA_UNCOLORED_R");
    const char *nasal=getenv("RELOQ_FA_US_NASALS");
    int neutral_r=mode && mode[0]=='1' && !mode[1];
    int neutral_nasal=nasal && nasal[0]=='1' && !nasal[1];
    if ((!neutral_r && !neutral_nasal) || st!=2 || !source || count>32)
        return reloq61_typed_test_string_s(DELTA_ENTRY_test_string_s,state,st,count,source);
    uint8_t mapped[32];
    for (unsigned i=0;i<count;i++)
        mapped[i]=(neutral_r && source[i]==19) ||
                  (neutral_nasal && (source[i]==16 || source[i]==17)) ? 255 : source[i];
    return test_string_s(state,st,count,mapped);
}
'''
    for filename, phones in files.items():
        path = work / filename
        text = path.read_text(encoding='utf8')
        for phone in phones:
            marker = f'int32_t reloq_fifi_stanfin_ph_{phone}('
            start = text.index(marker)
            end = text.index(f'\nstatic int32_t evv_stanfin_ph_{phone}(', start)
            block = text[start:end]
            for name, target in (('testFldeq', 'field'), ('test_string_s', 'string')):
                block = block.replace('reloq61_typed_' + name + '(DELTA_ENTRY_' + name + ', state,',
                                      'persian_uncolored_vowel_' + target + '(state,')
            text = text[:start] + block + text[end:]
        first = min(phones, key=lambda phone: text.index(f'int32_t reloq_fifi_stanfin_ph_{phone}('))
        marker = f'int32_t reloq_fifi_stanfin_ph_{first}('
        text = text.replace(marker, helper + '\n' + marker, 1)
        include = '#include "reloq61_rule_calls_fifi.h"\n'
        if text.count(include) != 1:
            raise RuntimeError('Cannot locate Persian vowel rule imports')
        if '#include <stdlib.h>' not in text:
            text = text.replace(include, include + '#include <stdlib.h>\n', 1)
        path.write_text(text, encoding='utf8')


# Second-formant shift of the front /ae/ in the "clear vowels" variant (Hz).
CLEAR_FRONT_A_F2 = 200


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
            /* A small, vowel-local head-size adjustment: raise the first
               three targets by two percent and the upper two by 1.5 percent,
               retaining three quarters of the previous upper resonance
               shift while moving towards German long /a/. Avoid changing the
               speaker's head-size setting, duration, source, or bandwidths.
               Native negative inheritance markers must remain untouched. */
            int16_t *frequencies[]={&STATE(int16_t,s448),&STATE(int16_t,s449),
                &STATE(int16_t,s450),&STATE(int16_t,s451),
                &STATE(int16_t,s452),&STATE(int16_t,s453),
                &STATE(int16_t,s454),&STATE(int16_t,s455),
                &STATE(int16_t,s456),&STATE(int16_t,s457)};
            for (unsigned i=0;i<sizeof frequencies/sizeof frequencies[0];++i)
                if (*frequencies[i]>0) {
                    int scale=i<6 ? 1020 : 1015;
                    *frequencies[i]=(int16_t)((scale*(int)*frequencies[i]+500)/1000);
                }
        }
        if (!r0) persian_smaller_vowel(state,'A');
        {
            /* Listening variant: Persian /ae/ is a front vowel. Move only
               its second resonance forward; F1, the upper resonances,
               bandwidths, timing and inheritance markers are unchanged. */
            const char *clear=getenv("RELOQ_FA_CLEAR_VOWELS");
            if (!r0 && clear && (clear[0]=='1' || clear[0]=='2') && !clear[1]) {
                /* "2" is the half step for comparison by listeners. */
                int shift=clear[0]=='1' ? __FRONT_A_F2__ : __FRONT_A_F2__/2;
                if (STATE(int16_t,s450)>0) STATE(int16_t,s450)+=shift;
                if (STATE(int16_t,s451)>0) STATE(int16_t,s451)+=shift;
            }
        }
    }
'''.replace('__FRONT_A_F2__', str(CLEAR_FRONT_A_F2)), 1)
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
static void persian_smaller_vowel(void *state,int phone)
{
    const char *mode=getenv("RELOQ_FA_SMALLER_VOWELS");
    if (!mode || mode[0]!='1' || mode[1]) return;
    /* A three-percent shift corresponds to a modest head-size reduction.
       Only positive vowel targets are changed: inherited endpoints, source,
       timing and the speaker's global head-size setting remain intact. */
    int16_t *values[]={&STATE(int16_t,s448),&STATE(int16_t,s449),
        &STATE(int16_t,s450),&STATE(int16_t,s451),
        &STATE(int16_t,s452),&STATE(int16_t,s453),
        &STATE(int16_t,s454),&STATE(int16_t,s455),
        &STATE(int16_t,s456),&STATE(int16_t,s457)};
    int16_t female=0;
    memcpy(&female,(const unsigned char *)state+DG_BASE+3092+12,sizeof female);
    for (unsigned i=0;i<sizeof values/sizeof values[0];++i)
        /* On the female back-A, moving F1 across a native threshold changes
           voicing and causes a disproportionate extra jump. Leave that one
           target alone; its other formants still receive the adjustment. */
        if (!(female && phone=='a' && i<2))
        if (*values[i]>0 && *values[i]<30000)
            *values[i]=(int16_t)((103*(int)*values[i]+50)/100);
}
static void persian_back_a_f2(void *state,int16_t *target,int delta)
{
    if (*target<=0 || !delta) return;
    int16_t female=0,head=50,scale=100;
    memcpy(&female,(const unsigned char *)state+DG_BASE+3092+12,sizeof female);
    memcpy(&head,(const unsigned char *)state+DG_BASE+3092+14,sizeof head);
    memcpy(&scale,(const unsigned char *)state+DG_BASE+3070,sizeof scale);
    if (head==50) scale=100;
    if (female && scale>0) {
        /* The female voice's F2 mapping has abrupt jumps at these edges.
           Stay below a crossed edge rather than turning a small requested
           shift into a much larger one for some head sizes. */
        const int limits[]={1100,1300,1700};
        int before=*target*scale/100;
        int after=(*target+delta)*scale/100;
        for (unsigned i=0;i<sizeof limits/sizeof limits[0];++i) {
            if (before<=limits[i] && after>limits[i]) {
                int maximum=((limits[i]+1)*100-1)/scale-*target;
                if (maximum<0) maximum=0;
                if (delta>maximum) delta=maximum;
                break;
            }
        }
    }
    *target+=(int16_t)delta;
}

static void persian_open_vowel(void *state,int phone)
{
    const char *us=getenv("RELOQ_FA_US_ARTICULATION");
    if (!us || us[0]!='1' || us[1]) return;
    int16_t female=0;
    memcpy(&female,(const unsigned char *)state+DG_BASE+3092+12,sizeof female);
    const char *neutral_r=getenv("RELOQ_FA_UNCOLORED_R");
    int after_r=0,before_r=0;
    if (neutral_r && neutral_r[0]=='1' && !neutral_r[1]) {
        after_r=persian_probe_front_a_neighbour(state,STATE(int32_t,w60),1)==19;
        before_r=persian_probe_front_a_neighbour(state,STATE(int32_t,w61),0)==19;
    }
    if (after_r || before_r) {
        /* Keep each high vowel's own neutral target at the R-facing edge.
           The opposite edge can still respond to its own neighboring phone. */
        if (phone=='i' || phone=='u') {
            int f1=phone=='i' ? 250 : 280;
            int f2=phone=='i' ? 2170 : 700;
            int f3=phone=='i' ? 2900 : 2150;
            if (after_r) {
                STATE(int16_t,s448)=f1; STATE(int16_t,s450)=f2; STATE(int16_t,s452)=f3;
            }
            if (before_r) {
                STATE(int16_t,s449)=f1; STATE(int16_t,s451)=f2; STATE(int16_t,s453)=f3;
            }
        }
        /* The Finnish high-vowel rule can inherit F4 from the R. Keep the
           current speaker's neutral F4 instead, before vowel-local shaping. */
        if (after_r && STATE(int16_t,s454)==-2) STATE(int16_t,s454)=STATE(int16_t,s391);
        if (before_r && STATE(int16_t,s455)==-2) STATE(int16_t,s455)=STATE(int16_t,s391);
    }
    int f2=0,f3=0,f4=0,f5=0,b1=0,b3=0,b5=0;
    /* Use the /o/ nucleus and source targets from "two" in every context.
       Do not replace inheritance sentinels: the acoustic engine still
       interpolates into and out of neighbouring consonants. Other vowels
       retain their own centres, and /u/ keeps its restored dev121 centres. */
    if (phone=='a') {
        const char *adjust=getenv("RELOQ_FA_BACK_A_F2");
        /* A small frontward F2-only listening candidate for the vowel in
           Iran. Keep F1, the other resonances, timing and source unchanged. */
        if (adjust && adjust[0]=='1' && !adjust[1]) f2=50;
        f3=female ? -100 : 0; f4=200; b3=100;
    }
    else if (phone=='e') { f3=female ? -80 : 0; f4=50; b1=35; b3=125; }
    else if (phone=='i') { f3=female ? -80 : 0; f4=50; b3=100; }
    else if (phone=='o') {
        if (STATE(int16_t,s448)>0) STATE(int16_t,s448)=440;
        if (STATE(int16_t,s449)>0) STATE(int16_t,s449)=440;
        const char *clear=getenv("RELOQ_FA_CLEAR_VOWELS");
        /* Listening variant: a slightly less retracted Persian /o/. */
        int o_f2=!clear || clear[1] ? 900 : clear[0]=='1' ? 1000 : clear[0]=='2' ? 950 : 900;
        if (STATE(int16_t,s450)>0) STATE(int16_t,s450)=(int16_t)o_f2;
        if (STATE(int16_t,s451)>0) STATE(int16_t,s451)=(int16_t)o_f2;
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
    if (phone=='a') {
        persian_back_a_f2(state,&STATE(int16_t,s450),f2);
        persian_back_a_f2(state,&STATE(int16_t,s451),f2);
    } else {
        if (STATE(int16_t,s450)>0) STATE(int16_t,s450)+=f2;
        if (STATE(int16_t,s451)>0) STATE(int16_t,s451)+=f2;
    }
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
    text = text.replace(dispatcher, FRONT_A_HELPER + '\n' + vowel_helper + '\n' + dispatcher, 1)
    begin = text.index(dispatcher)
    end = text.index('\nstatic int32_t evv_stanfin_vals(', begin)
    section = text[begin:end]
    for phone in ('a', 'e', 'i', 'o', 'u'):
        call = f'    r0 = reloq_call_fifi_stanfin_ph_{phone}(state, (int32_t)(FIELD(0)));\n'
        if section.count(call) != 1:
            raise RuntimeError(f'Cannot locate Persian vowel dispatcher for {phone}')
        if phone == 'u':
            section = section.replace(call, r'''    {
        int16_t defaults[]={STATE(int16_t,s397),STATE(int16_t,s398),STATE(int16_t,s399),
            STATE(int16_t,s400),STATE(int16_t,s401),STATE(int16_t,s454),
            STATE(int16_t,s455),STATE(int16_t,s456),STATE(int16_t,s457)};
        r0 = reloq_call_fifi_stanfin_ph_u(state, (int32_t)(FIELD(0)));
        if (!r0) {
            persian_open_vowel(state,'u');
            const char *us=getenv("RELOQ_FA_US_ARTICULATION");
            if (us && us[0]=='1' && !us[1]) {
                /* Finnish's alveolar obstruent context opens and fronts U
                   even at the nucleus. Keep the Persian U target before
                   S/Z/T/D; the native interpolator still joins the phones. */
                unsigned next=persian_probe_front_a_neighbour(state,STATE(int32_t,w61),0);
                if(next==2 || next==5 || next==9 || next==10) {
                    if(STATE(int16_t,s448)>0)STATE(int16_t,s448)=280;
                    if(STATE(int16_t,s449)>0)STATE(int16_t,s449)=280;
                    if(STATE(int16_t,s450)>0)STATE(int16_t,s450)=700;
                    if(STATE(int16_t,s451)>0)STATE(int16_t,s451)=700;
                }
                /* German long /u/: 250/750/2400 Hz, compared to the
                   native 280/700/2150. Shift the contextual endpoints by
                   the same amount and retain inheritance markers. Restore
                   the voice's own German-like bandwidths and upper targets. */
                int16_t *f1[]={&STATE(int16_t,s448),&STATE(int16_t,s449)};
                int16_t *f2[]={&STATE(int16_t,s450),&STATE(int16_t,s451)};
                int16_t *f3[]={&STATE(int16_t,s452),&STATE(int16_t,s453)};
                for (unsigned i=0;i<2;i++) {
                    if (*f1[i]>30) *f1[i]-=30;
                    if (*f2[i]>0) *f2[i]+=50;
                    if (*f3[i]>0) *f3[i]+=250;
                }
                /* These saved bandwidths have already gone through the
                   voice transformation. The German first bandwidth is
                   120 Hz for male voices and 300 Hz for female voices,
                   including small female head sizes. */
                int16_t female=0;
                memcpy(&female,(const unsigned char *)state+DG_BASE+3092+12,sizeof female);
                STATE(int16_t,s397)=female?300:120; STATE(int16_t,s398)=defaults[1];
                STATE(int16_t,s399)=defaults[2]; STATE(int16_t,s400)=defaults[3];
                STATE(int16_t,s401)=defaults[4]; STATE(int16_t,s454)=defaults[5];
                STATE(int16_t,s455)=defaults[6]; STATE(int16_t,s456)=defaults[7];
                STATE(int16_t,s457)=defaults[8];
            }
            persian_smaller_vowel(state,'u');
        }
    }
''', 1)
        else:
            section = section.replace(call, call +
                f"    if (!r0) persian_open_vowel(state,'{phone}');\n" +
                (f"    if (!r0) persian_smaller_vowel(state,'{phone}');\n" if phone != 'e' else ''), 1)
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

    # Source tilt is scheduled later than the vowel formants. Correct the
    # female tilt at the point where the current sonorant is emitted; changing
    # it inside the vowel target rule is overwritten before this call. Keep
    # the voice-dependent baseline and restore the surrounding state afterward.
    sonorant_file = work / 'delta_rules_c13_fifi.c'
    text = sonorant_file.read_text(encoding='utf8')
    helper_point = 'int32_t reloq_fifi_insert_acoustic_vals(void *state, int32_t p0)\n{\n'
    if text.count(helper_point) != 1:
        raise RuntimeError('Cannot locate Persian acoustic-value scheduler')
    text = text.replace(helper_point, FRONT_A_HELPER + '\n' + helper_point + r'''    {
        const char *selected=getenv("RELOQ_FA_US_ARTICULATION");
        if (selected && selected[0]=='1' && !selected[1]) {
            unsigned current=persian_probe_front_a_neighbour(state,STATE(int32_t,w61),1);
            unsigned following=persian_probe_front_a_neighbour(state,STATE(int32_t,w61),0);
            if(STATE(int32_t,f10)>=0 && STATE(int32_t,f11)>STATE(int32_t,f10))
                reloq61_phone_identity_trace_emit(current);
            unsigned target=current>0 && current<27 &&
                persian_probe_following_final_h(state,STATE(int32_t,w61))?RELOQ_FA_TUNE_BEFORE_FINAL_H:
                current==31?RELOQ_FA_TUNE_FRONT_A:current==34?RELOQ_FA_TUNE_BACK_A:
                current==29?RELOQ_FA_TUNE_E:current==33?RELOQ_FA_TUNE_O:
                current==32?RELOQ_FA_TUNE_U:
                current==19 && following==27?RELOQ_FA_TUNE_R_BEFORE_I:
                current==19 && following==34?RELOQ_FA_TUNE_R_BEFORE_BACK_A:
                current==19?RELOQ_FA_TUNE_R:RELOQ_FA_TUNE_OTHER;
            reloq61_fa_phone_emit(state,STATE(int32_t,f10),STATE(int32_t,f11),target);
        }
    }
''', 1)
    text = text.replace('#include "reloq_syllable_trace.h"',
                        '#include "reloq_syllable_trace.h"\n#include "reloq_persian_tuning.h"', 1)
    son_call = '    r0 = reloq_call_fifi_do_son_amps(state, (int32_t)(FIELD(0)));\n'
    if text.count(son_call) != 1:
        raise RuntimeError('Cannot locate Persian sonorant source scheduler')
    text = text.replace(son_call, r'''    {
        const char *british=getenv("RELOQ_FA_BRITISH_FRONT_A");
        int16_t female=0;
        memcpy(&female,(const unsigned char *)state+DG_BASE+3092+12,sizeof female);
        int selected=british && british[0]=='1' && !british[1] && female &&
            persian_probe_front_a_neighbour(state,STATE(int32_t,w61),1)==31;
        int16_t old_target=STATE(int16_t,s405),old_mode=STATE(int16_t,s436);
        if (selected) {
            STATE(int16_t,s405)=STATE(int16_t,s444);
            STATE(int16_t,s436)=1;
        }
        r0 = reloq_call_fifi_do_son_amps(state, (int32_t)(FIELD(0)));
        if (selected) {
            STATE(int16_t,s405)=old_target;
            STATE(int16_t,s436)=old_mode;
        }
    }
''', 1)
    sonorant_file.write_text(text, encoding='utf8')

    # The nucleus AV is recalculated *inside* the sonorant scheduler, after
    # the U formants are selected. Its Finnish female target is about 5 dB
    # below the German U, making otherwise matching resonances too quiet.
    # Apply the measured source correction only to this vowel and voice;
    # retain native stress/dynamics, consonant transitions and the male voice.
    # Restore the calculated source values after the acoustic points are added.
    source_file = work / 'delta_rules_c15_fifi.c'
    text = source_file.read_text(encoding='utf8')
    if 'static unsigned persian_probe_front_a_neighbour(' not in text:
        raise RuntimeError('Cannot locate Persian sonorant phone selector')
    entry = 'int32_t reloq_fifi_do_son_amps(void *state, int32_t p0)\n{\n'
    if text.count(entry) != 1:
        raise RuntimeError('Cannot locate Persian sonorant source rule')
    begin = text.index(entry)
    end = text.index('\nstatic int32_t evv_do_son_amps(', begin)
    section = text[begin:end]
    section = section.replace(entry, entry +
        '    volatile int16_t saved_u_av=0, saved_u_avs=0;\n    volatile int restore_u_av=0;\n', 1)
    nuc = '    r0 = reloq_call_fifi_get_nuc_AV(state, (int32_t)(FIELD(0)));\n'
    if section.count(nuc) != 1:
        raise RuntimeError('Cannot locate Persian vowel source target')
    section = section.replace(nuc, nuc + r'''    {
        const char *us=getenv("RELOQ_FA_US_ARTICULATION");
        int16_t female=0;
        memcpy(&female,(const unsigned char *)state+DG_BASE+3092+12,sizeof female);
        if (us && us[0]=='1' && !us[1] && female &&
            persian_probe_front_a_neighbour(state,STATE(int32_t,w61),1)==32) {
            saved_u_av=STATE(int16_t,s394); saved_u_avs=STATE(int16_t,s395);
            restore_u_av=1;
            if (STATE(int16_t,s394)>0 && STATE(int16_t,s394)<=75) STATE(int16_t,s394)+=5;
            if (STATE(int16_t,s395)>0 && STATE(int16_t,s395)<=75) STATE(int16_t,s395)+=5;
        }
    }
''', 1)
    for label in ('L5185:;', 'L5204:;'):
        if section.count(label) != 1:
            raise RuntimeError('Cannot locate Persian sonorant rule exit')
        section = section.replace(label, label + r'''
    if (restore_u_av) {
        STATE(int16_t,s394)=saved_u_av;
        STATE(int16_t,s395)=saved_u_avs;
    }''', 1)
    text = text[:begin] + section + text[end:]
    source_file.write_text(text, encoding='utf8')

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
static int persian_direct_following_vowel(void *state)
{
    delta_state *d=state;
    int32_t position=STATE(int32_t,w61);
    if (!d || !position) return 0;
    int32_t *point=vmover(d,EVV_AT(int32_t *,position),2);
    int32_t node=point?vgetsc(d,0,0,EVV_REF(point),2):0;
    if (!node || (*EVV_AT(int32_t *,node)&2)) return 0;
    void *fields=TFLDS(EVV_AT(void *,node));
    unsigned cls=*(const unsigned char *)delta_record_field(&vstmtbl[2],1,fields);
    unsigned onset=*(const unsigned char *)delta_record_field(&vstmtbl[2],9,fields);
    /* Native H can be represented as an onset feature of the next vowel,
       rather than a separate phone. Such a word starts with a consonant. */
    return cls==1 && onset!=1 && onset!=2;
}
static int persian_r_follows_h(void *state)
{
    delta_state *d=state;
    int32_t position=STATE(int32_t,w61);
    if (!d || !position) return 0;
    delta_node *point=vmovel(d,EVV_AT(delta_node *,position),2);
    int32_t node=point?vgetsc(d,1,0,EVV_REF(point),2):0;
    if (!node || (*EVV_AT(int32_t *,node)&2)) return 0;
    void *fields=TFLDS(EVV_AT(void *,node));
    unsigned phone=*(const unsigned char *)delta_record_field(&vstmtbl[2],0,fields);
    unsigned onset=*(const unsigned char *)delta_record_field(&vstmtbl[2],9,fields);
    /* The native mapper folds H into this R's onset feature. */
    return phone==19 && (onset==1 || onset==2);
}
/* Restrict the US /y/ donor to the first actual phone of each word. Use the
   provider's native word fence, and restore every scanner register: looking
   at a preceding phone alone would confuse the preceding word with this one. */
static int persian_articulation_word_initial(void *state,int32_t token_ref)
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
    int initial=!setscan_l(d,2) && advance_tok(d);
    d->lpta=left; d->rpta=right; memcpy(&v->scan_ptr,scan,sizeof scan);
    v->compared_equal=compared; v->fence_count=fences; v->testing=testing;
    memcpy(EVV_AT(void *,d->fence_index),index,d->nstmts);
    memcpy(EVV_AT(void *,d->fence_marks),marks,d->nstmts);
    memcpy(EVV_AT(void *,d->fence_chars),chars,d->nstmts);
    return initial;
}
/* Check the two phones before /x/ within the same native word. The word
   fence is essential: an /i r/ ending one word must not change /x/ in the
   next. As with the other context probes, leave all scanner state intact. */
static int persian_x_after_ir(void *state,int32_t token_ref)
{
    const char *x=getenv("RELOQ_FA_PROBE_X");
    const char *tuning=getenv("RELOQ_FA_R_AFTER_I");
    delta_state *d=state;
    if (!x || x[0]!='1' || x[1] || !tuning || tuning[0]!='1' || tuning[1] ||
        !d || !d->vars || d->nstmts<5 || d->nstmts>256 || !token_ref) return 0;
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
    int selected=!setscan_l(d,2) && !testFldeq(d,2,0,19) && !advance_tok(d) &&
                 !testFldeq(d,2,0,27) && !advance_tok(d);
    d->lpta=left; d->rpta=right; memcpy(&v->scan_ptr,scan,sizeof scan);
    v->compared_equal=compared; v->fence_count=fences; v->testing=testing;
    memcpy(EVV_AT(void *,d->fence_index),index,d->nstmts);
    memcpy(EVV_AT(void *,d->fence_marks),marks,d->nstmts);
    memcpy(EVV_AT(void *,d->fence_chars),chars,d->nstmts);
    return selected;
}
/* The Brazilian word-final R is selected only immediately after a vowel in
   the same word. Consonant clusters retain the earlier Persian R for now. */
int persian_pt_coda_r_selected(void *state)
{
    const char *mode=getenv("RELOQ_FA_BRAZILIAN_CODA_R");
    if (!mode || mode[0]!='1' || mode[1]) return 0;
    delta_state *d=state;
    int32_t begin=STATE(int32_t,w60);
    if (!d || !begin || !persian_probe_r_word_final(state,STATE_AT(w60,-4)) ||
        persian_articulation_word_initial(state,STATE_AT(w60,-4))) return 0;
    delta_node *point=vmovel(d,EVV_AT(delta_node *,begin),2);
    int32_t node=point?vgetsc(d,1,0,EVV_REF(point),2):0;
    if (!node || (*EVV_AT(int32_t *,node)&2)) return 0;
    if (*(const unsigned char *)delta_record_field(&vstmtbl[2],1,
                                                    TFLDS(EVV_AT(void *,node)))!=1) return 0;
    unsigned following=persian_articulation_following(state);
    if (persian_r_follows_h(state) && following && following!=36) return 0;
    /* A directly following vowel begins the next word. In that position
       the R joins the vowel instead of taking the word-final fricative.
       A pause/phrase boundary is phone 36, so it keeps the final R. */
    return !persian_direct_following_vowel(state);
}
/* The unvoiced final R may also follow a stop within the same word. */
int persian_f_coda_r_selected(void *state)
{
    const char *mode=getenv("RELOQ_FA_VOICELESS_F_R");
    if (!mode || mode[0]!='1' || mode[1]) return 0;
    delta_state *d=state;
    int32_t begin=STATE(int32_t,w60);
    if (!d || !begin || !persian_probe_r_word_final(state,STATE_AT(w60,-4)) ||
        persian_articulation_word_initial(state,STATE_AT(w60,-4))) return 0;
    delta_node *point=vmovel(d,EVV_AT(delta_node *,begin),2);
    int32_t node=point?vgetsc(d,1,0,EVV_REF(point),2):0;
    if (!node || (*EVV_AT(int32_t *,node)&2)) return 0;
    unsigned phone=*(const unsigned char *)delta_record_field(&vstmtbl[2],0,
                                                               TFLDS(EVV_AT(void *,node)));
    if (phone>=1 && phone<=7) return 1; /* preserve p,t,k,b,d,g,f */
    /* The same unvoiced coda can follow other consonants when another
       word continues directly. Keep isolated words and pauses unchanged. */
    unsigned next=persian_articulation_following(state);
    return next && next!=36 && ((phone>=8 && phone<=23) || persian_r_follows_h(state));
}
static void persian_r_less_retroflex(void *state)
{
    const char *mode=getenv("RELOQ_FA_R_ALVEOLAR");
    if (mode && mode[0]=='1' && !mode[1]) {
        /* Legacy experimental all-R trial; not selected by the Persian core. */
        if (STATE(int16_t,s452)>0 && STATE(int16_t,s452)<2450) STATE(int16_t,s452)=2450;
        if (STATE(int16_t,s453)>0 && STATE(int16_t,s453)<2450) STATE(int16_t,s453)=2450;
        if (STATE(int16_t,s454)>0 && STATE(int16_t,s454)<3150) STATE(int16_t,s454)=3150;
        if (STATE(int16_t,s455)>0 && STATE(int16_t,s455)<3150) STATE(int16_t,s455)=3150;
        return;
    }
    mode=getenv("RELOQ_FA_R_AFTER_I");
    if (!mode || mode[0]!='1' || mode[1]) return;
    delta_state *d=state;
    int32_t begin=STATE(int32_t,w60);
    if (!d || !begin) return;
    delta_node *point=vmovel(d,EVV_AT(delta_node *,begin),2);
    int32_t node=point?vgetsc(d,1,0,EVV_REF(point),2):0;
    unsigned previous=node && !(*EVV_AT(int32_t *,node)&2) ?
        *(const unsigned char *)delta_record_field(&vstmtbl[2],0,TFLDS(EVV_AT(void *,node))) : 0;
    unsigned following=persian_articulation_following(state);
    if (following==33) return; /* Finnish R before /o/. */
    if (previous!=27 && following!=27) return;
    if (previous==27 && following!=27 &&
        persian_articulation_word_initial(state,STATE_AT(w60,-4))) return;
    /* After /i/ inside the word and directly before a vowel, retain some
       of the anti-retroflex adjustment without the previous sharp F3 jump.
       Word-final R, consonant clusters and ordinary R before /i/ keep their
       existing paths. Finnish's accepted R before /o/ remains above. */
    if (previous==27 && persian_direct_following_vowel(state) &&
        !persian_articulation_word_initial(state,STATE_AT(w60,-4)) &&
        !persian_probe_r_word_final(state,STATE_AT(w60,-4))) {
        if (STATE(int16_t,s452)>0 && STATE(int16_t,s452)<2700) {
            int value=STATE(int16_t,s452)+175;
            STATE(int16_t,s452)=(int16_t)(value<2700?value:2700);
        }
        if (STATE(int16_t,s453)>0 && STATE(int16_t,s453)<2700) {
            int value=STATE(int16_t,s453)+175;
            STATE(int16_t,s453)=(int16_t)(value<2700?value:2700);
        }
        return;
    }
    /* After the back /a/ and before /i/, the former 2700-Hz target can
       make the third resonance conspicuous in names such as Bakhtiari.
       Keep the user's R-before-I level filter and the vowel's own targets. */
    if (previous==34 && following==27 &&
        !persian_articulation_word_initial(state,STATE_AT(w60,-4))) {
        if (STATE(int16_t,s452)>0 && STATE(int16_t,s452)<2525) STATE(int16_t,s452)=2525;
        if (STATE(int16_t,s453)>0 && STATE(int16_t,s453)<2525) STATE(int16_t,s453)=2525;
        return;
    }
    /* The low F3 of Finnish R can sound retroflex near /i/. Before I,
       bring the R targets to 2700 Hz; after I, retain the accepted gentler
       adjustment. The vowels' own targets, F4, source and timing stay intact. */
    if (STATE(int16_t,s452)>0 && STATE(int16_t,s452)<2700)
        STATE(int16_t,s452)=(following==27 || STATE(int16_t,s452)>2350)?2700:STATE(int16_t,s452)+350;
    if (STATE(int16_t,s453)>0 && STATE(int16_t,s453)<2700)
        STATE(int16_t,s453)=(following==27 || STATE(int16_t,s453)>2350)?2700:STATE(int16_t,s453)+350;
}
/* Only act if the third and fourth R resonances nearly collide at either
   endpoint. This covers the same acoustic problem in different surrounding
   phones and in both Persian text and Pinglish. Avoid changing final R and
   the independent vowel targets. */
static void persian_r_separate_upper_resonances(void *state)
{
    const char *mode=getenv("RELOQ_FA_R_AFTER_I");
    if (!mode || mode[0]!='1' || mode[1] ||
        persian_probe_r_word_final(state,STATE_AT(w60,-4))) return;
    int f3=STATE(int16_t,s452),f3_end=STATE(int16_t,s453);
    int f4=STATE(int16_t,s454),f4_end=STATE(int16_t,s455);
    if (!((f3>0 && f4>0 && f4-f3<250) ||
          (f3_end>0 && f4_end>0 && f4_end-f3_end<250))) return;
    if (f3>0 && f4>0 && f4<f3+600) STATE(int16_t,s454)=(int16_t)(f3+600);
    if (f3_end>0 && f4_end>0 && f4_end<f3_end+600) STATE(int16_t,s455)=(int16_t)(f3_end+600);
}
/* The accepted back-A has a much higher F3/F4 than Finnish R. Move only
   the release end of the R towards that vowel before its own targets are
   emitted; the beginning of the R, the A nucleus, timing, and excitation
   stay intact. This follows the actual neighbouring phone, including across
   a word boundary, and never changes an R before another vowel or pause. */
static void persian_r_release_to_back_a(void *state)
{
    const char *mode=getenv("RELOQ_FA_UNCOLORED_R");
    if (!mode || mode[0]!='1' || mode[1] ||
        persian_articulation_following(state)!=34 ||
        !persian_direct_following_vowel(state)) return;
    int f3=STATE(int16_t,s453),f4=STATE(int16_t,s455);
    if (f3>0 && f3<2500) STATE(int16_t,s453)=(int16_t)((f3+2*2500+1)/3);
    if (f4>0 && f4<3400) STATE(int16_t,s455)=(int16_t)((f4+2*3400+1)/3);
}
'''
    if text.count(include) != 1:
        raise RuntimeError('Cannot locate consonant dispatcher includes')
    text = text.replace(include, include + '#include <stdlib.h>\n', 1)
    marker = 'int32_t reloq_fifi_fin_con_vals(void *state, int32_t p0)\n{\n'
    if text.count(marker) != 1:
        raise RuntimeError('Cannot locate consonant dispatcher')
    # STATE offsets are declared after the includes in generated sources.
    final_r = FINNISH_L_WORD_FINAL.replace('persian_probe_l_word_final',
                                           'persian_probe_r_word_final').replace(
                                               'testFldeq(d,2,0,21)', 'testFldeq(d,2,0,19)')
    final_t = FINNISH_L_WORD_FINAL.replace('persian_probe_l_word_final',
                                           'persian_probe_t_word_final').replace(
                                               'testFldeq(d,2,0,21)', 'testFldeq(d,2,0,2)')
    text = text.replace(marker, us_consonant_rules() + '\n' + german_j_rules() + '\n' + brazilian_r_rules() +
                        '\n' + final_r + '\n' + final_t + '\n' + helper + '\n' + marker, 1)
    for phone in 'ptkbdgfvszSZ':
        call = f'    r0 = reloq_call_fifi_fin_ph_{phone}(state, (int32_t)(FIELD(0)));\n'
        if text.count(call) != 1:
            raise RuntimeError(f'Cannot locate Persian consonant dispatcher for {phone}')
        condition = (' && !persian_articulation_special_k(state)' if phone == 'k' else '')
        key = 'RELOQ_FA_US_STOPS' if phone in 'ptkbdg' else 'RELOQ_FA_US_FRICATIVES'
        t_release = (r'''            if (!r0 && persian_articulation_following(state)>=4 &&
                persian_articulation_following(state)<=6 &&
                !persian_probe_t_word_final(state,STATE_AT(w60,-4)) &&
                STATE(int16_t,s430)>0 && STATE(int16_t,s430)<60) {
                int level=STATE(int16_t,s430)+8;
                STATE(int16_t,s430)=(int16_t)(level<60?level:60);
            }
''' if phone == 't' else '')
        replacement = ('    {\n' + f'        const char *us=getenv("{key}");\n' +
                       f"        if (us && us[0]=='1' && !us[1]{condition})\n" +
                       '        {\n' + f'            r0=persian_us_eng_ph_{phone}(state,(int32_t)(FIELD(0)));\n' +
                       t_release + '        }\n' +
                       '        else\n' + call + '    }\n')
        text = text.replace(call, replacement, 1)
    call = '    r0 = reloq_call_fifi_fin_ph_G(state, (int32_t)(FIELD(0)));\n'
    if text.count(call) != 1:
        raise RuntimeError('Cannot locate Persian /x/ dispatcher')
    text = text.replace(call, call + r'''    if (!r0 && persian_x_after_ir(state,STATE_AT(w60,-4))) {
        /* Do not pull the voiced r toward the very low /x/ starting F3.
           Keep /x/'s existing end target, noise bands, source and timing. */
        STATE(int16_t,s452)=2200;
    }
''', 1)
    call = '    r0 = reloq_call_fifi_fin_ph_j(state, (int32_t)(FIELD(0)));\n'
    if text.count(call) != 1:
        raise RuntimeError('Cannot locate Persian /j/ dispatcher')
    text = text.replace(call, r'''    {
        const char *us=getenv("RELOQ_FA_US_GLIDE");
        if (us && us[0]=='1' && !us[1] &&
            persian_articulation_word_initial(state,STATE_AT(w60,-4))) {
            r0=persian_de_stanger_ph_j(state,(int32_t)(FIELD(0)));
            if (!r0) r0=persian_de_ger_ph_j(state,(int32_t)(FIELD(0)));
        } else {
''' + call + '        }\n    }\n', 1)
    call = '    r0 = reloq_call_fifi_fin_ph_r(state, (int32_t)(FIELD(0)));\n'
    if text.count(call) != 1:
        raise RuntimeError('Cannot locate Persian /r/ dispatcher')
    text = text.replace(call, r'''    {
        if (persian_pt_coda_r_selected(state))
            r0=persian_pt_por_ph_r(state,(int32_t)(FIELD(0)));
        else {
''' + call + r'''            if (!r0) {
                persian_r_less_retroflex(state);
                if (persian_f_coda_r_selected(state)) {
                    /* Brazilian R-style frication bands, without voicing.
                       The scheduler treats this R as a non-sonorant below. */
                    STATE(int16_t,s402)=0; STATE(int16_t,s403)=0;
                    STATE(int16_t,s405)=0; STATE(int16_t,s404)=50;
                    STATE(int16_t,s408)=0; STATE(int16_t,s409)=0;
                    STATE(int16_t,s410)=65; STATE(int16_t,s411)=45;
                    STATE(int16_t,s412)=0; STATE(int16_t,s413)=0;
                    STATE(int16_t,s432)=0; STATE(int16_t,s433)=0;
                    STATE(int16_t,s434)=0; STATE(int16_t,s436)=0;
                } else {
                    persian_r_separate_upper_resonances(state);
                    persian_r_release_to_back_a(state);
                }
            }
        }
    }
''', 1)
    # Leave both nasal acoustic targets to the native Finnish phone rules.
    # Their US-derived phone and transition durations are selected separately
    # by prepare_q_phone and prepare_auto_finnish_h above.
    for phone in 'mn':
        call = f'    r0 = reloq_call_fifi_fin_ph_{phone}(state, (int32_t)(FIELD(0)));\n'
        if text.count(call) != 1:
            raise RuntimeError(f'Cannot locate Persian nasal dispatcher for {phone}')
    consonant_file.write_text(text, encoding='utf8')
    scheduler_file = work / 'delta_rules_c13_fifi.c'
    text = scheduler_file.read_text(encoding='utf8')
    include = '#include "reloq61_rule_calls_fifi.h"\n'
    call = '    r0 = reloq_call_fifi_do_son_amps(state, (int32_t)(FIELD(0)));\n'
    if text.count(include) != 1 or text.count(call) != 1:
        raise RuntimeError('Cannot locate Persian sonorant scheduler')
    text = text.replace(include, include + 'extern int persian_f_coda_r_selected(void *);\n'
                        'extern int reloq61_array_bank_zero_segment(void *,int,int32_t,int32_t);\n', 1)
    # Voicing points may already be planned ahead while visiting the vowel.
    # Replace only this phone's points before emitting its frication.
    silent_source = r'''    if (persian_f_coda_r_selected(state)) {
        STATE(int16_t,s402)=0; STATE(int16_t,s403)=0; STATE(int16_t,s405)=0;
        int32_t from=delta_fixed_int(STATE(int32_t,f10));
        int32_t to=delta_fixed_int(STATE(int32_t,f11));
        (void)reloq61_array_bank_zero_segment(state,STATE(int16_t,s64),from,to);
        (void)reloq61_array_bank_zero_segment(state,STATE(int16_t,s65),from,to);
        (void)reloq61_array_bank_zero_segment(state,STATE(int16_t,s60),from,to);
        STATE(int16_t,s432)=1; STATE(int16_t,s433)=1; STATE(int16_t,s436)=1;
        r0 = reloq_call_fifi_do_non_son_amps(state, (int32_t)(FIELD(0)));
    } else
'''
    text = text.replace(call, silent_source + call, 1)
    fallback = 'L972:;\n    r0 = reloq_call_fifi_do_non_son_amps(state, (int32_t)(FIELD(0)));\n'
    if text.count(fallback) != 1:
        raise RuntimeError('Cannot locate Persian non-sonorant scheduler')
    text = text.replace(fallback, 'L972:;\n' + silent_source +
                        '    r0 = reloq_call_fifi_do_non_son_amps(state, (int32_t)(FIELD(0)));\n', 1)
    scheduler_file.write_text(text, encoding='utf8')
    duration_file = work / 'delta_rules_c23_fifi.c'
    text = duration_file.read_text(encoding='utf8')
    include = '#include "reloq61_rule_calls_fifi.h"\n'
    call = '    r0 = reloq_call_fifi_fin_ph_r_dur(state, (int32_t)(FIELD(0)));\n'
    if text.count(include) != 1 or text.count(call) != 1:
        raise RuntimeError('Cannot locate Persian R duration dispatcher')
    text = text.replace(include, include +
                        'extern int persian_pt_coda_r_selected(void *);\n'
                        'extern int32_t persian_pt_por_ph_r_dur(void *,int32_t);\n', 1)
    text = text.replace(call,
                        '    if (persian_pt_coda_r_selected(state))\n'
                        '        r0=persian_pt_por_ph_r_dur(state,(int32_t)(FIELD(0)));\n'
                        '    else\n' + call, 1)
    duration_file.write_text(text, encoding='utf8')


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
    prepare_french_nasals(work)
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
    modified.append(work / 'delta_rules_c04_fifi.c')
    for filename in ('delta_rules_c19_fifi.c', 'delta_rules_c21_fifi.c', 'delta_rules_c27_fifi.c'):
        dest = work / filename
        if not dest.exists():
            shutil.copyfile(PROVIDER / 'lang' / 'fifi' / filename, dest)
        modified.append(dest)
    prepare_uncolored_r_vowels(work)
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
