"""Exercise the real NVDA language/queue contract with a private FA voice.

This factory is only for tests. Its Persian IDs are not public ECI dialects.
Explicit NVDA language switches use the regular per-language engines. The
accepted shared sentence contour and transitions are not integrated yet.
"""
from __future__ import annotations

from pathlib import Path

if __package__:
    from .direct_fa_sequence import ScopedDirectPersianEngine
    from .hybrid_fa_frontend import DirectFirstPersian
    from .._reloq_eci_direct import DirectEciEngine
    from .._reloq_languages import Language, LanguageEngine, US_ENGLISH, GERMAN, FINNISH
else:
    from direct_fa_sequence import ScopedDirectPersianEngine
    from hybrid_fa_frontend import DirectFirstPersian
    from _reloq_eci_direct import DirectEciEngine
    from _reloq_languages import Language, LanguageEngine, US_ENGLISH, GERMAN, FINNISH

PERSIAN_PROBE = Language(0x7f0000, 'Persian test', 'fa_IR', 'utf-8', US_ENGLISH.voices)
PINGLISH_PROBE = Language(0x7f0001, 'Persian (Pinglish) test', 'fa_Latn', 'utf-8', US_ENGLISH.voices)
PROFILES = {profile.id: profile for profile in
            (PERSIAN_PROBE, PINGLISH_PROBE, US_ENGLISH, GERMAN, FINNISH)}


class OwnedPersianEngine(ScopedDirectPersianEngine):
    """One worker owns the acoustic handle and optional isolated reader."""
    def __init__(self, library, native_library, worker, espeak_library, data_parent, *, pinglish=False):
        frontend = DirectFirstPersian(native_library, worker, espeak_library, data_parent)
        try:
            super().__init__(Path(library), frontend, allow_fallback=True, pinglish=pinglish)
        except BaseException:
            frontend.close()
            raise

    def set_dictionary_path(self, path):
        self._voice.set_dictionary_path(path)

    def close(self):
        try:
            super().close()
        finally:
            self._frontend.close()


class PersianProbeLanguageEngine(LanguageEngine):
    """Use ordinary language switches until continuous contour is integrated."""


def make_language_engine(library, native_library, worker, espeak_library, data_parent,
                         *, language=PERSIAN_PROBE.id, voice='1', languages=None):
    """Construct the regular LanguageEngine with a test-only Persian factory."""
    def factory(dll, *, dialect, encoding, native_voice):
        if dialect in (PERSIAN_PROBE.id, PINGLISH_PROBE.id):
            if encoding != 'utf-8' or not native_voice:
                raise ValueError('the Persian test requires a native UTF-8 voice')
            return OwnedPersianEngine(dll, native_library, worker, espeak_library, data_parent,
                                      pinglish=dialect == PINGLISH_PROBE.id)
        return DirectEciEngine(dll, dialect=dialect, encoding=encoding, native_voice=native_voice)
    return PersianProbeLanguageEngine(library, language=language, voice=voice,
                                      languages=PROFILES if languages is None else languages,
                                      engine_factory=factory)
