"""Generate losslessly compressed, independently readable Unicode-name blocks.

The original front-coded tables remain the source of truth. Compress four of
their 16-name blocks together with a small LZSS format; the runtime expands
only the block containing the requested character. No language data or
spoken name is inferred or changed.
"""
from collections import defaultdict
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'src'
GROUP = 4
LOCALES = 10


def array(data, symbol):
    match = re.search(rb'\b' + symbol.encode('ascii') + rb'\s*\[[^=]*=\s*\{', data)
    if not match:
        raise ValueError('Missing array: ' + symbol)
    end = data.index(b'};', match.end())
    return [int(m.group()) for m in re.finditer(rb'\d+', data[match.end():end])]


def compress(data):
    # One control byte describes eight literals or 12-bit-distance matches.
    # Matches of at least 18 bytes have an unsigned extension byte sequence.
    result = bytearray()
    positions = defaultdict(list)
    at = 0
    while at < len(data):
        control_at = len(result)
        result.append(0)
        control = 0
        for bit in range(8):
            if at == len(data):
                break
            key = data[at:at + 3]
            candidates = positions.get(key, ()) if len(key) == 3 else ()
            longest = distance = 0
            for previous in reversed(candidates[-16:]):
                delta = at - previous
                if delta > 4096:
                    break
                length = 3
                limit = min(len(data) - at, 273)
                while length < limit and data[previous + length] == data[at + length]:
                    length += 1
                if length > longest:
                    longest, distance = length, delta
                if longest == limit:
                    break
            if longest >= 3:
                control |= 1 << bit
                value = ((min(longest, 18) - 3) << 12) | (distance - 1)
                result.extend((value & 255, value >> 8))
                if longest >= 18:
                    remaining = longest - 18
                    while remaining >= 255:
                        result.append(255)
                        remaining -= 255
                    result.append(remaining)
                count = longest
            else:
                result.append(data[at])
                count = 1
            for index in range(at, at + count):
                key = data[index:index + 3]
                if len(key) == 3:
                    chain = positions[key]
                    chain.append(index)
                    if len(chain) > 32:
                        del chain[:16]
            at += count
        result[control_at] = control
    return result


def decompress(data):
    out = bytearray()
    at = 0
    while at < len(data):
        control = data[at]
        at += 1
        for bit in range(8):
            if at == len(data):
                break
            if control & (1 << bit):
                value = data[at] | data[at + 1] << 8
                at += 2
                distance, length = (value & 4095) + 1, (value >> 12) + 3
                if length == 18:
                    while True:
                        extension = data[at]
                        at += 1
                        length += extension
                        if extension != 255:
                            break
                for _ in range(length):
                    out.append(out[-distance])
            else:
                out.append(data[at])
                at += 1
    return out


def emit_array(lines, typename, name, values, columns):
    lines.append(f'static const {typename} {name}[] = {{\n')
    for at in range(0, len(values), columns):
        lines.append(','.join(map(str, values[at:at + columns])) + ',\n')
    lines.append('};\n')


def main():
    legacy = b''.join((SOURCE / f'reloq_unicode_names_part{i:02}.inc').read_bytes()
                      for i in range(1, 8))
    points = array(legacy, 'local_name_codepoints')
    offsets = array(legacy, 'local_name_blocks')
    data = bytes(array(legacy, 'local_name_text'))
    blocks_per_locale = (len(points) + 15) // 16
    assert len(offsets) == LOCALES * blocks_per_locale
    assert offsets[0] == 0 and all(a < b for a, b in zip(offsets, offsets[1:]))
    compressed, groups = bytearray(), []
    maximum = 0
    for locale in range(LOCALES):
        first = locale * blocks_per_locale
        for block in range(0, blocks_per_locale, GROUP):
            end = (offsets[first + block + GROUP] if block + GROUP < blocks_per_locale
                   else offsets[first + blocks_per_locale] if locale + 1 < LOCALES
                   else len(data))
            part = data[offsets[first + block]:end]
            packed = compress(part)
            assert decompress(packed) == part
            groups.append(len(compressed))
            compressed.extend(packed)
            maximum = max(maximum, len(part))
    groups.append(len(compressed))
    group_count = (blocks_per_locale + GROUP - 1) // GROUP
    lines = ['/* Lossless blocks generated from the original Unicode name tables. */\n',
             f'#define LOCAL_NAME_GROUPS_PER_LOCALE {group_count}\n',
             f'#define LOCAL_NAME_GROUP_MAX_BYTES {maximum}\n']
    emit_array(lines, 'uint32_t', 'local_name_codepoints', points, 16)
    emit_array(lines, 'uint32_t', 'local_name_packed_offsets', groups, 16)
    emit_array(lines, 'unsigned char', 'local_name_packed_text', compressed, 24)
    # Keep source files reasonably sized for compilers and code review.
    parts = []
    current, size = [], 0
    for line in lines:
        if size >= 2_000_000:
            parts.append(''.join(current))
            current, size = [], 0
        current.append(line)
        size += len(line)
    parts.append(''.join(current))
    for i, part in enumerate(parts, 1):
        (SOURCE / f'reloq_unicode_packed_part{i:02}.inc').write_text(part, encoding='ascii')
    (SOURCE / 'reloq_unicode_packed.inc').write_text(''.join(
        f'#include "reloq_unicode_packed_part{i:02}.inc"\n' for i in range(1, len(parts) + 1)),
        encoding='ascii')
    print(f'{len(points)} characters, {LOCALES} locales: {len(data):,} -> {len(compressed):,} bytes; '
          f'{len(groups)-1:,} independent blocks; largest decoded block {maximum:,} bytes')


if __name__ == '__main__':
    main()
