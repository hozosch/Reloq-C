"""The previous Windows host cache is accepted only for the checked edit."""
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from reuse_persian_host_cache import ALLOWED, EXPECTED_KEY, eligible

assert eligible(EXPECTED_KEY, ALLOWED)
assert not eligible(EXPECTED_KEY, ())
assert not eligible(EXPECTED_KEY + '-other', ALLOWED)
assert not eligible(EXPECTED_KEY, ALLOWED | {'src/reloq_eci.c'})
assert not eligible(EXPECTED_KEY, ALLOWED | {'experiments/persian/direct_fa_numbers.c'})
print('Passed verified Windows host cache safeguards.')
