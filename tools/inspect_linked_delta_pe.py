"""Inspect a linked x86 Delta language module without distributing its bytes.

This is a first-stage inventory for stripped PE32 modules. It finds imports,
exports and likely generated-rule entry points; it does not recover rule
names, decode their bodies, or produce a working language.
"""
from __future__ import annotations

import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import struct


class PEError(ValueError):
    pass


class PE32:
    def __init__(self, data: bytes):
        self.data = data
        if self.take(0, 2) != b'MZ':
            raise PEError('missing DOS header')
        pe = self.u32(0x3c)
        if self.take(pe, 4) != b'PE\0\0':
            raise PEError('missing PE header')
        coff = pe + 4
        if self.u16(coff) != 0x14c:
            raise PEError('expected a 32-bit x86 PE')
        count = self.u16(coff + 2)
        if not 1 <= count <= 96:
            raise PEError('invalid section count')
        self.symbol_count = self.u32(coff + 12)
        optional = coff + 20
        optional_size = self.u16(coff + 16)
        self.take(optional, optional_size)
        if optional_size < 112 or self.u16(optional) != 0x10b:
            raise PEError('expected a PE32 optional header')
        self.image_base = self.u32(optional + 28)
        directory_count = self.u32(optional + 92)
        self.directories = [self.u32(optional + 96 + 8 * i) if i < directory_count else 0
                            for i in (0, 1)]
        self.sections = []
        for i in range(count):
            at = optional + optional_size + 40 * i
            self.take(at, 40)
            name = self.take(at, 8).split(b'\0', 1)[0].decode('ascii', 'replace')
            size, rva, raw_size, raw_offset = struct.unpack_from('<IIII', data, at + 8)
            flags = self.u32(at + 36)
            if raw_size:
                self.take(raw_offset, raw_size)
            self.sections.append((name, rva, min(size or raw_size, raw_size), raw_offset, flags))

    def take(self, offset, length):
        if offset < 0 or length < 0 or offset > len(self.data) or length > len(self.data) - offset:
            raise PEError('truncated PE data')
        return self.data[offset:offset + length]

    def u16(self, offset):
        return struct.unpack('<H', self.take(offset, 2))[0]

    def u32(self, offset):
        return struct.unpack('<I', self.take(offset, 4))[0]

    def rva(self, value, length=1):
        for _, rva, size, offset, _ in self.sections:
            if rva <= value and value - rva <= size and length <= size - (value - rva):
                return offset + value - rva
        raise PEError(f'RVA outside file-backed sections: {value:#x}')

    def string(self, rva):
        start = self.rva(rva)
        section_end = next(offset + size for _, base, size, offset, _ in self.sections
                           if base <= rva < base + size)
        end = self.data.find(b'\0', start, min(section_end, start + 4096))
        if end < 0:
            raise PEError('unterminated PE string')
        return self.data[start:end].decode('ascii', 'strict')

    def imports(self):
        if not self.directories[1]:
            return {}
        result = {}
        for index in range(1024):
            at = self.rva(self.directories[1] + index * 20, 20)
            fields = struct.unpack('<IIIII', self.take(at, 20))
            if not any(fields):
                return result
            original, _, _, name, first = fields
            dll = self.string(name)
            thunk = original or first
            values = []
            for item in range(65536):
                value = self.u32(self.rva(thunk + item * 4, 4))
                if not value:
                    break
                values.append(f'#{value & 0xffff}' if value & 0x80000000 else self.string(value + 2))
            else:
                raise PEError('unterminated import table')
            if dll.lower() in result:
                raise PEError('duplicate imported module')
            result[dll.lower()] = values
        raise PEError('unterminated import directory')

    def exports(self):
        if not self.directories[0]:
            return set()
        directory = self.rva(self.directories[0], 40)
        count, names = self.u32(directory + 24), self.u32(directory + 32)
        if count > 65536:
            raise PEError('invalid export count')
        if not count:
            return set()
        table = self.rva(names, count * 4)
        return {self.string(self.u32(table + 4 * i)) for i in range(count)}

    def rule_candidates(self):
        executable = [(rva, size, offset) for _, rva, size, offset, flags in self.sections
                      if flags & 0x20000000 and size]
        calls = []
        for rva, size, offset in executable:
            code = self.take(offset, size)
            starts, pos = [], 0
            while (pos := code.find(b'\x55\x8b\xec', pos)) >= 0:
                starts.append(pos)
                pos += 3
            for index, start in enumerate(starts):
                end = min(start + 192, starts[index + 1] if index + 1 < len(starts) else len(code), len(code))
                for pos in range(start + 3, max(start + 3, end - 7)):
                    if code[pos] != 0xe8 or code[pos + 5:pos + 8] != b'\x83\xc4\x18':
                        continue
                    displacement = struct.unpack_from('<i', code, pos + 1)[0]
                    target = rva + pos + 5 + displacement
                    if any(base <= target < base + span for base, span, _ in executable):
                        calls.append((rva + start, target))
        counts = Counter(target for _, target in calls)
        leaders = counts.most_common(2)
        if not leaders or leaders[0][1] < 3 or (len(leaders) > 1 and leaders[0][1] < 4 * leaders[1][1]):
            return None
        target, occurrences = leaders[0]
        entries = sorted({start for start, candidate in calls if candidate == target})
        return {'runtime_target': self.image_base + target, 'call_sites': occurrences,
                'rule_entry_candidates': [self.image_base + value for value in entries],
                'method': 'x86 frame prologue and repeated six-argument local call'}


def inspect(module: Path, dependencies: list[Path]):
    raw = module.read_bytes()
    binary = PE32(raw)
    imports = binary.imports()
    supplied = {}
    for path in dependencies:
        key = path.name.lower()
        if key in supplied:
            raise PEError('duplicate supplied dependency')
        supplied[key] = PE32(path.read_bytes())
    checked = {}
    for name, requested in imports.items():
        if name not in supplied:
            continue
        symbols = supplied[name].exports()
        named = [symbol for symbol in requested if not symbol.startswith('#')]
        missing = sorted(set(named) - symbols)
        checked[name] = {'named_imports': len(named), 'matched': sum(symbol in symbols for symbol in named),
                         'missing': missing, 'ordinal_imports': len(requested) - len(named)}
    extra = {name: {'exports': len(dep.exports()),
                    'eci_generate_phonemes': 'eciGeneratePhonemes' in dep.exports()}
             for name, dep in supplied.items() if name not in imports}
    candidate = binary.rule_candidates()
    return {'module': module.name, 'sha256': hashlib.sha256(raw).hexdigest(), 'machine': 'x86 PE32',
            'coff_symbol_count': binary.symbol_count, 'exports': sorted(binary.exports()),
            'imports': {name: len(values) for name, values in imports.items()},
            'checked_dependencies': checked, 'other_supplied': extra, 'candidates': candidate,
            'note': 'Candidate addresses need further validation; rule names and bodies are not recovered.'}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('module', type=Path)
    parser.add_argument('--dependency', action='append', type=Path, default=[])
    parser.add_argument('--entries', action='store_true', help='include candidate addresses in the output')
    args = parser.parse_args()
    try:
        result = inspect(args.module, args.dependency)
    except (OSError, PEError, UnicodeError) as error:
        parser.exit(2, f'Cannot inspect module: {error}\n')
    if result['candidates'] and not args.entries:
        entries = result['candidates'].pop('rule_entry_candidates')
        result['candidates']['rule_entry_count'] = len(entries)
    print(json.dumps(result, indent=2, ensure_ascii=False))


if __name__ == '__main__':
    main()
