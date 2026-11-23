"""ECI pause commands touching Hangul must behave like separated commands."""
from pathlib import Path
import argparse

from _reloq_eci_direct import (
    DirectEciEngine, EciIndex, NATIVE_SPEED, PROCESS_TAGS, SAMPLE_RATE,
)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    checks = 0
    for encoding in ('utf8', 'cp949'):
        engine = DirectEciEngine(args.library.resolve(), dialect=0xa0000,
                                 encoding=encoding, native_voice=True)
        try:
            engine.set_parameter(PROCESS_TAGS, 1)
            for rate in (1, 3):
                engine.set_parameter(SAMPLE_RATE, rate)
                for speed in (50, 98):
                    engine.set_parameter(NATIVE_SPEED, speed)

                    def render(parts):
                        pcm, indices = bytearray(), []
                        assert engine.speak(parts, pcm.extend,
                                            lambda index: indices.append((index, len(pcm))))
                        return bytes(pcm), indices

                    for word in ('안녕', '테스트', '안녕하세요'):
                        for duration in (0, 1, 50, 250):
                            for following in ('', '. 세계'):
                                tag = f'`p{duration}{following}'
                                # Model Eloquence_64: the pause is a separate
                                # eciAddText fragment, with an index at each edge.
                                actual = render([word, EciIndex(11), tag, EciIndex(12)])
                                separated = render([word + ' ', EciIndex(11), tag, EciIndex(12)])
                                assert actual[0] and actual == separated, (
                                    encoding, rate, speed, word, tag,
                                    len(actual[0]), len(separated[0]), actual[1], separated[1])
                                assert [index for index, _ in actual[1]] == [11, 12]
                                assert actual[1][-1][1] == len(actual[0])
                                # Joining the source must not change the result.
                                joined = render([word + tag])
                                assert joined[0] == actual[0], (encoding, word, tag)
                                checks += 1
        finally:
            engine.close()
    print(f'Passed {checks} Korean ECI pause boundaries and source-index checks.')


if __name__ == '__main__':
    main()
