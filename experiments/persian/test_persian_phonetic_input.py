"""Verify q/x/? input and its scoped playback against the accepted phones."""
from __future__ import annotations
import argparse
import ctypes as C
from pathlib import Path
from direct_fa_sequence import EciIndex, EciText, ScopedDirectPersianEngine
from hybrid_fa_frontend import DirectFirstPersian
from _reloq_eci_direct import DirectEciEngine, PROCESS_TAGS


def control(tag): return EciText(tag,tag_control=tag)


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--library',required=True,type=Path)
    p.add_argument('--shim',type=Path,help='optional local build helper')
    args=p.parse_args()
    frontend=DirectFirstPersian(args.library,None,None,None)
    engine=ScopedDirectPersianEngine(args.library,frontend,allow_fallback=True)
    shim=C.CDLL(str(args.shim.resolve())) if args.shim else None
    fn=shim.eciPersianInputToLegacy if shim else engine._phonetic_input
    assert fn is not None
    fn.argtypes=(C.c_char_p,C.c_size_t,C.c_void_p,C.c_size_t,C.POINTER(C.c_size_t),C.POINTER(C.c_size_t))
    fn.restype=C.c_int
    engine._phonetic_input=fn
    def convert(tag):
        raw=tag.encode('ascii'); buf=C.create_string_buffer(len(raw)+2); n=C.c_size_t();err=C.c_size_t()
        status=fn(raw,len(raw),buf,len(buf),C.byref(n),C.byref(err))
        return status,buf.value.decode('ascii'),n.value,err.value
    for source,target in (('`[.1qa]','`[.1Ra]'),('`[.1xa]','`[.1Ga]'),
                          ('`[.0mAn.1hA.0?An]','`[.0mAn.1hA.0wAn]'),
                          ('`[.1h?a]','`[.1hwa]'),
                          ('`[.1solh]','`[.1solhw]'),('`[.1solhw]','`[.1solhw]'),
                          ('`[.1qa.0xi.0?u]','`[.1Ra.0Gi.0wu]')):
        assert convert(source)[:2]==(0,target),(source,convert(source))
    # Transactional failure: a final h followed by a glottal stop is
    # ambiguous in the temporary carrier and is never silently collapsed.
    raw=b'`[.1Ah?]';buf=C.create_string_buffer(b'unchanged',30);n=C.c_size_t();err=C.c_size_t()
    assert fn(raw,len(raw),buf,len(buf),C.byref(n),C.byref(err))==1 and buf.value==b'unchanged'
    def render(parts):
        audio,indexes=bytearray(),[]
        assert engine.speak(parts,audio.extend,lambda value:indexes.append((value,len(audio))))
        return bytes(audio),indexes
    checks=0
    try:
        for source,target in (('`[.1qa]','`[.1Ra]'),('`[.1xa]','`[.1Ga]'),
                              ('`[.1?a]','`[.1wa]'),('`[.1solh]','`[.1solhw]'),
                              ('`[.1h?a]','`[.1hwa]'),
                              ('`[.0mAn.1hA.0?An]','`[.0mAn.1hA.0wAn]')):
            for suffix in ((),('?',)):
                a=render((EciIndex(1),control(source),EciIndex(2),*suffix))
                b=render((EciIndex(1),control(target),EciIndex(2),*suffix))
                assert a==b,(source,target,suffix)
                checks+=1
        assert render((control('`[.1h?a]'),))[0] not in (
            render((control('`[.1ha]'),))[0],render((control('`[.1?a]'),))[0])
        # Ordinary Finnish uses its original input; the Persian transform is
        # not installed on the generic ECI handle.
        finn=DirectEciEngine(args.library.resolve(),dialect=0x90000,encoding='utf8',native_voice=True)
        try:
            finn.set_parameter(PROCESS_TAGS,1)
            shown=finn._phonemes_for_probe('`[.1qa]')
            assert shown and '`[.1Ra]' not in shown,shown
        finally:finn.close()
    finally:
        engine.close();frontend.close()
    print(f'Passed Persian q/x/? and final-H input, punctuation, isolation and {checks} PCM/index comparisons.')

if __name__=='__main__':main()
