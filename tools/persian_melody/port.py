"""Generate the US-English melody rules for the Persian provider.

Everything the English `generate_intonation` reaches is translated into the
Persian provider's numbering and emitted as a self-contained set of static
`persian_us_*` rules. The set includes rules that are identical in both
providers, so that only one call has to choose between the two melodies: the
call of `generate_intonation` itself. Nothing selects the US rules unless the
scoped key RELOQ_FA_US_MELODY is "1".
"""
from __future__ import annotations

import collections
from pathlib import Path
import re

from translate import LANG, Records, load_rules, shared_symbols, translate

# Rules that are replaced as a whole, with the prepared file that calls each.
# `generate_intonation` produces the pitch targets; `adjust_accents` decides
# before that which words carry an accent. The per-word and per-syllable setup
# rules (`init_word_vars`, `init_syll_vars`) are deliberately not here: taken
# over whole they also change durations.
ROOTS = {'generate_intonation': 'delta_rules_c04_fifi.c',
         'adjust_accents': 'delta_rules_c05_fifi.c',
         # Chooses the tones of each phrase from its kind. The rule is the
         # same text in both providers but asks which language is speaking:
         # a Finnish question falls, an English one rises.
         'get_nuclear_accent': 'delta_rules_c05_fifi.c'}
# The Persian duration rules find the word that carries the sentence accent
# before the melody is made, with the Finnish rule. The US melody looks for it
# again with the US rule; the variables that point at it are put back after.
NUCLEUS_RULE = 'find_nucleus'
NUCLEUS_VARIABLES = ('w24', 'w26', 'w27')
ROOT_RULES = tuple(ROOTS) + (NUCLEUS_RULE,)
FILES = set(ROOTS.values())
SHARED = ('struct persian_us_saved { struct delta_field_overlay *overlay; int16_t level[4],language,word[2]; };\n'
          'int persian_us_melody_selected(void);\n'
          'void persian_us_enter(void *state,struct persian_us_saved *saved);\n'
          'void persian_us_leave(void *state,const struct persian_us_saved *saved);\n'
          'void persian_us_keep(int on);\n'
          'void persian_us_done(void);\n'
          'void persian_us_begin(void);\n'
          'void persian_us_nucleus(void *state,int restore);')
# Roots whose Persian rule must still run: the Persian durations depend on what
# it decides. The US rule runs first and its decisions are kept in an overlay
# that only the US rules read.
PAIRED = ('adjust_accents', 'get_nuclear_accent')
HOME = 'delta_rules_c04_fifi.c'
# Word stress is counted on different scales. English: 3 for a stressed word,
# 5 for the word carrying the sentence accent, 6 and 7 for emphasis. The
# Persian provider (Finnish rules): 1, 2, 3 and 4. The levels are kept in four
# variables that each provider sets once, and in the word records. While the
# US rules run, the variables hold the English numbers and the records are
# read on the English scale. Name: (English, Persian).
STRESS = {'s236': (5, 2), 's237': (3, 1), 's242': (6, 3), 's243': (7, 4)}
KEY = 'RELOQ_FA_US_MELODY'
WIDTH = 8


def authored(language, prefix):
    """The provider's authored constants and the offset of each symbol."""
    root = LANG / language
    data = (root / f'delta_authored_{prefix}.c').read_text(encoding='utf8')
    data = data.split(f'uint8_t {prefix}_evv_authored_data[] = {{', 1)[1].split('};', 1)[0]
    data = re.sub(r'/\*.*?\*/', '', data, flags=re.S)
    raw = bytes(int(v, 0) for v in re.findall(r'(?<![A-Za-z_])(?:0[xX][0-9a-fA-F]+|[0-9]+)', data))
    table = (root / f'delta_rules_{prefix}.c').read_text(encoding='utf8')
    table = table.split(f'{prefix}_delta_rule_sym[] = {{', 1)[1].split('};', 1)[0]
    offsets = [int(v) for v in re.findall(prefix + r'_evv_authored_data \+ (\d+)', table)]
    return raw, offsets


def constant(raw, offsets, index):
    following = [o for o in offsets if o > offsets[index]]
    return raw[offsets[index]:min(following) if following else len(raw)]


def reachable(rules, roots):
    seen, todo = set(), list(roots)
    while todo:
        name = todo.pop()
        if name in seen or name not in rules:
            continue            # a machine-level helper both providers have
        seen.add(name)
        todo.extend(re.findall(r'reloq_call_enus61_(\w+)\(', rules[name]))
    return sorted(seen)


def generate():
    """Return (C source, report) for the US melody rules."""
    english, persian = load_rules('enus61', 'enus61'), load_rules('fifi', 'fifi')
    table, symbols = Records(), shared_symbols(english, persian)
    names = reachable(english, ROOT_RULES)
    for name, (us, own) in STRESS.items():
        for rules, value in ((english, us), (persian, own)):
            wanted = 'STATE(int16_t, %s) = (%d);' % (name, value)
            if not any(wanted in body for body in rules.values()):
                raise RuntimeError('unexpected stress scale: ' + name)
    stress_field = table.field(4, 7)
    layout = (LANG / 'fifi' / HOME).read_text(encoding='utf8')
    offsets = {name: int(value) for name, value in
               re.findall(r'^#define DG_(\w+)\s+\(DG_BASE \+\s*(\d+)\)', layout, re.M)}
    ordered = sorted(set(offsets.values()))
    english_raw, english_offsets = authored('enus61', 'enus61')
    persian_raw, persian_offsets = authored('fifi', 'fifi')
    by_bytes = {}
    for index in range(len(persian_offsets)):
        by_bytes.setdefault(constant(persian_raw, persian_offsets, index), index)
    embedded, report = [], collections.Counter()
    helpers = (LANG / 'fifi/reloq61_rule_calls_fifi.h').read_text(encoding='utf8')

    def symbol(match):
        index = int(match.group(1))
        data = constant(english_raw, english_offsets, index)
        if data in by_bytes:
            report['constants matched by content'] += 1
            return 'delta_sym_ref[%d]' % by_bytes[data]
        if len(data) > WIDTH:
            raise RuntimeError('English constant %d is longer than expected' % index)
        if data not in embedded:
            embedded.append(data)
        report['constants embedded'] += 1
        return 'persian_us_symbol(%d)' % embedded.index(data)

    bodies, prototypes = [], []
    for name in names:
        text = translate(english[name], table, symbols, report)
        text = re.sub(r'delta_sym_ref\[EN(\d+)\]', symbol, text)
        def call(match):
            if match.group(1) in names:
                return 'persian_us_' + match.group(1) + '('
            if 'reloq_call_fifi_' + match.group(1) + '(' not in helpers:
                raise RuntimeError('the Persian provider lacks the helper ' + match.group(1))
            report['shared helpers called'] += 1
            return 'reloq_call_fifi_' + match.group(1) + '('
        text = re.sub(r'reloq_call_enus61_(\w+)\(', call, text)
        text = text.replace('reloq61_english_question_tones(', 'persian_us_phrase_tones(')
        head = re.match(r'int32_t reloq_enus61_' + name + r'\(([^)]*)\)', text)
        if not head:
            raise RuntimeError('unexpected rule heading: ' + name)
        linkage = '' if name in ROOT_RULES else 'static '
        signature = f'{linkage}int32_t persian_us_{name}({head.group(1)})'
        text = signature + text[head.end():]
        if 'enus61' in text:
            raise RuntimeError('untranslated English reference in ' + name)
        prototypes.append(signature + ';')
        bodies.append(f'/* {name}, translated from the US-English provider */\n' + text)
        report['rules'] += 1
        report['lines'] += text.count('\n')
    rows = ',\n'.join('    {' + ','.join(str(b) for b in data.ljust(WIDTH, b'\0')) + '}'
                      for data in embedded) or '    {0}'
    nucleus = []
    for name in NUCLEUS_VARIABLES:
        if name not in offsets:
            raise RuntimeError('the melody file does not declare ' + name)
        later = [o for o in ordered if o > offsets[name]]
        nucleus.append((offsets[name], later[0] - offsets[name]))
    nucleus_text = ',\n'.join('    {%d,%d}' % f for f in nucleus)
    nucleus_bytes = sum(size for _, size in nucleus)
    helper = f'''
/* ---- US-English sentence melody for the Persian voice --------------------
 * Generated by tools/persian_melody/port.py from the US-English provider.
 * Do not edit: change the generator. Selected only by the scoped key
 * {KEY}; the provider's own melody remains the default. */
#include <stdatomic.h>
{SHARED}
int reloq61_english_is_question(void *state,int32_t phrase);
int reloq61_english_question_time(void *state,int32_t phrase,int16_t,int16_t,int32_t,int32_t *);
void reloq61_english_question_tones(void *state,int32_t phrase,int16_t high,
    int16_t low_nucleus,int16_t *nucleus,int16_t *phrase_tone,int16_t *boundary_tone);
static const unsigned char persian_us_constants[][{WIDTH}]={{
{rows}
}};
static _Atomic int32_t persian_us_constants_ref;
static int32_t persian_us_symbol(unsigned index)
{{
    int32_t result=atomic_load_explicit(&persian_us_constants_ref,memory_order_acquire);
    if (!result) {{
        void *copy=evv_arena_alloc(sizeof persian_us_constants);
        if (!copy) return 0;
        memcpy(copy,persian_us_constants,sizeof persian_us_constants);
        int32_t made=EVV_REF(copy),expected=0;
        if (atomic_compare_exchange_strong_explicit(&persian_us_constants_ref,&expected,made,
                                                    memory_order_release,memory_order_acquire))
            result=made;
        else {{ evv_arena_free(copy); result=expected; }}
    }}
    return result+(int32_t)(index*{WIDTH});
}}
int persian_us_melody_selected(void)
{{
    const char *mode=getenv("{KEY}");
    return mode && mode[0]=='1' && !mode[1];
}}
int reloq61_phrase_kind(void *state,int32_t phrase);
/* The tones of a phrase are named when the phrase is recognised, long before
 * the melody, by rules that are not carried over. For a phrase that ends in
 * a comma the Persian provider (Finnish rules) names a rise that stays high;
 * US English names a high accent that falls back to low. Questions are then
 * settled by the product's own policy as in the US provider. */
static void persian_us_phrase_tones(void *state,int32_t phrase,int16_t high,
    int16_t low,int16_t *nucleus,int16_t *phrase_tone,int16_t *boundary_tone)
{{
    if (reloq61_phrase_kind(state,phrase)==1) {{
        *nucleus=high;
        *phrase_tone=low;
        /* The boundary tone is left as named: both providers name a low one
         * after a comma and a high one after three dots. */
    }}
    reloq61_english_question_tones(state,phrase,high,low,nucleus,phrase_tone,boundary_tone);
}}
/* Word stress as the US rules count it. */
static int16_t persian_us_stress(int16_t stored)
{{
    switch (stored) {{
    case 2: return 5;
    case 1: return 3;
    case 3: return 6;
    case 4: return 7;
    default: return stored;
    }}
}}
/* What the US rules decide about words and phrases, kept apart from what the
 * Persian rules decide about the same records. */
static _Thread_local struct delta_field_overlay persian_us_overlay;
static _Thread_local int persian_us_spent;
void persian_us_enter(void *state,struct persian_us_saved *saved)
{{
    persian_us_overlay.statements[0]=4;persian_us_overlay.statements[1]=5;
    persian_us_overlay.scaled_statement=4;persian_us_overlay.scaled_field={stress_field};
    persian_us_overlay.scale=persian_us_stress;
    persian_us_overlay.mode=DELTA_OVERLAY_VIEW;
    saved->overlay=delta_field_overlay;
    delta_field_overlay=&persian_us_overlay;
    /* The rules ask which language is speaking; here it is US English. */
    saved->language=STATE(int16_t, s473);STATE(int16_t, s473)=STATE(int16_t, s474);
    /* The stress of the word in hand, copied from its record before the
     * melody is asked for. */
    saved->word[0]=STATE(int16_t, s6);STATE(int16_t, s6)=persian_us_stress(saved->word[0]);
    saved->word[1]=STATE(int16_t, s7);STATE(int16_t, s7)=persian_us_stress(saved->word[1]);
    saved->level[0]=STATE(int16_t, s236);STATE(int16_t, s236)=5;
    saved->level[1]=STATE(int16_t, s237);STATE(int16_t, s237)=3;
    saved->level[2]=STATE(int16_t, s242);STATE(int16_t, s242)=6;
    saved->level[3]=STATE(int16_t, s243);STATE(int16_t, s243)=7;
}}
void persian_us_leave(void *state,const struct persian_us_saved *saved)
{{
    STATE(int16_t, s236)=saved->level[0];
    STATE(int16_t, s237)=saved->level[1];
    STATE(int16_t, s242)=saved->level[2];
    STATE(int16_t, s243)=saved->level[3];
    STATE(int16_t, s473)=saved->language;
    STATE(int16_t, s6)=saved->word[0];STATE(int16_t, s7)=saved->word[1];
    delta_field_overlay=saved->overlay;
}}
/* The variables that point at the accented word: keep the Persian rules'
 * choice aside, or put it back. */
static const struct {{ unsigned short offset,size; }} persian_us_nucleus_fields[]={{
{nucleus_text}
}};
static _Thread_local unsigned char persian_us_nucleus_own[{nucleus_bytes}];
void persian_us_nucleus(void *state,int restore)
{{
    unsigned char *at=persian_us_nucleus_own;
    for (unsigned i=0;i<sizeof persian_us_nucleus_fields/sizeof persian_us_nucleus_fields[0];++i) {{
        unsigned char *variable=(unsigned char *)state+DG_BASE+persian_us_nucleus_fields[i].offset;
        if (restore) memcpy(variable,at,persian_us_nucleus_fields[i].size);
        else memcpy(at,variable,persian_us_nucleus_fields[i].size);
        at+=persian_us_nucleus_fields[i].size;
    }}
}}
/* While the Persian rule decides, keep what the fields held before. */
void persian_us_keep(int on)
{{
    persian_us_overlay.mode=DELTA_OVERLAY_KEEP;
    delta_field_overlay=on?&persian_us_overlay:0;
}}
/* The melody is made. It may be asked for again for the same records, so the
 * overlay stays until the accent rules start on the next utterance. */
void persian_us_done(void)
{{
    persian_us_spent=1;
}}
/* The accent rules start: forget what belonged to an utterance already spoken. */
void persian_us_begin(void)
{{
    if (!persian_us_spent) return;
    persian_us_overlay.count=0;
    persian_us_spent=0;
}}
'''
    # Each rule file declares only the variables its own rules use.
    everywhere = {}
    for path in sorted((LANG / 'fifi').glob('delta_rules_c*_fifi.c')):
        for name, value in re.findall(r'^#define DG_(\w+)\s+\(DG_BASE \+\s*(\d+)\)',
                                      path.read_text(encoding='utf8'), re.M):
            if everywhere.setdefault(name, value) != value:
                raise RuntimeError('the provider places %s in two places' % name)
    body_text = '\n'.join(bodies)
    used = set(re.findall(r'STATE(?:_D|_AT)?\(\s*(?:[\w ]+\s*,\s*)?([swcf]\d+)\b', body_text))
    missing = sorted(used - set(offsets), key=lambda n: (n[0], int(n[1:])))
    def place(name):
        """Where a variable sits that no Persian rule file mentions."""
        if name in everywhere:
            return int(everywhere[name])
        kind, number = name[0], int(name[1:])
        known = sorted((int(n[1:]), int(v)) for n, v in everywhere.items() if n[0] == kind)
        below = [k for k in known if k[0] < number]
        above = [k for k in known if k[0] > number]
        if not below or not above:
            raise RuntimeError('cannot place the variable ' + name)
        (low, low_at), (high, high_at) = below[-1], above[0]
        step = (high_at - low_at) / (high - low)
        if step != int(step):
            raise RuntimeError('cannot place the variable ' + name)
        return low_at + int(step) * (number - low)
    declared = ''.join('#define DG_%s (DG_BASE + %d)\n' % (n, place(n)) for n in missing)
    report['variables declared'] = len(missing)
    source = declared + helper + '\n'.join(prototypes) + '\n\n' + '\n\n'.join(bodies) + '\n'
    return source, report


def install(work: Path):
    """Add the US melody to the prepared rule files and choose it at each root call."""
    source, report = generate()
    call = re.compile(r'^(\s*)r0 = reloq_call_fifi_(\w+)\((.*)\);$', re.M)
    for filename in sorted(FILES | {HOME}):
        path = work / filename
        text = path.read_text(encoding='utf8')
        first_rule = re.search(r'^/\* \w+, from \w+\.obj \*/\n', text, re.M)
        if not first_rule:
            raise RuntimeError('Cannot locate the first rule of ' + filename)
        for rule, home in ROOTS.items():
            if home != filename:
                continue
            found = [m for m in call.finditer(text) if m.group(2) == rule]
            if len(found) != 1:
                raise RuntimeError('Cannot locate the Persian call of ' + rule)
            m = found[0]
            pad, args = m.group(1), m.group(3)
            if rule in PAIRED:
                chosen = (f'{pad}if (persian_us_melody_selected()) {{\n'
                          f'{pad}    struct persian_us_saved persian_us_kept;\n'
                          f'{pad}    persian_us_begin();\n'
                          f'{pad}    persian_us_enter(state,&persian_us_kept);\n'
                          f'{pad}    (void)persian_us_{rule}({args});\n'
                          f'{pad}    persian_us_leave(state,&persian_us_kept);\n'
                          f'{pad}    persian_us_keep(1);\n'
                          f'{pad}    r0 = reloq_call_fifi_{rule}({args});\n'
                          f'{pad}    persian_us_keep(0);\n'
                          f'{pad}}} else\n'
                          f'{pad}    r0 = reloq_call_fifi_{rule}({args});')
            else:
                chosen = (f'{pad}if (persian_us_melody_selected()) {{\n'
                          f'{pad}    struct persian_us_saved persian_us_kept;\n'
                          f'{pad}    persian_us_enter(state,&persian_us_kept);\n'
                          f'{pad}    persian_us_nucleus(state,0);\n'
                          f'{pad}    (void)persian_us_{NUCLEUS_RULE}({args});\n'
                          f'{pad}    r0 = persian_us_{rule}({args});\n'
                          f'{pad}    persian_us_nucleus(state,1);\n'
                          f'{pad}    persian_us_leave(state,&persian_us_kept);\n'
                          f'{pad}    persian_us_done();\n'
                          f'{pad}}} else\n'
                          f'{pad}    r0 = reloq_call_fifi_{rule}({args});')
            text = text[:m.start()] + chosen + text[m.end():]
        start = first_rule.start()
        if filename == HOME:
            added = source
            if '#include <stdlib.h>' not in text[:start]:
                added = '#include <stdlib.h>\n' + added
        else:
            heads = [re.search(r'^int32_t persian_us_' + rule + r'\([^)]*\)', source, re.M).group(0)
                     for rule, home in ROOTS.items() if home == filename]
            added = ('/* The US-English melody rules live in ' + HOME + '. */\n'
                     + SHARED + '\n' + ''.join(h + ';\n' for h in heads))
        path.write_text(text[:start] + added + '\n' + text[start:], encoding='utf8')
    return report


if __name__ == '__main__':
    generated, summary = generate()
    print(dict(summary))
    print(len(generated.split('\n')), 'lines generated')
