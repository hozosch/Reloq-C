"""Published package updates, independent of speech and dictionary workers."""
from dataclasses import dataclass
import hashlib
import json
from pathlib import Path
import re
import threading
from urllib.parse import unquote, urlsplit
from urllib.request import Request, urlopen

RELEASES = "https://api.github.com/repos/hozosch/Reloq-C/releases?per_page=30"
LIMIT = 128 * 1024 * 1024
PACKAGE = re.compile(r"reloq-c-synth-x64-(\d+\.\d+\.\d+)\.nvda-addon\Z")
BUILD_MARKER = re.compile(r"(?m)^<!-- ReLoq-C build:([1-9]\d{0,11}) -->$")
MAX_BUILD_EPOCH = 253402300799


@dataclass(frozen=True)
class Update:
    version: str
    name: str
    url: str
    size: int
    sha256: str
    build_epoch: int = 0

    @property
    def identity(self):
        return f"{self.version}@{self.build_epoch}" if self.build_epoch else self.version


def build_epoch(body):
    """Read the release's build timestamp, never its later upload time."""
    matches = BUILD_MARKER.findall(body.replace('\r\n', '\n')) if isinstance(body, str) else []
    if len(matches) != 1:
        return 0
    value = int(matches[0])
    return value if value <= MAX_BUILD_EPOCH else 0


def installed_build_epoch():
    try:
        metadata = json.loads((Path(__file__).resolve().parents[2] / 'doc/source.json').read_text(encoding='utf8'))
        value = metadata.get('build_epoch')
        return value if type(value) is int and 0 < value <= MAX_BUILD_EPOCH else 0
    except (OSError, ValueError, TypeError, AttributeError):
        return 0


def version(value):
    if not re.fullmatch(r"\d+\.\d+\.\d+", value):
        raise ValueError("Invalid package version")
    return tuple(map(int, value.split('.')))


def select(releases, installed, installed_revision=0):
    development = re.fullmatch(r"(\d+\.\d+\.\d+)-(?:dev|rc)\d+", installed)
    current = version(development[1] if development else installed)
    if not isinstance(releases, list):
        raise ValueError("Invalid release list")
    available = []
    for release in releases:
        if not isinstance(release, dict) or release.get('draft') or release.get('prerelease'):
            continue
        revision = build_epoch(release.get('body'))
        for asset in release.get('assets', []):
            if not isinstance(asset, dict):
                continue
            name = asset.get('name', '')
            match = PACKAGE.fullmatch(name) if isinstance(name, str) else None
            if not match:
                continue
            candidate = version(match[1])
            if candidate < current or (candidate == current and not development and
                    not (installed_revision > 0 and revision > installed_revision)):
                continue
            url, size, digest = (asset.get(k) for k in ('browser_download_url', 'size', 'digest'))
            if not isinstance(url, str) or not isinstance(digest, str):
                raise ValueError("Update metadata is missing its download or SHA-256 checksum")
            parts = urlsplit(url)
            if (parts.scheme != 'https' or parts.netloc != 'github.com' or parts.query or parts.fragment
                    or not parts.path.startswith('/hozosch/Reloq-C/releases/download/')
                    or unquote(parts.path.rsplit('/', 1)[-1]) != name):
                raise ValueError("Invalid package download URL")
            if type(size) is not int or not 0 < size <= LIMIT:
                raise ValueError("Invalid package size")
            if not re.fullmatch(r'sha256:[0-9a-fA-F]{64}', digest):
                raise ValueError("Invalid package SHA-256 checksum")
            available.append(Update(match[1], name, url, size, digest[7:].lower(), revision))
    return max(available, key=lambda update: (version(update.version), update.build_epoch), default=None)


def _request(url):
    return Request(url, headers={'User-Agent': 'ReLoq-C-updater', 'Accept': 'application/vnd.github+json'})


def check(installed, *, fetch=urlopen, installed_revision=None):
    with fetch(_request(RELEASES), timeout=20) as response:
        data = response.read(2 * 1024 * 1024 + 1)
    if len(data) > 2 * 1024 * 1024:
        raise ValueError("Release list is too large")
    revision = installed_build_epoch() if installed_revision is None else installed_revision
    return select(json.loads(data), installed, revision)


def download(update, directory, *, fetch=urlopen):
    # A candidate can also be supplied by a caller; validate it before I/O.
    body = f'<!-- ReLoq-C build:{update.build_epoch} -->' if update.build_epoch else ''
    validated = select([{'body': body, 'assets': [dict(name=update.name, browser_download_url=update.url,
        size=update.size, digest='sha256:' + update.sha256)]}], '0.0.0')
    if validated != update:
        raise ValueError("Invalid update candidate")
    target = Path(directory) / update.name
    partial = target.with_suffix(target.suffix + '.part')
    digest, size = hashlib.sha256(), 0
    try:
        with fetch(_request(update.url), timeout=20) as response, partial.open('wb') as output:
            while block := response.read(65536):
                size += len(block)
                if size > update.size:
                    raise ValueError("Downloaded package exceeds its published size")
                digest.update(block)
                output.write(block)
        if size != update.size or digest.hexdigest() != update.sha256:
            raise ValueError("Downloaded package does not match its published size or SHA-256 checksum")
        partial.replace(target)
        return target
    finally:
        partial.unlink(missing_ok=True)


class AutomaticUpdates:
    """Check outside NVDA's UI and speech threads, stopping promptly on exit."""
    def __init__(self, installed, enabled, notify, *, fetch=check, interval=86400,
                 startup_delay=15):
        self.installed, self.enabled, self.notify = installed, enabled, notify
        self.fetch, self.interval, self.startup_delay = fetch, interval, startup_delay
        self.stopped = threading.Event()
        self.thread = None

    def start(self):
        if self.thread is not None:
            return
        self.thread = threading.Thread(target=self._run, name='ReLoq automatic updates', daemon=True)
        self.thread.start()

    def _run(self):
        if self.stopped.wait(self.startup_delay):
            return
        while not self.stopped.is_set():
            try:
                if self.enabled():
                    update = self.fetch(self.installed)
                    if update is not None and not self.stopped.is_set():
                        self.notify(update)
            except Exception:
                # A failed background check must never interrupt speech.
                pass
            if self.stopped.wait(self.interval):
                return

    def stop(self):
        self.stopped.set()
