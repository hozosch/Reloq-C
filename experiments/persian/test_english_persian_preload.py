"""Check that selecting US English prepares Persian without changing English."""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
import nvda_driver_probe as driver
from _reloq_eci_direct import EciIndex, EciLanguage
from _reloq_languages import LanguageEngine


def render(engine, parts):
    pcm, indices = bytearray(), []
    assert engine.speak(parts, pcm.extend, indices.append)
    return bytes(pcm), tuple(indices)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    library = parser.parse_args().library.resolve()
    missing = Path('/unavailable')
    prepared = driver.make_language_engine(library, library, None, missing, missing,
                                           language=driver.US_ENGLISH.id, native_sentences=True)
    baseline = LanguageEngine(library, language=driver.US_ENGLISH.id, languages=driver.PROFILES)
    try:
        persian = prepared._engines[driver.PERSIAN_PROBE.id]
        assert not persian._frontend._reference_available
        assert prepared._active is prepared._engines[driver.US_ENGLISH.id]
        if (os.cpu_count() or 1) > 1:
            assert prepared._persian_contour_worker._engine is not None
            if os.environ.get('RELOQ_FA_STREAM_PREFIX', '1') == '1':
                assert prepared._persian_capture_worker._engine is not None
        english = (EciIndex(1), 'Hello, this is a test.', EciIndex(2))
        assert render(prepared, english) == render(baseline, english)
        mixed = (EciIndex(3), 'Hello ', EciLanguage(driver.PERSIAN_PROBE.id, '1'),
                 'سلام دنیا', EciLanguage(driver.US_ENGLISH.id, '1'), ' world', EciIndex(4))
        audio, indices = render(prepared, mixed)
        assert audio and indices == (3, 4)
        assert prepared._engines[driver.PERSIAN_PROBE.id] is persian
        assert prepared._active is prepared._engines[driver.US_ENGLISH.id]
        prepared.select_voice(driver.PERSIAN_PROBE.id, '1')
        assert prepared._engines[driver.PERSIAN_PROBE.id] is persian
        assert prepared._engines[driver.US_ENGLISH.id] is not None
        audio, indices = render(prepared, (EciIndex(5), 'سلام دنیا', EciIndex(6)))
        assert audio and indices == (5, 6)
    finally:
        baseline.close()
        prepared.close()
    print('Passed English/Persian preload, English isolation and mixed-text indexes.')


if __name__ == '__main__':
    main()
