"""German complete compound entries and independent abbreviation controls."""
import argparse
from pathlib import Path
import tempfile

from _reloq_eci_direct import DICTIONARY, PROCESS_TAGS, EciIndex
from test_eci_german_rc18 import engine
from test_eci_german_clock_native import phonemes, render


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    args = parser.parse_args()
    checks = 0
    with tempfile.TemporaryDirectory() as temp:
        root = Path(temp)
        (root / 'DEURoot.dic').write_bytes(
            b'eti-eloquence\t`[.1kIr.0S@]\neloquence\t`[.1tEst]\n')
        e, plain = engine(args.library), engine(args.library)
        try:
            e.set_dictionary_path(root)
            for tags in (0, 1):
                for disabled in (0, 1):
                    for item in (e, plain):
                        item.set_parameter(PROCESS_TAGS, tags)
                        item.set_parameter(DICTIONARY, disabled)
                    for text in ('ETI-Eloquence', 'eti-eloquence', '„ETI-Eloquence“',
                                 'Das ist ETI-Eloquence.', 'Das Wort „ETI-Eloquence“ ist gut.'):
                        p = phonemes(e, text)
                        assert 'kIr' in p and 'tEst' not in p, (tags, disabled, text, p)
                        assert p != phonemes(plain, text), text
                        audio, indices = render(e, [text, EciIndex(7)])
                        assert audio and indices == [(7, len(audio))], (text, indices)
                        checks += 1
                    for text in ('Test-Eloquence', 'NVDA-Eloquence'):
                        p = phonemes(e, text)
                        assert 'tEst' in p, (tags, disabled, text, p)
                        checks += 1
                    if tags:
                        for control in ('`da0 ', '`da1 '):
                            for text in ('ETI-Eloquence', 'Test-Eloquence'):
                                p = phonemes(e, control + text)
                                assert ('kIr' if text.startswith('ETI') else 'tEst') in p, (control, text, p)
                                checks += 1
        finally:
            e.close()
            plain.close()
        # Text aliases must not be substituted inside an explicit phoneme
        # entry when processing the same input for speech and phonemes.
        (root / 'DEUmain.dic').write_bytes(b'tEst\tFenster\n')
        e, plain = engine(args.library), engine(args.library)
        try:
            e.set_dictionary_path(root)
            spr = '`[.1tEst]'
            assert phonemes(e, spr) == phonemes(plain, spr)
            assert render(e, [spr]) == render(plain, [spr])
            checks += 2
        finally:
            e.close()
            plain.close()
        # Main text aliases take priority over the full Root entry.
        (root / 'DEUmain.dic').write_bytes(b'ETI-Eloquence\tFenster\n')
        e = engine(args.library)
        try:
            e.set_dictionary_path(root)
            p = phonemes(e, 'ETI-Eloquence')
            assert p == phonemes(e, 'Fenster') and 'kIr' not in p, p
            checks += 1
        finally:
            e.close()
    print(f'German rc19: {checks} whole/part compound, abbreviation, tag, quote and index checks')


if __name__ == '__main__':
    main()
