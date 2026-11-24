"""Translate US-English melody rules into the numbering of the Persian provider.

The two providers share a rule machine but number three things differently:
some global variables, the authored constants, and the fields and named
values of the phone and word records. This module rewrites a rule recovered
from the English provider so that it reads and writes the Persian provider's
records. Field and value numbers are often loaded into a local before the
call that uses them, sometimes in a different basic block, so the rewrite
follows each literal from its assignment to its use (reaching definitions)
instead of substituting numbers blindly.

Run it as a script to check the translator against the rules both providers
share: a translated English rule should reproduce the Persian provider's own
text wherever the two languages agree.
"""
from __future__ import annotations

import collections
from pathlib import Path
import re
import sys

LANG = Path(__file__).resolve().parents[2] / 'vendor/openevv61/lang'
ABSENT = 255  # a value no Persian record carries: the test can never succeed


def load_rules(language, prefix):
    rules = {}
    for path in sorted((LANG / language).glob('delta_rules_c*.c')):
        text = path.read_text(encoding='utf8')
        for match in re.finditer(r'^int32_t reloq_' + prefix + r'_(\w+)\(', text, re.M):
            name = match.group(1)
            end = text.find('\nstatic int32_t evv_' + name + '(', match.start())
            if end >= 0:
                rules[name] = text[match.start():end]
    return rules


def load_statements(path):
    statements, current, field = [], None, None
    for line in path.read_text(encoding='utf8').split('\n'):
        match = re.match(r'^statement (\S+)', line)
        if match:
            current, field = {'name': match.group(1), 'fields': []}, None
            statements.append(current)
            continue
        match = re.match(r'^  field (\S+)', line)
        if match and current is not None:
            field = {'name': match.group(1), 'values': []}
            current['fields'].append(field)
            continue
        match = re.match(r'^    (?:value (.*)|(hole))$', line)
        if match and field is not None:
            field['values'].append(None if match.group(2) else match.group(1))
    return statements


class Records:
    """Field and value numbers of the English records in Persian numbering."""

    def __init__(self):
        english = load_statements(LANG / 'enus61/enus61.statements')
        persian = load_statements(LANG / 'fifi/fifi.statements')
        self.fields, self.values = {}, {}
        for number, (source, target) in enumerate(zip(english, persian)):
            if source['name'] != target['name']:
                raise RuntimeError('the providers number their record types differently')
            names = {field['name']: index for index, field in enumerate(target['fields'])}
            for index, field in enumerate(source['fields']):
                mapped = names.get(field['name'])
                self.fields[number, index] = mapped
                if mapped is None:
                    continue
                known = {value: i for i, value in enumerate(target['fields'][mapped]['values'])
                         if value is not None}
                self.values[number, index] = {i: known.get(value)
                                              for i, value in enumerate(field['values'])
                                              if value is not None}

    def field(self, statement, field):
        return self.fields.get((statement, field), field)

    def value(self, statement, field, value):
        table = self.values.get((statement, field))
        if not table or value not in table:
            return value            # a plain number, not a named value
        mapped = table[value]
        return ABSENT if mapped is None else mapped


LOCATION = r'(?:AT\(int32_t, -\d+\)|reloq_local_\d+\.i32)'
LITERAL_DEF = re.compile(r'^\s*(' + LOCATION + r') = \((-?\d+)\);$')
COPY_DEF = re.compile(r'^\s*(' + LOCATION + r') = \(\(int32_t\)(' + LOCATION + r')\);$')
ANY_DEF = re.compile(r'^\s*(' + LOCATION + r') = ')
LABEL = re.compile(r'^(L\d+):;$')
GOTO = re.compile(r'goto (L\d+);')
CALL = re.compile(r'reloq61_typed_(testFldeq|npush_fld)\(DELTA_ENTRY_\w+, state, (.*)\);')
ARGUMENT = re.compile(r'\(uint8_t\)\((?:(-?\d+)|\(int32_t\)(' + LOCATION + r'))\)')


def variables(text):
    """Global variables: `w` from 82 up are twelve lower in the Persian provider."""
    def word(match):
        number = int(match.group(2))
        if 70 <= number < 82:
            raise ValueError('rule uses an English-only variable w%d' % number)
        return match.group(1) + 'w%d' % (number - 12 if number >= 82 else number)
    return re.sub(r'(STATE(?:_D|_AT)?\(\s*(?:\w+\s*,\s*)?)w(\d+)\b', word, text)


class Flow:
    """Reaching literal definitions of the rule's locals, one rule at a time."""

    def __init__(self, lines):
        self.lines = lines
        labels = {match.group(1): index for index, line in enumerate(lines)
                  for match in [LABEL.match(line.strip())] if match}
        self.successors = []
        for index, line in enumerate(lines):
            stripped, targets = line.strip(), []
            jump = GOTO.search(stripped)
            if jump and jump.group(1) in labels:
                targets.append(labels[jump.group(1)])
            unconditional = stripped.startswith('goto ') or 'RELOQ_RULE_RETURN' in stripped
            if not unconditional and index + 1 < len(lines):
                targets.append(index + 1)
            self.successors.append(targets)
        self.state_in = [None] * len(lines)
        work = collections.deque([0])
        self.state_in[0] = {}
        while work:
            index = work.popleft()
            out = self._transfer(index, self.state_in[index])
            for target in self.successors[index]:
                merged = self._merge(self.state_in[target], out)
                if merged != self.state_in[target]:
                    self.state_in[target] = merged
                    work.append(target)

    def _transfer(self, index, state):
        line = self.lines[index]
        match = LITERAL_DEF.match(line)
        if match:
            return {**state, match.group(1): frozenset([index])}
        match = COPY_DEF.match(line)
        if match:
            return {**state, match.group(1): state.get(match.group(2), frozenset([None]))}
        match = ANY_DEF.match(line)
        if match:
            return {**state, match.group(1): frozenset([None])}
        return state

    @staticmethod
    def _merge(old, new):
        if old is None:
            return dict(new)
        merged = dict(old)
        for key in set(old) | set(new):
            merged[key] = old.get(key, frozenset([None])) | new.get(key, frozenset([None]))
        return merged

    def definitions(self, index, location):
        """Lines whose literal reaches `location` at line `index`, or None if unknown."""
        found = (self.state_in[index] or {}).get(location)
        if not found or None in found:
            return None
        return found


def records(text, table, report):
    """Rewrite record field and value numbers in one rule."""
    lines = text.split('\n')
    flow = Flow(lines)
    wanted = {}          # line -> set of replacement literals for its definition
    direct = []          # (line, argument position, replacement)

    def operand(index, argument):
        literal, location = argument
        if literal is not None:
            return {int(literal)}, None
        sources = flow.definitions(index, location)
        if sources is None:
            return None, None
        return {int(LITERAL_DEF.match(lines[s]).group(2)) for s in sources}, sources

    def request(index, position, sources, mapping):
        """mapping: old literal -> new literal."""
        if sources is None:
            old = int(ARGUMENT.findall(CALL.search(lines[index]).group(2))[position][0])
            if mapping[old] != old:
                direct.append((index, position, mapping[old]))
            return
        for source in sources:
            old = int(LITERAL_DEF.match(lines[source]).group(2))
            wanted.setdefault(source, set()).add(mapping[old])

    for index, line in enumerate(lines):
        call = CALL.search(line)
        if not call or flow.state_in[index] is None:
            continue
        arguments = ARGUMENT.findall(call.group(2))
        needed = 3 if call.group(1) == 'testFldeq' else 2
        if len(arguments) != needed:
            report['unparsed'] += 1
            continue
        statements, _ = operand(index, arguments[0])
        fields, field_sources = operand(index, arguments[1])
        if statements is None or fields is None or len(statements) != 1:
            report['unresolved'] += 1
            continue
        statement = next(iter(statements))
        request(index, 1, field_sources, {f: table.field(statement, f) for f in fields})
        if any(table.field(statement, f) is None for f in fields):
            report['english-only field'] += 1
        if needed == 3:
            values, value_sources = operand(index, arguments[2])
            if values is None or len(fields) != 1:
                if any((statement, f) in table.values and
                       any(k != v for k, v in table.values[statement, f].items()) for f in fields):
                    report['unresolved'] += 1
                continue
            field = next(iter(fields))
            request(index, 2, value_sources,
                    {v: table.value(statement, field, v) for v in values})
        report['calls'] += 1

    for index, replacements in wanted.items():
        if len(replacements) != 1:
            report['conflict'] += 1
            continue
        replacement = next(iter(replacements))
        if replacement is None:
            replacement = ABSENT
        match = LITERAL_DEF.match(lines[index])
        lines[index] = lines[index][:match.start(2)] + str(replacement) + lines[index][match.end(2):]
    for index, position, replacement in direct:
        call = CALL.search(lines[index])
        spans = [m.span(1) for m in ARGUMENT.finditer(call.group(2))]
        start, end = spans[position]
        offset = call.start(2)
        value = ABSENT if replacement is None else replacement
        lines[index] = lines[index][:offset + start] + str(value) + lines[index][offset + end:]
    return '\n'.join(lines)


def translate(text, table, symbols, report):
    """One English rule in Persian numbering. Rule names keep the English prefix."""
    text = variables(text)
    text = re.sub(r'delta_sym_ref\[(\d+)\]',
                  lambda m: 'delta_sym_ref[%s]' % symbols.get(m.group(1), 'EN' + m.group(1)), text)
    return records(text, table, report)


def shared_symbols(english, persian):
    """Authored-constant numbers learned from lines that differ only in them."""
    votes = collections.defaultdict(collections.Counter)
    for name in set(english) & set(persian):
        a = english[name].replace('enus61', 'L').split('\n')
        b = persian[name].replace('fifi', 'L').split('\n')
        if len(a) != len(b):
            continue
        for x, y in zip(a, b):
            if re.sub(r'delta_sym_ref\[\d+\]', '@', x) != re.sub(r'delta_sym_ref\[\d+\]', '@', y):
                continue
            for source, target in zip(re.findall(r'delta_sym_ref\[(\d+)\]', x),
                                      re.findall(r'delta_sym_ref\[(\d+)\]', y)):
                votes[source][target] += 1
    return {source: counter.most_common(1)[0][0] for source, counter in votes.items()}


def main():
    english, persian = load_rules('enus61', 'enus61'), load_rules('fifi', 'fifi')
    table, symbols = Records(), shared_symbols(english, persian)
    report = collections.Counter()
    same = different = lines_same = lines_different = 0
    examples = []
    for name in sorted(set(english) & set(persian)):
        try:
            translated = translate(english[name], table, symbols, report)
        except ValueError:
            report['english-only variable'] += 1
            continue
        a = translated.replace('enus61', 'L').split('\n')
        b = persian[name].replace('fifi', 'L').split('\n')
        if len(a) != len(b):
            continue
        bad = [(x, y) for x, y in zip(a, b) if x != y]
        lines_same += len(a) - len(bad)
        lines_different += len(bad)
        if bad:
            different += 1
            examples.append((name, len(bad), bad[0]))
        else:
            same += 1
    print(f'shared rules of equal length: {same + different}; '
          f'translated English text equals the Persian provider in {same}')
    print(f'lines: {lines_same} equal, {lines_different} different')
    print('record calls rewritten or confirmed:', report['calls'], '| unresolved:',
          report['unresolved'], '| conflicts:', report['conflict'], '| unparsed:', report['unparsed'])
    if '--differences' in sys.argv:
        for name, count, (x, y) in examples:
            print(f'{name} ({count}):\n   translated {x.strip()[:150]}\n   persian    {y.strip()[:150]}')


if __name__ == '__main__':
    main()
