"""Verify Persian script and Pinglish use one native word/phrase frontend."""
from __future__ import annotations
import argparse
import ctypes
from pathlib import Path
from direct_fa_sequence import EciIndex, project
from hybrid_fa_frontend import DirectFirstPersian


def runtime_tests(library):
    from nvda_driver_probe import make_language_engine, PERSIAN_PROBE, PINGLISH_PROBE

    original, fallback = DirectFirstPersian.project_direct, DirectFirstPersian._fallback
    calls = []
    def checked(frontend, text, events, *, pinglish=False):
        result = original(frontend, text, events, pinglish=pinglish)
        calls.append((pinglish, result is not None))
        return result
    def no_fallback(_frontend):
        raise AssertionError('Covered Persian input unexpectedly fell back to eSpeak')
    DirectFirstPersian.project_direct, DirectFirstPersian._fallback = checked, no_fallback
    engines = []
    try:
        for mode in (PERSIAN_PROBE, PINGLISH_PROBE):
            engines.append(make_language_engine(library, library, None, Path('/unavailable'),
                                                 Path('/unavailable'), language=mode.id))
        # The unknown word must use the shared complete native analyzer even
        # without an eSpeak reference, instead of spelling its characters.
        script = ('سلام دنیا؟', 'کتاب و خانه.', 'او آمد.', 'ژژژژژ')
        for text in script:
            output = []
            for mode, engine in enumerate(engines):
                calls.clear(); pcm = bytearray(); marks = []
                assert engine.speak((EciIndex(1), text, EciIndex(2)), pcm.extend, marks.append)
                assert pcm and marks == [1, 2] and (bool(mode), True) in calls, (text, calls)
                output.append(bytes(pcm))
            assert output[0] == output[1], text
        for text in ('salam va donya', 'سلام khane را', 'salam دنیا؟'):
            calls.clear(); pcm = bytearray(); marks = []
            assert engines[1].speak((EciIndex(1), text, EciIndex(2)), pcm.extend, marks.append)
            assert pcm and marks == [1, 2] and (True, True) in calls, (text, calls)
    finally:
        for engine in engines:
            engine.close()
        DirectFirstPersian.project_direct, DirectFirstPersian._fallback = original, fallback


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
    if shim is None:
        runtime_tests(args.library)
    print(f'Passed {count} Persian/Pinglish shared-frontend position comparisons and mode isolation.')

if __name__=='__main__':main()
