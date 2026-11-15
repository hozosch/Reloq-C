"""Exercise the real NVDA language/queue contract with a private FA voice.

This factory is only for tests. Its Persian ID is not a public ECI dialect,
and it deliberately rejects mixed-language jobs until the accepted shared
sentence contour and acoustic transitions are available online.
"""
from __future__ import annotations

from pathlib import Path

from direct_fa_sequence import ScopedDirectPersianEngine
from hybrid_fa_frontend import DirectFirstPersian
from text_frontend import UnsupportedPronunciation
from _reloq_eci_direct import DirectEciEngine, EciCapitalWord, EciLanguage, EciText
from _reloq_languages import Language, LanguageEngine, US_ENGLISH, GERMAN, FINNISH

PERSIAN_PROBE = Language(0x7f0000, 'Persian test', 'fa_IR', 'utf-8', US_ENGLISH.voices)
PROFILES = {profile.id: profile for profile in (PERSIAN_PROBE, US_ENGLISH, GERMAN, FINNISH)}


class OwnedPersianEngine(ScopedDirectPersianEngine):
    """One worker owns the acoustic handle and optional isolated reader."""
    def __init__(self, library, native_library, worker, espeak_library, data_parent):
        frontend = DirectFirstPersian(native_library, worker, espeak_library, data_parent)
        try:
            super().__init__(Path(library), frontend, allow_fallback=True)
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
    """Do not play a partially planned sentence across language switches."""
    def speak(self, parts, on_pcm, on_index):
        self._assert_owner()
        parts = tuple(parts)
        key = self._home
        languages = set()
        for part in parts:
            if isinstance(part, EciLanguage):
                key = self._key(part.language, part.voice)
            elif isinstance(part, EciCapitalWord):
                if part.original:
                    languages.add(key[0])
            elif isinstance(part, (str, EciText)):
                text = part if isinstance(part, str) else part.text
                if text:
                    languages.add(key[0])
        if PERSIAN_PROBE.id in languages and len(languages) > 1:
            raise UnsupportedPronunciation('continuous Persian language switching is not integrated')
        return super().speak(parts, on_pcm, on_index)


def make_language_engine(library, native_library, worker, espeak_library, data_parent,
                         *, language=PERSIAN_PROBE.id, voice='1'):
    """Construct the regular LanguageEngine with a test-only Persian factory."""
    def factory(dll, *, dialect, encoding, native_voice):
        if dialect == PERSIAN_PROBE.id:
            if encoding != 'utf-8' or not native_voice:
                raise ValueError('the Persian test requires a native UTF-8 voice')
            return OwnedPersianEngine(dll, native_library, worker, espeak_library, data_parent)
        return DirectEciEngine(dll, dialect=dialect, encoding=encoding, native_voice=native_voice)
    return PersianProbeLanguageEngine(library, language=language, voice=voice,
                                      languages=PROFILES, engine_factory=factory)
