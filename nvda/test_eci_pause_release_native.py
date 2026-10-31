"""Short German pauses retain the original release and skip only planned quiet frames."""
from pathlib import Path
import argparse
import array
import json
from _reloq_eci_direct import (DirectEciEngine, EciIndex, SAMPLE_RATE, PAUSE_MODE,
                              NATIVE_SPEED, GENDER, HEAD_SIZE, BREATHINESS, PROCESS_TAGS)

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, default=ROOT / 'eci/libreloq_eci.so')
    parser.add_argument('--report', type=Path)
    args = parser.parse_args()
    engine = DirectEciEngine(args.library.resolve(), encoding='utf8', experimental_generic=True)
    records = []
    try:
        engine.set_parameter(NATIVE_SPEED, 98)
        for gender, head, breath in ((0, 50, 0), (1, 22, 60), (1, 29, 0)):
            for key, value in ((GENDER, gender), (HEAD_SIZE, head), (BREATHINESS, breath)):
                engine.set_parameter(key, value)
            for rate in range(7):
                engine.set_parameter(SAMPLE_RATE, rate)
                for first, more in (('Mann.', ' Danach geht es weiter.'),
                                    ('Hallo,', ' danach geht es weiter.'),
                                    ('Kamm.', ''), ('Tag.', ' Ja.'), ('Eins,', ' zwei.')):
                    streams, marks = [], []
                    for mode in range(3):
                        engine.set_parameter(PAUSE_MODE, mode)
                        pcm = bytearray()
                        positions = []
                        assert engine.speak([first, EciIndex(7), more, EciIndex(9)], pcm.extend,
                                            lambda n: positions.append((n, len(pcm) // 2)))
                        assert [n for n, _ in positions] == [7, 9]
                        assert positions[-1][1] == len(pcm) // 2
                        streams.append(array.array('h', pcm))
                        marks.append(positions)
                    original, final, always = streams
                    # Final-only mode is an exact original prefix. Internal
                    # pauses and the preceding release cannot be re-planned.
                    assert original[:len(final)] == final, (first, rate, head, 'final release')
                    if more:
                        assert marks[0][0] == marks[1][0]
                    # Before the first omitted quiet body, every sample must
                    # still match. This detects a future formant knot moving
                    # backwards into the release when its clock is shortened.
                    boundary = marks[2][0][1]
                    assert original[:boundary] == always[:boundary], (first, rate, head, 'internal release')
                    if not more:
                        assert final == always
                    assert 0 < len(always) <= len(final) < len(original)
                    records.append({'text': first + more, 'rate': rate, 'gender': gender,
                                    'head': head, 'breath': breath,
                                    'samples': list(map(len, streams)), 'indices': marks})
        engine.set_parameter(PROCESS_TAGS, 1)
        explicit_cases = 0
        for rate in range(7):
            engine.set_parameter(SAMPLE_RATE, rate)
            for text in ('Haus`p250', 'Haus`p250 neu`p100', '`p250 Haus`p100',
                         'Haus`p250 `vh22 neu`p100'):
                streams = []
                for mode in range(3):
                    engine.set_parameter(PAUSE_MODE, mode)
                    pcm = bytearray()
                    assert engine.speak([text], pcm.extend, lambda n: None)
                    streams.append(pcm)
                assert streams[0] == streams[1] == streams[2], (rate, text)
                explicit_cases += 1
    finally:
        engine.close()
    report = {'cases': len(records), 'modes': 3, 'exact_original_release_and_compacted_indices': True,
              'explicit_pause_cases': explicit_cases,
              'no_audio_files': True, 'records': records}
    if args.report:
        args.report.write_text(json.dumps(report, ensure_ascii=False, indent=2) + '\n')
    print(f'German release: {len(records)} rate/model/text triplets retain original release curves; explicit durations unchanged')


if __name__ == '__main__':
    main()
