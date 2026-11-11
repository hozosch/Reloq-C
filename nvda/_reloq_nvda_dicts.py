"""Copy and apply NVDA speech dictionaries for explicit language/voice scopes.

The source remains untouched. Copies retain NVDA's own dictionary format,
including regular expressions, case sensitivity, entry order and comments.
"""
from __future__ import annotations

import hashlib
import os
from pathlib import Path
import re
import threading

_NAME = re.compile(r"^([0-9a-f]{1,8})-(all|[1-8])-([0-9a-f]{16})-(.+)\.dic$", re.I)
_MAX_BYTES = 8 * 1024 * 1024


def _classes():
    try:
        from speechDictHandler.types import SpeechDict, SpeechDictEntry, EntryType
        return SpeechDict, SpeechDictEntry, EntryType
    except ImportError:
        from speechDictHandler import SpeechDict, SpeechDictEntry
        return SpeechDict, SpeechDictEntry, None


def _validate(data, *, allow_empty=False):
    if len(data) > _MAX_BYTES or b"\0" in data:
        raise ValueError("invalid or oversized NVDA speech dictionary")
    text = data.decode("utf-8-sig")
    _dict, entry_class, enum = _classes()
    count = 0
    for number, line in enumerate(text.splitlines(), 1):
        if not line.strip() or line.startswith("#"):
            continue
        fields = line.split("\t")
        if len(fields) != 4 or fields[2] not in ("0", "1"):
            raise ValueError(f"invalid NVDA speech dictionary line {number}")
        try:
            kind = int(fields[3])
            if enum is None:
                if kind not in (0, 1, 2): raise ValueError("unsupported entry type")
            else:
                kind = enum(kind)
            entry_class(fields[0].replace(r"\#", "#"), fields[1].replace(r"\#", "#"), "",
                        caseSensitive=bool(int(fields[2])), type=kind)
        except Exception as error:
            raise ValueError(f"invalid NVDA speech dictionary line {number}") from error
        count += 1
    if not count and not allow_empty:
        raise ValueError("the selected file contains no NVDA speech dictionary entries")
    return count


def dictionary_format(data):
    """Identify a complete NVDA speech dictionary; otherwise use native ECI.

    NVDA stores two numeric options in separate tab-delimited columns. An
    ECI replacement can itself contain tabs, so callers can override the
    suggested format when selecting a file.
    """
    if len(data) > _MAX_BYTES or b"\0" in data:
        raise ValueError("invalid or oversized dictionary")
    try:
        text = data.decode("utf-8-sig")
    except UnicodeDecodeError:
        return "eci"
    lines = [line for line in text.splitlines() if line.strip() and not line.startswith("#")]
    if not lines:
        return "eci"
    fields = [line.split("\t") for line in lines]
    if all(len(row) == 4 and row[2] in ("0", "1") and row[3].isdigit() for row in fields):
        # Invalid options or a broken regex must not be treated as native
        # text, where those option columns would be spoken as replacements.
        _validate(data)
        return "nvda"
    if any(len(row) == 4 and row[2] in ("0", "1") and row[3].isdigit() for row in fields):
        raise ValueError("mixed or invalid NVDA speech dictionary")
    return "eci"


def identify_file(path):
    with Path(path).open("rb") as stream:
        return dictionary_format(stream.read(_MAX_BYTES + 1))


def migrate_nvda_sources(sources, imported):
    """Move wrongly selected NVDA files to their matching speech-dict path.

    The original is never changed. Native sources continue in their original
    order; NVDA entries retain case, match type, comments and replacement.
    """
    native, migrated = [], []
    for source in sources:
        if getattr(source, "format", None) == "eci":
            native.append(source)
            continue
        try:
            format_name = identify_file(source.path)
        except OSError:
            native.append(source)
            continue
        if format_name == "nvda":
            migrated.append(source)
        else:
            native.append(source)
    # Validate the complete selection before creating any imported copies.
    created = []
    try:
        for source in migrated:
            path, _count, fresh = imported.import_file(source.path, source.language, "all")
            if fresh:
                created.append(path)
    except BaseException:
        for path in reversed(created):
            try:
                imported.remove(path)
            except (OSError, ValueError):
                pass
        raise
    return tuple(native), tuple(migrated)


class ImportedDictionaries:
    def __init__(self, profile):
        self.directory = Path(profile) / "reloqNVDAImported"
        self._lock = threading.RLock()
        self._records = ()
        self.errors = ()
        self.reload()

    @property
    def records(self):
        with self._lock:
            return self._records

    def reload(self):
        result, errors = [], []
        if self.directory.is_dir():
            dictionary_class, _, _ = _classes()
            for path in sorted(self.directory.glob("*.dic"), key=lambda p: p.name.casefold()):
                match = _NAME.fullmatch(path.name)
                if not match or path.is_symlink() or not path.is_file():
                    continue
                try:
                    with path.open("rb") as stream:
                        data = stream.read(_MAX_BYTES + 1)
                    _validate(data, allow_empty=True)
                    dictionary = dictionary_class()
                    dictionary.load(str(path))
                    result.append((path, int(match[1], 16), match[2].lower(), dictionary))
                except (OSError, UnicodeError, ValueError) as error:
                    errors.append((str(path), str(error)))
        with self._lock:
            self._records = tuple(result)
            self.errors = tuple(errors)

    def import_file(self, source, language, voice):
        if type(language) is not int or language < 0 or language > 0xffffffff:
            raise ValueError("invalid language")
        if voice not in ("all", "1", "2", "3", "4", "5", "6", "7", "8"):
            raise ValueError("invalid voice")
        source = Path(source)
        with source.open("rb") as stream:
            data = stream.read(_MAX_BYTES + 1)
        # An intentionally empty NVDA dictionary is still useful: the copy
        # can be opened in NVDA's editor and filled in after import.
        count = _validate(data, allow_empty=True)
        stem = re.sub(r"[^A-Za-z0-9_.-]+", "_", source.stem).strip("._")[:60] or "dictionary"
        digest = hashlib.sha256(data).hexdigest()[:16]
        self.directory.mkdir(parents=True, exist_ok=True)
        prefix = f"{language:x}-{voice}-{digest}-"
        for existing in self.directory.glob(prefix + "*.dic"):
            if existing.read_bytes() == data:
                return existing, count, False
        target = self.directory / (prefix + stem + ".dic")
        created = False
        try:
            with target.open("xb") as stream:
                created = True
                stream.write(data)
            self.reload()
        except BaseException:
            if created:
                try: target.unlink()
                except FileNotFoundError: pass
            raise
        return target, count, True

    def remove(self, path):
        path = Path(path)
        if not any(record[0] == path for record in self.records):
            raise ValueError("unknown imported dictionary")
        path.unlink()
        self.reload()

    def process(self, text, language, voice):
        for _, record_language, record_voice, dictionary in self.records:
            if record_language == language and record_voice in ("all", str(voice)):
                text = dictionary.sub(text)
        return text

    def matches(self, text, language, voice):
        for _, record_language, record_voice, dictionary in self.records:
            if record_language == language and record_voice in ("all", str(voice)):
                if any(entry.compiled.search(text) for entry in dictionary):
                    return True
        return False
