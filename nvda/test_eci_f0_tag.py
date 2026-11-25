"""Persistent F0 tag through real ECI and the NVDA language transport."""
import argparse
from pathlib import Path
import threading
from _reloq_eci_direct import (DirectEciEngine, EciText, EciLanguage, EciIndex,
                               PROCESS_TAGS, PITCH_GLIDE_MS, SAMPLE_RATE)
from _reloq_languages import LanguageEngine
from test_eci_german_clock_native import render


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    lib = parser.parse_args().library.resolve()
    count = 0
    for dialect, phrase in ((0x40000, '`vf0 `vbhz125 `[.1ma] `vbhz185 `[.1ma] `vbhz125 `[.1ma]'),
                            (0x10000, '`vf0 `vbhz125 `[.1ma] `vbhz185 `[.1ma] `vbhz125 `[.1ma]'),
                            (0x120000, '`vf0 `vbhz125 γεια `vbhz185 σου.')):
        eng = DirectEciEngine(lib, dialect=dialect, encoding='utf8', native_voice=True)
        try:
            eng.set_parameter(PROCESS_TAGS, 1)
            eng.set_parameter(SAMPLE_RATE, 3)
            eng.set_parameter(PITCH_GLIDE_MS, 80)
            reference = render(eng, [phrase])
            eng.set_parameter(PITCH_GLIDE_MS, 0)
            tagged = render(eng, ['`vfg80 ', phrase])
            assert tagged == reference, (hex(dialect), 'leading tag must match configured glide')
            assert eng.get_parameter(PITCH_GLIDE_MS) == 80
            assert render(eng, [phrase]) == reference, (hex(dialect), 'glide not persistent')
            zero = render(eng, ['`vfg0 ', phrase])
            assert eng.get_parameter(PITCH_GLIDE_MS) == 0
            assert zero != reference and len(zero[0]) == len(reference[0]), (hex(dialect), 'disable')
            assert render(eng, [phrase]) == zero, (hex(dialect), 'zero not persistent')
            eng.set_parameter(PITCH_GLIDE_MS, 0)
            split = render(eng, ['`vf', 'g80 ', phrase])
            assert split == reference and eng.get_parameter(PITCH_GLIDE_MS) == 80
            for invalid in ('`vfg501 ', '`vfg-1 ', '`vfg999999999 '):
                render(eng, [invalid + 'Test.'])
                assert eng.get_parameter(PITCH_GLIDE_MS) == 80, invalid
            eng.set_parameter(PROCESS_TAGS, 0)
            render(eng, ['`vfg0 Test.'])
            assert eng.get_parameter(PITCH_GLIDE_MS) == 80, 'disabled tags changed state'
            eng.set_parameter(PROCESS_TAGS, 1)
            assert render(eng, ['`vfg0'])[0] == b'', 'tag alone must be silent'
            assert eng.get_parameter(PITCH_GLIDE_MS) == 0, 'tag-only command did not persist'
            count += 9
        finally:
            eng.close()
    eng = DirectEciEngine(lib, dialect=0x40000, encoding='utf8', native_voice=True)
    try:
        eng.set_parameter(PROCESS_TAGS, 1)
        phrase = '`vf0 `vbhz125 `[.1ma] `vfg80 `vbhz185 `[.1ma] `vbhz125 `[.1ma]'
        expected = '`vfg80 `vf0 `vbhz125 `[.1ma] `vbhz185 `[.1ma] `vbhz125 `[.1ma]'
        eng.set_parameter(PITCH_GLIDE_MS, 0)
        mid = render(eng, [phrase])
        eng.set_parameter(PITCH_GLIDE_MS, 0)
        early = render(eng, [expected])
        assert mid == early, 'enabling immediately before first pitch change should glide'
        assert eng.get_parameter(PITCH_GLIDE_MS) == 80
        prefix = '`vf0 `vbhz125 `[.1ma] `vbhz185 `[.1ma] '
        suffix = '`vbhz125 `[.1ma]'
        eng.set_parameter(PITCH_GLIDE_MS, 80)
        all_smooth = render(eng, [prefix, EciIndex(7), suffix])
        eng.set_parameter(PITCH_GLIDE_MS, 80)
        switched = render(eng, [prefix, EciIndex(7), '`vfg0 ', suffix])
        assert switched[1] == all_smooth[1] and len(switched[0]) == len(all_smooth[0])
        boundary = switched[1][0][1]
        assert switched[0][:boundary] == all_smooth[0][:boundary]
        assert switched[0][boundary:] != all_smooth[0][boundary:]
        assert eng.get_parameter(PITCH_GLIDE_MS) == 0
        eng.set_parameter(PITCH_GLIDE_MS, 0)
        implicit = render(eng, ['`vfg40 Hallo. `l1.0 `vf0 `vbhz125 Hello `vbhz185 world.'])
        eng.set_parameter(PITCH_GLIDE_MS, 0)
        explicit = render(eng, ['`vfg40 Hallo. `l1.0 `vfg40 `vf0 `vbhz125 Hello `vbhz185 world.'])
        assert implicit == explicit and eng.get_parameter(PITCH_GLIDE_MS) == 40
        assert render(eng, ['`vfg0'])[0] == b'' and eng.get_parameter(PITCH_GLIDE_MS) == 0
        count += 7
    finally:
        eng.close()
    for dialect, text in ((0x60000, '你好 `vbhz170 世界。'),
                          (0x80000, '今日は `vbhz170 世界。'),
                          (0xa0000, '안녕하세요 `vbhz170 세상입니다.'),
                          (0x60000, '这是 Windows `vbhz170 系统。')):
        eng = DirectEciEngine(lib, dialect=dialect, encoding='utf8', native_voice=True)
        try:
            eng.set_parameter(PROCESS_TAGS, 1)
            eng.set_parameter(SAMPLE_RATE, 3)
            eng.set_parameter(PITCH_GLIDE_MS, 80)
            reference = render(eng, [text])
            eng.set_parameter(PITCH_GLIDE_MS, 0)
            tagged = render(eng, ['`vfg80 ', text])
            assert tagged == reference, (hex(dialect), text, 'leading tag')
            assert eng.get_parameter(PITCH_GLIDE_MS) == 80
            count += 2
        finally:
            eng.close()
    for dialect, first, second in ((0x60000, '你好 ', '世界。'),
                                   (0x80000, '今日は ', '世界。'),
                                   (0xa0000, '안녕하세요 ', '세상입니다.')):
        eng = DirectEciEngine(lib, dialect=dialect, encoding='utf8', native_voice=True)
        try:
            eng.set_parameter(PROCESS_TAGS, 1)
            eng.set_parameter(SAMPLE_RATE, 3)
            prefix = '`vf0 `vbhz125 ' + first
            suffix = '`vbhz170 ' + second
            eng.set_parameter(PITCH_GLIDE_MS, 80)
            reference = render(eng, [prefix, suffix])
            eng.set_parameter(PITCH_GLIDE_MS, 0)
            assert render(eng, [prefix, '`vfg80 ', suffix]) == reference, (hex(dialect), 'mid enable')
            eng.set_parameter(PITCH_GLIDE_MS, 0)
            direct = render(eng, [prefix, suffix])
            eng.set_parameter(PITCH_GLIDE_MS, 80)
            assert render(eng, [prefix, '`vfg0 ', suffix]) == direct, (hex(dialect), 'mid disable')
            count += 2
        finally:
            eng.close()
    multi = LanguageEngine(lib)
    try:
        multi.set_parameter(PROCESS_TAGS, 1)
        out = bytearray()
        vfg = lambda value: EciText(f'`vfg{value}', tag_control=f'`vfg{value}')
        assert multi.speak([vfg(40), EciText(' Hallo Welt.')], out.extend, lambda _: None)
        assert multi._parameters[PITCH_GLIDE_MS] == 40
        out.clear()
        assert multi.speak([EciLanguage(0x10000, '1'), 'Hello world.'], out.extend, lambda _: None)
        assert multi._engines[0x10000].get_parameter(PITCH_GLIDE_MS) == 40
        out.clear()
        assert multi.speak([EciLanguage(0x10000, '1'), vfg(0), 'Hello world.'], out.extend, lambda _: None)
        assert multi._parameters[PITCH_GLIDE_MS] == 0
        assert multi._engines[0x40000].get_parameter(PITCH_GLIDE_MS) == 0
        count += 4
    finally:
        multi.close()
    # Stopping speech must preserve a tag already reached, but must not
    # apply a later tag merely because it was queued in the same sentence.
    def cancelled_state(text):
        first_audio, release = threading.Event(), threading.Event()
        shared = {}
        def worker():
            e = DirectEciEngine(lib, dialect=0x40000, encoding='utf8', native_voice=True)
            shared['engine'] = e
            try:
                e.set_parameter(PROCESS_TAGS, 1)
                def on_pcm(_):
                    first_audio.set()
                    assert release.wait(5), 'cancellation did not arrive'
                shared['success'] = e.speak([text], on_pcm, lambda _: None)
                shared['glide'] = e.get_parameter(PITCH_GLIDE_MS)
            except BaseException as error:
                shared['error'] = error
                first_audio.set()
            finally:
                e.close()
        thread = threading.Thread(target=worker)
        thread.start()
        assert first_audio.wait(10), 'synthesis did not start'
        if 'error' not in shared:
            shared['engine'].cancel()
        release.set()
        thread.join(10)
        assert not thread.is_alive(), 'cancellation hung'
        if 'error' in shared:
            raise shared['error']
        assert shared['success'] is False
        return shared['glide']
    assert cancelled_state('`vfg40 Die ersten Worte werden gesprochen. Noch ein Satz.') == 40
    assert cancelled_state('Die ersten Worte werden gesprochen. `vfg40 Noch ein Satz.') == 0
    count += 2
    print(f'F0 tag: {count} persisted, disabled, positioned, multilingual and API comparisons passed')

if __name__ == '__main__':
    main()
