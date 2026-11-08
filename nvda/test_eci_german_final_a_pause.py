"""German final A before pauses, through the public ECI text interface."""
import argparse
from pathlib import Path

from _reloq_eci_direct import DirectEciEngine, EciIndex, GENDER, PROCESS_TAGS, SAMPLE_RATE, SAMPLE_RATES
from test_eci_german_clock_native import phonemes, render


def open_engine(path):
    engine = DirectEciEngine(path.resolve(), dialect=0x40000, encoding='utf8', native_voice=True)
    engine.set_parameter(PROCESS_TAGS, 1)
    return engine


def text_vs_phonemes(engine, text):
    audio = render(engine, [text])[0]
    explicit = render(engine, [phonemes(engine, text)])[0]
    return len(audio) - len(explicit), audio != explicit


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--baseline', type=Path, help='library before the pause-boundary change')
    args = parser.parse_args()
    engine = open_engine(args.library)
    baseline = open_engine(args.baseline) if args.baseline else None
    checks = 0
    try:
        for gender in (0, 1):
            engine.set_parameter(GENDER, gender)
            for rate in SAMPLE_RATES:
                engine.set_parameter(SAMPLE_RATE, rate)
                deltas = []
                for text in ('Pizza, Test', 'Sofa, Test', 'Kamera, Test', 'Thema, Test',
                             'Pizza: Test', 'Pizza; Test', 'Pizza,Test'):
                    delta, changed = text_vs_phonemes(engine, text)
                    assert changed and delta > 0, (gender, rate, text, delta)
                    if text.endswith(', Test'):
                        deltas.append(delta)
                    checks += 1
                # The four endings must all be selected within one utterance.
                delta, changed = text_vs_phonemes(engine, 'Pizza, Sofa, Kamera, Thema, Test')
                assert changed and delta == sum(deltas), (gender, rate, delta, deltas)
                checks += 1

        engine.set_parameter(GENDER, 0)
        engine.set_parameter(SAMPLE_RATE, 1)
        single, _ = text_vs_phonemes(engine, 'Pizza, Test')
        delta, _ = text_vs_phonemes(engine, 'Pizza, ' * 20 + 'Test')
        assert delta == 20 * single, (delta, single)
        checks += 1
        comma = render(engine, ['Pizza, Test'])[0]
        dash = render(engine, ['Pizza - Test'])[0]
        for text in ('Pizza — Test', 'Pizza – Test'):
            assert render(engine, [text])[0] == dash, text
            checks += 1
        assert len(render(engine, ['Pizza … Test'])[0]) == len(comma)
        checks += 1
        connected = render(engine, ['Pizza Test'])[0]
        for text in ('Pizza » Test', 'Pizza « Test', 'Pizza “ Test'):
            assert render(engine, [text])[0] == connected, text
            checks += 1
        for text in ('Pizza `p1 Test', 'Pizza `p80 Test'):
            delta, changed = text_vs_phonemes(engine, text)
            assert changed and delta > 0, (text, delta)
            checks += 1

        if baseline is not None:
            for e in (engine, baseline):
                e.set_parameter(GENDER, 0)
                e.set_parameter(SAMPLE_RATE, 1)
                e.set_parameter(PROCESS_TAGS, 1)
            for text in ('Pizza', 'Pizza.', 'Pizza. Test', 'Pizza... Test',
                         'Pizza Test', 'Pizza ist gut', 'Pizza-Test',
                         'Pizza.test', 'www.pizza.de', 'https://pizza.de',
                         'Pizza » Test', 'Pizza « Test', 'Pizza “ Test', 'Pizza ¼ Test',
                         'Pizza `p0 Test', 'Pizza `vv100 Test'):
                assert render(engine, [text]) == render(baseline, [text]), text
                checks += 1
            for text in ('Pizza, Test', 'Pizza: Test', 'Pizza; Test',
                         'Kamera, Test', 'Pizza (Test)', 'Pizza - Test',
                         'Pizza — Test', 'Pizza – Test', 'Pizza … Test',
                         'Pizza `p1 Test', 'Pizza `p80 Test'):
                new = render(engine, [text])[0]
                old = render(baseline, [text])[0]
                assert new != old and len(new) > len(old), text
                checks += 1
            # ECI indices do not prevent several endings in the same phrase
            # from being found, even before a final A that was already correct.
            parts = ['Sofa', EciIndex(1), ', Pizza', EciIndex(2), ', Thema und Kamera.']
            old_audio, old_indices = render(baseline, parts)
            new_audio, new_indices = render(engine, parts)
            assert [i for i, _ in old_indices] == [i for i, _ in new_indices] == [1, 2]
            assert new_indices[0][1] > old_indices[0][1]
            assert new_indices[1][1] - new_indices[0][1] > old_indices[1][1] - old_indices[0][1]
            assert len(new_audio) > len(old_audio)
            checks += 1
            for e in (engine, baseline):
                e.set_parameter(PROCESS_TAGS, 0)
            text = 'Pizza `p80 Test'
            assert render(engine, [text]) == render(baseline, [text]), text
            checks += 1
    finally:
        engine.close()
        if baseline is not None:
            baseline.close()
    print(f'German final A: {checks} pause, multiple-word, phonetic and control comparisons')


if __name__ == '__main__':
    main()
