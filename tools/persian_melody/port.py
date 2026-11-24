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
         'adjust_accents': 'delta_rules_c05_fifi.c'}
ROOT_RULES = tuple(ROOTS)
HOME = 'delta_rules_c04_fifi.c'
# The speech module's per-word and per-syllable setup rules (`init_word_vars`,
# `init_syll_vars`) differ between the providers, but the English versions
# cannot run on Persian records: tried as replacements they change durations,
# and tried on a private copy of the variables they write they fail and take
# the whole utterance with them. The copy mechanism ("the US view") is kept
# for rules that can be run this way; none is listed at present.
SETUP_RULES = ()
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
    names = reachable(english, ROOT_RULES + SETUP_RULES)
    # Everything the setup rules can write: direct assignments and variables
    # handed to the machine by address.
    view_names = set()
    for rule in reachable(english, SETUP_RULES):
        translated = translate(english[rule], table, symbols, collections.Counter())
        view_names |= set(re.findall(r'STATE\(\w+, (\w+)\) = ', translated))
        view_names |= set(re.findall(r'STATE_AT\((\w+),', translated))
    layout = (LANG / 'fifi' / HOME).read_text(encoding='utf8')
    offsets = {name: int(value) for name, value in
               re.findall(r'^#define DG_(\w+)\s+\(DG_BASE \+\s*(\d+)\)', layout, re.M)}
    ordered = sorted(set(offsets.values()))
    fields = []
    for name in sorted(view_names, key=lambda n: offsets[n]):
        later = [o for o in ordered if o > offsets[name]]
        if not later:
            raise RuntimeError('cannot size the variable ' + name)
        fields.append((offsets[name], later[0] - offsets[name]))
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
        # The rules ask whether the current language is English by comparing
        # it with the English language number. In this provider the current
        # language is the provider's own; its number sits in s483.
        text = re.sub(r'\bs474\b', 's483', text)
        def call(match):
            if match.group(1) in names:
                return 'persian_us_' + match.group(1) + '('
            if 'reloq_call_fifi_' + match.group(1) + '(' not in helpers:
                raise RuntimeError('the Persian provider lacks the helper ' + match.group(1))
            report['shared helpers called'] += 1
            return 'reloq_call_fifi_' + match.group(1) + '('
        text = re.sub(r'reloq_call_enus61_(\w+)\(', call, text)
        head = re.match(r'int32_t reloq_enus61_' + name + r'\(([^)]*)\)', text)
        if not head:
            raise RuntimeError('unexpected rule heading: ' + name)
        linkage = '' if name in ROOTS else 'static '
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
    fields_text = ',\n'.join('    {%d,%d}' % f for f in fields)
    total = sum(size for _, size in fields)
    report['view variables'] = len(fields)
    report['view bytes'] = total
    helper = f'''
/* ---- US-English sentence melody for the Persian voice --------------------
 * Generated by tools/persian_melody/port.py from the US-English provider.
 * Do not edit: change the generator. Selected only by the scoped key
 * {KEY}; the provider's own melody remains the default. */
#include <stdatomic.h>
int reloq61_english_is_question(void *state,int32_t phrase);
int reloq61_english_question_time(void *state,int32_t phrase,int16_t,int16_t,int32_t,int32_t *);
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
/* The US view of the variables the setup rules write: offset and size. */
static const struct {{ unsigned short offset,size; }} persian_us_fields[]={{
{fields_text}
}};
struct persian_us_view {{ void *state; int valid; unsigned char bytes[{total}]; }};
static _Thread_local struct persian_us_view persian_us_shadow,persian_us_saved;
static void persian_us_read(void *state,struct persian_us_view *view)
{{
    unsigned char *at=view->bytes;
    for (unsigned i=0;i<sizeof persian_us_fields/sizeof persian_us_fields[0];++i) {{
        memcpy(at,(unsigned char *)state+DG_BASE+persian_us_fields[i].offset,persian_us_fields[i].size);
        at+=persian_us_fields[i].size;
    }}
}}
static void persian_us_write(void *state,const struct persian_us_view *view)
{{
    const unsigned char *at=view->bytes;
    for (unsigned i=0;i<sizeof persian_us_fields/sizeof persian_us_fields[0];++i) {{
        memcpy((unsigned char *)state+DG_BASE+persian_us_fields[i].offset,at,persian_us_fields[i].size);
        at+=persian_us_fields[i].size;
    }}
}}
/* Keep the Persian values aside and make the US view current. */
static void persian_us_enter(void *state)
{{
    persian_us_read(state,&persian_us_saved);
    if (persian_us_shadow.valid && persian_us_shadow.state==state)
        persian_us_write(state,&persian_us_shadow);
}}
/* Remember the US view and put the Persian values back. */
static void persian_us_leave(void *state)
{{
    persian_us_read(state,&persian_us_shadow);
    persian_us_shadow.valid=1;persian_us_shadow.state=state;
    persian_us_write(state,&persian_us_saved);
}}
'''
    source = helper + '\n'.join(prototypes) + '\n\n' + '\n\n'.join(bodies) + '\n'
    return source, report


def install(work: Path):
    """Add the US melody to the prepared rule files and choose it at each root call."""
    source, report = generate()
    call = re.compile(r'^(\s*)r0 = reloq_call_fifi_(\w+)\((.*)\);$', re.M)
    for filename in sorted(set(ROOTS.values()) | {HOME}):
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
            if filename == HOME:
                chosen = (f'{pad}if (persian_us_melody_selected()) {{\n'
                          f'{pad}    persian_us_enter(state);\n'
                          f'{pad}    r0 = persian_us_{rule}({args});\n'
                          f'{pad}    persian_us_leave(state);\n'
                          f'{pad}}} else\n'
                          f'{pad}    r0 = reloq_call_fifi_{rule}({args});')
            else:
                chosen = (f'{pad}r0 = persian_us_melody_selected()\n'
                          f'{pad}    ? persian_us_{rule}({args})\n'
                          f'{pad}    : reloq_call_fifi_{rule}({args});')
            text = text[:m.start()] + chosen + text[m.end():]
        if filename == HOME:
            for rule in SETUP_RULES:
                found = [m for m in call.finditer(text) if m.group(2) == rule]
                if len(found) != 1:
                    raise RuntimeError('Cannot locate the Persian call of ' + rule)
                m = found[0]
                pad, args = m.group(1), m.group(3)
                # Run the English setup rule first, from the same starting
                # values the Persian rule is about to see, keep its result as
                # the US view, and put the starting values back.
                before = (f'{pad}if (persian_us_melody_selected()) {{\n'
                          f'{pad}    persian_us_read(state,&persian_us_saved);\n'
                          f'{pad}    (void)persian_us_{rule}({args});\n'
                          f'{pad}    persian_us_leave(state);\n'
                          f'{pad}}}\n')
                text = text[:m.start()] + before + text[m.start():]
        start = first_rule.start()
        if filename == HOME:
            added = source
            if '#include <stdlib.h>' not in text[:start]:
                added = '#include <stdlib.h>\n' + added
        else:
            heads = [re.search(r'^int32_t persian_us_' + rule + r'\([^)]*\)', source, re.M).group(0)
                     for rule, home in ROOTS.items() if home == filename]
            added = ('/* The US-English melody rules live in ' + HOME + '. */\n'
                     'int persian_us_melody_selected(void);\n' + ''.join(h + ';\n' for h in heads))
        path.write_text(text[:start] + added + '\n' + text[start:], encoding='utf8')
    return report


if __name__ == '__main__':
    generated, summary = generate()
    print(dict(summary))
    print(len(generated.split('\n')), 'lines generated')
