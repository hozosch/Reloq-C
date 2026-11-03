"""Repeated cancellation of the filename from issue #10; no audio device used."""
import argparse
from pathlib import Path
import threading
import time

from _reloq_eci_direct import DirectEciEngine, EciIndex
from _reloq_eci_transport import EciTextTransport
from _reloq_languages import LanguageEngine

ROOT = Path(__file__).resolve().parents[1]
FILENAME = ('20171111 - Can_A_Real_Meal_Easy_Bake_Oven_Cook_Raw_Meat_'
            'Macaroni_Cheese_-_Sloppy_Joe_Dinner.vtt  Extension: vtt, '
            'Size: 22.0 kB, Attributes: ---A--------, Modified: '
            '11/2/2026 6:20:58 PM')


class Player:
    def feed(self, data, onDone=None):
        if onDone:
            onDone()

    def idle(self): pass
    def stop(self): pass
    def pause(self, switch): pass
    def close(self): pass


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, default=ROOT / 'eci/libreloq_eci.so')
    args = parser.parse_args()
    for language in (0x10000, 0x40000):
        condition, completed = threading.Condition(), set()
        errors, indexes = [], []

        def done(number):
            with condition:
                completed.add(number)
                condition.notify_all()

        transport = EciTextTransport(
            lambda: LanguageEngine(args.library.resolve(), language=language,
                                   voice='1', engine_factory=DirectEciEngine),
            Player(), indexes.append, done, errors.append, complete_on_error=True)
        try:
            for i in range(160):
                transport.submit([FILENAME, EciIndex(i)])
                time.sleep((0.0, .001, .003, .008)[i % 4])
                transport.cancel()
                if i % 20 == 19:
                    number = transport.submit(['Next filename.', EciIndex(1000 + i)])
                    with condition:
                        assert condition.wait_for(lambda: number in completed, 30), (language, i)
                    assert 1000 + i in indexes, (language, i, indexes[-5:])
                    assert not errors, (language, i, errors)
            number = transport.submit([FILENAME, EciIndex(999)])
            with condition:
                assert condition.wait_for(lambda: number in completed, 30), (language, errors)
            assert not errors and 999 in indexes, (language, errors, indexes[-5:])
        finally:
            assert transport.shutdown()
        print(f'{language:#x}: 160 rapid filename cancellations; every following job completed')
    print('LanguageEngine and EciTextTransport used; a real NVDA/WavePlayer session is not covered.')


if __name__ == '__main__':
    main()
