"""Verify Persian script and Pinglish use one native word/phrase frontend."""
from __future__ import annotations
import argparse
import ctypes
from pathlib import Path
from direct_fa_sequence import EciIndex, EciPitch, project
from hybrid_fa_frontend import DirectFirstPersian


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--library',required=True,type=Path);p.add_argument('--shim',type=Path)
    args=p.parse_args()
    f=DirectFirstPersian(args.library,None,None,None)
    shim=ctypes.CDLL(str(args.shim.resolve())) if args.shim else None
    if shim:
        f._project_text=shim.eciPersianProjectTextUtf8
        f._project_text.argtypes=(ctypes.c_char_p,ctypes.c_size_t,ctypes.c_int,
            ctypes.POINTER(ctypes.c_size_t),ctypes.POINTER(ctypes.c_ubyte),ctypes.c_size_t,
            ctypes.POINTER(ctypes.c_void_p),ctypes.POINTER(ctypes.c_size_t),ctypes.POINTER(ctypes.c_size_t))
        f._project_text.restype=ctypes.c_int
    assert f._project_text is not None
    count=0
    try:
        for text in ('سلام دنیا؟','من ایران','سلام را','سلام است','کتاب و خانه.',
                     'به ایران','۱۲: سلام','شماره ۹۶','او آمد.','سلام، خوب؟',
                     'راه‌اندازی رایانه','بختیاری‌زاده','zirkhat'):
            if text=='zirkhat':
                assert f.project_direct(text,(),pinglish=False) is None
                assert f.project_direct(text,(),pinglish=True) is not None
                continue
            a=f.project_direct(text,(),pinglish=False)
            b=f.project_direct(text,(),pinglish=True)
            assert a==b and a is not None,(text,a,b)
            for pos in range(len(text)+1):
                parts=(EciIndex(1),text[:pos],EciIndex(2),text[pos:],EciIndex(3))
                assert project(parts,f,pinglish=False)==project(parts,f,pinglish=True),(text,pos)
                count+=1
        # Common script segments in an explicitly Pinglish-mode phrase stay
        # Persian; unrelated/unknown words retain the old fallback.
        for text in ('سلام khane','salam دنیا','khane، سلام','سلام khane را'):
            assert f.project_direct(text,(),pinglish=True) is not None,text
        assert f.project_direct('phone سلام',(),pinglish=True) is None
        raw='سلام'.encode('utf8');out=ctypes.c_void_p(123);error=ctypes.c_size_t(99)
        status=f._project_text(raw,len(raw),2,None,None,0,ctypes.byref(out),None,ctypes.byref(error))
        assert status==-1 and not out.value and error.value==0
    finally:f.close()
    print(f'Passed {count} Persian/Pinglish shared-frontend position comparisons and mode isolation.')

if __name__=='__main__':main()
