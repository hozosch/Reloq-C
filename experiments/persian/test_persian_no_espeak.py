"""Exercise the current voice when no eSpeak executable, DLL or data exists."""
from __future__ import annotations
import argparse
from pathlib import Path
import sys
import tempfile

sys.path.insert(0,str(Path(__file__).resolve().parents[2]/'nvda'))
from _reloq_eci_direct import EciIndex
from nvda_driver_probe import PERSIAN_PROBE, PINGLISH_PROBE, make_language_engine


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library',required=True,type=Path)
    args=parser.parse_args()
    with tempfile.TemporaryDirectory() as temp:
        absent=Path(temp)/'no-espeak-library'
        for profile,cases in ((PERSIAN_PROBE,('سلام دنیا','سلام ژژژژژ دنیا',
                                             'سلام NVDA دنیا','۱۲۳')),
                              (PINGLISH_PROBE,('salam donya','salam phone donya'))):
            engine=make_language_engine(args.library,args.library,None,absent,Path(temp),
                                        language=profile.id)
            try:
                frontend=engine._engines[profile.id]._frontend
                assert not frontend._reference_available
                for text in cases:
                    middle=len(text)//2
                    pcm,marks=bytearray(),[]
                    assert engine.speak((EciIndex(1),text[:middle],EciIndex(2),text[middle:],
                                         EciIndex(3)),pcm.extend,marks.append),text
                    assert pcm and marks==[1,2,3],(text,marks,len(pcm))
                    assert frontend._worker is None,text
            finally:engine.close()
    print('Passed Persian, Pinglish and mixed speech without eSpeak, including general unknown-word rules.')


if __name__=='__main__':main()
