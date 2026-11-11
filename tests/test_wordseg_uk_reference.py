"""Check British comparisons without importing American vowel mergers."""
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
sys.path.insert(0, str(ROOT / 'vendor/openevv61/tools/measure'))
import wordseg_uk_reference as uk


def agrees(native, reference):
    return uk.same(uk.native(native), uk.ipa(reference))


assert agrees('`[.1h@.0mx.0fon]', '/ˈhɒməfəʊn/')
assert not agrees('`[.1ho.0mo.2fon]', '/ˈhɒməfəʊn/')
assert agrees('`[.1fYl.2sw@p]', '/ˈfaɪlˌswɒp/')
assert agrees('`[.1kAn.0dL.2lYt]', '/ˈkændəlˌlaɪt/')
# A consonant such as the linking r is never erased by a vowel merger.
assert not agrees('`[.1af.0tx.2I.0mXJ]', '/ˈɑːftəɹˌɪmɪdʒ/')
try:
    uk.ipa('/ˈfɑ̃/')
except ValueError:
    pass
else:
    raise AssertionError('unsupported IPA must not be silently accepted')
print('British reference comparisons passed')
