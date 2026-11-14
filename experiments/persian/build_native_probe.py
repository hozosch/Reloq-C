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
    return value && value[0] >= '1' && value[0] <= '6' && !value[1]
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
    redirect = r'''    const char *fa_l=getenv("RELOQ_FA_PROBE_L");
    if (fa_l && fa_l[0]=='2' && !fa_l[1])
        return persian_probe_german_l(state,p0);
'''
    text = text.replace('#include <stdlib.h>\n', '#include <stdlib.h>\n#include <stdio.h>\n', 1)
    text = text.replace(marker, FINNISH_L_CONTEXT + '\n' + rule + '\n' + marker + redirect, 1)
    destination.write_text(text, encoding='utf8')
    native = prepare('dede61', 'delta_rules_c05_dede61.c', 'ger_ph_l',
                     'L2680', GERMAN_L_TRACE, '#include <stdio.h>\n', work)
    return [destination, native]

Q_PATCH = r'''
    {
        int variant = persian_probe_setting("RELOQ_FA_PROBE_Q");
        if (variant && (variant==6 || persian_probe_source_q(state, STATE(int32_t,w61)))) {
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
    }
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
    }
'''

Q_CLUSTER_HELPER = r'''
/* Private listening build: schedule the selected q's release when the
   ordinary Finnish stop+stop path would suppress it. Source spelling is a
   probe switch, not a replacement for a Persian phoneme identity. */
static int persian_probe_before_stop(void *state)
{
    const char *setting=getenv("RELOQ_FA_PROBE_Q");
    if (!setting || setting[1] || setting[0]<'3' || setting[0]>'5') return 0;
    delta_state *d=state;
    int32_t position=STATE(int32_t,w61);
    if (!position) return 0;
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
static void persian_auto_h_insert(void *state)
{
    const char *setting=getenv("RELOQ_FA_PROBE_H");
    if (!setting || (setting[0]!='2' && setting[0]!='3' && setting[0]!='4' &&
                     setting[0]!='5') || setting[1]) return;
    delta_state *d=state;
    int32_t begin=STATE(int32_t,w60),end=STATE(int32_t,w61);
    int32_t previous=persian_auto_h_phone(d,begin,1);
    int32_t stop=persian_auto_h_phone(d,end,1);
    int32_t vowel=persian_auto_h_phone(d,STATE(int32_t,w63),0);
    if (!stop || !vowel || persian_auto_h_field(vowel,1)!=1 ||
        (previous && persian_auto_h_field(previous,1)!=1)) return;
    unsigned phone=persian_auto_h_field(stop,0);
    const char *qsetting=getenv("RELOQ_FA_PROBE_Q");
    int forced_q=qsetting && qsetting[0]=='6' && !qsetting[1];
    if (phone<1 || phone>3 || (phone==3 && (forced_q || persian_auto_h_is_q(d,end)))) return;
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
    modified.extend(prepare_german_l(work))
    modified.extend(prepare_x(work))
    modified.append(prepare_q_cluster(work))
    modified.extend(prepare_question_rise(work))
    modified.append(prepare_finnish_h(work))
    modified.append(prepare_auto_finnish_h(work))
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
