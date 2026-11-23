"""Pitch marks must take effect after German punctuation without moving audio."""
from pathlib import Path
import argparse

from _reloq_eci_direct import DirectEciEngine, EciIndex, EciPitch


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--library", type=Path, required=True)
    args = parser.parse_args()
    checks = 0
    for dialect in (0x40000, 0x10000, 0x20000):
        voice = DirectEciEngine(args.library.resolve(), dialect=dialect,
                                encoding="cp1252", native_voice=True)
        try:
            def render(parts):
                audio, indices = [], []
                assert voice.speak(parts, audio.append, indices.append)
                return b"".join(audio), indices

            for punctuation in (" ", ", ", ". ", ": ", "! ", "? "):
                prefix = "Hieb" + punctuation
                plain, _ = render([prefix + "Hieb."])
                marked, _ = render([prefix, EciPitch(85), "Hieb."])
                indexed, indices = render([prefix, EciIndex(42), EciPitch(85), "Hieb."])
                assert len(plain) == len(marked) and plain != marked, (dialect, punctuation)
                assert indexed == marked and indices == [42], (dialect, punctuation)
                checks += 1
        finally:
            voice.close()
    print(f"Passed {checks} pitch changes at punctuation and index boundaries.")


if __name__ == "__main__":
    main()
