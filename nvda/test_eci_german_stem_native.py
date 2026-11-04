"""German konzentrier stem in the native text and phoneme paths."""
import argparse
from pathlib import Path
import tempfile
from _reloq_eci_direct import DirectEciEngine,EciIndex,PROCESS_TAGS
from test_eci_german_clock_native import phonemes,render


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library',type=Path,required=True);args=parser.parse_args()
    engine=DirectEciEngine(args.library.resolve(),dialect=0x40000,native_voice=True)
    try:
        engine.set_parameter(PROCESS_TAGS,1)
        examples={
            'konzentrieren':'`[.0kOn.0TEn.1tri.0r@n]',
            'KONZENTRIERTEST':'`[.0kOn.0TEn.1trir.0t@st]',
            'unkonzentriert':'`[.1Un.0kOn.0TEn.2trirt]',
            'Zentrieren':'`[.0TEn.1tri.0r@n]',
            'Konzentration':'`[.2kOn.0TEn.0trA.1Tjon]',
        }
        for word,expected in examples.items():
            got=phonemes(engine,word)
            assert expected in got,(word,got,expected)
            pcm,marks=render(engine,[word,EciIndex(7)])
            assert pcm and marks==[(7,len(pcm))]
    finally:engine.close()
    # A caller's explicit, different pronunciation must win over the native
    # stem correction. The temporary dictionary is private to this handle.
    with tempfile.TemporaryDirectory() as tmp:
        lex=Path(tmp)/'override.rlex';word=b'konzentrieren';spr=b'`[.1tEst]'
        lex.write_bytes(b'RELOQ-DE-SPR-1\n'+word.hex().upper().encode()+b'\t'+spr.hex().upper().encode()+b'\n')
        engine=DirectEciEngine(args.library.resolve(),native_voice=True,lexicon_path=lex)
        try:assert '`[.1tEst]' in phonemes(engine,'konzentrieren')
        finally:engine.close()
    print(f'German stem: {len(examples)} native word/phoneme/index cases and explicit dictionary precedence passed')

if __name__=='__main__':main()
