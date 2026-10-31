"""Collect compact release entries from the existing C rule interfaces.

Only C sources are read: no binary modules, lifter or network is needed.
Normal builds consume checked-in native_entries_*.c and C binding macros.
Rule bodies stay untouched. Diagnostic descriptors remain in development
builds, while release entries contain only a name and native C call.
"""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1] / "vendor/openevv61"
TABLE = re.compile(r"const delta_rule (\w+)_delta_rules\[\] = \{\n(.*?)\n\};", re.S)
PIECE = re.compile(r"const delta_rule_c \w+\[\] = \{\n(.*?)\n\};", re.S)
WRAPPER = re.compile(r"static\s+int32_t\s+(evv_\w+)\s*\(\s*void\s*\*\s*\w+\s*,\s*const\s+int32_t\s*\*\s*\w+\s*,\s*int\s+\w+\s*\)\s*\{(.*?)\n\}", re.S)


def update(folder):
    language = folder.name
    metadata = folder / f"delta_rules_{language}.c"
    if not metadata.exists():
        return 0
    source = metadata.read_text()
    matches = list(TABLE.finditer(source)); descriptor = matches[-1]
    names = re.findall(r'^\s*\{ "([^"]+)", "[^"]+", [^}]+ \},$', descriptor[2], re.M)
    bindings = {}; wrappers = {}
    for path in sorted(folder.glob("delta_rules_c*.c")):
        text = path.read_text()
        wrappers.update(WRAPPER.findall(text))
        for piece in PIECE.finditer(text):
            for number, function in re.findall(r"\{ (\d+), (\w+) \}", piece[1]):
                number = int(number)
                if number in bindings:
                    raise ValueError((language, number, "duplicate"))
                bindings[number] = function
    if sorted(bindings) != list(range(len(names))):
        raise ValueError((language, "incomplete C entry coverage"))
    lines = ["/* Compact native C entries. Regenerate with tools/compact_native_entries.py. */",
        '#include "delta_lang.h"', '#if RELOQ61_NATIVE_ENTRIES',
        '#include "reloq61_entry_bindings.h"']
    if language == 'dede61':
        lines += ['#include "reloq61_word_edges.h"',
            'extern int32_t reloq61_normalize_german_letters(void *, const int32_t *, int);']
    for number, name in enumerate(names):
        if not re.fullmatch(r'[A-Za-z_]\w*', name):
            raise ValueError((language, name, 'invalid C name'))
        body = wrappers[bindings[number]]
        match = re.fullmatch(r"\s*return (reloq_\w+)\(state(.*?)\);\s*", body, re.S)
        if match:
            arity = len(re.findall(r"nargs>", match[2]))
            expected = ''.join(f',nargs>{i}?args[{i}]:0' for i in range(arity))
            if re.sub(r'\s+', '', match[2]) != expected or not 1 <= arity <= 13:
                raise ValueError((language, name, "unexpected C argument projection"))
            lines.append(f'RELOQ61_ENTRY{arity}(entry_{name}, {match[1]})')
        else:
            match = re.fullmatch(r'\s*return (reloq61_\w+)\(state,\s*args,\s*nargs\);\s*', body)
            if not match:
                raise ValueError((language, name, "unexpected authored C entry"))
            lines.append(f'RELOQ61_ENTRY_BOUND(entry_{name}, {match[1]})')
    lines += [f'const delta_rule {language}_delta_rules[] = {{']
    lines.extend(f'    {{ "{name}", entry_{name} }},' for name in names)
    lines += ['};', '#endif', '']
    (folder / f'native_entries_{language}.c').write_text('\n'.join(lines))
    compact = f'#if !RELOQ61_NATIVE_ENTRIES\n{descriptor[0]}\n#endif'
    marker = '#if RELOQ61_NATIVE_ENTRIES' if len(matches) == 2 else '#if !RELOQ61_NATIVE_ENTRIES'
    start = source.rfind(marker, 0, matches[0].start())
    end = source.index('\n#endif', descriptor.end()) + len('\n#endif') if start >= 0 else descriptor.end()
    if start < 0:
        start = descriptor.start()
    metadata.write_text(source[:start] + compact + source[end:])
    config = folder / f"delta_lang_{language}.c"; text = config.read_text()
    old = f"    {language}_delta_rule_native,\n    &{language}_delta_native_index,"
    new = "#if RELOQ61_NATIVE_ENTRIES\n    0, 0,\n#else\n" + old + "\n#endif"
    if new not in text:
        if old not in text:
            raise ValueError((language, "native config missing"))
        config.write_text(text.replace(old, new))
    return len(names)


if __name__ == "__main__":
    header = ['/* Native C argument projections, no runtime instruction decoder. */',
        '#ifndef RELOQ61_ENTRY_BINDINGS_H', '#define RELOQ61_ENTRY_BINDINGS_H',
        '#define RELOQ61_ENTRY_ARG(n) (nargs>(n)?args[n]:0)']
    for arity in range(1, 14):
        args = ', '.join(f'RELOQ61_ENTRY_ARG({i})' for i in range(arity))
        types = ', '.join(['int32_t'] * arity)
        header += [f'#define RELOQ61_ENTRY{arity}(name, call) '+chr(92),
            f'extern int32_t call(void *, {types}); '+chr(92),
            'static int32_t name(void *state, const int32_t *args, int nargs) '+chr(92),
            f'{{ return call(state, {args}); }}']
    header += ['#define RELOQ61_ENTRY_BOUND(name, call) '+chr(92),
        'static int32_t name(void *state, const int32_t *args, int nargs) '+chr(92),
        '{ return call(state, args, nargs); }', '#endif', '']
    (ROOT / 'src/reloq61_entry_bindings.h').write_text('\n'.join(header))
    print('Direct native rule entries:', sum(update(folder) for folder in sorted((ROOT/'lang').iterdir()) if folder.is_dir()))
