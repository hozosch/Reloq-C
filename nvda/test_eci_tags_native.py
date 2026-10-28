"""Inline controls use one utterance, including fragmented NVDA input."""
from pathlib import Path
from _reloq_eci_direct import (
    DirectEciEngine, EciIndex, EciPitch, PROCESS_TAGS, NATIVE_SPEED,
    NATIVE_PITCH, NATIVE_INFLECTION, NATIVE_VOLUME,
)

ROOT = Path(__file__).resolve().parents[1]


def main():
    from sys import path
    path.insert(0, str(ROOT / "addon"))
    from build_synth import merged_german_lexicon
    import tempfile
    with tempfile.TemporaryDirectory() as folder:
        lex = Path(folder) / "german.rlex"
        lex.write_bytes(merged_german_lexicon())
        engine = DirectEciEngine(ROOT / "eci/libreloq_eci.so",
                                experimental_generic=True, lexicon_path=lex)
        try:
            for parameter, value in [(NATIVE_SPEED, 50), (NATIVE_PITCH, 65),
                                     (NATIVE_INFLECTION, 30), (NATIVE_VOLUME, 100)]:
                engine.set_parameter(parameter, value)

            def render(parts):
                pcm, indices = [], []
                assert engine.speak(parts, pcm.append, indices.append), parts
                return b"".join(pcm), indices

            plain, _ = render(["Hallo Welt."])
            engine.set_parameter(PROCESS_TAGS, 1)
            assert render(["Hallo Welt."])[0] == plain
            assert render(["Hallo `p0 Welt."])[0] == plain
            pause50 = render(["Hallo `p50 Welt."])[0]
            pause250 = render(["Hallo `p250 Welt."])[0]
            assert len(pause250) - len(pause50) == 200 * 11 * 2
            leading = render(["`p250 Hallo Welt."])[0]
            assert not any(leading[:250 * 11 * 2])
            assert any(leading[251 * 11 * 2:])
            assert len(render(["Hallo `vs80 Welt."])[0]) < len(plain)
            assert render(["`vv50 Hallo Welt."])[0] != plain
            assert render(["`vb80 Hallo Welt."])[0] != plain
            assert render(["`vf0 Hallo Welt."])[0] != plain
            # A tag in one request cannot mutate the configured NVDA slider.
            assert render(["Hallo Welt."])[0] == plain
            cases = ["`ts1abc 123 `ts0 Ende.", "Hallo `[.1ha] Welt.",
                     "Hallo `[!3,1,0,0;.0te.1a.0tR] Welt.",
                     "`[!2,1,0,2;.1kO.0mA].",
                     "Hallo `[!65,0,0,0;.1a] Welt.",
                     "Hallo `p250 Welt.", "`p250 Hallo Welt.",
                     "Hallo `vb80 Welt.", "Hallo `vf0 Welt.",
                     "Hallo `vs80 Welt.", "Hallo `vv50 Welt.",
                     "Hallo `foo42 Welt.", "Hallo `vb999999999 Welt.",
                     "Hallo `[.1bEt Welt.", "`ts1`vb80 A `ts0 Welt."]
            count = 0
            for text in cases:
                expected, _ = render([text])
                assert expected
                for cut in sorted({0, 1, 3, len(text) // 2, len(text)}):
                    actual, indices = render([text[:cut], EciIndex(17), text[cut:], EciIndex(23)])
                    assert actual == expected, (text, cut)
                    assert indices == [17, 23]
                    count += 1
            actual, indices = render(["Hallo `vb80 ", EciPitch(50), "Welt.", EciIndex(41)])
            assert actual and indices == [41]
            # A uniform native speed tag must use exactly the slider's
            # planning path, independent of the configured slider value.
            speed_cases = 0
            for speed in (40, 50, 80, 100, 150):
                engine.set_parameter(NATIVE_SPEED, speed)
                expected, _ = render(["Stimme bei Großbuchstaben anheben."])
                for slider in (50, 98, 150):
                    engine.set_parameter(NATIVE_SPEED, slider)
                    actual, _ = render([f"`vs{speed} Stimme bei Großbuchstaben anheben."])
                    assert actual == expected, (speed, slider)
                    speed_cases += 1
            # Every word in these utterances has an explicit speed. Moving
            # the slider must not rescale boundaries, F0 or explicit pauses.
            for text in ("`vs80 Hallo `vs40 Welt.",
                         "`vs150 Sie `vs50 hören `vs100 eine Stimme.",
                         "`vs80 Ein `p50 Text `vs50 nach einer Pause."):
                engine.set_parameter(NATIVE_SPEED, 50)
                expected, _ = render([text])
                for slider in (98, 150):
                    engine.set_parameter(NATIVE_SPEED, slider)
                    actual, _ = render([text])
                    assert actual == expected, (text, slider)
                    speed_cases += 1
            engine.set_parameter(NATIVE_SPEED, 50)
            for pitch in (25, 81, 93):
                engine.set_parameter(NATIVE_PITCH, pitch)
                expected, _ = render(["Wortanalyse."])
                engine.set_parameter(NATIVE_PITCH, 65)
                assert render([f"`vb{pitch} Wortanalyse."])[0] == expected, pitch
            engine.set_parameter(PROCESS_TAGS, 0)
            assert render(["Hallo Welt."])[0] == plain
            for value in [-1, 2]:
                try:
                    engine.set_parameter(PROCESS_TAGS, value)
                except ValueError:
                    pass
                else:
                    raise AssertionError("invalid tag mode accepted")
            print(f"ECI tags: {count} fragmentation/index and {speed_cases} exact tag/slider speed cases plus pause, voice, mode and malformed-input checks passed")
        finally:
            engine.close()


if __name__ == "__main__":
    main()
