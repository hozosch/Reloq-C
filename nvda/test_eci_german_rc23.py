"""German connected number onsets and stem repairs through the ECI API."""
import argparse
from pathlib import Path
import tempfile

from _reloq_eci_direct import DirectEciEngine, EciIndex, GENDER, PROCESS_TAGS, SAMPLE_RATE, SAMPLE_RATES
from test_eci_german_clock_native import phonemes, render
from test_eci_german_overlay_case import lexicon


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--baseline', type=Path)
    args = parser.parse_args()
    engine = DirectEciEngine(args.library.resolve(), dialect=0x40000,
                             encoding='utf8', experimental_generic=True)
    checks = 0
    baseline = DirectEciEngine(args.baseline.resolve(), dialect=0x40000,
        encoding='utf8', experimental_generic=True) if args.baseline else None
    try:
        engine.set_parameter(PROCESS_TAGS, 1)
        if baseline:
            baseline.set_parameter(PROCESS_TAGS, 1)
        for word in ('samt', 'Samt'):
            assert '[.1zAmt]' in phonemes(engine, word), word
            assert render(engine, [word]) == render(engine, ['`[.1zAmt]']), word
            checks += 2
        assert '[.1sAmt]' in phonemes(engine, '`[.1sAmt]')
        assert render(engine, ['samt']) != render(engine, ['`[.1sAmt]'])
        checks += 2
        for word in ('veröffentlicht', 'veröffentlichte', 'veröffentlichtes'):
            p = phonemes(engine, word)
            assert 'f@nt' in p and 'fEnt' not in p, (word, p)
            checks += 1
        for word in ('veröffentlichen', 'Veröffentlichung', 'öffentlich'):
            assert 'f@nt' in phonemes(engine, word), word
            checks += 1
        for gender in (0, 1):
            engine.set_parameter(GENDER, gender)
            if baseline:
                baseline.set_parameter(GENDER, gender)
            for rate in SAMPLE_RATES:
                engine.set_parameter(SAMPLE_RATE, rate)
                if baseline:
                    baseline.set_parameter(SAMPLE_RATE, rate)
                    for control in ('vierund', '44', '22', '23', 'veröffentlichen', 'samtig'):
                        assert render(engine, [control]) == render(baseline, [control]), (gender, rate, control)
                        checks += 1
                for word in ('42', '43', 'zweiundvierzig', 'dreiundvierzig',
                             '81', '82', '83', '84', '85', '86', '87', '88', '89',
                             'zweiundachtzig'):
                    p = phonemes(engine, word)
                    assert render(engine, [word, EciIndex(9)]) == render(
                        engine, [p, EciIndex(9)]), (gender, rate, word, p)
                    checks += 1
        # A user's explicit pronunciation must still take precedence.
        with tempfile.TemporaryDirectory() as temporary:
            engine.set_parameter(GENDER, 0)
            engine.set_parameter(SAMPLE_RATE, 1)
            path = Path(temporary) / 'custom.rlex'
            lexicon(path, (('samt', '`[.1sAmt]'),
                           ('veröffentlicht', "`[.0fEr.1'OE'.0fEnt.0lIXt]")))
            custom = DirectEciEngine(args.library.resolve(), dialect=0x40000,
                encoding='utf8', experimental_generic=True, lexicon_path=path)
            try:
                custom.set_parameter(PROCESS_TAGS, 1)
                assert '[.1sAmt]' in phonemes(custom, 'samt')
                assert render(custom, ['samt']) != render(engine, ['samt'])
                assert 'fEnt' in phonemes(custom, 'veröffentlicht')
                checks += 3
            finally:
                custom.close()
            (Path(temporary) / 'DEURoot.dic').write_bytes(b'samt\t`[.1sAmt]\n')
            root = DirectEciEngine(args.library.resolve(), dialect=0x40000,
                encoding='utf8', experimental_generic=True)
            try:
                root.set_dictionary_path(temporary)
                root.set_parameter(PROCESS_TAGS, 1)
                assert '[.1sAmt]' in phonemes(root, 'samt')
                checks += 1
            finally:
                root.close()
    finally:
        engine.close()
        if baseline:
            baseline.close()
    print(f'German rc23: {checks} stem, numeric, phonetic and dictionary checks')


if __name__ == '__main__':
    main()
