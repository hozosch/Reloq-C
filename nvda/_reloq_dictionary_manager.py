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
import shutil
import tempfile
import threading
from urllib.request import urlopen

if __package__:
    from ._reloq_dictionaries import FILES, LIMIT, SHA, _blob, fetch_master, validate_snapshot
else:
    from _reloq_dictionaries import FILES, LIMIT, SHA, _blob, fetch_master, validate_snapshot

PREFIXES = {0x10000: "ENU", 0x10001: "ENG", 0x20000: "ESP", 0x20001: "ESM",
            0x30000: "FRA", 0x30001: "FRC", 0x40000: "DEU", 0x50000: "ITA",
            0x60000: "CHS", 0x60001: "CHS", 0x70000: "PTB", 0x80000: "JPN",
            0x90000: "FIN", 0xa0000: "KOR", 0x120000: "ELL"}
SUFFIXES = ("main", "Root", "abbr")
_LOWER = bytes(c + 32 if 65 <= c <= 90 or 0xc0 <= c <= 0xde and c != 0xd7 else c for c in range(256))
NAMES = frozenset(prefix + suffix + ".dic" for prefix in PREFIXES.values() for suffix in SUFFIXES)


def directory_sources(directory):
    """Discover canonical volume filenames, including newly created files."""
    found = {}
    for language, prefix in PREFIXES.items():
        for volume, suffix in enumerate(SUFFIXES):
            path = Path(directory) / (prefix + suffix + ".dic")
            if path.is_file():
                found.setdefault(path.name, DictionarySource(str(path.resolve()), language, volume))
    return tuple(found[name] for name in sorted(found))


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


def load_sources(cache, *, settings_path=None):
    try:
        path = Path(settings_path) if settings_path is not None else Path(cache) / "sources.json"
        data = json.loads(path.read_text(encoding="utf8"))
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


def merge_sources(community, sources, cache, *, replacements=None):
    tables = {}
    conflicts = 0
    commit = validate_snapshot(community)["commit"] if community else None

    def add(name, data):
        nonlocal conflicts
        table = tables.setdefault(name, {})
        for key, value in _entries(data, name):
            # Root lookup folds German keys; Main is a case-sensitive text
            # normalization stage and must retain distinct spelling variants.
            identity = key.translate(_LOWER) if name == "DEURoot.dic" else key
            if identity in table:
                conflicts += 1
            table[identity] = (key, value)

    if community:
        for name in FILES:
            if name.endswith(".dic"):
                add(name, _read(Path(community) / name))
    for name, data in (replacements or {}).items():
        if name not in NAMES:
            raise ValueError("invalid local dictionary filename")
        tables[name] = {}
        add(name, data)
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
    def __init__(self, bundled, cache, deliver, on_error, *, fetch=urlopen, watch_interval=0.5,
                 settings_path=None, legacy_settings=None):
        self.bundled, self.cache = Path(bundled), Path(cache)
        self.settings_path = Path(settings_path) if settings_path is not None else self.cache / "sources.json"
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
            if not self.settings_path.exists() and legacy_settings is not None and Path(legacy_settings).is_file():
                self._sources = load_sources(self.cache, settings_path=legacy_settings)
                _atomic_json(self.settings_path, {"version": 1, "sources": [s.record() for s in self._sources]})
            else:
                self._sources = load_sources(self.cache, settings_path=self.settings_path)
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
            try:
                manifest = validate_snapshot(self.bundled)
                target = self.cache / manifest["commit"]
                if not target.exists():
                    self.cache.mkdir(parents=True, exist_ok=True)
                    with tempfile.TemporaryDirectory(prefix="installed-", dir=self.cache) as temporary:
                        stage = Path(temporary) / "snapshot"
                        stage.mkdir()
                        for name in (*FILES, "manifest.json"):
                            shutil.copyfile(self.bundled / name, stage / name)
                        validate_snapshot(stage)
                        try:stage.rename(target)
                        except OSError:
                            if not target.is_dir():raise
                validate_snapshot(target)
                return target
            except (OSError, ValueError, KeyError, TypeError):
                # The installed dictionary folder is also user editable.
                # Treat those bytes as local data, never as a verified
                # download or as a reason to discard all other volumes.
                return None

    def _install_latest(self, snapshot):
        """Refresh the editable installed files, retaining locally edited volumes."""
        latest = validate_snapshot(snapshot)
        previous = json.loads((self.bundled / "manifest.json").read_text(encoding="utf8"))
        for name in FILES:
            path = self.bundled / name
            if path.exists():
                current = _read(path)
                # A local change, including a new file, keeps precedence.
                if name not in previous["files"] or _blob(current) != previous["files"][name]["sha"]:
                    continue
                if _blob(current) == latest["files"][name]["sha"]:
                    continue
            elif name in previous["files"]:
                continue  # A locally removed volume stays removed.
            with tempfile.NamedTemporaryFile(dir=self.bundled, prefix="update-", delete=False) as stream:
                temporary = Path(stream.name)
                stream.write(_read(Path(snapshot) / name))
            try:os.replace(temporary, path)
            finally:temporary.unlink(missing_ok=True)
        _atomic_json(self.bundled / "manifest.json", latest)

    def _state(self, enabled, sources):
        paths = [Path(s.path) for s in sources]
        folders = [self.bundled] if enabled else []
        for folder in folders:
            paths.extend(folder / name for name in sorted(NAMES))
        result = []
        for path in paths:
            try:
                st = path.stat()
                value = (st.st_dev, st.st_ino, st.st_size, st.st_mtime_ns, st.st_ctime_ns)
            except OSError as error:
                value = (error.errno,)
            result.append((str(path), value))
        return tuple(result)

    def _bundle_changes(self):
        manifest = json.loads((self.bundled / "manifest.json").read_text(encoding="utf8"))
        expected = manifest["files"]
        changes = {}
        for name in NAMES:
            path = self.bundled / name
            if not path.exists():
                if name in expected:
                    changes[name] = b""
                continue
            data = _read(path)
            if name not in expected or _blob(data) != expected[name]["sha"]:
                changes[name] = data
        return changes

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
            if self._watch_thread is None:
                self._watch_thread = threading.Thread(target=self._watch,
                    name="Eloquence dictionary file monitor", daemon=True)
                self._watch_thread.start()
        self._watch_event.set()
        if not enabled and not sources:
            self._emit(revision, None)
        # File verification and merging belong to the background worker.
        # In particular, do not emit an older cached set before applying a
        # locally edited bundled volume, or read a large root file on the UI.
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
                enabled = self._enabled
                busy, accepted = self._working_revision is not None, self._watched_state
                revision = self._revision
            if not busy:
                state = self._state(enabled, sources)
                if state != accepted and state == candidate:
                    candidate = None
                    self._request(update=False, watch_revision=revision)
                else:
                    candidate = state if state != accepted else None
            self._watch_event.wait(self._watch_interval)
            self._watch_event.clear()

    def _build(self, community, sources, enabled):
        replacements = self._bundle_changes() if enabled else {}
        if enabled and community is None:
            replacements = {s.name: _read(s.path) for s in directory_sources(self.bundled)}
        if not sources and not replacements:
            if community:
                manifest = validate_snapshot(community)
                return community, {"community_commit": manifest["commit"], "own_sources": 0,
                                   "files": {n: {} for n in FILES if n.endswith(".dic")}}
            return None, {"community_commit": None, "own_sources": 0, "files": {}}
        return merge_sources(community, sources, self.cache, replacements=replacements)

    def _work(self, revision, enabled, sources, update, completed, save):
        state = self._state(enabled, sources)
        committed = False
        try:
            community = self._community() if enabled else None
            path, details = self._build(community, sources, enabled)
            if self._state(enabled, sources) != state:
                raise OSError("dictionary source changed while it was being read")
            with self._lock:
                if self._closed or self._revision != revision:
                    return
                if save:
                    _atomic_json(self.settings_path, {"version": 1, "sources": [s.record() for s in sources]})
                self._committed_sources = sources
                self._watched_state = state
                committed = True
            self._emit(revision, path)
            if update:
                latest = fetch_master(self.cache, fetch=self.fetch, publish=False)
                if not self.current(revision):
                    return
                self._install_latest(latest)
                installed_state = self._state(enabled, sources)
                if enabled and (community is None or installed_state != state or
                                latest.name != validate_snapshot(community)["commit"]):
                    path, details = self._build(latest, sources, enabled)
                if self._state(enabled, sources) != installed_state:
                    raise OSError("dictionary source changed while it was being read")
                with self._lock:
                    if not self._closed and self._revision == revision:
                        _atomic_json(self.cache / "current.json", {"commit": latest.name})
                        self._watched_state = installed_state
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
