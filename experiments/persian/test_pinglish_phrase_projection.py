"""Compare shared-C Pinglish phrase projection with the previous real path."""
from __future__ import annotations
import argparse
import ctypes
from pathlib import Path
import random
import re

from direct_fa_sequence import EciIndex, EciPitch, project
from hybrid_fa_frontend import DirectFirstPersian
from pinglish_frontend import ALIASES
from text_frontend import UnsupportedPronunciation


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--library',required=True,type=Path)
    ap.add_argument('--shim',type=Path,help='optional local build helper')
    args=ap.parse_args()
    frontend=DirectFirstPersian(args.library,None,None,None)
    shim=ctypes.CDLL(str(args.shim.resolve())) if args.shim else None
    if shim is not None:
        frontend._project_pinglish=shim.eciPersianPinglishProjectUtf8
        frontend._project_pinglish.argtypes=frontend._project_direct.argtypes
        frontend._project_pinglish.restype=ctypes.c_int
        frontend._project_text=shim.eciPersianProjectTextUtf8
        frontend._project_text.argtypes=(ctypes.c_char_p,ctypes.c_size_t,ctypes.c_int,
            ctypes.POINTER(ctypes.c_size_t),ctypes.POINTER(ctypes.c_ubyte),ctypes.c_size_t,
            ctypes.POINTER(ctypes.c_void_p),ctypes.POINTER(ctypes.c_size_t),ctypes.POINTER(ctypes.c_size_t))
        frontend._project_text.restype=ctypes.c_int
    native=frontend._project_pinglish
    shared=frontend._project_text
    assert native is not None
    count=0
    def compare(parts):
        nonlocal count
        frontend._project_pinglish=None
        frontend._project_text=None
        try:
            try: old=project(parts,frontend,pinglish=True)
            except (UnsupportedPronunciation,ValueError) as error: old=type(error)
        finally:
            frontend._project_pinglish=native
            frontend._project_text=shared
        try: new=project(parts,frontend,pinglish=True)
        except (UnsupportedPronunciation,ValueError) as error: new=type(error)
        assert old==new,(parts,old,new)
        count+=1
    try:
        for text in ('salam va donya','khane ketabam','mikhaham ra','ketabha ast',
                     'khane3 ketabam','a man','ketabam 66','khane, salam?',
                     'سلام دنیا؟','سلام khane','salam دنیا','khane، سلام','سلام khane را'):
            assert frontend.project_direct(text,(),pinglish=True) is not None,text
        for text in ('phone salam','salam (iran)','šalam'):
            assert frontend.project_direct(text,(),pinglish=True) is None,text
        examples=('salam', 'SalAm DONYA?', 'man iran', 'salam va donya', 'salam ra',
                  'salam ast', 'be iran', 'salam 66', 'salam-iran', 'yek, do?',
                  'salam physics', 'salam3', 'salam (iran)', 'šalam', 'salam x',
                  'khane ketabam', 'mikhaham ra', 'ketabha ast', 'NVDA',
                  'khane3 ketabam', 'a man', 'ketabam 66', 'khane, salam?',
                  'سلام دنیا؟', 'سلام khane', 'salam دنیا', 'khane، سلام',
                  'سلام khane را', 'khane است', 'salam دنیا؟', 'سلام3')
        for text in examples:
            compare((text,))
            for point in range(len(text)+1):
                compare((EciIndex(1),text[:point],EciIndex(2),text[point:],EciIndex(3)))
        rng=random.Random(176)
        words=tuple(ALIASES) + ('khane','khaneh','chetori','parantez','rayane',
            'mikhaham','miravam','nemiravam','tehrani','futbal','footbal',
            'ketabam','ketabha','khubtar','donyaye','x','a','z','nvda',
            'phone','this','zooon','64','66')
        for _ in range(1600):
            text=rng.choice((' ','  ', ', ', '? ', ' - ')).join(
                rng.choice((word,word.upper(),word.title())) for word in
                (rng.choice(words) for _ in range(rng.randrange(1,5))))
            point=rng.randrange(len(text)+1)
            compare((EciIndex(10),text[:point],EciIndex(11),text[point:],EciIndex(12)))
        lexicon=Path(__file__).with_name('direct_fa_pinglish_lexicon.inc').read_text(encoding='utf8')
        entries=re.findall(r'^\s*\{"([a-z]+)", "[^"\\]+"\},',lexicon,re.M)
        assert len(entries)>25000
        for _ in range(600):
            word=rng.choice(entries)
            text=rng.choice(('salam ', 'man ', ''))+word+rng.choice(('', ' ra', ' ast', ', 66?'))
            point=rng.randrange(len(text)+1)
            compare((EciIndex(10),text[:point],EciIndex(11),text[point:],EciIndex(12)))
        scripts=('سلام','دنیا','کتاب','خوب','ایران','است','را','۱۲','گزینه')
        for _ in range(400):
            text=rng.choice((' ', '، ', ', ', ' ? ')).join(rng.choice(words+scripts) for _ in range(3))
            point=rng.randrange(len(text)+1)
            compare((EciIndex(20),text[:point],EciIndex(21),text[point:],EciIndex(22)))
        # Arabic blocks also contain punctuation, controls, digits and marks.
        # Only use the native shortcut when it agrees with the old tokenizer.
        for cp in range(0x0600, 0x0700):
            for text in (f'salam {chr(cp)} donya', f'salam{chr(cp)}donya'):
                compare((EciIndex(30), text, EciIndex(31)))
        for text in ('salam donya','man iran','salam ra'):
            for point in range(len(text)+1):
                compare((text[:point],EciPitch(55),text[point:]))
    finally:frontend.close()
    print(f'Passed {count} native Pinglish phrase, punctuation, index and control comparisons.')

if __name__=='__main__':main()
