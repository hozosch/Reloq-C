"""Optional ECI dictionary data; update immutable snapshots from current master.

Network work runs outside the speech worker. Every file is fetched at the same
commit and checked against its Git blob ID before the set becomes visible.
Speech uses the bundled or last complete set when an update is unavailable.
"""
from __future__ import annotations

import hashlib
import json
import os
from pathlib import Path
import re
import tempfile
import threading
from urllib.request import Request, urlopen

REPOSITORY = "eigencrow/IBMTTSDictionaries"
FILES = tuple(prefix + suffix + ".dic" for prefix in ("DEU", "ENU")
              for suffix in ("main", "Root", "abbr")) + ("LICENSE.md",)
SHA = re.compile(r"[0-9a-f]{40}\Z")
LIMIT = 8 * 1024 * 1024


def _get(url, limit, fetch):
    request = Request(url, headers={"User-Agent": "Eloquence-C-dictionaries",
                                   "Accept": "application/vnd.github+json"})
    with fetch(request, timeout=8) as response:
        data = response.read(limit + 1)
    if len(data) > limit:
        raise ValueError("dictionary download exceeds size limit")
    return data


def _blob(data):
    return hashlib.sha1(b"blob " + str(len(data)).encode("ascii") + b"\0" + data).hexdigest()


def validate_snapshot(directory):
    directory = Path(directory)
    manifest = json.loads((directory / "manifest.json").read_text(encoding="utf-8"))
    if (manifest.get("repository") != REPOSITORY or manifest.get("branch") != "master"
            or not SHA.fullmatch(manifest.get("commit", ""))
            or set(manifest.get("files", {})) != set(FILES)):
        raise ValueError("invalid dictionary snapshot manifest")
    for name in FILES:
        data = (directory / name).read_bytes()
        record = manifest["files"][name]
        if len(data) > LIMIT or len(data) != record["size"] or _blob(data) != record["sha"]:
            raise ValueError(f"dictionary snapshot failed verification: {name}")
    return manifest


def fetch_master(cache, *, fetch=urlopen, publish=True):
    """Resolve master afresh; never use a release tag or a pinned old commit."""
    cache = Path(cache)
    api = f"https://api.github.com/repos/{REPOSITORY}"
    head = json.loads(_get(api + "/branches/master", 1024 * 1024, fetch))
    commit = head["commit"]["sha"]
    if not isinstance(commit, str) or not SHA.fullmatch(commit):
        raise ValueError("invalid master commit")
    cache.mkdir(parents=True, exist_ok=True)
    target = cache / commit
    if target.is_dir():
        validate_snapshot(target)
    else:
        tree = json.loads(_get(api + f"/git/trees/{commit}", 2 * 1024 * 1024, fetch))
        if tree.get("truncated"):
            raise ValueError("incomplete dictionary tree")
        records = {r["path"]: {"sha": r["sha"], "size": r["size"]}
                   for r in tree["tree"] if r.get("type") == "blob" and r["path"] in FILES}
        if set(records) != set(FILES):
            raise ValueError("master is missing an ECI dictionary volume")
        with tempfile.TemporaryDirectory(prefix="download-", dir=cache) as temporary:
            stage = Path(temporary) / "snapshot"
            stage.mkdir()
            for name, record in records.items():
                data = _get(f"https://raw.githubusercontent.com/{REPOSITORY}/{commit}/{name}", LIMIT, fetch)
                if len(data) != record["size"] or _blob(data) != record["sha"]:
                    raise ValueError(f"Git blob verification failed: {name}")
                (stage / name).write_bytes(data)
            manifest = {"repository": REPOSITORY, "branch": "master", "commit": commit,
                        "files": records}
            (stage / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
            validate_snapshot(stage)
            try:
                stage.rename(target)
            except OSError:
                if not target.is_dir():
                    raise
                validate_snapshot(target)
    # Runtime managers publish only the currently selected generation.
    if not publish:
        return target
    # Publish a complete set. A failed refresh leaves the previous pointer.
    with tempfile.NamedTemporaryFile(mode="w", encoding="ascii", dir=cache,
                                     prefix="current-", suffix=".json", delete=False) as file:
        pointer = Path(file.name)
        json.dump({"commit": commit}, file)
    try:
        os.replace(pointer, cache / "current.json")
    finally:
        pointer.unlink(missing_ok=True)
    return target


class CommunityDictionaries:
    def __init__(self, bundled, cache, deliver, on_error, *, fetch=urlopen):
        self.bundled, self.cache = Path(bundled), Path(cache)
        self.deliver, self.on_error, self.fetch = deliver, on_error, fetch
        self._lock = threading.Lock()
        self._revision, self._closed, self._enabled = 0, False, False

    def current(self, revision):
        with self._lock:
            return not self._closed and revision == self._revision

    def set_enabled(self, enabled):
        with self._lock:
            if self._closed:
                return
            self._revision += 1
            revision = self._revision
            self._enabled = enabled
        if not enabled:
            self.deliver(revision, None)
            return
        chosen = self.bundled
        try:
            commit = json.loads((self.cache / "current.json").read_text(encoding="ascii"))["commit"]
            if not isinstance(commit, str) or not SHA.fullmatch(commit):
                raise ValueError("invalid cached commit")
            chosen = self.cache / commit
            validate_snapshot(chosen)
        except (OSError, ValueError, KeyError, TypeError):
            chosen = self.bundled
            validate_snapshot(chosen)
        baseline = validate_snapshot(chosen)["commit"]
        self.deliver(revision, chosen)
        threading.Thread(target=self._refresh, args=(revision, baseline),
                         name="Eloquence dictionary update", daemon=True).start()

    def _refresh(self, revision, baseline):
        try:
            snapshot = fetch_master(self.cache, fetch=self.fetch)
            if self.current(revision) and snapshot.name != baseline:
                self.deliver(revision, snapshot)
        except Exception as error:
            if self.current(revision):
                self.on_error(error)

    def close(self):
        with self._lock:
            self._closed = True
            self._revision += 1
