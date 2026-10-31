"""Short German pauses must preserve every nonzero sample of the long mode."""
from pathlib import Path
import argparse
import array
import json
from _reloq_eci_direct import (DirectEciEngine, EciIndex, SAMPLE_RATE, PAUSE_MODE,
                              NATIVE_SPEED, GENDER, HEAD_SIZE, BREATHINESS)

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
                    # A late residual sample can follow a long exact-zero
                    # run inside the planned quiet body. Shortening that
                    # run changes positions, while retaining every audible
                    # sample on the original acoustic clock.
                    audible = [v for v in original if v]
                    for stream, positions in zip(streams[1:], marks[1:]):
                        assert [v for v in stream if v] == audible, (first, rate, head)
                        before = [v for v in original[:marks[0][0][1]] if v]
                        assert [v for v in stream[:positions[0][1]] if v] == before
                    assert 0 < len(always) <= len(final) < len(original)
                    records.append({'text': first + more, 'rate': rate, 'gender': gender,
                                    'head': head, 'breath': breath,
                                    'samples': list(map(len, streams)), 'indices': marks})
    finally:
        engine.close()
    report = {'cases': len(records), 'modes': 3, 'preserved_nonzero_pcm_and_source_boundaries': True,
              'no_audio_files': True, 'records': records}
    if args.report:
        args.report.write_text(json.dumps(report, ensure_ascii=False, indent=2) + '\n')
    print(f'German release: {len(records)} rate/model/text triplets preserve all nonzero PCM samples')


if __name__ == '__main__':
    main()
