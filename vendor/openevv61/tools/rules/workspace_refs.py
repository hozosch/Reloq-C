"""Share a bounded workspace's native C reference across its literal offsets.

This offline pass accepts only addresses proven to be inside the function's
declared arena workspace. Stored references, dynamic offsets and external
addresses retain their existing checked conversion. The workspace itself is
checked once, after allocation succeeds and before any landing is planted.
No analysis or synthesis result is retained between calls.
"""
import re

FUNCTION = re.compile(r'((?:static )?int32_t \w+\([^\n]*\)\n\{)(.*?)(\n\})', re.S)
SLOT = re.compile(r'\bSLOT\((-?\d+)\)')
GUARD = '    if (frame == 0)\n        return 0;'
DECLARATION = '\n\n    const int32_t reloq_workspace_ref = REG_REF(frame);'


def bind(text):
    def convert(match):
        head, body, tail = match.groups()
        slots = SLOT.findall(body)
        if not slots:
            return match[0]
        size = re.search(r'evv_frame_push\((\d+)\)', body)
        bias = re.search(r'base = frame \+ (\d+);', body)
        # An unknown shape is left checked; it is never inferred from its name.
        if not size or not bias:
            return match[0]
        size, bias = int(size[1]), int(bias[1])
        assert body.count(GUARD) == 1
        assert 'reloq_workspace_ref' not in body
        assert not re.search(r'\bSLOT\(', SLOT.sub('', body))
        assert all(0 <= bias + int(offset) < size for offset in slots)
        # Neither the allocated start nor its local base may be reassigned.
        assert len(re.findall(r'\bframe\s*=(?!=)', body)) == 1
        assert len(re.findall(r'\bbase\s*=(?!=)', body)) == 1
        body = body.replace(GUARD, GUARD + DECLARATION, 1)
        body = SLOT.sub(lambda m: 'RELOQ61_WORKSPACE_REF(frame, reloq_workspace_ref, '
                        + str(bias + int(m[1])) + ')', body)
        return head + body + tail
    return FUNCTION.sub(convert, text)


def convert_bodies(units):
    return [(name, bind(text)) for name, text in units]


if __name__ == '__main__':
    from pathlib import Path
    root = Path(__file__).resolve().parents[2]
    functions = sites = 0
    for path in sorted((root/'lang').glob('*/delta_rules_c??_*.c')):
        before = path.read_text()
        after = bind(before)
        functions += after.count(DECLARATION) - before.count(DECLARATION)
        sites += before.count('SLOT(') - after.count('SLOT(')
        path.write_text(after)
    print(functions, 'bounded C workspaces;', sites, 'local reference sites')
