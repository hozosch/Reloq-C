"""Run the filename test with a Windows first-chance access-violation trace."""
import ctypes as c
from pathlib import Path
import runpy
import sys

if sys.platform != 'win32' or c.sizeof(c.c_void_p) != 8:
    raise SystemExit('This diagnostic requires 64-bit Windows')


class ExceptionRecord(c.Structure):
    _fields_ = [('code', c.c_uint32), ('flags', c.c_uint32),
                ('nested', c.c_void_p), ('address', c.c_void_p),
                ('count', c.c_uint32), ('padding', c.c_uint32),
                ('information', c.c_size_t * 15)]


class ExceptionPointers(c.Structure):
    _fields_ = [('record', c.POINTER(ExceptionRecord)), ('context', c.c_void_p)]


kernel = c.WinDLL('kernel32', use_last_error=True)
callback_type = c.WINFUNCTYPE(c.c_long, c.POINTER(ExceptionPointers))
kernel.AddVectoredExceptionHandler.argtypes = (c.c_ulong, callback_type)
kernel.AddVectoredExceptionHandler.restype = c.c_void_p
kernel.RemoveVectoredExceptionHandler.argtypes = (c.c_void_p,)
kernel.GetModuleHandleExW.argtypes = (c.c_uint32, c.c_void_p, c.POINTER(c.c_void_p))
kernel.GetModuleHandleExW.restype = c.c_int
kernel.GetModuleFileNameW.argtypes = (c.c_void_p, c.c_wchar_p, c.c_uint32)
kernel.GetModuleFileNameW.restype = c.c_uint32


@callback_type
def handler(info):
    record = info.contents.record.contents
    if (record.code == 0xC0000005 and record.count >= 2 and
            record.information[0] == 1 and record.information[1] == 0):
        address = record.address or 0
        module = c.c_void_p()
        name = c.create_unicode_buffer(1024)
        if kernel.GetModuleHandleExW(6, address, c.byref(module)):
            kernel.GetModuleFileNameW(module, name, len(name))
        registers = c.cast(info.contents.context, c.POINTER(c.c_uint64))
        names = ('rax', 'rcx', 'rdx', 'rbx', 'rsp', 'rbp', 'rsi', 'rdi',
                 'r8', 'r9', 'r10', 'r11', 'r12', 'r13', 'r14', 'r15', 'rip')
        state = ' '.join(f'{n}={registers[15+i]:x}' for i, n in enumerate(names))
        print(f'ISSUE10_ACCESS_VIOLATION module={name.value} '
              f'ip={address:x} base={module.value or 0:x} '
              f'rva={address-(module.value or 0):x} '
              f'action={record.information[0]} target={record.information[1]:x} {state}',
              file=sys.stderr, flush=True)
    return 0  # Continue the normal Windows/ctypes exception handling.


token = kernel.AddVectoredExceptionHandler(1, handler)
if not token:
    raise OSError(c.get_last_error(), 'AddVectoredExceptionHandler failed')
root = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(root / 'addon'))
try:
    runpy.run_path(str(root / 'addon' / 'test_synth_native.py'), run_name='__main__')
finally:
    kernel.RemoveVectoredExceptionHandler(token)
