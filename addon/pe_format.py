"""Checks for the native libraries included with the add-on packages."""
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


def assert_x86_64_elf(library: bytes) -> None:
    """The Linux package's libreloq_eci.so: a 64-bit little-endian x86-64 shared object."""
    if len(library) < 64 or library[:4] != b"\x7fELF":
        raise ValueError("libreloq_eci.so is not an ELF file")
    if library[4] != 2 or library[5] != 1:
        raise ValueError("libreloq_eci.so must be 64-bit little-endian ELF")
    kind, machine = struct.unpack_from("<HH", library, 16)
    if kind != 3 or machine != 62:
        raise ValueError("libreloq_eci.so must be an x86-64 shared object")


def glibc_floor(library: bytes) -> tuple[int, ...]:
    """The newest GLIBC symbol version an ELF file names, which a system must provide."""
    import re
    versions = {tuple(int(part) for part in match.split(b"."))
                for match in re.findall(rb"GLIBC_(\d+(?:\.\d+)+)\0", library)}
    return max(versions, default=())
