"""Verify mixed reference words survive Persian bracket/comma boundaries."""
from __future__ import annotations
import argparse
from pathlib import Path
import sys

ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'nvda'))
from _reloq_eci_direct import EciIndex, EciText
from _reloq_persian_projection import ProjectedJob
from direct_fa_sequence import _safe_native_punctuation
from text_frontend import native_sequence_accepted


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--library',type=Path)
    p.add_argument('--espeak-library',type=Path)
    p.add_argument('--data-parent',type=Path)
    args=p.parse_args()
    a,b='`[.1a]','`[.1ma]'
    assert native_sequence_accepted(a+' ) , '+b,a+' '+b)
    assert native_sequence_accepted(a+'), '+b,a+', '+b)
    assert not native_sequence_accepted(a+', '+b,a+' '+b)
    assert not native_sequence_accepted(a+' ) , '+b,a+' `[.1na]')
    tagged='('+a+'),'+b
    parts=(EciText('('+a+')'),EciIndex(4),EciText(','+b))
    out=_safe_native_punctuation(ProjectedJob('',tagged,parts,False))
    assert out.tagged=='( '+a+' ),'+b, out.tagged
    assert [x.value for x in out.parts if isinstance(x,EciIndex)]==[4]
    if any((args.library,args.espeak_library,args.data_parent)):
        assert all((args.library,args.espeak_library,args.data_parent))
        from nvda_driver_probe import make_language_engine,PERSIAN_PROBE,PINGLISH_PROBE
        for profile in (PERSIAN_PROBE,PINGLISH_PROBE):
            engine=make_language_engine(args.library,args.library,None,args.espeak_library,
                                        args.data_parent,language=profile.id)
            try:
                for text in ('«چلاق آینیشمان»، نوشته','«چلاق آینیشمان» , نوشته',
                             'چلاق آینیشمان، نوشته','«آینیشمان»'):
                    audio=bytearray();marks=[]
                    assert engine.speak((EciIndex(1),text,EciIndex(2)),audio.extend,marks.append)
                    assert audio and marks==[1,2],(text,marks,len(audio))
            finally:engine.close()
    print('Persian bracket, comma, fallback and index checks passed.')

if __name__=='__main__':main()
