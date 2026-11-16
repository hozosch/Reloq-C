"""Exercise protected ECI tags and preservation of Persian word stress."""
from __future__ import annotations

import argparse
from pathlib import Path
import re
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import EciIndex, EciLanguage, EciText, PROCESS_TAGS
from _reloq_languages import US_ENGLISH
from direct_fa_sequence import project_resilient, without_generated_stress
from nvda_driver_probe import PERSIAN_PROBE, PINGLISH_PROBE, make_language_engine


def control(value):
    speed = value if value.startswith(('`vs', '`vwpm')) else None
    return EciText(value, speed_control=speed, tag_control=value)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('scope-probe', 'native-library', 'espeak-library', 'data-parent'):
        parser.add_argument('--' + name, type=Path, required=True)
    args = parser.parse_args()
    engine = make_language_engine(args.scope_probe, args.native_library, None,
                                  args.espeak_library, args.data_parent)

    def render(*parts):
        audio, indexes = bytearray(), []
        assert engine.speak(parts, audio.extend, lambda mark: indexes.append((mark, len(audio))))
        return bytes(audio), indexes

    try:
        engine.set_parameter(PROCESS_TAGS, 1)
        fa = engine._engines[PERSIAN_PROBE.id]
        source = ('سلام ', EciIndex(4), 'دنیا')
        for language, words in ((PERSIAN_PROBE.id, ('سلام ', 'دنیا')),
                                (PINGLISH_PROBE.id, ('salam ', 'donya'))):
            engine.select_voice(language, '1')
            current = engine._engines[language]
            normal, _ = render(*words)
            slow, marks = render(control('`vs30'), words[0], EciIndex(10), words[1])
            fast, _ = render(control('`vs180'), *words)
            assert slow and fast and len(slow) > len(fast) * 1.5, (language, len(slow), len(fast))
            assert [v for v, _ in marks] == [10] and 0 < marks[0][1] < len(slow)
            quiet, _ = render(control('`vv20'), *words)
            pitched, _ = render(control('`vbhz220'), *words)
            delayed, _ = render(words[0], control('`p250'), words[1])
            assert quiet != normal and pitched != normal
            assert len(delayed) > len(normal) + 3000, (language, len(delayed), len(normal))
            with_tag, _ = render(control('`[.1sAd]'))
            without_tag, _ = render(control('`[.0sAd]'))
            assert with_tag and without_tag and with_tag != without_tag
            prepared = project_resilient((words[0], control('`[.1sAd]'), words[1]),
                                         current._frontend, pinglish=current._pinglish)
            unmarked = without_generated_stress(prepared)
            assert not re.search(r'\.[12]', unmarked.tagged), unmarked.tagged
            assert any(isinstance(item, EciText) and item.tag_control == '`[.1sAd]' and
                       item.text == '`[.1sAd]' for item in unmarked.parts)
            assert render(words[0], control('`[.1sAd]'), EciIndex(11), words[1])[0]
            observed, previous_speak = [], current._speak_prepared
            def observe(job, *args, **kwargs):
                if job.source:
                    observed.append(job.tagged)
                return previous_speak(job, *args, **kwargs)
            current._speak_prepared = observe
            try:
                assert render(*words)[0]
            finally:
                current._speak_prepared = previous_speak
            assert any(re.search(r'\.[12]', value) for value in observed), (language, observed)

        engine.select_voice(PERSIAN_PROBE.id, '1')
        # A persistent pitch/speed control crosses an explicit or automatic
        # English language switch. A pause/phoneme tag is not replayed.
        mixed = (control('`vs35'), control('`vb80'), 'سلام ',
                 EciLanguage(US_ENGLISH.id, '1'), 'hello ',
                 EciLanguage(PERSIAN_PROBE.id, '1'), 'دنیا')
        mixed_fast = (control('`vs180'), control('`vb80'), 'سلام ',
                      EciLanguage(US_ENGLISH.id, '1'), 'hello ',
                      EciLanguage(PERSIAN_PROBE.id, '1'), 'دنیا')
        slow, _ = render(*mixed)
        fast, _ = render(*mixed_fast)
        assert len(slow) > len(fast) * 1.4, (len(slow), len(fast))
        # Disabling user tags must not disable the internal phoneme tags.
        engine.set_parameter(PROCESS_TAGS, 0)
        plain, _ = render(*source)
        literal, _ = render(EciText('`vs30'), *source)
        assert plain and literal and literal != plain
        assert fa._voice._dll.eciGetParam(fa._voice._handle, PROCESS_TAGS) == 1
    finally:
        engine.close()
    print('Passed Persian/Pinglish ECI controls, explicit phonemes and retained word stress.')


if __name__ == '__main__':
    main()
