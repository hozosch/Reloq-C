"""Finnish native ü rules; all PCM stays in RAM."""
from pathlib import Path
from _reloq_eci_direct import (DirectEciEngine,EciText,EciIndex,SAMPLE_RATE,
    PROCESS_TAGS,HEAD_SIZE,GENDER)
ROOT=Path(__file__).resolve().parents[1]

def main():
    e=DirectEciEngine(ROOT/'eci/libreloq_eci.so',dialect=0x90000,
        encoding='utf-8',experimental_generic=True)
    def render(text,characters=False):
        blocks=[];marks=[]
        assert e.speak([EciText(text,characters),EciIndex(7)],blocks.append,marks.append)
        assert marks==[7]
        return b''.join(blocks)
    count=0
    try:
        for rate in [0,1,6]:
            e.set_parameter(SAMPLE_RATE,rate)
            for gender,head in [(0,50),(1,22)]:
                e.set_parameter(GENDER,gender);e.set_parameter(HEAD_SIZE,head)
                for a,b in [('Müller.','Myller.'),('Ülkü.','Ylky.'),
                    ('Mu\u0308ller.','Myller.'),('U\u0308lku\u0308.','Ylky.')]:
                    assert render(a)==render(b),(rate,gender,a,b)
                    count+=1
                # The internal character-mode markers survive with user
                # annotations disabled. Native spelling names the letter.
                e.set_parameter(PROCESS_TAGS,0)
                name=render('saksalainen y')
                for letter in ['ü','Ü','u\u0308','U\u0308']:
                    assert render(letter,True)==name,(rate,gender,letter)
                    count+=1
        e.set_parameter(PROCESS_TAGS,0)
        literal=render('Müller `ts2 ABC.')
        e.set_parameter(PROCESS_TAGS,1)
        assert literal!=render('Müller `ts2 ABC.'),'disabled user tags executed'
    finally:e.close()
    print(count,'native Finnish ü/y and character-name comparisons passed; no audio files')

if __name__=='__main__':main()
