"""Promote private scalar workspace cells to ordinary C automatic storage.

This is an offline, conservative escape analysis, not a runtime mechanism.
An address handed to another operation protects everything from that address
to the allocation's end. Only the fixed scope record, landing, fence arrays
and the single-word backtracking flag have reviewed, bounded footprints.
Mixed-width cells use a C union. Values observable after a nonlocal return
remain volatile; all other private values are available to the optimizer.
No value survives a language-function call.
"""
import re

FUNCTION = re.compile(r'(int32_t reloq_\w+\([^\n]*\)\n\{)(.*?)(\n\})', re.S)
ACCESS = re.compile(r'(?<!\w)AT\((u?int(?:8|16|32)_t), (-?\d+)\)')
ADDRESS = re.compile(r'RELOQ61_WORKSPACE_REF\(frame, reloq_workspace_ref, (\d+)\)')
POINTER = re.compile(r'\(([A-Za-z0-9_ ]+?)\s*\*\)\(base \+ \((-?\d+)\)\)')
WIDTH = {'int8_t': 1, 'uint8_t': 1, 'int16_t': 2, 'uint16_t': 2,
         'int32_t': 4, 'uint32_t': 4}
MEMBER = {'int8_t': 'i8', 'uint8_t': 'u8', 'int16_t': 'i16',
          'uint16_t': 'u16', 'int32_t': 'i32', 'uint32_t': 'u32'}
MARKER = '    /* Private scalar cells: automatic C storage, no escaped address. */'


def landing_reachable(lines):
    """Overapproximate the statements reachable after the one saved landing.

    Fallthrough is retained even between switch cases. An uncertain shape is
    refused rather than making a nonvolatile value depend on setjmp rules.
    """
    roots = []
    for i, line in enumerate(lines):
        if 'EVV_LAND_SAVE(' not in line:
            continue
        following = '\n'.join(lines[i:i+5])
        m = re.search(r'if \(r0 != 0\) goto (L\d+);', following)
        if not m:
            return None
        roots.append(m[1])
    if len(roots) != 1:
        return None
    labels = {m[1]: i for i, line in enumerate(lines)
              if (m := re.fullmatch(r'(L\d+):;', line))}
    if roots[0] not in labels:
        return None
    pending = [labels[roots[0]]]
    visited = set()
    while pending:
        i = pending.pop()
        if i in visited or i >= len(lines):
            continue
        visited.add(i)
        line = lines[i].strip()
        targets = re.findall(r'\bgoto (L\d+);', line)
        if any(target not in labels for target in targets):
            return None
        pending.extend(labels[target] for target in targets)
        if (not re.fullmatch(r'goto L\d+;', line)
                and not re.search(r'\b(?:RELOQ_RULE_RETURN|return)\b', line)):
            pending.append(i+1)
    return visited


def plan(body):
    """Return cells proven private, or refuse an unrecognized workspace."""
    if (re.search(r'&\s*AT\(|\b(?:SLOT|PARAMAT)\(', body)
            or re.search(r'\bAT\(', ACCESS.sub('', body))):
        return None
    size = re.search(r'unsigned char \*frame = evv_frame_push\((\d+)\);', body)
    bias = re.search(r'unsigned char \*base = frame \+ (\d+);', body)
    if not size or not bias:
        return None
    size, bias = int(size[1]), int(bias[1])
    if body.count('unsigned char *frame =') != 1 or body.count('unsigned char *base =') != 1:
        return None
    lines = body.splitlines()
    reachable = landing_reachable(lines)
    if reachable is None:
        return None
    protected = [(0, 192)]
    scope_count = landing_address_count = 0
    for line in lines:
        if 'reloq61_typed_ventproc(' in line:
            # This scope's record, landing and three 12-byte fence arrays.
            # Reject other layouts; never infer a footprint from a type name.
            pointers = POINTER.findall(line)
            if [(t.strip(), bias+int(o)) for t, o in pointers] != [
                    ('delta_actrec', 0), ('uint8_t', 180), ('uint8_t', 168),
                    ('uint8_t', 156), ('void', 92)]:
                return None
            scope_count += 1
            continue
        for m in ADDRESS.finditer(line):
            at = int(m[1])
            if not 0 <= at < size:
                return None
            if line.strip() == 'ARG('+m[0]+');' and at == 92:
                landing_address_count += 1
            else:
                protected.append((at, size))
        for m in POINTER.finditer(line):
            at = bias+int(m[2])
            if not 0 <= at < size:
                return None
            width = (4 if m[1].strip() == 'int32_t'
                     and 'reloq61_typed_backtrack_function(' in line else size-at)
            protected.append((at, at+width))
        # Account for every use of base/frame, including future hand edits.
        rest = ACCESS.sub('', ADDRESS.sub('', POINTER.sub('', line)))
        if any(token in rest for token in ['unsigned char *frame =', 'unsigned char *base =',
                                           'unsigned char *param =', 'if (frame == 0)',
                                           'REG_REF(frame)', 'memset(frame, 0,']):
            continue
        if re.fullmatch(r'\s*memcpy\(base \+ \d+, &p\d+, 4\);', rest):
            continue
        if re.search(r'\b(?:base|frame)\b', rest):
            return None
    if scope_count != 1 or landing_address_count != 1:
        return None
    cells = {}
    unsafe = set()
    for i, line in enumerate(lines):
        for m in ACCESS.finditer(line):
            at, width = bias+int(m[2]), WIDTH[m[1]]
            if not 0 <= at <= size-width:
                return None
            cell = at//4*4
            if at+width > cell+4 or at % width:
                unsafe.update(range(cell, (at+width+3)//4*4, 4))
            info = cells.setdefault(cell, {'accesses': 0, 'volatile': False})
            info['accesses'] += 1
            info['volatile'] |= i in reachable
    return {cell: info for cell, info in cells.items()
            if cell not in unsafe and cell+4 <= size
            and not any(a < cell+4 and cell < b for a, b in protected)}


def bind(text):
    def convert(m):
        head, body, tail = m.groups()
        if MARKER in body:
            return m[0]
        cells = plan(body)
        if not cells:
            return m[0]
        bias = int(re.search(r'base = frame \+ (\d+);', body)[1])
        def access(m):
            at = bias+int(m[2]);cell = at//4*4
            if cell not in cells:
                return m[0]
            member = MEMBER[m[1]]
            suffix = ('['+str((at-cell)//WIDTH[m[1]])+']'
                      if WIDTH[m[1]] < 4 else '')
            return 'reloq_local_'+str(cell)+'.'+member+suffix
        body = ACCESS.sub(access, body)
        declarations = '\n'+MARKER+'\n'
        # The union preserves all signed/unsigned and narrow views of a cell.
        # The supported targets are little endian. An escaped record is never
        # reinterpreted as one of these automatic variables.
        for cell, info in sorted(cells.items()):
            declarations += ('    '+('volatile ' if info['volatile'] else '')
                +'union { int32_t i32; uint32_t u32; int16_t i16[2]; '
                +'uint16_t u16[2]; int8_t i8[4]; uint8_t u8[4]; } '
                +'reloq_local_'+str(cell)+' = {0};\n')
        guard = '    if (frame == 0)\n        return 0;'
        if body.count(guard) != 1:
            return m[0]
        # Initialize before the saved landing. The zeroed arena frame stays
        # intact for its addressable records and established diagnostics.
        body = body.replace(guard, guard+declarations, 1)
        return head+body+tail
    return FUNCTION.sub(convert, text)


def convert_bodies(units):
    return [(name, bind(text)) for name, text in units]


if __name__ == '__main__':
    from pathlib import Path
    root = Path(__file__).resolve().parents[2]
    for path in sorted((root/'lang').glob('*/delta_rules_c??_*.c')):
        path.write_text(bind(path.read_text()))
