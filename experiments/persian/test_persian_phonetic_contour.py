"""Keep the chosen Persian contour for supported explicit phonetic groups."""
from __future__ import annotations
import argparse
import ctypes
from pathlib import Path
from direct_fa_sequence import EciIndex, EciText
from nvda_driver_probe import make_language_engine, PERSIAN_PROBE, PINGLISH_PROBE
import continuous_driver


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--library',required=True,type=Path)
    p.add_argument('--shim',type=Path)
    args=p.parse_args()
    shim=ctypes.CDLL(str(args.shim.resolve())) if args.shim else None
    engine=make_language_engine(args.library,args.library,None,None,None)
    previous=continuous_driver.fit_us_word_pitch
    observed=[]
    def capture(*args,**kwargs):
        result=previous(*args,**kwargs)
        observed.append(bool(result))
        return result
    continuous_driver.fit_us_word_pitch=capture
    try:
        for lang in (PERSIAN_PROBE.id,PINGLISH_PROBE.id):
            engine.select_voice(lang,'1')
            persian=engine._engines[lang]
            if shim is not None:
                fn=shim.eciPersianInputToLegacy
                fn.argtypes=(ctypes.c_char_p,ctypes.c_size_t,ctypes.c_void_p,ctypes.c_size_t,
                             ctypes.POINTER(ctypes.c_size_t),ctypes.POINTER(ctypes.c_size_t))
                fn.restype=ctypes.c_int
                persian._phonetic_input=fn
            def render(tag,ending):
                observed.clear();pcm=bytearray();indices=[]
                assert engine.speak((EciIndex(1),EciText(tag,tag_control=tag),EciIndex(2),ending),
                                    pcm.extend,lambda i:indices.append((i,len(pcm))))
                assert pcm and [v for v,_ in indices]==[1,2]
                assert any(observed),(tag,ending,'chosen contour not transferred')
                return bytes(pcm),tuple(indices)
            for new,old in (('`[.1qa]','`[.1Ra]'),('`[.1xa]','`[.1Ga]'),
                            ('`[.1?a]','`[.1wa]'),('`[.1h?a]','`[.1hwa]'),
                            ('`[.1solh]','`[.1solhw]')):
                for ending in ('','?'):
                    assert render(new,ending)==render(old,ending),(lang,new,ending)
    finally:
        continuous_driver.fit_us_word_pitch=previous
        engine.close()
    print('Passed explicit Persian and Pinglish phonetics through the selected contour, including question punctuation.')

if __name__=='__main__':main()
