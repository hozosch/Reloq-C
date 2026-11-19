"""Inspect fallback pronunciation for a reported Persian surname family."""
from pathlib import Path
import argparse
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from direct_fa_sequence import project_resilient
from nvda_driver_probe import PERSIAN_PROBE, make_language_engine


def main():
    p = argparse.ArgumentParser()
    for name in ('library', 'espeak', 'data'):
        p.add_argument('--' + name, type=Path, required=True)
    args = p.parse_args()
    owner = make_language_engine(args.library, args.library, None, args.espeak, args.data)
    try:
        engine = owner._activate(owner._key(PERSIAN_PROBE.id, '1'))
        for word in ('خواجه‌ئیان', 'خواجه', 'علی'):
            result = project_resilient((word,), engine._frontend)
            print('Reported name:', word, result.tagged, flush=True)
            assert '`[' in result.tagged, (word, result.tagged)
    finally:
        owner.close()


if __name__ == '__main__':
    main()
