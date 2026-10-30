"""Published package updates, independent of speech and dictionary workers."""
from dataclasses import dataclass
import hashlib
import json
from pathlib import Path
import re
from urllib.parse import unquote, urlsplit
from urllib.request import Request, urlopen

RELEASES = "https://api.github.com/repos/hozosch/Reloq-C/releases?per_page=30"
LIMIT = 128 * 1024 * 1024
PACKAGE = re.compile(r"reloq-c-synth-experimental-x64-(\d+\.\d+\.\d+)\.nvda-addon\Z")


@dataclass(frozen=True)
class Update:
    version: str
    name: str
    url: str
    size: int
    sha256: str


def version(value):
    if not re.fullmatch(r"\d+\.\d+\.\d+", value):
        raise ValueError("Invalid package version")
    return tuple(map(int, value.split('.')))


def select(releases, installed):
    current = version(installed)
    if not isinstance(releases, list):
        raise ValueError("Invalid release list")
    available = []
    for release in releases:
        if not isinstance(release, dict) or release.get('draft'):
            continue
        for asset in release.get('assets', []):
            if not isinstance(asset, dict):
                continue
            name = asset.get('name', '')
            match = PACKAGE.fullmatch(name) if isinstance(name, str) else None
            if not match or version(match[1]) <= current:
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
            available.append(Update(match[1], name, url, size, digest[7:].lower()))
    return max(available, key=lambda update: version(update.version), default=None)


def _request(url):
    return Request(url, headers={'User-Agent': 'ReLoq-C-updater', 'Accept': 'application/vnd.github+json'})


def check(installed, *, fetch=urlopen):
    with fetch(_request(RELEASES), timeout=20) as response:
        data = response.read(2 * 1024 * 1024 + 1)
    if len(data) > 2 * 1024 * 1024:
        raise ValueError("Release list is too large")
    return select(json.loads(data), installed)


def download(update, directory, *, fetch=urlopen):
    # A candidate can also be supplied by a caller; validate it before I/O.
    validated = select([{'assets': [dict(name=update.name, browser_download_url=update.url,
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
