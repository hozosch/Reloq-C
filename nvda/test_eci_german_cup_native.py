"""The German native dictionary's Cup root uses a short a in context."""
import argparse
from pathlib import Path
import re
import tempfile

from _reloq_eci_direct import DirectEciEngine


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    args = parser.parse_args()
    engine = DirectEciEngine(args.library.resolve(), dialect=0x40000,
                             encoding='utf8', native_voice=True)
    try:
        for text in ('cup', 'Cup', 'CUP', 'cups', 'Cups', 'Weltcup', 'Weltcups',
                     'Cupslalom', 'Cupfinale', 'Das ist ein Cup.', 'Der Cup geht weiter.'):
            phones = engine._phonemes_for_probe(text)
            assert 'kAp' in phones and "k'OE'p" not in phones, (text, phones)
        assert re.findall(r'`\[([^]]+)\]', engine._phonemes_for_probe('cup')) == \
               re.findall(r'`\[([^]]+)\]', engine._phonemes_for_probe('Kapp'))
        # Caller-supplied pronunciations still take priority over the default.
        with tempfile.TemporaryDirectory() as folder:
            Path(folder, 'DEUmain.dic').write_bytes(b"cup\t`[.1k'OE'p]\n")
            engine.set_dictionary_path(folder)
            assert "k'OE'p" in engine._phonemes_for_probe('cup')
            engine.set_dictionary_path(None)
    finally:
        engine.close()
    print('German Cup: corrected native dictionary, case, compounds, sentence and custom dictionary passed')


if __name__ == '__main__':
    main()
