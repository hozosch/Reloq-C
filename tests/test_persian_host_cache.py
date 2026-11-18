"""The previous Windows host cache is accepted only for the checked edit."""
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from reuse_persian_host_cache import (ALLOWED, EXPECTED_KEY, RECENT_ALLOWED, RECENT_KEY,
                                      NEWEST_ALLOWED, NEWEST_KEY, TRACE_ALLOWED, TRACE_KEY,
                                      DEV149_ALLOWED, DEV149_KEY, eligible)

assert eligible(EXPECTED_KEY, ALLOWED)
assert not eligible(EXPECTED_KEY, ())
assert not eligible(EXPECTED_KEY + '-other', ALLOWED)
assert not eligible(EXPECTED_KEY, ALLOWED | {'src/reloq_eci.c'})
assert not eligible(EXPECTED_KEY, ALLOWED | {'experiments/persian/direct_fa_numbers.c'})
assert eligible(RECENT_KEY, RECENT_ALLOWED)
assert not eligible(RECENT_KEY, ())
assert not eligible(RECENT_KEY, RECENT_ALLOWED | {'src/reloq_rules61.c'})
assert not eligible(RECENT_KEY + '-other', RECENT_ALLOWED)
assert eligible(NEWEST_KEY, NEWEST_ALLOWED)
assert not eligible(NEWEST_KEY, ())
assert not eligible(NEWEST_KEY, NEWEST_ALLOWED | {'src/reloq_rules61.c'})
assert not eligible(NEWEST_KEY + '-other', NEWEST_ALLOWED)
assert eligible(TRACE_KEY, TRACE_ALLOWED)
assert not eligible(TRACE_KEY, ())
assert not eligible(TRACE_KEY, TRACE_ALLOWED | {'src/reloq_core.c'})
assert not eligible(TRACE_KEY + '-other', TRACE_ALLOWED)
assert eligible(DEV149_KEY, DEV149_ALLOWED)
assert not eligible(DEV149_KEY, ())
assert not eligible(DEV149_KEY, DEV149_ALLOWED | {'src/reloq_core.c'})
assert not eligible(DEV149_KEY, {'include/reloq_syllable_trace.h'})
print('Passed verified Windows host cache safeguards.')
