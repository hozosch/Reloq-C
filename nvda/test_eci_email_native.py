"""Email recognition survives NVDA fragments, indexes and tag boundaries."""
from pathlib import Path
import sys
import tempfile
from _reloq_eci_direct import (
    DirectEciEngine, EciIndex, NATIVE_SPEED, PROCESS_TAGS, SAMPLE_RATE,
)

ROOT = Path(__file__).resolve().parents[1]


def main():
    sys.path.insert(0, str(ROOT / "addon"))
    from build_synth import merged_german_lexicon
    cases = [
        "Kontakt: anna@example.org.",
        "Bitte an vorname.nachname@example.de senden.",
        "<ab_12@test-domain.com>",
        "A1@ab.net und b@c.de",
        "test@@example.com; test@localhost; Ende.",
        "`vs80 anna@example.org `vs50 Ende.",
        "`ts1 a1@b.de `ts0 Ende.",
    ]
    checks = 0
    with tempfile.TemporaryDirectory() as folder:
        lex = Path(folder) / "german.rlex"
        lex.write_bytes(merged_german_lexicon())
        engine = DirectEciEngine(ROOT / "eci/libreloq_eci.so",
                                experimental_generic=True, lexicon_path=lex)
        try:
            engine.set_parameter(PROCESS_TAGS, 1)
            def render(parts):
                pcm, indices = [], []
                assert engine.speak(parts, pcm.append, indices.append)
                return b"".join(pcm), indices
            for rate in (1, 5):
                engine.set_parameter(SAMPLE_RATE, rate)
                for speed in (50, 98):
                    engine.set_parameter(NATIVE_SPEED, speed)
                    for text in cases:
                        expected, _ = render([text])
                        assert expected and any(expected)
                        for cut in sorted({0, text.index("@"), text.index("@") + 1,
                                           text.index("."), len(text)}):
                            actual, marks = render([
                                text[:cut], EciIndex(17), text[cut:], EciIndex(23),
                            ])
                            assert actual == expected, (rate, speed, text, cut)
                            assert marks == [17, 23], (text, cut, marks)
                            checks += 1
        finally:
            engine.close()
    print(f"Email: {checks} fragmented/indexed PCM checks at two rates and speeds passed")


if __name__ == "__main__":
    main()
