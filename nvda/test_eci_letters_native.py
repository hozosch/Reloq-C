"""Letter-name expansion must preserve PCM and native completion indices."""
from pathlib import Path
from _reloq_eci_direct import DirectEciEngine, EciIndex, GENERIC_RATE

ROOT = Path(__file__).resolve().parents[1]


def main():
    engine = DirectEciEngine(ROOT / 'eci/libreloq_eci.so',
                            experimental_generic=True,
                            lexicon_path=ROOT / 'data/german_apple_observed.rlex')
    checks = 0

    def render(parts):
        buffers, indices = [], []
        assert engine.speak(parts, buffers.append, indices.append)
        pcm = b''.join(buffers)
        assert pcm and any(pcm)
        return pcm, indices

    try:
        for speed in (50, 80):
            engine.set_parameter(GENERIC_RATE, speed)
            for text in ('CD', 'NVDA', 'HTML', 'F L M N R C ß',
                         'A B C 1 2 3 Ä Ö Ü ß', 'CD Anna öffnet alle Türen.'):
                pcm, _ = render([text])
                marked, indices = render([text, EciIndex(91)])
                assert marked == pcm and indices == [91]
                checks += 1
        # Explicit character boundaries and the C letter-expansion route
        # must not turn a letter name back into ordinary word pronunciation.
        engine.set_parameter(GENERIC_RATE, 50)
        for joined, separated in [('CD', 'C D'), ('NVDA', 'N V D A'),
                                  ('HTML', 'H T M L')]:
            assert render([joined])[0] == render([separated])[0]
            checks += 1
    finally:
        engine.close()
    print(f'{checks} native spelling checks: audio, indices, speed and equivalent letter expansion passed')


if __name__ == '__main__':
    main()
