"""Exhaustively compare the compact name table with the original table."""
import ctypes as ct
from pathlib import Path
import re
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[1]
REFERENCE = r'''
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "reloq_unicode_names.inc"
#define COUNT(a) (sizeof(a)/sizeof *(a))
int name_reference(uint32_t cp,unsigned locale,char *text,size_t capacity)
{
    if(locale>=COUNT(local_name_blocks)||!text||!capacity)return 0;
    size_t lo=0,hi=COUNT(local_name_codepoints);
    while(lo<hi){size_t m=lo+(hi-lo)/2;if(local_name_codepoints[m]<cp)lo=m+1;else hi=m;}
    if(lo==COUNT(local_name_codepoints)||local_name_codepoints[lo]!=cp)return 0;
    size_t offset=local_name_blocks[locale][lo/16],length=0;
    for(size_t i=0;i<=lo%16;++i) {
        if(offset>sizeof local_name_text-2)return -1;
        unsigned prefix=(unsigned)local_name_text[offset]|((unsigned)local_name_text[offset+1]<<8);
        offset+=2;
        const unsigned char *end=memchr(local_name_text+offset,0,sizeof local_name_text-offset);
        if(!end||prefix>length)return -1;
        size_t tail=(size_t)(end-local_name_text)-offset;
        if(prefix>=capacity||tail>=capacity-prefix)return -1;
        memcpy(text+prefix,local_name_text+offset,tail);
        length=prefix+tail;text[length]=0;offset=(size_t)(end-local_name_text)+1;
    }
    return 1;
}
size_t name_count(void){return COUNT(local_name_codepoints);}
uint32_t name_point(size_t i){return local_name_codepoints[i];}
'''


def main():
    with tempfile.TemporaryDirectory(prefix='reloq-unicode-check-') as folder:
        tmp = Path(folder)
        source = tmp / 'reference.c'
        source.write_text(REFERENCE, encoding='ascii')
        flags = ['cc', '-O2', '-fPIC', '-shared', '-I', str(ROOT / 'src')]
        old_path, new_path = tmp / 'reference.so', tmp / 'compact.so'
        subprocess.run([*flags, str(source), '-o', str(old_path)], check=True)
        subprocess.run([*flags, str(ROOT / 'src/reloq_unicode_names.c'), '-o', str(new_path)], check=True)
        old, new = ct.CDLL(str(old_path)), ct.CDLL(str(new_path))
        old.name_count.restype = ct.c_size_t
        old.name_point.argtypes = [ct.c_size_t]
        old.name_point.restype = ct.c_uint32
        for fun in (old.name_reference, new.reloq_unicode_name_local):
            fun.argtypes = [ct.c_uint32, ct.c_uint, ct.c_void_p, ct.c_size_t]
            fun.restype = ct.c_int
        a, b = ct.create_string_buffer(1024), ct.create_string_buffer(1024)
        started = time.monotonic()
        checks = 0
        for locale in range(10):
            for index in range(old.name_count()):
                cp = old.name_point(index)
                left = old.name_reference(cp, locale, a, len(a))
                right = new.reloq_unicode_name_local(cp, locale, b, len(b))
                assert left == right and (left != 1 or a.value == b.value), (locale, hex(cp), left, right)
                checks += 1
                if index % 127 == 0:
                    for capacity in (0, 1, 2, 5, 16, 32, 128):
                        left = old.name_reference(cp, locale, a, capacity)
                        right = new.reloq_unicode_name_local(cp, locale, b, capacity)
                        assert left == right and (left != 1 or a.value == b.value), (locale, hex(cp), capacity)
                        checks += 1
        for cp in (0x10ffff, 0xdead, 0x1ffff):
            assert old.name_reference(cp, 0, a, 1024) == new.reloq_unicode_name_local(cp, 0, b, 1024)
        assert new.reloq_unicode_name_local(65, 10, b, 1024) == 0
        assert new.reloq_unicode_name_local(65, 0, None, 1024) == 0
        print(f'{checks:,} Unicode-name and short-buffer comparisons passed in {time.monotonic()-started:.1f}s')
        print(f'Native table object: {old_path.stat().st_size:,} -> {new_path.stat().st_size:,} bytes')


if __name__ == '__main__':
    main()
