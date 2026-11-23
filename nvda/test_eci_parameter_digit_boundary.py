"""A queued voice change must not consume following digit text (#56)."""
from __future__ import annotations
import argparse
from pathlib import Path
from _reloq_eci_direct import (DirectEciEngine, EciIndex, EciProsody, EciPitchHz,
                               NATIVE_PITCH, PROCESS_TAGS)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--library', required=True, type=Path)
    args = p.parse_args()
    checks = 0
    for dialect in (0x10000, 0x10001, 0x40000, 0x20000, 0x130000):
        e = DirectEciEngine(args.library.resolve(), dialect=dialect, encoding='utf8', native_voice=True)
        try:
            for tags in (0, 1):
                e.set_parameter(PROCESS_TAGS, tags)
                for change in (EciProsody(6, 80), EciProsody(7, 75),
                               EciProsody(8, 200), EciPitchHz(180)):
                    def render(second):
                        pcm = bytearray(); marks = []
                        assert e.speak(['Batteries, ', change, EciIndex(11), second,
                                        EciIndex(12)], pcm.extend, lambda i: marks.append((i,len(pcm))))
                        return bytes(pcm), marks
                    for text in ('90 percent charged.', '1234.', '0', '3.14'):
                        actual = render(text); separated = render(' ' + text)
                        assert actual[0] == separated[0] and actual[0], (hex(dialect), tags, change, text)
                        assert [i for i,_ in actual[1]] == [11,12]
                        assert actual[1][-1][1] == len(actual[0])
                        checks += 1
                # Use the public native parameter API in the precise order
                # reported by the iOS caller, rather than only a voice param.
                def native(second):
                    pcm=bytearray();marks=[]
                    e.set_parameter(NATIVE_PITCH, 60)
                    e._on_pcm=pcm.extend;e._on_index=lambda i: marks.append((i,len(pcm)))
                    try:
                        left=b'Batteries, ';right=second.encode('utf8');d=e._dll;h=e._handle
                        assert d.eciAddTextUtf8(h,left,len(left),0)
                        assert d.eciSetParam(h,NATIVE_PITCH,65)>=0
                        assert d.eciInsertIndex(h,31)
                        assert d.eciAddTextUtf8(h,right,len(right),0)
                        assert d.eciInsertIndex(h,32)
                        assert d.eciSynthesize(h) and d.eciSynchronize(h)
                    finally:
                        e._on_pcm=e._on_index=None
                    return bytes(pcm),marks
                x,y=native('90 percent charged.'),native(' 90 percent charged.')
                assert x[0]==y[0] and x[0], (hex(dialect), tags, 'native pitch')
                assert [i for i,_ in x[1]]==[31,32] and x[1][-1][1]==len(x[0])
                checks+=1
        finally:e.close()
    print(f'Passed {checks} queued voice/digit boundaries and end indices across five languages.')

if __name__=='__main__':main()
