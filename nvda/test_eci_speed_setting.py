"""Native speed setters and VS tags must use the same duration planner."""
import argparse
from pathlib import Path

from test_eci_inline_queue import Engine, TEXTS


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    library = parser.parse_args().library
    checks = 0
    for language, parts in TEXTS.items():
        engine = Engine(library, language)
        try:
            text = ''.join(parts)
            # Include the native value used by the NVDA 60 slider, a slower
            # setting and a faster one. Compare the actual PCM, not just WPM.
            for speed in (62, 80, 84, 90, 95, 100, 106, 140):
                engine.tags(0)
                engine.voice(6, speed)
                configured = engine.speak(text)
                engine.voice(6, 50)
                engine.tags(1)
                tagged = engine.speak(f'`vs{speed} {text}')
                assert tagged == configured, (hex(language), speed, len(tagged), len(configured))
                checks += 1
            # The ordinary VB pitch scale must keep the same contract too;
            # exact Hertz and WPM each have their own wider acoustic tests.
            engine.voice(6, 106)
            for pitch in (40, 80):
                engine.tags(0)
                engine.voice(2, pitch)
                configured = engine.speak(text)
                engine.voice(2, 65)
                engine.tags(1)
                tagged = engine.speak(f'`vb{pitch} {text}')
                assert tagged == configured, (hex(language), pitch, 'VB pitch')
                checks += 1
            # A queued setter and an inline tag after the same text must
            # preserve the same context and take effect at the same word.
            prefix, word, suffix = parts
            engine.voice(6, 50)
            engine.tags(1)
            for speed in (62, 95, 106):
                engine.tags(1)
                engine.voice(6, 50)
                tagged = engine.speak(prefix + f'`vs{speed} ' + word + suffix)
                engine.tags(0)
                engine.voice(6, 50)
                engine.add(prefix)
                engine.voice(6, speed)
                engine.add(word + suffix)
                assert engine.finish() == tagged, (hex(language), speed, 'queued speed')
                checks += 1
        finally:
            engine.close()
    # Both extended clocks use the identical scheduler, not a post-render
    # speed approximation. Exercise English, German and Chinese with them.
    for language in (0x10000, 0x40000, 0x60000):
        engine = Engine(library, language)
        try:
            for sample_rate in (3, 6):
                assert engine.lib.eciSetParam(engine.h, 5, sample_rate) >= 0
                for speed in (90, 100, 106):
                    engine.tags(0)
                    engine.voice(6, speed)
                    configured = engine.speak(''.join(TEXTS[language]))
                    engine.voice(6, 50)
                    engine.tags(1)
                    assert engine.speak(f'`vs{speed} ' + ''.join(TEXTS[language])) == configured, (hex(language), sample_rate, speed)
                    checks += 1
        finally:
            engine.close()
    print(f'Voice controls: {checks} speed, pitch, queued and sample-rate comparisons passed')


if __name__ == '__main__':
    main()
