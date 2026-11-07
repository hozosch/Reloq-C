"""Exercise cached standard ECI dictionaries, including the issue 17 timings."""
import argparse
import ctypes as c
import os
from pathlib import Path
import statistics
import tempfile
import time


ROOT = Path(__file__).resolve().parents[1]
ENGLISH, GERMAN = 0x10000, 0x40000


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    args = parser.parse_args()
    lib = (c.WinDLL if os.name == 'nt' else c.CDLL)(str(args.library.resolve()))
    H, I = c.c_void_p, c.c_int
    spec = {
        'eciNewEx': (H, (I,)), 'eciDelete': (H, (H,)),
        'eciSetParam': (I, (H, I, I)), 'eciGetParam': (I, (H, I)),
        'eciReset': (I, (H,)), 'eciNewDict': (H, (H,)),
        'eciGetDict': (H, (H,)), 'eciSetDict': (I, (H, H)),
        'eciDeleteDict': (H, (H, H)), 'eciLoadDict': (I, (H, H, I, H)),
        'eciUpdateDict': (I, (H, H, I, H, H)),
        'eciSetDictionaryPath': (I, (H, H)),
        'eciClearInput': (I, (H,)), 'eciAddText': (I, (H, H)), 'eciStop': (I, (H,)),
        'eciSynthesize': (I, (H,)), 'eciSynchronize': (I, (H,)),
        'eciSetOutputBuffer': (I, (H, I, c.POINTER(c.c_short))),
    }
    for name, (result, params) in spec.items():
        function = getattr(lib, name)
        function.restype, function.argtypes = result, params
    callback_type = (c.WINFUNCTYPE if os.name == 'nt' else c.CFUNCTYPE)(I, H, I, I, H)
    lib.eciRegisterCallback.argtypes = (H, callback_type, H)
    lib.eciRegisterCallback.restype = None

    h = lib.eciNewEx(ENGLISH)
    assert h
    output = (c.c_short * 4096)()
    audio = bytearray()
    abort = [False]

    @callback_type
    def callback(handle, message, count, context):
        if message == 0:
            audio.extend(c.string_at(output, 2 * count))
            if abort[0]:
                abort[0] = False
                assert lib.eciStop(handle)
        return 1

    lib.eciRegisterCallback(h, callback, None)
    assert lib.eciSetOutputBuffer(h, len(output), output)

    def speak(text):
        audio.clear()
        assert lib.eciClearInput(h) and lib.eciAddText(h, text + b'.')
        assert lib.eciSynthesize(h) and lib.eciSynchronize(h) and audio
        return bytes(audio)

    def load(dictionary, volume, path):
        path = str(path)
        encoded = path.encode('mbcs' if os.name == 'nt' else 'utf-8')
        assert lib.eciLoadDict(h, dictionary, volume, encoded) == 0

    def select(language, dictionary):
        assert lib.eciSetParam(h, 9, language) >= 0
        assert lib.eciGetDict(h) == dictionary

    try:
        dictionaries = {}
        for language, prefix in ((ENGLISH, 'ENU'), (GERMAN, 'DEU')):
            assert lib.eciSetParam(h, 9, language) >= 0
            dictionary = lib.eciNewDict(h)
            assert dictionary
            dictionaries[language] = dictionary
            for volume, suffix in enumerate(('main', 'Root', 'abbr')):
                load(dictionary, volume, ROOT / 'data' / 'community' / f'{prefix}{suffix}.dic')
            assert lib.eciSetDict(h, dictionary) == 0

        english, german = dictionaries[ENGLISH], dictionaries[GERMAN]
        select(ENGLISH, english)

        def elapsed(action):
            start = time.perf_counter()
            action()
            return time.perf_counter() - start

        def same_dictionary():
            assert lib.eciSetDict(h, english) == 0 and lib.eciGetDict(h) == english

        def switch_pair():
            select(GERMAN, german)
            select(ENGLISH, english)

        # A median tolerates occasional scheduling interruptions. The old
        # implementation took over 100 ms per selection on the same data.
        same = statistics.median(elapsed(same_dictionary) for _ in range(9))
        switches = statistics.median(elapsed(switch_pair) for _ in range(9))
        assert same < 0.05, f'reselecting a loaded dictionary took {same:.3f}s'
        assert switches < 0.05, f'switching two loaded dictionaries took {switches:.3f}s'

        # A screen reader often cancels speech before switching language or
        # reselecting its dictionary.  The prepared large tables must survive
        # the cancellation and the recovered voice must remain sample-exact.
        expected = speak(b'Unigram and Amir')
        for _ in range(8):
            audio.clear()
            assert lib.eciClearInput(h) and lib.eciAddText(h, b'This is a longer message. ' * 40)
            abort[0] = True
            lib.eciSynthesize(h)
            assert not abort[0] and audio, 'the interruption must occur after audio starts'
            assert lib.eciClearInput(h) and lib.eciGetDict(h) == english
            switch_pair();same_dictionary()
            assert speak(b'Unigram and Amir') == expected, 'cancellation or switching lost dictionary state'

        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'cache-test.dic'
            path.write_bytes(b'Cachezz\tcat\n')
            select(GERMAN, german)
            # Editing an inactive set must replace its cached tables.
            assert lib.eciUpdateDict(h, english, 0, b'Cachezz', b'house') == 0
            select(ENGLISH, english)
            assert speak(b'Cachezz') == speak(b'house')

            select(GERMAN, german)
            missing = str(Path(directory) / 'missing.dic').encode('mbcs' if os.name == 'nt' else 'utf-8')
            assert lib.eciLoadDict(h, english, 0, missing) == 6
            select(ENGLISH, english)
            assert speak(b'Cachezz') == speak(b'house')
            select(GERMAN, german)
            load(english, 0, path)
            select(ENGLISH, english)
            assert speak(b'Cachezz') == speak(b'cat')
            path.write_bytes(b'Cachezz\tdog\n')
            load(english, 0, path)
            assert speak(b'Cachezz') == speak(b'dog')

            # Switching between distinct sets for one language, including
            # clearing, deletion and reset, must keep ownership separate.
            replacement = lib.eciNewDict(h)
            assert replacement
            path.write_bytes(b'Cachezz\tcat\n')
            load(replacement, 0, path)
            assert lib.eciSetDict(h, replacement) == 0
            assert speak(b'Cachezz') == speak(b'cat')
            assert lib.eciSetDict(h, english) == 0
            assert speak(b'Cachezz') == speak(b'dog')
            assert lib.eciSetDict(h, None) == 0 and not lib.eciGetDict(h)
            assert lib.eciSetDict(h, replacement) == 0
            assert speak(b'Cachezz') == speak(b'cat')
            assert not lib.eciDeleteDict(h, english)
            select(GERMAN, german)
            assert lib.eciReset(h) and lib.eciGetParam(h, 9) == ENGLISH
            assert lib.eciGetDict(h) == replacement
            assert speak(b'Cachezz') == speak(b'cat')
            select(GERMAN, german)
            select(ENGLISH, replacement)
            assert speak(b'Cachezz') == speak(b'cat')

            # The directory extension can replace a handle and later restore
            # it without losing the cached, edited dictionary.
            assert lib.eciSetDictionaryPath(h, str(ROOT / 'data' / 'community').encode('utf-8'))
            assert not lib.eciGetDict(h)
            assert lib.eciSetDict(h, replacement) == 0
            assert speak(b'Cachezz') == speak(b'cat')
            assert not lib.eciDeleteDict(h, replacement)
            assert not lib.eciGetDict(h)
            select(GERMAN, german)
            assert not lib.eciDeleteDict(h, german)
    finally:
        lib.eciDelete(h)

    print(f'Dictionary cache: same-set median {same * 1000:.3f} ms; '
          f'language-switch pair median {switches * 1000:.3f} ms; '
          'reload, edit, reset, directory and deletion checks passed')


if __name__ == '__main__':
    main()
