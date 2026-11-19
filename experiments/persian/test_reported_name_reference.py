"""Inspect fallback pronunciation for a reported Persian surname family."""
from pathlib import Path
import argparse
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from direct_fa_sequence import project_resilient
from nvda_driver_probe import PERSIAN_PROBE, make_language_engine
from _reloq_eci_direct import NATIVE_SPEED, SAMPLE_RATE
from test_persian_r_after_i_contexts import capture
from test_us_articulation import voice


def main():
    p = argparse.ArgumentParser()
    for name in ('library', 'espeak', 'data'):
        p.add_argument('--' + name, type=Path, required=True)
    args = p.parse_args()
    owner = make_language_engine(args.library, args.library, None, args.espeak, args.data)
    acoustic, scope = voice(args.library, 0x90000)
    try:
        acoustic.set_parameter(SAMPLE_RATE, 3)
        acoustic.set_parameter(NATIVE_SPEED, 82)
        engine = owner._activate(owner._key(PERSIAN_PROBE.id, '1'))
        for word in ('خواجه‌ئیان', 'خواجه', 'علی'):
            result = project_resilient((word,), engine._frontend)
            print('Reported name:', word, result.tagged, flush=True)
            assert '`[' in result.tagged, (word, result.tagged)
            if word == 'خواجه‌ئیان':
                native = engine._frontend.analyze_direct(word)
                assert native and not native.used_fallback, word
                assert engine._frontend._worker is None, 'known surname used a fallback'
                assert ".0'i:'.1an]" in result.tagged, result.tagged
                short = result.tagged.replace(".0'i:'.1an]", '.0i.1an]', 1)
                observed = capture(acoustic, scope, result.tagged)
                reference = capture(acoustic, scope, short)
                a,b=observed.syllables[5]; c,d=reference.syllables[5]
                assert b-a == d-c, (word, b-a, d-c)
                # The old short tag was itself too long in this hiatus.
                # Compare against a short /i/ before the same stressed vowel
                # after an ordinary consonant, not just against that tag.
                consonant_context = capture(acoustic, scope, '`[.1Ga.0Ji.1an]')
                x,y=consonant_context.syllables[4]
                assert b-a <= y-x, (word, b-a, y-x)
    finally:
        acoustic.close()
        owner.close()


if __name__ == '__main__':
    main()
