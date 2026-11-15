"""Check pause shortening and punctuation/stop protection at language switches."""
from __future__ import annotations

import numpy as np

from mixed_language_probe import last_phone_is_stop, shorten_boundary, splice


def main() -> None:
    count = 0
    for rate in (11025, 16000, 48000):
        prefix = np.full(rate // 10, 120, dtype='<i2').tobytes()
        silence = bytes((rate * 2 * 390) // 1000 // 2 * 2)
        audio = prefix + silence
        normal = shorten_boundary(audio, rate, 'NVDA')
        stop = shorten_boundary(audio, rate, 'stop', final_stop=True)
        assert len(normal) == len(prefix) + 2 * round(rate * .025)
        assert len(stop) == len(prefix) + 2 * round(rate * .070)
        assert shorten_boundary(audio, rate, 'NVDA,') == audio
        assert shorten_boundary(audio, rate, 'NVDA؟') == audio
        assert shorten_boundary(bytes(rate // 2 * 2), rate, 'silent') == bytes(rate // 2 * 2)
        assert splice([(audio, 'NVDA', False), (audio, 'word', False)], rate) == normal + audio
        count += 6
    assert last_phone_is_stop('`[.1stap]')
    assert not last_phone_is_stop('`[.1ka.0RAz]')
    assert not last_phone_is_stop('')
    print(f'Passed {count + 3} pause, punctuation and final-stop boundary checks.')


if __name__ == '__main__':
    main()
