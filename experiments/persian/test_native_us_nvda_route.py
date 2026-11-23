"""Compare the active native Persian/US route to the established NVDA planner."""
from __future__ import annotations
import argparse
from pathlib import Path
import sys
import threading
import time
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
import nvda_driver_probe as driver
from _reloq_eci_direct import EciIndex, EciText, EciPitch, GENDER, NATIVE_SPEED, SAMPLE_RATE
from _reloq_languages import ACOUSTIC_CLOCK_BITS


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    path = args.library.resolve()
    missing = Path('/unavailable')
    owners = [driver.make_language_engine(path, path, None, missing, missing, native_sentences=enabled)
              for enabled in (False, True)]
    native = driver.try_native_persian_sentence
    routes = []
    def observe(*parts, **kwargs):
        result = native(*parts, **kwargs)
        if result is not None:
            routes.append(result)
        return result
    def render(owner, parts):
        pcm, marks = bytearray(), []
        assert owner.speak(parts, pcm.extend, lambda value: marks.append((value, len(pcm))))
        return bytes(pcm), tuple(marks)
    checks = 0
    try:
        with patch.object(driver, 'try_native_persian_sentence', new=observe):
            for profile, texts in (
                    (driver.PERSIAN_PROBE, ('سلام', 'فوری', 'ظهور ناگهانی', 'کتاب را.', 'سلام دنیا؟', '۱۲')),
                    (driver.PINGLISH_PROBE, ('salaam', 'fori', 'salaam donyaa.'))):
                for rate in (1, 3, 6):
                    for gender in (0, 1):
                        for speed in (65, 82, 100):
                            for clock in (0, 1):
                                for owner in owners:
                                    owner.select_voice(profile.id, '1')
                                    for key, value in ((SAMPLE_RATE, rate), (GENDER, gender), (NATIVE_SPEED, speed)):
                                        owner.set_parameter(key, value)
                                    owner.set_acoustic_clocks(ACOUSTIC_CLOCK_BITS[profile.id] if clock else 0)
                                for text in texts:
                                    at = len(text) // 2
                                    parts = (EciIndex(1), text[:at], EciIndex(2), text[at:], EciIndex(3))
                                    before = len(routes)
                                    old = render(owners[0], parts)
                                    new = render(owners[1], parts)
                                    assert old == new, (profile.locale, rate, gender, speed, clock, text)
                                    assert len(routes) > before and routes[-1] is True, (profile.locale, text, 'native route was not used')
                                    checks += 1
            # The new route must not consume inputs whose prior handling is
            # still required, including host dictionary protections or inline
            # prosody. Mixed speech retains the matched language transitions.
            for profile, parts in (
                    (driver.PERSIAN_PROBE, (EciText('سلام دنیا', segmentation_guarded=True),)),
                    (driver.PERSIAN_PROBE, ('سلام ', EciPitch(65), 'دنیا')),
                    (driver.PERSIAN_PROBE, ('سلام دنیا NVDA',)),
                    (driver.PERSIAN_PROBE, ('برای راه‌اندازی رایانه باید این گزینه انتخاب شود.',))):
                for owner in owners:
                    owner.select_voice(profile.id, '1')
                before = len(routes)
                assert render(owners[0], parts) == render(owners[1], parts), parts
                assert len(routes) == before, (parts, 'unsupported input used native short route')
                checks += 1
            # The native short route owns its US pilot; it should not switch
            # the driver's active language to English for a Persian request.
            owner = owners[1]
            owner.select_voice(driver.PERSIAN_PROBE.id, '1')
            if getattr(owner._activate(owner._home)._voice._dll, 'eciPersianPrepareUsPilot', None):
                activate = owner._activate
                def assert_no_english(key):
                    assert key[0] != driver.US_ENGLISH.id, 'driver acquired a separate US pilot'
                    return activate(key)
                with patch.object(owner, '_activate', side_effect=assert_no_english):
                    assert render(owner, ('سلام دنیا',)) == render(owners[0], ('سلام دنیا',))
                checks += 1
            # Stop the owned transaction and verify the next request has no
            # stale stop, callback or queued samples.
            owner = owners[1]
            owner.select_voice(driver.PERSIAN_PROBE.id, '1')
            entered, ready = threading.Event(), threading.Event()
            def stop():
                ready.set()
                deadline = time.monotonic() + 3
                while time.monotonic() < deadline:
                    with owner._guard:
                        active = getattr(owner, '_native_persian_active', False)
                    if active:
                        entered.set(); owner.cancel(); return
                    time.sleep(.0002)
            thread = threading.Thread(target=stop, daemon=True)
            thread.start()
            assert ready.wait(2)
            discarded = bytearray()
            result = owner.speak(('پیکربندیِ پیکربندیِ پیکربندیِ',), discarded.extend, lambda _value: None)
            thread.join(5)
            assert entered.is_set() and not thread.is_alive() and not result
            assert render(owner, ('سلام دنیا',)) == render(owners[0], ('سلام دنیا',))
            checks += 1
    finally:
        for owner in owners:
            owner.close()
    print(f'Passed {checks} native NVDA Persian/US routes, clock changes, fallback and cancellation cases.')


if __name__ == '__main__':
    main()
