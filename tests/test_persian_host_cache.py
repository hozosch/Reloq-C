"""The previous Windows host cache is accepted only for the checked edit."""
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from reuse_persian_host_cache import (ALLOWED, EXPECTED_KEY, RECENT_ALLOWED, RECENT_KEY,
                                      NEWEST_ALLOWED, NEWEST_KEY, TRACE_ALLOWED, TRACE_KEY,
                                      DEV149_ALLOWED, DEV149_KEY, DEV150_ALLOWED, DEV150_KEY,
                                      DEV151_ALLOWED, DEV151_KEY, DEV152_ALLOWED, DEV152_KEY,
                                      DEV153_ALLOWED, DEV153_KEY, DEV154_ALLOWED, DEV154_KEY,
                                      DEV155_ALLOWED, DEV155_KEY, DEV156_ALLOWED, DEV156_KEY,
                                      DEV157_ALLOWED, DEV157_KEY, DEV158_ALLOWED, DEV158_KEY,
                                      DEV159_ALLOWED, DEV159_KEY, DEV160_ALLOWED, DEV160_KEY, DEV161_ALLOWED, DEV161_KEY,
                                      DEV162_ALLOWED, DEV162_KEY, DEV163_ALLOWED, DEV163_KEY,
                                      DEV164_ALLOWED, DEV164_KEY, DEV165_ALLOWED,
                                      DEV166_ALLOWED, DEV166_KEY, DEV168_ALLOWED, DEV168_KEY,
                                      DEV169_ALLOWED, DEV169_KEY, DEV170_ALLOWED, DEV170_KEY,
                                      DEV171_ALLOWED, DEV171_KEY, DEV172_ALLOWED, DEV172_KEY,
                                      DEV187_ALLOWED, DEV187_KEY, DEV188_ALLOWED, DEV188_KEY,
                                      DEV188_TRANSITION_ALLOWED, DEV188_TRANSITION_KEY,
                                      DEV189_ALLOWED, DEV189_KEY, DEV190_ALLOWED, DEV190_KEY,
                                      DEV191_ALLOWED, DEV191_KEY, DEV200_ALLOWED, DEV200_KEY,
                                      DEV201_ALLOWED, DEV201_KEY, DEV34_ALLOWED, DEV34_KEY,
                                      DEV36_ALLOWED, DEV36_KEY, DEV37_ALLOWED, DEV37_KEY,
                                      DEV43_ALLOWED, DEV43_KEY, DEV44_ALLOWED, DEV44_KEY, eligible)

assert eligible(DEV44_KEY, DEV44_ALLOWED)
assert not eligible(DEV44_KEY, ())
assert not eligible(DEV44_KEY, DEV44_ALLOWED | {'src/reloq_core.c'})
assert not eligible(DEV44_KEY, DEV44_ALLOWED - {'src/reloq_rules61.c'})

assert eligible(DEV43_KEY, DEV43_ALLOWED)
assert not eligible(DEV43_KEY, ())
assert not eligible(DEV43_KEY, DEV43_ALLOWED | {'src/reloq_core.c'})
assert not eligible(DEV43_KEY, DEV43_ALLOWED - {'src/reloq_rules61.c'})

assert eligible(DEV37_KEY, DEV37_ALLOWED)
assert not eligible(DEV37_KEY, ())
assert not eligible(DEV37_KEY, DEV37_ALLOWED | {'src/reloq_core.c'})

assert eligible(DEV36_KEY, DEV36_ALLOWED)
assert not eligible(DEV36_KEY, ())
assert not eligible(DEV36_KEY, DEV36_ALLOWED | {'src/reloq_core.c'})
assert not eligible(DEV36_KEY, DEV36_ALLOWED - {'src/reloq_eci.c'})

assert eligible(DEV34_KEY, DEV34_ALLOWED)
assert not eligible(DEV34_KEY, ())
assert not eligible(DEV34_KEY, DEV34_ALLOWED | {'src/reloq_core.c'})
assert not eligible(DEV34_KEY, DEV34_ALLOWED - {'experiments/persian/direct_fa_pinglish.c'})

assert eligible(DEV201_KEY, DEV201_ALLOWED)
assert not eligible(DEV201_KEY, ())
assert not eligible(DEV201_KEY, DEV201_ALLOWED | {'src/reloq_core.c'})
assert not eligible(DEV201_KEY, DEV201_ALLOWED - {'experiments/persian/direct_fa_lexicon.c'})

# actions/cache accepts at most ten keys including the primary key. Keep
# the newest guarded fallback and leave room for the key of the current build.
workflow = (Path(__file__).resolve().parents[1] /
            '.github/workflows/persian-windows-acoustic-probe.yml').read_text(encoding='utf8')
host_restore = workflow.split('id: host', 1)[1].split('restore-keys: |', 1)[1]
host_restore = host_restore.split('      - name:', 1)[0]
host_keys = [line.strip() for line in host_restore.splitlines() if line.strip()]
assert DEV44_KEY in host_keys and DEV43_KEY in host_keys and DEV37_KEY in host_keys and DEV36_KEY in host_keys and DEV34_KEY in host_keys and DEV201_KEY in host_keys and len(host_keys) <= 9, host_keys

assert eligible(DEV200_KEY, DEV200_ALLOWED)
assert not eligible(DEV200_KEY, ())
assert not eligible(DEV200_KEY, DEV200_ALLOWED | {'src/reloq_core.c'})
assert not eligible(DEV200_KEY, DEV200_ALLOWED - {'src/reloq_persian_plan.c'})
assert eligible(DEV191_KEY, DEV191_ALLOWED)
assert not eligible(DEV191_KEY, ())
assert not eligible(DEV191_KEY, DEV191_ALLOWED | {'src/reloq_core.c'})
assert eligible(DEV190_KEY, DEV190_ALLOWED)
assert not eligible(DEV190_KEY, ())
assert not eligible(DEV190_KEY, DEV190_ALLOWED | {'src/reloq_core.c'})

assert eligible(DEV189_KEY, DEV189_ALLOWED)
assert not eligible(DEV189_KEY, ())
assert not eligible(DEV189_KEY, DEV189_ALLOWED | {'src/reloq_core.c'})

assert eligible(DEV188_TRANSITION_KEY, DEV188_TRANSITION_ALLOWED)
assert not eligible(DEV188_TRANSITION_KEY, ())
assert not eligible(DEV188_TRANSITION_KEY, DEV188_TRANSITION_ALLOWED | {'src/reloq_core.c'})

assert eligible(DEV188_KEY, DEV188_ALLOWED)
assert not eligible(DEV188_KEY, ())
assert not eligible(DEV188_KEY, DEV188_ALLOWED | {'src/reloq_core.c'})

assert eligible(DEV187_KEY, DEV187_ALLOWED)
assert not eligible(DEV187_KEY, ())
assert not eligible(DEV187_KEY, DEV187_ALLOWED | {'src/reloq_core.c'})
assert not eligible(DEV187_KEY, DEV187_ALLOWED - {'experiments/persian/direct_fa_pinglish.c'})

assert eligible(DEV172_KEY, DEV172_ALLOWED)
assert not eligible(DEV172_KEY, ())
assert not eligible(DEV172_KEY, DEV172_ALLOWED | {'src/reloq_core.c'})
assert not eligible(DEV172_KEY, DEV172_ALLOWED - {'src/reloq_persian_frames.c'})

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
assert eligible(DEV150_KEY, DEV150_ALLOWED)
assert not eligible(DEV150_KEY, ())
assert not eligible(DEV150_KEY, DEV150_ALLOWED | {'src/reloq_core.c'})
assert not eligible(DEV150_KEY, {'src/reloq_persian_acoustics.c'})
assert eligible(DEV151_KEY, DEV151_ALLOWED)
assert not eligible(DEV151_KEY, ())
assert not eligible(DEV151_KEY, DEV151_ALLOWED | {'src/reloq_core.c'})
assert not eligible(DEV151_KEY, {'src/reloq_persian_acoustics.c'})
assert eligible(DEV152_KEY, DEV152_ALLOWED)
assert not eligible(DEV152_KEY, ())
assert not eligible(DEV152_KEY, DEV152_ALLOWED | {'src/reloq_core.c'})
assert not eligible(DEV152_KEY, {'src/reloq_persian_acoustics.c'})
assert eligible(DEV153_KEY, DEV153_ALLOWED)
assert not eligible(DEV153_KEY, ())
assert not eligible(DEV153_KEY, DEV153_ALLOWED | {'src/reloq_core.c'})
assert not eligible(DEV153_KEY, {'src/reloq_persian_acoustics.c'})
assert eligible(DEV154_KEY, DEV154_ALLOWED)
assert not eligible(DEV154_KEY, ())
assert not eligible(DEV154_KEY, DEV154_ALLOWED | {'src/reloq_core.c'})
assert not eligible(DEV154_KEY, {'src/reloq_persian_acoustics.c'})
assert eligible(DEV155_KEY, DEV155_ALLOWED)
assert not eligible(DEV155_KEY, ())
assert not eligible(DEV155_KEY, DEV155_ALLOWED | {'src/reloq_core.c'})
assert not eligible(DEV155_KEY, {'src/reloq_persian_acoustics.c'})
assert eligible(DEV156_KEY, DEV156_ALLOWED)
assert not eligible(DEV156_KEY, ())
assert not eligible(DEV156_KEY, DEV156_ALLOWED | {'src/reloq_core.c'})
assert not eligible(DEV156_KEY, {'src/reloq_persian_acoustics.c'})
assert eligible(DEV157_KEY, DEV157_ALLOWED)
assert not eligible(DEV157_KEY, ())
assert not eligible(DEV157_KEY, DEV157_ALLOWED | {'src/reloq_core.c'})
assert not eligible(DEV157_KEY, {'src/reloq_persian_acoustics.c'})
assert eligible(DEV158_KEY, DEV158_ALLOWED)
assert not eligible(DEV158_KEY, ())
assert not eligible(DEV158_KEY, DEV158_ALLOWED | {'src/reloq_core.c'})
assert not eligible(DEV158_KEY, DEV158_ALLOWED - {'src/reloq_noise_profile.c'})
assert eligible(DEV159_KEY, DEV159_ALLOWED)
assert not eligible(DEV159_KEY, ())
assert not eligible(DEV159_KEY, DEV159_ALLOWED | {'src/reloq_core.c'})
assert eligible(DEV160_KEY, DEV160_ALLOWED)
assert not eligible(DEV160_KEY, ())
assert not eligible(DEV160_KEY, DEV160_ALLOWED | {'src/reloq_core.c'})
assert eligible(DEV161_KEY, DEV161_ALLOWED)
assert not eligible(DEV161_KEY, ())
assert not eligible(DEV161_KEY, DEV161_ALLOWED | {'src/reloq_core.c'})
assert eligible(DEV162_KEY, DEV162_ALLOWED)
assert not eligible(DEV162_KEY, ())
assert not eligible(DEV162_KEY, DEV162_ALLOWED | {'src/reloq_core.c'})
assert eligible(DEV163_KEY, DEV163_ALLOWED)
assert not eligible(DEV163_KEY, ())
assert not eligible(DEV163_KEY, DEV163_ALLOWED | {'src/reloq_core.c'})
assert not eligible(DEV163_KEY, DEV163_ALLOWED - {'src/reloq_persian_frames.c'})
assert eligible(DEV164_KEY, DEV164_ALLOWED)
assert not eligible(DEV164_KEY, ())
assert not eligible(DEV164_KEY, DEV164_ALLOWED | {'src/reloq_core.c'})
assert not eligible(DEV164_KEY, DEV164_ALLOWED - {'experiments/persian/direct_fa_text.c'})
assert eligible(DEV164_KEY, DEV165_ALLOWED)
assert not eligible(DEV164_KEY, DEV165_ALLOWED | {'src/reloq_core.c'})
assert not eligible(DEV164_KEY, DEV165_ALLOWED - {'src/reloq_persian_frames.c'})
assert eligible(DEV166_KEY, DEV166_ALLOWED)
assert not eligible(DEV166_KEY, ())
assert not eligible(DEV166_KEY, DEV166_ALLOWED | {'src/reloq_core.c'})
assert not eligible(DEV166_KEY, DEV166_ALLOWED - {'experiments/persian/direct_fa_lexicon.c'})
assert eligible(DEV168_KEY, DEV168_ALLOWED)
assert not eligible(DEV168_KEY, ())
assert not eligible(DEV168_KEY, DEV168_ALLOWED | {'src/reloq_core.c'})
assert not eligible(DEV168_KEY, DEV168_ALLOWED - {'experiments/persian/direct_fa_lexicon.c'})
assert eligible(DEV169_KEY, DEV169_ALLOWED)
assert not eligible(DEV169_KEY, ())
assert not eligible(DEV169_KEY, DEV169_ALLOWED | {'src/reloq_core.c'})
assert not eligible(DEV169_KEY, DEV169_ALLOWED - {'experiments/persian/direct_fa_lexicon.c'})
assert eligible(DEV170_KEY, DEV170_ALLOWED)
assert not eligible(DEV170_KEY, ())
assert not eligible(DEV170_KEY, DEV170_ALLOWED | {'src/reloq_core.c'})
assert not eligible(DEV170_KEY, DEV170_ALLOWED - {'experiments/persian/direct_fa_lexicon.c'})
assert eligible(DEV171_KEY, DEV171_ALLOWED)
assert not eligible(DEV171_KEY, ())
assert not eligible(DEV171_KEY, DEV171_ALLOWED | {'src/reloq_core.c'})
print('Passed verified Windows host cache safeguards.')
