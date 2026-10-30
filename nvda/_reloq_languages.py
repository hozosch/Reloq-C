"""Language/voice interface shared by the NVDA driver and its worker.

Only implemented native modules belong in LANGUAGES. Adding a language requires
its native implementation, UTF-8 admission and character names; the
NVDA settings and language-command routing do not contain German special cases.
"""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import threading
from typing import Callable


def N_(message: str) -> str:
    """Mark English UI labels for extraction; the NVDA UI translates them.

    This shared worker module also runs without NVDA. Speech normalization,
    language IDs and saved voice IDs must never depend on the UI locale.
    """
    return message

if __package__:
    from ._reloq_eci_direct import DirectEciEngine, EciLanguage, EciPitch, GERMAN_DIALECT, ENGLISH_DIALECT, NATIVE_PITCH, NATIVE_INFLECTION, NATIVE_VOLUME, HEAD_SIZE, ROUGHNESS, BREATHINESS, GENDER
else:
    from _reloq_eci_direct import DirectEciEngine, EciLanguage, EciPitch, GERMAN_DIALECT, ENGLISH_DIALECT, NATIVE_PITCH, NATIVE_INFLECTION, NATIVE_VOLUME, HEAD_SIZE, ROUGHNESS, BREATHINESS, GENDER


@dataclass(frozen=True)
class Voice:
    id: str
    name: str
    parameters: tuple[tuple[int, int], ...] = ()
    aliases: tuple[str, ...] = ()


@dataclass(frozen=True)
class Language:
    id: int
    name: str
    locale: str
    encoding: str
    voices: tuple[Voice, ...]

    def voice(self, value: str | None) -> Voice:
        # Old ReLoq profiles used a language-specific voice ID. OpenEVV also
        # used language:preset IDs; the language now has its own setting.
        value = str(value).rsplit(":", 1)[-1] if value is not None else None
        for voice in self.voices:
            if value == voice.id or value in voice.aliases:
                return voice
        if value is None:
            return self.voices[0]
        raise ValueError(f"voice {value!r} is unavailable for {self.locale}")


def _preset(ident, name, gender, head, pitch, inflection, roughness, breathiness, volume):
    # Shared native voice parameter sets. Rate belongs to
    # the NVDA user and is deliberately preserved when a preset changes.
    parameters = tuple(zip((GENDER, HEAD_SIZE, NATIVE_PITCH, NATIVE_INFLECTION,
                            ROUGHNESS, BREATHINESS, NATIVE_VOLUME),
                           (gender, head, pitch, inflection, roughness, breathiness, volume)))
    return Voice(str(ident), name, parameters,
                 ("deu_experimental",) if ident == 1 else ())


GERMAN = Language(
    GERMAN_DIALECT, N_("German"), "de_DE", "utf-8",
    (
        # Eloquence_64's preset names; IDs and anatomy settings are stable.
        _preset(1, "Reed", 0, 50, 65, 30, 0, 0, 92),
        _preset(2, "Shelly", 1, 50, 81, 30, 0, 50, 100),
        _preset(3, "Bobby", 1, 22, 93, 35, 0, 0, 90),
        _preset(4, "Rocko", 0, 86, 56, 47, 0, 0, 93),
        _preset(5, "Glen", 0, 50, 69, 34, 0, 0, 92),
        _preset(6, "Sandy", 1, 56, 89, 35, 0, 40, 95),
        _preset(7, "Grandma", 1, 45, 68, 30, 3, 40, 90),
        _preset(8, "Grandpa", 0, 30, 61, 44, 18, 20, 90),
    ),
)
# Anatomy presets are shared. Pronunciation and acoustic rules belong to
# the selected language; choosing a preset never switches language modules.
US_ENGLISH = Language(
    ENGLISH_DIALECT, "American English", "en_US", "utf-8",
    tuple(Voice(v.id, v.name, v.parameters) for v in GERMAN.voices),
)
BRAZILIAN_PORTUGUESE = Language(
    0x70000, "Brazilian Portuguese", "pt_BR", "utf-8",
    tuple(Voice(v.id, v.name, v.parameters) for v in GERMAN.voices),
)
FINNISH = Language(
    0x90000, N_("Finnish"), "fi_FI", "utf-8",
    tuple(Voice(v.id, v.name, v.parameters) for v in GERMAN.voices),
)
BRITISH_ENGLISH = Language(0x10001, "British English", "en_GB", "utf-8", US_ENGLISH.voices)
SPANISH_SPAIN = Language(0x20000, "Castilian Spanish", "es_ES", "utf-8", US_ENGLISH.voices)
SPANISH_MEXICO = Language(0x20001, "Latin American Spanish", "es_MX", "utf-8", US_ENGLISH.voices)
FRENCH_FRANCE = Language(0x30000, "French", "fr_FR", "utf-8", US_ENGLISH.voices)
FRENCH_CANADA = Language(0x30001, "French Canadian", "fr_CA", "utf-8", US_ENGLISH.voices)
ITALIAN = Language(0x50000, N_("Italian"), "it_IT", "utf-8", US_ENGLISH.voices)
CHINESE_CHINA = Language(0x60000, "Mandarin Chinese", "zh_CN", "utf-8", US_ENGLISH.voices)
# Both scripts use this native module. Accept saved Taiwan IDs without
# presenting a second language choice or opening a second worker.
LANGUAGE_ALIASES = {0x60001: 0x60000}
JAPANESE = Language(0x80000, N_("Japanese"), "ja_JP", "utf-8", US_ENGLISH.voices)
KOREAN = Language(0xa0000, N_("Korean"), "ko_KR", "utf-8", US_ENGLISH.voices)
GREEK = Language(0x120000, N_("Greek"), "el_GR", "utf-8", US_ENGLISH.voices)
LANGUAGES = {p.id: p for p in (GERMAN, US_ENGLISH, BRITISH_ENGLISH,
    SPANISH_SPAIN, SPANISH_MEXICO, FRENCH_FRANCE, FRENCH_CANADA, ITALIAN,
    BRAZILIAN_PORTUGUESE, FINNISH, CHINESE_CHINA, JAPANESE, KOREAN, GREEK)}
# Eloquence_64's shipped module order, with Greek at the end. Presentation
# does not change the preference for a bare language command such as "en".
LANGUAGE_DISPLAY_ORDER = (0x60000, 0x40000, 0x10001, 0x10000, 0x20001,
    0x20000, 0x90000, 0x30000, 0x30001, 0x50000, 0x80000, 0xa0000,
    0x70000, 0x120000)


def language_for(locale: str | None, home: int, languages=LANGUAGES) -> int | None:
    """NVDA None restores home; an unsupported locale leaves speech alone."""
    if not locale:
        return LANGUAGE_ALIASES.get(home, home)
    requested = locale.replace("-", "_").casefold()
    if home==GREEK.id and requested.split("_",1)[0]=="en":
        return home
    for language in languages.values():
        if requested == language.locale.replace("-", "_").casefold():
            return language.id
    root = requested.split("_", 1)[0]
    candidates = [language.id for language in languages.values()
                  if language.locale.replace("-", "_").casefold().split("_", 1)[0] == root]
    return home if home in candidates else candidates[0] if candidates else None


class LanguageEngine:
    """Worker-owned engines with home selection and scoped language changes.

    Persistent controls survive cancel in EciTextTransport. Temporary language
    switches stay inside one transport job, so only its final playback reports
    completion. Repeated or unsupported language marks never split a sentence.
    All native handles are created, configured and closed by the worker.
    """

    def __init__(self, dll: str | Path, *, language: int = GERMAN.id,
                 voice: str = "1", languages=None, engine_factory=DirectEciEngine):
        self._dll = Path(dll)
        self._languages = dict(LANGUAGES if languages is None else languages)
        self._factory = engine_factory
        self._owner = threading.get_ident()
        self._guard = threading.RLock()
        self._cancelled = threading.Event()
        self._engines = {}
        self._engine_keys = {}
        self._engine_parameters = {}
        self._parameters = {}
        self._dictionary_path = None
        self._active = None
        self._closed = False
        self.select_voice(language, voice)

    def _assert_owner(self):
        if threading.get_ident() != self._owner:
            raise RuntimeError("language engine belongs to its creating worker")
        if self._closed:
            raise RuntimeError("language engine has been closed")

    def _key(self, language, voice):
        self._assert_owner()
        language = LANGUAGE_ALIASES.get(language, language)
        profile = self._languages[language]
        return language, profile.voice(voice).id

    def _parameter_for(self, key, parameter, value):
        # A scoped language change may fall back to a different available
        # preset. Restrict model controls only if that language's catalog
        # actually declares a limited model set. Keep the user's home
        # settings intact for the return to that language.
        profile = self._languages[key[0]]
        if parameter == GENDER:
            supported = {dict(v.parameters)[GENDER] for v in profile.voices
                         if GENDER in dict(v.parameters)}
            if supported and value not in supported:
                return dict(profile.voice(key[1]).parameters).get(GENDER, value)
        return value

    def _activate(self, key):
        self._assert_owner()
        profile = self._languages[key[0]]
        language = key[0]
        engine = self._engines.get(language)
        created = engine is None
        if created:
            engine = self._factory(
                self._dll, dialect=profile.id, encoding=profile.encoding,
                experimental_generic=True,
            )
            self._engines[language] = engine
            self._engine_parameters[language] = {}
        try:
            if created and self._dictionary_path is not None:
                engine.set_dictionary_path(self._dictionary_path)
            wanted = dict(profile.voice(key[1]).parameters)
            wanted.update((parameter, self._parameter_for(key, parameter, value))
                          for parameter, value in self._parameters.items())
            current = self._engine_parameters[language]
            for parameter, value in wanted.items():
                if current.get(parameter) != value:
                    engine.set_parameter(parameter, value)
                    current[parameter] = value
            self._engine_keys[language] = key
        except BaseException:
            if created:
                del self._engines[language]
                del self._engine_parameters[language]
                engine.close()
            raise
        with self._guard:
            self._active = engine
        return engine

    def select_voice(self, language, voice):
        key = self._key(language, voice)
        if hasattr(self, "_home") and self._home != key:
            # One queued selection changes the complete preset atomically.
            # Cached engines must not reapply the previous voice's gender
            # or source controls when the user switches back to a voice.
            for parameter, value in self._languages[key[0]].voice(key[1]).parameters:
                self.set_parameter(parameter, value)
        self._activate(key)
        self._home = key

    def set_dictionary_path(self, path):
        self._assert_owner()
        # NVDA passes immutable, content-addressed dictionary snapshots.
        # Repeated settings/update notifications for the same snapshot do
        # not replace every already initialized language and its tables.
        path = str(path) if path is not None else None
        if path == self._dictionary_path:
            return
        changed = []
        try:
            for engine in self._engines.values():
                engine.set_dictionary_path(path)
                changed.append(engine)
        except BaseException:
            for engine in changed:
                engine.set_dictionary_path(self._dictionary_path)
            raise
        self._dictionary_path = path

    def set_parameter(self, parameter, value):
        self._assert_owner()
        for language, engine in self._engines.items():
            adapted = self._parameter_for(self._engine_keys[language], parameter, value)
            engine.set_parameter(parameter, adapted)
            self._engine_parameters[language][parameter] = adapted
        self._parameters[parameter] = value

    def speak(self, parts, on_pcm, on_index):
        self._assert_owner()
        # Validate all selections before any speech escapes this job.
        for part in parts:
            if isinstance(part, EciLanguage):
                self._key(part.language, part.voice)
        self._cancelled.clear()
        key = self._home
        pending = []
        pitch = None

        def flush():
            if self._cancelled.is_set():
                return False
            if not pending:
                return True
            engine = self._activate(key)
            if self._cancelled.is_set():
                return False
            result = engine.speak(tuple(pending), on_pcm, on_index)
            pending.clear()
            return result and not self._cancelled.is_set()

        try:
            for part in parts:
                if isinstance(part, EciLanguage):
                    next_key = self._key(part.language, part.voice)
                    if next_key == key:
                        continue
                    if not flush():
                        return False
                    key = next_key
                    # Capital/spelling pitch remains active across a real
                    # language boundary until NVDA sends its reset command.
                    if pitch is not None:
                        pending.append(pitch)
                else:
                    if isinstance(part, EciPitch):
                        pitch = part
                    pending.append(part)
            return flush()
        finally:
            self._activate(self._home)

    def cancel(self):
        self._cancelled.set()
        with self._guard:
            if self._active is not None:
                self._active.cancel()

    def close(self):
        self._assert_owner()
        with self._guard:
            self._active = None
            self._closed = True
            engines, self._engines = tuple(self._engines.values()), {}
            self._engine_keys.clear()
            self._engine_parameters.clear()
        # Closing a native dictionary can be expensive. Cancel needs this
        # guard on NVDA's UI thread and must see the detached state at once.
        for engine in engines:
            engine.close()
