"""Legacy ECI input and explicit breaks used by Eloquence_64 v28."""
from pathlib import Path
import argparse

from _reloq_eci_direct import DirectEciEngine, EciIndex


def render(engine, parts):
    audio, indices = [], []
    assert engine.speak(parts, audio.append, indices.append)
    return b"".join(audio), indices


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--library", type=Path, required=True)
    args = parser.parse_args()
    engines = [DirectEciEngine(args.library.resolve(), dialect=0x60000,
                               encoding=encoding, native_voice=True)
               for encoding in ("gb18030", "utf8")]
    checks = 0
    try:
        for engine in engines:
            # Eloquence_64 uses the standard eciInputType=1 setting.
            assert engine._dll.eciSetParam(engine._handle, 1, 1) >= 0
        for text in ("Tonhöhe", "Lautstärke", "Ä Ö Ü ä ö ü ß", "你好 ä 世界。",
                     "中文 😀 Englisch.", "你好 € Welt."):
            parts = [EciIndex(10), text, EciIndex(11)]
            narrow, wide = (render(engine, parts) for engine in engines)
            assert narrow[0] and narrow == wide, text
            assert narrow[1] == [10, 11], (text, narrow[1])
            checks += 1
        legacy, unicode = engines
        assert legacy._dll.eciAddText(legacy._handle, b"Price \x80.")
        legacy_audio, _ = render(legacy, [])
        unicode_audio, _ = render(unicode, ["Price €."])
        assert legacy_audio and legacy_audio == unicode_audio, "CP936 euro compatibility"
        assert not legacy._dll.eciAddText(legacy._handle, b"\x81\x30")
        assert render(legacy, ["你好。"]) == render(unicode, ["你好。"])  # recovered after invalid input
        checks += 2
        for engine in engines:
            lengths = {}
            for duration in (50, 250, 1000):
                prefix = "`pp0 `da1 `vv100 你好"
                pause = f"`p{duration}."
                suffix = "`pp0 `da1 世界。"
                fragmented = render(engine, [prefix, pause, suffix])
                joined = render(engine, [prefix + pause + suffix])
                indexed = render(engine, [prefix, EciIndex(1), pause, EciIndex(2), suffix])
                assert fragmented[0] and fragmented[0] == joined[0] == indexed[0]
                assert indexed[1] == [1, 2]
                lengths[duration] = len(fragmented[0])
                checks += 1
            # At 11.025 kHz the native renderer produces 11 samples per ms.
            assert lengths[250] - lengths[50] == 200 * 11 * 2, lengths
            assert lengths[1000] - lengths[250] == 750 * 11 * 2, lengths

        # Eloquence_64 inserts a pause annotation immediately before sentence
        # punctuation. Its p0 leaves the normal pause intact; p1 shortens it.
        # Exercise both the Mandarin path and its embedded English voice.
        for first, last, punctuation in (
            ("你好", "世界", ","),
            ("Hallo", "Welt", ","),
            ("Tonhöhe", "Lautstärke", ","),
            ("Hello", "world", "?"),
            ("你好 Hello", "世界", ","),
        ):
            outputs = []
            for engine in engines:
                plain = render(engine, [f"{first}{punctuation} {last}"])[0]
                normal = render(engine, [f"{first} `p0{punctuation} {last}"])[0]
                short = render(engine, [f"{first} `p1{punctuation} {last}"])[0]
                p50 = render(engine, [f"{first} `p50{punctuation} {last}"])[0]
                p250 = render(engine, [f"{first} `p250{punctuation} {last}"])[0]
                assert normal == plain, (first, punctuation, "p0")
                assert len(short) < len(normal) - 1100 * 2, (first, punctuation)
                assert len(p250) - len(p50) == 200 * 11 * 2, (first, punctuation)
                outputs.append((plain, normal, short, p50, p250))
                checks += 3
            assert outputs[0] == outputs[1], (first, punctuation, "encoding")
    finally:
        for engine in engines:
            engine.close()
    print(f"Passed {checks} GB18030, Unicode and explicit-break checks.")


if __name__ == "__main__":
    main()
