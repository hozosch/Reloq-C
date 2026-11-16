"""Check early mixed-sentence replay against complete uncached planning."""
from __future__ import annotations

import argparse
from pathlib import Path
import re
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import EciIndex, EciLanguage, EciPitch, GENDER, HEAD_SIZE, SAMPLE_RATE
from _reloq_persian_frames import FrameCapture, FrameStream
import direct_fa_sequence
from nvda_driver_probe import PERSIAN_PROBE, US_ENGLISH, make_language_engine


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for option in ('scope-probe', 'native-library', 'worker', 'espeak-library', 'data-parent'):
        parser.add_argument('--' + option, type=Path, required=option != 'worker')
    args = parser.parse_args()
    engine = make_language_engine(args.scope_probe, args.native_library, args.worker,
                                  args.espeak_library, args.data_parent)
    original_available, original_append = FrameStream.available, FrameStream.append
    original_capture = FrameCapture.capture
    counts = []
    def track(self, frames, first, end, marks=()):
        counts.append(end - first)
        return original_append(self, frames, first, end, marks)
    FrameStream.append = track
    def clear():
        # Cross-request plan, phone, template and uppercase caches are all
        # emptied. Initialised voice handles and rule tables remain loaded.
        for name in ('_persian_plan_cache', '_persian_phoneme_cache',
                     '_persian_ach_cache', '_uppercase_cache'):
            cache = getattr(engine, name, None)
            if cache is not None:
                cache.clear()
        for voice in engine._engines.values():
            cache = getattr(getattr(voice, '_frontend', None), '_resilient_word_cache', None)
            if cache is not None:
                cache.clear()
    def render(parts):
        pcm, indexes = bytearray(), []
        assert engine.speak(parts, pcm.extend, lambda value: indexes.append((value, len(pcm))))
        return bytes(pcm), indexes
    examples = (
        (EciIndex(1), 'NVDA ', EciIndex(2), 'خوب است.', EciIndex(3)),
        (EciIndex(4), 'NVDA ', EciIndex(5), 'سلام دنیا.', EciIndex(6)),
        (EciIndex(7), EciPitch(75), 'computer ', EciIndex(8), 'کتاب', EciIndex(9)),
        (EciIndex(10), 'weather ', EciIndex(11), 'خانه است.', EciIndex(12)),
        (EciIndex(13), 'system ', EciIndex(14), 'زود است.', EciIndex(15)),
        (EciIndex(16), 'NV', EciIndex(17), 'DA ', EciIndex(18), 'فایل', EciIndex(19)),
        (EciLanguage(US_ENGLISH.id, '1'), 'NVDA ', EciIndex(20),
         EciLanguage(PERSIAN_PROBE.id, '1'), 'سلام', EciIndex(21)),
        (EciIndex(22), 'سلام ', EciIndex(23), 'window ', EciIndex(24), 'خوب است.', EciIndex(25)),
        (EciIndex(26), 'من کتاب را دوست دارم ', EciIndex(27), 'NVDA خوب است.', EciIndex(28)),
        (EciIndex(33), 'NVDA خوب ', EciIndex(34), 'window سلام', EciIndex(35)),
        # A selected within-Persian U-to-A glide stays on the complete route.
        (EciIndex(36), 'او آمد و ', EciIndex(37), 'NVDA خوب است.', EciIndex(38)),
        # These words need reference analysis and an internal word anchor.
        (EciIndex(39), 'NVDA ', EciIndex(40), 'پژوهشگر ', EciIndex(41), 'سلام', EciIndex(42)),
        (EciIndex(43), 'سلام ', EciIndex(44), 'پژوهشگر ', EciIndex(45), 'window', EciIndex(46)),
    )
    compared = streamed = 0
    try:
        for rate, gender, head in ((1, 0, 50), (3, 0, 50), (6, 1, 22), (3, 1, 86)):
            for key, value in ((SAMPLE_RATE, rate), (GENDER, gender), (HEAD_SIZE, head)):
                engine.set_parameter(key, value)
            for parts in examples:
                clear()
                FrameStream.available = staticmethod(lambda _dll: False)
                full = render(parts)
                clear()
                FrameStream.available = staticmethod(original_available)
                before = len(counts)
                incremental = render(parts)
                assert incremental == full, (rate, gender, head, parts,
                                              len(incremental[0]), len(full[0]),
                                              incremental[1], full[1])
                compared += 1
                streamed += len(counts) > before
        assert streamed >= 12, (compared, streamed)

        # A reference fallback must still be rejected by the complete native
        # pilot before the English prefix can start playing.
        clear()
        pcm, rejected = bytearray(), []
        original_validator = direct_fa_sequence.native_sequence_accepted
        def reject_pilot(tagged, shown):
            if not rejected and len(re.findall(r'`\[[^]]+\]', tagged)) > 2:
                rejected.append(len(pcm))
                return False
            return original_validator(tagged, shown)
        direct_fa_sequence.native_sequence_accepted = reject_pilot
        try:
            assert engine.speak(('NVDA ', EciIndex(47), 'پژوهشگر ', EciIndex(48), 'سلام'),
                                pcm.extend, lambda _: None)
        finally:
            direct_fa_sequence.native_sequence_accepted = original_validator
        assert rejected == [0] and pcm, (rejected, len(pcm))

        # Deliberately fail the second context only after the English prefix
        # has started. The remaining English and Persian text and all NVDA
        # indexes must still be delivered once through the fallback route.
        clear()
        pcm, indexes, injected = bytearray(), [], []
        def fail_second(self, voice, parts, **kwargs):
            if (not kwargs.get('audio', True) and getattr(voice, 'key', (None,))[0] == PERSIAN_PROBE.id
                    and pcm and not injected):
                injected.append(True)
                raise RuntimeError('injected later-context failure')
            return original_capture(self, voice, parts, **kwargs)
        FrameCapture.capture = fail_second
        parts = (EciIndex(30), 'NVDA ', EciIndex(31), 'سلام دنیا.', EciIndex(32))
        assert engine.speak(parts, pcm.extend, lambda value: indexes.append((value, len(pcm))))
        assert injected and len(pcm) > 5000
        assert [value for value, _ in indexes] == [30, 31, 32], indexes
        assert indexes[0][1] == 0 and indexes[-1][1] == len(pcm)

        # Stop after the prefix and while planning the remaining language;
        # the handle must be released for the next utterance.
        clear()
        pcm, cancelled = bytearray(), []
        def stop_second(self, voice, parts, **kwargs):
            if (not kwargs.get('audio', True) and getattr(voice, 'key', (None,))[0] == PERSIAN_PROBE.id
                    and pcm and not cancelled):
                cancelled.append(True)
                engine.cancel()
            return original_capture(self, voice, parts, **kwargs)
        FrameCapture.capture = stop_second
        assert not engine.speak(('NVDA سلام',), pcm.extend, lambda _: None)
        assert cancelled and pcm
        FrameCapture.capture = original_capture
        assert render(('سلام.',))[0]
    finally:
        FrameStream.available = staticmethod(original_available)
        FrameStream.append = original_append
        FrameCapture.capture = original_capture
        engine.close()
    print(f'Passed {compared} uncached PCM/index comparisons ({streamed} streamed), failure recovery and cancellation.')


if __name__ == '__main__':
    main()
