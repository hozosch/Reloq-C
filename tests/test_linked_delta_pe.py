"""Exercise the stripped-PE inventory with small synthetic PE32 images."""
import struct
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from inspect_linked_delta_pe import PE32, PEError, inspect


def image(*, targets=None, exported=b'foo', ordinal=False):
    data = bytearray(0xa00)
    data[:2] = b'MZ'
    struct.pack_into('<I', data, 0x3c, 0x80)
    data[0x80:0x84] = b'PE\0\0'
    coff, optional = 0x84, 0x98
    struct.pack_into('<HH', data, coff, 0x14c, 2)
    struct.pack_into('<H', data, coff + 16, 0xe0)
    struct.pack_into('<H', data, optional, 0x10b)
    struct.pack_into('<I', data, optional + 28, 0x10000000)
    struct.pack_into('<I', data, optional + 92, 16)
    struct.pack_into('<I', data, optional + 96, 0x2090)
    struct.pack_into('<I', data, optional + 104, 0x2000)
    sections = optional + 0xe0
    data[sections:sections + 8] = b'.text\0\0\0'
    struct.pack_into('<IIII', data, sections + 8, 0x400, 0x1000, 0x400, 0x200)
    struct.pack_into('<I', data, sections + 36, 0x60000020)
    sections += 40
    data[sections:sections + 8] = b'.rdata\0\0'
    struct.pack_into('<IIII', data, sections + 8, 0x400, 0x2000, 0x400, 0x600)
    struct.pack_into('<I', data, sections + 36, 0x40000040)
    for i, target in enumerate(targets or [0x1300] * 5):
        start = 0x200 + 0x20 * i
        data[start:start + 8] = b'\x55\x8b\xec' + b'\x90' * 5
        data[start + 8] = 0xe8
        struct.pack_into('<i', data, start + 9, target - (0x1000 + 0x20 * i + 13))
        data[start + 13:start + 16] = b'\x83\xc4\x18'
    struct.pack_into('<IIIII', data, 0x600, 0x2050, 0, 0, 0x2040, 0x2050)
    data[0x640:0x648] = b'dep.dll\0'
    struct.pack_into('<II', data, 0x650, 0x2070, 0x80000007 if ordinal else 0)
    data[0x672:0x676] = b'foo\0'
    struct.pack_into('<I', data, 0x690 + 24, 1)
    struct.pack_into('<I', data, 0x690 + 32, 0x20d0)
    struct.pack_into('<I', data, 0x6d0, 0x20e0)
    data[0x6e0:0x6e4] = exported + b'\0'
    return bytes(data)


class LinkedDeltaPETests(unittest.TestCase):
    def test_imports_exports_and_candidates(self):
        binary = PE32(image(ordinal=True))
        self.assertEqual(binary.imports(), {'dep.dll': ['foo', '#7']})
        self.assertEqual(binary.exports(), {'foo'})
        candidate = binary.rule_candidates()
        self.assertEqual(candidate['runtime_target'], 0x10001300)
        self.assertEqual(candidate['call_sites'], 5)
        self.assertEqual(len(candidate['rule_entry_candidates']), 5)
        with tempfile.TemporaryDirectory() as folder:
            module, dependency = Path(folder) / 'voice.syn', Path(folder) / 'dep.dll'
            module.write_bytes(image(ordinal=True))
            dependency.write_bytes(image())
            checked = inspect(module, [dependency])['checked_dependencies']['dep.dll']
            self.assertEqual(checked, {'named_imports': 1, 'matched': 1,
                                       'missing': [], 'ordinal_imports': 1})
            dependency.write_bytes(image(exported=b'bar'))
            self.assertEqual(inspect(module, [dependency])['checked_dependencies']['dep.dll']['missing'], ['foo'])

    def test_no_dominant_target_is_not_identified_as_rules(self):
        binary = PE32(image(targets=[0x1300, 0x1300, 0x1300, 0x1340, 0x1340]))
        self.assertIsNone(binary.rule_candidates())

    def test_truncated_and_non_x86_images_are_rejected(self):
        with self.assertRaises(PEError):
            PE32(b'MZ')
        with self.assertRaises(PEError):
            PE32(image()[:-1])
        bad = bytearray(image())
        struct.pack_into('<H', bad, 0x84, 0x8664)
        with self.assertRaises(PEError):
            PE32(bytes(bad))

    def test_strings_cannot_run_into_another_section(self):
        bad = bytearray(image())
        bad[-1] = ord('A')
        with self.assertRaises(PEError):
            PE32(bytes(bad)).string(0x23ff)

    def test_missing_directories_are_empty(self):
        data = bytearray(image())
        struct.pack_into('<I', data, 0x98 + 92, 0)
        binary = PE32(bytes(data))
        self.assertEqual(binary.imports(), {})
        self.assertEqual(binary.exports(), set())


if __name__ == '__main__':
    unittest.main()
