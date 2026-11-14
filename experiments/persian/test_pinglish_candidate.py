"""Compare the partial Pinglish rules with a recorded eSpeak corpus."""
from __future__ import annotations

import ctypes
import json
from pathlib import Path
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent
ENGLISH, PINGLISH = 0, 1
EN_US, CANDIDATE, NEEDS_CONTEXT, INVALID, OVERFLOW = range(5)


def main() -> None:
    with tempfile.TemporaryDirectory(prefix='reloq-fa-test-') as tmp:
        binary = Path(tmp) / 'candidate.so'
        subprocess.run(['cc', '-std=c11', '-Wall', '-Wextra', '-Werror',
                        '-fPIC', '-shared', str(HERE / 'pinglish_candidate.c'),
                        '-o', str(binary)], check=True)
        f = ctypes.CDLL(str(binary)).fa_latin_candidate
        f.argtypes = [ctypes.c_int, ctypes.c_char_p, ctypes.c_void_p, ctypes.c_size_t]
        f.restype = ctypes.c_int

        def run(mode: int, word: str, capacity: int = 256) -> tuple[int, str]:
            buf = ctypes.create_string_buffer(capacity)
            status = f(mode, word.encode('utf8'), buf, capacity)
            return status, buf.value.decode('ascii')

        corpus = [json.loads(line) for line in (HERE / 'reference_espeak_1_51.jsonl').read_text(encoding='utf8').splitlines()]
        samples = [r for r in corpus if r['voice'] == 'fa-latn']
        compared = 0
        for row in samples:
            text = row['text']
            assert run(ENGLISH, text) == (EN_US, ''), text
            status, candidate = run(PINGLISH, text)
            if '(en)' in row['phonemes']:
                assert status == NEEDS_CONTEXT, (text, status, row['phonemes'])
                continue
            # This stage deliberately excludes stress and duration. Compare
            # only the phone sequence, keeping multi-character phone names.
            unstressed = row['phonemes'].replace("'", '').replace(',', '')
            assert (status, candidate) == (CANDIDATE, unstressed), (text, candidate, unstressed)
            compared += 1
        assert compared == 19, compared
        assert run(PINGLISH, 'khane') == (CANDIDATE, 'xane')
        assert run(PINGLISH, 'GHaza') == (CANDIDATE, 'q1aza')
        assert run(PINGLISH, 'cheese') == (CANDIDATE, 'tSise')
        assert run(PINGLISH, 'city') == (CANDIDATE, 'sitj')
        for word in ('k', 'father', 'photo', 'zooo', 'a\'a', 'سلام', 'khane!'):
            assert run(PINGLISH, word) == (NEEDS_CONTEXT, ''), word
        assert run(PINGLISH, '') == (INVALID, '')
        assert run(PINGLISH, 'khane', 4) == (OVERFLOW, '')
        assert run(PINGLISH, 'khane', 5) == (CANDIDATE, 'xane')
        assert run(12, 'khane') == (INVALID, '')
        print(f'Compared {compared} Pinglish phone sequences with eSpeak NG 1.51; '
              f'{len(samples)} normal-mode words route to US English.')
        print('Stress, dictionary exceptions and conversion to ReLoq phones remain unimplemented.')


if __name__ == '__main__':
    main()
