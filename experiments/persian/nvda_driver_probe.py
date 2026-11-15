"""Exercise the real NVDA language/queue contract with a private FA voice.

This factory is only for tests. Its Persian IDs are not public ECI dialects.
Explicit and automatic English switches can use the selected continuous
sentence plan when the private host exposes frame capture and replay.
"""
from __future__ import annotations

from dataclasses import replace
from pathlib import Path
import unicodedata

if __package__:
    from .direct_fa_sequence import ScopedDirectPersianEngine
    from .hybrid_fa_frontend import DirectFirstPersian
    from .continuous_driver import sentence_jobs, try_continuous, try_persian_glides
    from .._reloq_eci_direct import (DirectEciEngine, EciCapitalWord, EciIndex, EciLanguage,
                                     EciPitch, EciPitchHz, EciProsody, EciText)
    from .._reloq_languages import Language, LanguageEngine, US_ENGLISH, GERMAN, FINNISH
else:
    from direct_fa_sequence import ScopedDirectPersianEngine
    from hybrid_fa_frontend import DirectFirstPersian
    from continuous_driver import sentence_jobs, try_continuous, try_persian_glides
    from _reloq_eci_direct import (DirectEciEngine, EciCapitalWord, EciIndex, EciLanguage,
                                  EciPitch, EciPitchHz, EciProsody, EciText)
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


def _script_runs(text):
    """Preserve every character while separating Latin from other letters."""
    runs, prefix, current, kind = [], '', '', None
    for char in text:
        strong = ('en' if unicodedata.name(char, '').startswith('LATIN ') else
                  'fa' if char.isalpha() else None)
        if strong is None:
            if kind is None:
                prefix += char
            else:
                current += char
            continue
        if kind is None:
            kind, current, prefix = strong, prefix, ''
        elif strong != kind:
            runs.append((kind, current))
            kind, current = strong, ''
        current += char
    if kind is None:
        return (('fa', prefix),) if prefix else ()
    runs.append((kind, current))
    return tuple(runs)


class PersianProbeLanguageEngine(LanguageEngine):
    """Use English for explicit and clear Latin-script boundaries."""
    def speak(self, parts, on_pcm, on_index):
        self._assert_owner()
        logical = actual = self._home
        output = []

        def select(key):
            nonlocal actual
            if key != actual:
                # Events at a boundary describe the next text, rather than
                # making the old engine synthesize an event-only fragment.
                events = []
                while output and isinstance(output[-1], (EciIndex, EciPitch, EciPitchHz, EciProsody)):
                    events.append(output.pop())
                output.append(EciLanguage(*key))
                output.extend(reversed(events))
                actual = key

        for part in parts:
            if isinstance(part, EciLanguage):
                logical = self._key(part.language, part.voice)
                select(logical)
                continue
            text = part if isinstance(part, str) else part.text if isinstance(part, EciText) else None
            ordinary_text = (text is not None and '`' not in text and
                             (not isinstance(part, EciText) or
                              not part.character_mode and not part.speed_control))
            if logical[0] == PERSIAN_PROBE.id and ordinary_text:
                runs = _script_runs(text)
                if any(kind == 'en' for kind, _ in runs):
                    english = self._key(US_ENGLISH.id, logical[1])
                    for kind, fragment in runs:
                        select(english if kind == 'en' else logical)
                        output.append(replace(part, text=fragment) if isinstance(part, EciText) else fragment)
                    continue
            if isinstance(part, EciCapitalWord) and logical[0] == PERSIAN_PROBE.id:
                runs = _script_runs(part.original)
                if runs and all(kind == 'en' for kind, _ in runs):
                    select(self._key(US_ENGLISH.id, logical[1]))
                    output.append(part)
                    continue
            if text is not None or isinstance(part, EciCapitalWord):
                select(logical)
            output.append(part)
        try:
            # Sentence splitting is needed only when this request contains a
            # Persian/English switch. Keep unrelated languages on their
            # original native utterance path.
            key, spoken = self._home, set()
            for item in output:
                if isinstance(item, EciLanguage):
                    key = self._key(item.language, item.voice)
                elif isinstance(item, (str, EciText, EciCapitalWord)):
                    value = item if isinstance(item, str) else item.text if isinstance(item, EciText) else item.original
                    if value.strip():
                        spoken.add(key[0])
            allowed = {US_ENGLISH.id, PERSIAN_PROBE.id, PINGLISH_PROBE.id}
            mixed = (spoken <= allowed and US_ENGLISH.id in spoken and
                     bool(spoken & {PERSIAN_PROBE.id, PINGLISH_PROBE.id}))
            jobs = sentence_jobs(output, self._home) if mixed else ()
            if not jobs:
                jobs = (tuple(output),)
            self._cancelled.clear()
            for job in jobs:
                if self._cancelled.is_set():
                    return False
                result = try_continuous(self, job, on_pcm, on_index,
                                        fa_ids=(PERSIAN_PROBE.id, PINGLISH_PROBE.id),
                                        english_id=US_ENGLISH.id, german_id=GERMAN.id,
                                        reset_cancellation=False)
                if result is None and not self._cancelled.is_set():
                    result = try_persian_glides(self, job, on_pcm, on_index,
                                                fa_ids=(PERSIAN_PROBE.id, PINGLISH_PROBE.id))
                if result is None and not self._cancelled.is_set():
                    result = super().speak(job, on_pcm, on_index, reset_cancellation=False)
                if not result or self._cancelled.is_set():
                    return False
            return True
        finally:
            self._activate(self._home)


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
