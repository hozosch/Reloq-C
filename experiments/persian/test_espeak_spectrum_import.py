"""Exercise eSpeak source formats and imports into an actual Eloquence phone."""
from __future__ import annotations
import argparse
from array import array
from pathlib import Path
import struct
import tempfile

from test_phoneme_editor import package


def example(kind, *, empty=False):
    data=bytearray(kind+b'\x04\0\0\0test'+struct.pack('<hhhh',2,100,0,0))
    for frame in range(2):
        # Four zero-valued extended-precision numbers are valid.
        data.extend(b'\0'*40)
        data.extend(struct.pack('<Hhh',2,0,100))
        if kind==b'SPECTSQ2':data.extend(b'\0'*4)
        for i in range(9):
            frequency=(200,450,1700,2500,3300,3900,4700,5500,6500)[i]+50*frame
            height=0 if empty or i==4 else 1000
            data.extend(struct.pack('<hhhhhh',0,0,frequency,height,400,500))
            if kind==b'SPECTSQ2':data.extend(struct.pack('<hhh',100,0,100))
        if kind!=b'SPECTSEQ':data.extend(b'\0'*28)
        data.extend(b'\0'*4)
    return bytes(data)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--dll',required=True,type=Path)
    parser.add_argument('--real-dir',type=Path)
    args=parser.parse_args()
    with tempfile.TemporaryDirectory() as temp:
        engine,_,_,_=package(Path(temp))
        from soundlab_test import espeak_spectrum as spec
        for kind in (b'SPECTSEQ',b'SPECTSEK',b'SPECTSQ2'):
            data=example(kind);parsed=spec.parse(data)
            assert parsed.name=='test' and parsed.kind==kind.decode() and len(parsed.frames)==2
            frame=parsed.frames[1]
            assert frame.active and frame.peaks[0].frequency==250
            assert [p.frequency for p in frame.peaks[1:6]]==[500,1750,2550,3350,3950]
            assert frame.peaks[4].height==0 and frame.peaks[5].left_width==400
            for damaged in (data[:7],data[:-1],data[:20],b'NOT_A_SPECTRUM'):
                try:spec.parse(damaged)
                except ValueError:pass
                else:raise AssertionError((kind,'invalid file accepted'))
        try:spec.parse(example(b'SPECTSEQ')+b'\0'*(spec.MAX_BYTES+1))
        except ValueError:pass
        else:raise AssertionError('oversized source accepted')
        p=engine.defaults();p.update(language='Englisch (Großbritannien)',phone='i',rate=3)
        loaded=engine.load(args.dll,p)
        original,_=engine.render(args.dll,p,loaded,modified=False)
        frame=spec.parse(example(b'SPECTSEQ')).frames[1]
        p['frequencies'][3]=75;p['gains_db'][1]=-2
        target,n,skipped=engine.import_frequencies(p,loaded,frame)
        assert n==4 and skipped==1 and target[3]==75
        assert [loaded.frequencies[i]+target[i] for i in (0,1,2,4)]==[500,1750,2550,3950]
        p['frequencies']=target
        modified,_=engine.render(args.dll,p,loaded,modified=True)
        assert len(original)==len(modified) and original!=modified
        samples=array('h');samples.frombytes(modified);assert max(map(abs,samples))>100
        unchanged,_=engine.render(args.dll,p,loaded,modified=False)
        assert unchanged==original and p['gains_db'][1]==-2
        try:engine.import_frequencies(p,loaded,spec.parse(example(b'SPECTSEQ',empty=True)).frames[0])
        except ValueError:pass
        else:raise AssertionError('inactive peaks accepted')
        if args.real_dir:
            for i,expected in ((0,'SPECTSQ2'),(1,'SPECTSQ2'),(2,'SPECTSEQ'),(3,'SPECTSEQ')):
                sequence=spec.load(args.real_dir/f'espeak_sample_{i}.spec')
                assert sequence.kind==expected and any(frame.active for frame in sequence.frames)
        print('Passed three spectral formats, bounds, selected-frame transfer and British-English audio.')


if __name__=='__main__':main()
