"""Merge ECI data sources and refresh them without blocking NVDA speech.

Own files remain untouched. Native lexical keys are combined per language and
volume; later own files override earlier files and the community snapshot.
Only complete immutable data directories are handed to the speech worker.
"""
from __future__ import annotations

from dataclasses import dataclass
import hashlib
import json
import os
from pathlib import Path
import tempfile
import threading
from urllib.request import urlopen

if __package__:
    from ._reloq_dictionaries import FILES, LIMIT, SHA, fetch_master, validate_snapshot
else:
    from _reloq_dictionaries import FILES, LIMIT, SHA, fetch_master, validate_snapshot

PREFIXES = {0x10000: "ENU", 0x10001: "ENG", 0x20000: "ESP", 0x20001: "ESM",
            0x30000: "FRA", 0x30001: "FRC", 0x40000: "DEU", 0x50000: "ITA",
            0x60000: "CHS", 0x60001: "CHS", 0x70000: "PTB", 0x80000: "JPN",
            0x90000: "FIN", 0xa0000: "KOR", 0x120000: "ELL"}
SUFFIXES = ("main", "Root", "abbr")
_LOWER = bytes(c + 32 if 65 <= c <= 90 or 0xc0 <= c <= 0xde and c != 0xd7 else c for c in range(256))
NAMES = frozenset(prefix + suffix + ".dic" for prefix in PREFIXES.values() for suffix in SUFFIXES)


@dataclass(frozen=True)
class DictionarySource:
    path: str
    language: int
    volume: int

    def __post_init__(self):
        if (not isinstance(self.path, str) or not self.path or "\0" in self.path
                or not Path(self.path).is_absolute() or type(self.language) is not int
                or self.language not in PREFIXES or type(self.volume) is not int
                or not 0 <= self.volume < 3):
            raise ValueError("invalid own dictionary source")

    @property
    def name(self):
        return PREFIXES[self.language] + SUFFIXES[self.volume] + ".dic"

    def record(self):
        return {"path": self.path, "language": self.language, "volume": self.volume}


def _atomic_json(path, value):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(mode="w", encoding="utf8", dir=path.parent,
                                     prefix="write-", suffix=".json", delete=False) as file:
        temporary = Path(file.name)
        json.dump(value, file, ensure_ascii=False, indent=2)
        file.write("\n")
    try:
        os.replace(temporary, path)
    finally:
        temporary.unlink(missing_ok=True)


def load_sources(cache):
    try:
        data = json.loads((Path(cache) / "sources.json").read_text(encoding="utf8"))
    except FileNotFoundError:
        return ()
    if not isinstance(data, dict) or data.get("version") != 1 or not isinstance(data.get("sources"), list):
        raise ValueError("invalid dictionary source settings")
    return tuple(DictionarySource(**row) for row in data["sources"])


def _read(path):
    with Path(path).open("rb") as stream:
        data = stream.read(LIMIT + 1)
    if len(data) > LIMIT:
        raise ValueError(f"dictionary exceeds 8 MiB: {path}")
    if b"\0" in data:
        raise ValueError(f"dictionary contains a NUL character: {path}")
    return data


def _source_state(sources):
    """Track writes and editor-style atomic replacements without reading PCM or data."""
    result = []
    for source in sources:
        try:
            stat = Path(source.path).stat()
            value = (stat.st_dev, stat.st_ino, stat.st_size, stat.st_mtime_ns, stat.st_ctime_ns)
        except OSError as error:
            value = (error.errno,)
        result.append((source, value))
    return tuple(result)


def validate_sources(sources):
    """Validate selected local data before NVDA closes its settings dialog."""
    for source in sources:
        tuple(_entries(_read(source.path), source.path))


def _entries(data, name):
    # Native dictionaries use CP1252 bytes and ASCII whitespace. No UTF-8
    # transcoding, phoneme rewriting, word splitting or speech normalization.
    for number, line in enumerate(data.splitlines(), 1):
        if b"\t" not in line:
            continue
        key, value = line.split(b"\t", 1)
        key = key.strip(b" \t\r\n\v\f")
        value = value.strip(b" \t\r\n\v\f")
        if not key or not value:
            continue
        if len(key) > 128 or len(value) > 512:
            raise ValueError(f"dictionary entry too long: {name}, line {number}")
        yield key, value


def validate_merged(directory):
    directory = Path(directory)
    manifest = json.loads((directory / "manifest.json").read_text(encoding="utf8"))
    if manifest.get("format") != "reloq-native-dictionaries-1":
        raise ValueError("invalid merged dictionary snapshot")
    for name, row in manifest["files"].items():
        if name not in NAMES:
            raise ValueError("invalid dictionary filename")
        data = _read(directory / name)
        if len(data) != row["size"] or hashlib.sha256(data).hexdigest() != row["sha256"]:
            raise ValueError(f"invalid merged dictionary data: {name}")
    return manifest


def merge_sources(community, sources, cache):
    tables = {}
    conflicts = 0
    commit = validate_snapshot(community)["commit"] if community else None

    def add(name, data):
        nonlocal conflicts
        table = tables.setdefault(name, {})
        for key, value in _entries(data, name):
            # German lexical keys use the same case folding as the native
            # word table. Preserve the winning source's actual key/value bytes.
            identity = key.translate(_LOWER) if name in ("DEUmain.dic", "DEURoot.dic") else key
            if identity in table:
                conflicts += 1
            table[identity] = (key, value)

    if community:
        for name in FILES:
            if name.endswith(".dic"):
                add(name, _read(Path(community) / name))
    for source in sources:
        add(source.name, _read(source.path))
    encoded = {name: b"".join(key + b"\t" + value + b"\n" for key, value in sorted(table.values()))
               for name, table in tables.items()}
    if any(len(data) > LIMIT for data in encoded.values()):
        raise ValueError("merged dictionary exceeds 8 MiB")
    identity = hashlib.sha256(b"reloq-native-dictionaries-1\0")
    for name, data in sorted(encoded.items()):
        identity.update(name.encode("ascii") + b"\0" + str(len(data)).encode("ascii") + b"\0" + data)
    directory = Path(cache) / "merged"
    directory.mkdir(parents=True, exist_ok=True)
    target = directory / identity.hexdigest()
    manifest = {"format": "reloq-native-dictionaries-1", "community_commit": commit,
                "own_sources": len(sources), "conflicts": conflicts,
                "files": {name: {"size": len(data), "sha256": hashlib.sha256(data).hexdigest(),
                                 "entries": len(tables[name])} for name, data in encoded.items()}}
    if target.is_dir():
        validate_merged(target)
        return target, manifest
    with tempfile.TemporaryDirectory(prefix="merge-", dir=directory) as temporary:
        stage = Path(temporary) / "snapshot"
        stage.mkdir()
        for name, data in encoded.items():
            (stage / name).write_bytes(data)
        (stage / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf8")
        validate_merged(stage)
        try:
            stage.rename(target)
        except OSError:
            if not target.is_dir():
                raise
            validate_merged(target)
    return target, manifest


class DictionaryManager:
    def __init__(self, bundled, cache, deliver, on_error, *, fetch=urlopen, watch_interval=0.5):
        self.bundled, self.cache = Path(bundled), Path(cache)
        self.deliver, self.on_error, self.fetch = deliver, on_error, fetch
        self._lock = threading.Lock()
        self._revision, self._closed, self._enabled = 0, False, False
        self._last_delivery = None
        self._working_revision = None
        self._watch_interval = watch_interval
        self._watch_event = threading.Event()
        self._watch_thread = None
        self._watched_state = None
        try:
            self._sources = load_sources(self.cache)
        except (OSError, ValueError, TypeError, KeyError) as error:
            self._sources = ()
            self.on_error(error)
        self._committed_sources = self._sources

    @property
    def sources(self):
        with self._lock:
            return self._sources

    def current(self, revision):
        with self._lock:
            return not self._closed and revision == self._revision

    def _community(self):
        try:
            commit = json.loads((self.cache / "current.json").read_text(encoding="ascii"))["commit"]
            if not isinstance(commit, str) or not SHA.fullmatch(commit):
                raise ValueError("invalid cached master commit")
            path = self.cache / commit
            validate_snapshot(path)
            return path
        except (OSError, ValueError, KeyError, TypeError):
            validate_snapshot(self.bundled)
            return self.bundled

    def _emit(self, revision, path):
        with self._lock:
            if self._closed or revision != self._revision or self._last_delivery == (revision, path):
                return
            self._last_delivery = (revision, path)
        self.deliver(revision, path)

    def set_enabled(self, enabled):
        if type(enabled) is not bool:
            raise ValueError("community dictionary setting must be boolean")
        with self._lock:
            self._enabled = enabled
        return self._request(update=enabled)

    def set_sources(self, sources, completed=None):
        sources = tuple(source if isinstance(source, DictionarySource) else DictionarySource(**source)
                        for source in sources)
        with self._lock:
            self._sources = sources
        return self._request(update=False, completed=completed, save=True)

    def configure(self, sources, enabled, completed=None):
        """Apply a settings panel's sources and switch as one revision."""
        if type(enabled) is not bool:
            raise ValueError("community dictionary setting must be boolean")
        sources = tuple(source if isinstance(source, DictionarySource) else DictionarySource(**source)
                        for source in sources)
        with self._lock:
            self._sources, self._enabled = sources, enabled
        return self._request(update=False, completed=completed, save=True)

    def refresh(self, completed=None):
        # Reload every selected local source too. The GitHub setting controls
        # whether a network request is made, independently of own files.
        with self._lock:
            enabled = self._enabled
        return self._request(update=enabled, completed=completed)

    def _request(self, *, update, completed=None, save=False, watch_revision=None):
        with self._lock:
            if self._closed:
                return None
            if watch_revision is not None and (self._revision != watch_revision or self._working_revision is not None):
                return None
            self._revision += 1
            revision, enabled, sources = self._revision, self._enabled, self._sources
            self._working_revision = revision
            if sources and self._watch_thread is None:
                self._watch_thread = threading.Thread(target=self._watch,
                    name="Eloquence dictionary file monitor", daemon=True)
                self._watch_thread.start()
        self._watch_event.set()
        if not enabled and not sources:
            self._emit(revision, None)
        elif enabled and not sources:
            self._emit(revision, self._community())
        threading.Thread(target=self._work, args=(revision, enabled, sources, update, completed, save),
                         name="Eloquence dictionary update", daemon=True).start()
        return revision

    def _watch(self):
        # Two matching observations debounce ordinary writes. Stat also
        # follows a replacement at the original filename after an editor
        # renames its temporary file. No network request or speech-thread I/O.
        candidate = None
        while True:
            with self._lock:
                if self._closed:
                    return
                sources = self._committed_sources
                busy, accepted = self._working_revision is not None, self._watched_state
                revision = self._revision
            if sources and not busy:
                state = _source_state(sources)
                if state != accepted and state == candidate:
                    candidate = None
                    self._request(update=False, watch_revision=revision)
                else:
                    candidate = state if state != accepted else None
            self._watch_event.wait(self._watch_interval if sources else None)
            self._watch_event.clear()

    def _build(self, community, sources):
        if not sources:
            if community:
                manifest = validate_snapshot(community)
                return community, {"community_commit": manifest["commit"], "own_sources": 0,
                                   "files": {n: {} for n in FILES if n.endswith(".dic")}}
            return None, {"community_commit": None, "own_sources": 0, "files": {}}
        return merge_sources(community, sources, self.cache)

    def _work(self, revision, enabled, sources, update, completed, save):
        state = _source_state(sources)
        committed = False
        try:
            community = self._community() if enabled else None
            path, details = self._build(community, sources)
            if _source_state(sources) != state:
                raise OSError("dictionary source changed while it was being read")
            with self._lock:
                if self._closed or self._revision != revision:
                    return
                if save:
                    _atomic_json(self.cache / "sources.json", {"version": 1, "sources": [s.record() for s in sources]})
                self._committed_sources = sources
                self._watched_state = state
                committed = True
            self._emit(revision, path)
            if update:
                latest = fetch_master(self.cache, fetch=self.fetch, publish=False)
                if enabled and (community is None or latest.name != validate_snapshot(community)["commit"]):
                    path, details = self._build(latest, sources)
                with self._lock:
                    if not self._closed and self._revision == revision:
                        _atomic_json(self.cache / "current.json", {"commit": latest.name})
            if not self.current(revision):
                return
            self._emit(revision, path)
            if completed:
                completed(None, details)
        except Exception as error:
            with self._lock:
                active = not self._closed and self._revision == revision
                if active:
                    if not committed:
                        self._sources = self._committed_sources
                    # Report an invalid saved file once. Its next write,
                    # disappearance or return will be checked automatically.
                    self._watched_state = state
            if active:
                self.on_error(error)
                if completed:
                    completed(error, None)
        finally:
            with self._lock:
                if self._working_revision == revision:
                    self._working_revision = None
            self._watch_event.set()

    def close(self):
        with self._lock:
            self._closed = True
            self._revision += 1
        self._watch_event.set()
