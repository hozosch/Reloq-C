"""Index-only NVDA callbacks through the real ECI and transport queue.

NVDA uses indexes to trigger speech callbacks, including some sounds. This
test does not emulate a Remote connection or its separate tone/wave messages.
"""
import argparse
from pathlib import Path
from threading import Event

from _reloq_eci_direct import DirectEciEngine, EciIndex, EciText, PAUSE_MODE
from _reloq_eci_transport import EciTextTransport


class Player:
    def feed(self, pcm, onDone=None):
        if onDone:
            onDone()

    def idle(self): pass
    def stop(self): pass
    def pause(self, switch): pass
    def close(self): pass


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    library = parser.parse_args().library.resolve()
    for mode in (0, 1, 2):
        events, errors, done = [], [], Event()

        def finish(job):
            events.append(('done', job))
            done.set()

        transport = EciTextTransport(
            lambda: DirectEciEngine(library, native_voice=True, encoding='utf8'),
            Player(), lambda index: events.append(('index', index)), finish,
            errors.append,
        )
        try:
            transport.set_parameter(PAUSE_MODE, mode)
            job = transport.submit([EciIndex(23), EciIndex(24)])
            assert done.wait(5) and not errors, (mode, events, errors)
            assert events == [('index', 23), ('index', 24), ('done', job)], (mode, events)
            events.clear();done.clear()
            job = transport.submit([EciText('Hallo.', False), EciIndex(25)])
            assert done.wait(5) and not errors, (mode, events, errors)
            assert events == [('index', 25), ('done', job)], (mode, events)
        finally:
            assert transport.shutdown()
    print('NVDA callback indexes: index-only and speech callbacks passed in all pause modes')


if __name__ == '__main__':
    main()
