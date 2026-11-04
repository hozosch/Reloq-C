"""Checks for the 64-bit Windows DLL included with the add-on."""
import struct


def assert_x64_pe(dll: bytes) -> None:
    if len(dll) < 0x100 or dll[:2] != b"MZ":
        raise ValueError("eci.dll is not a Windows PE DLL")
    pe_offset = struct.unpack_from("<I", dll, 0x3C)[0]
    if pe_offset + 26 > len(dll) or dll[pe_offset:pe_offset + 4] != bytes((0x50, 0x45, 0, 0)):
        raise ValueError("eci.dll has no valid PE header")
    machine, = struct.unpack_from("<H", dll, pe_offset + 4)
    magic, = struct.unpack_from("<H", dll, pe_offset + 24)
    if machine != 0x8664 or magic != 0x20B:
        raise ValueError("eci.dll must be PE32+ x86-64; ARM64 and 32-bit are not bundled")
