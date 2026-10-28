"""Pause policy is applied to generated boundaries, before synthesis."""
from pathlib import Path
from _reloq_eci_direct import (
    DirectEciEngine, EciIndex, PROCESS_TAGS, PAUSE_MODE, NATIVE_SPEED,
    NATIVE_PITCH, NATIVE_INFLECTION, NATIVE_VOLUME,
)
ROOT = Path(__file__).resolve().parents[1]


def main():
    import sys
    import tempfile
    sys.path.insert(0, str(ROOT / 'addon'))
    from build_synth import merged_german_lexicon
    with tempfile.TemporaryDirectory() as folder:
        lex = Path(folder) / 'german.rlex'
        lex.write_bytes(merged_german_lexicon())
        engine = DirectEciEngine(ROOT / 'eci/libreloq_eci.so',
                                experimental_generic=True, lexicon_path=lex)
        try:
            for parameter, value in [(NATIVE_SPEED, 50), (NATIVE_PITCH, 65),
                                     (NATIVE_INFLECTION, 30), (NATIVE_VOLUME, 100)]:
                engine.set_parameter(parameter, value)

            def render(parts, mode):
                engine.set_parameter(PAUSE_MODE, mode)
                pcm, indices = [], []
                assert engine.speak(parts, pcm.append, indices.append), (parts, mode)
                return b''.join(pcm), indices

            count = 0
            for text in ('Haus', 'Haus.', 'Mann.', 'Baum.', 'Neu?',
                         'Haus, neu.', 'Haus; neu.', 'Haus. Neu.',
                         'Der Mann spricht, dann geht er nach Hause.'):
                original, _ = render([text], 0)
                final, _ = render([text], 1)
                always, _ = render([text], 2)
                assert 0 < len(always) <= len(final) < len(original), text
                # Mode 1 changes only the quiet tail: all emitted samples,
                # including the complete last consonant, remain unchanged.
                assert original.startswith(final), text
                for mode, expected in enumerate((original, final, always)):
                    for cut in (1, len(text) // 2, len(text)):
                        pcm, indices = render([text[:cut], EciIndex(7),
                                               text[cut:], EciIndex(9)], mode)
                        assert pcm == expected, (text, mode, cut)
                        assert indices == [7, 9]
                        count += 1
            assert len(render(['Haus, neu.'], 2)[0]) < len(render(['Haus, neu.'], 1)[0])
            assert render(['Haus neu.'], 1)[0] == render(['Haus neu.'], 2)[0]

            engine.set_parameter(PROCESS_TAGS, 1)
            for mode in range(3):
                assert render(['Haus`p0'], mode)[0] == render(['Haus'], mode)[0]
                p1 = render(['Haus`p1'], mode)[0]
                assert p1 == render(['Haus`p2'], mode)[0] == render(['Haus`p3'], mode)[0]
                assert len(render(['Haus`p4'], mode)[0]) - len(p1) == 11 * 2
                assert len(render(['Haus`p250'], mode)[0]) - len(p1) == 247 * 11 * 2
            # Explicit internal and final breaks are independent of policy.
            for text in ('Haus`p250', 'Haus`p250 neu`p100', '`p250 Haus`p100'):
                expected = render([text], 0)[0]
                assert render([text], 1)[0] == expected
                assert render([text], 2)[0] == expected
            for value in (-1, 3, True):
                try:
                    engine.set_parameter(PAUSE_MODE, value)
                except ValueError:
                    pass
                else:
                    raise AssertionError('invalid pause mode accepted')
            print(f'ECI pause modes: {count} fragment/index cases; original tails, internal boundaries and explicit pauses passed')
        finally:
            engine.close()


if __name__ == '__main__':
    main()
