"""Use the shared Persian analysis API without NVDA or eSpeak."""
from __future__ import annotations
import argparse
import ctypes
from pathlib import Path
import sys


class Event(ctypes.Structure):
    _fields_=[(name,ctypes.c_uint32) for name in ('kind','value','flags','syllable','stress')]


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library',required=True,type=Path)
    args=parser.parse_args()
    loader=ctypes.WinDLL if sys.platform=='win32' else ctypes.CDLL
    dll=loader(str(args.library.resolve()))
    analyze=dll.eciPersianAnalyzeRunsUtf8
    analyze.argtypes=(ctypes.c_char_p,ctypes.c_size_t,ctypes.c_int,
        ctypes.POINTER(ctypes.c_size_t),ctypes.POINTER(ctypes.c_ubyte),ctypes.c_size_t,
        ctypes.POINTER(ctypes.c_size_t),ctypes.POINTER(ctypes.c_ubyte),ctypes.c_size_t,
        ctypes.POINTER(ctypes.POINTER(Event)),ctypes.POINTER(ctypes.c_size_t),
        ctypes.POINTER(ctypes.c_size_t),ctypes.POINTER(ctypes.c_size_t))
    analyze.restype=ctypes.c_int
    complete=dll.eciPersianAnalyzeRunsCompleteUtf8
    complete.argtypes,complete.restype=analyze.argtypes,analyze.restype
    dll.eciPersianRelease.argtypes=(ctypes.c_void_p,)
    for source,mode,expected in (('سلام NVDA دنیا',0,['NVDA']),
                                 ('salam phone donya',1,['phone']),
                                 ('NVDAسلام',0,['NVDA'])):
        raw=source.encode('utf8');length=len(raw)
        ends=(ctypes.c_size_t*1)(length);kinds=(ctypes.c_ubyte*1)(0)
        offsets=(ctypes.c_size_t*(length+1))(*range(length+1))
        indexes=(ctypes.c_ubyte*(length+1))(*([1]*(length+1)))
        mapped=(ctypes.c_size_t*(length+1))()
        events=ctypes.POINTER(Event)();count=ctypes.c_size_t();error=ctypes.c_size_t()
        status=analyze(raw,length,mode,ends,kinds,1,offsets,indexes,length+1,
                       ctypes.byref(events),ctypes.byref(count),mapped,ctypes.byref(error))
        try:
            assert status==0,(source,status,error.value)
            foreign=[raw[e.value:e.value+e.flags].decode('ascii') for e in events[:count.value] if e.kind==8]
            assert foreign==expected,(source,foreign)
            assert any(e.kind==2 for e in events[:count.value])
            assert list(mapped)==sorted(mapped)
        finally:dll.eciPersianRelease(events)
    source='سلام ژژژژژ NVDA دنیا';raw=source.encode('utf8');length=len(raw)
    ends=(ctypes.c_size_t*1)(length);kinds=(ctypes.c_ubyte*1)(0)
    offsets=(ctypes.c_size_t*(length+1))(*range(length+1))
    indexes=(ctypes.c_ubyte*(length+1))(*([1]*(length+1)))
    mapped=(ctypes.c_size_t*(length+1))();events=ctypes.POINTER(Event)()
    count=ctypes.c_size_t();error=ctypes.c_size_t()
    status=complete(raw,length,0,ends,kinds,1,offsets,indexes,length+1,
                    ctypes.byref(events),ctypes.byref(count),mapped,ctypes.byref(error))
    try:
        assert status==0 and count.value>0,(status,error.value)
        assert [raw[e.value:e.value+e.flags].decode('ascii') for e in events[:count.value] if e.kind==8]==['NVDA']
        assert list(mapped)==sorted(mapped)
    finally:dll.eciPersianRelease(events)
    print('Passed shared C Persian/Pinglish and mixed-English analysis without NVDA or eSpeak.')


if __name__=='__main__':main()
