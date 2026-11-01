#!/usr/bin/env python3
"""Collect verified installables and check a release's source/asset manifest."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess


def sha256(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def names(version):
    if not re.fullmatch(r'\d+\.\d+\.\d+', version):
        raise ValueError('invalid release version')
    return (f'reloq-c-sapi5-{version}-setup.exe',
            f'reloq-c-synth-experimental-x64-{version}.nvda-addon')


def release_version(version):
    """Stable 1.0 is packaged as SemVer 1.0.0 for existing update clients."""
    names(version)
    return version[:-2] if version.endswith('.0') else version


def android_name(version, name):
    return re.fullmatch(rf'reloq-c-android-arm64-{re.escape(version)}(?:-preview\d+)?\.apk', name)


def collect(source, destination):
    version = re.search(r'^version\s*=\s*(\S+)', Path('addon/synth/manifest.ini').read_text(), re.M)[1].strip('"')
    required = names(version)
    files = [path for path in source.rglob('*') if path.is_file() and
             (path.name in required or android_name(version, path.name))]
    if len(files) != 3 or not set(required).issubset(path.name for path in files):
        raise ValueError('expected one SAPI installer, one NVDA add-on and one ARM64 APK')
    destination.mkdir(parents=True, exist_ok=False)
    manifest = {'version': version, 'release_version': release_version(version),
                'source_commit': os.environ['GITHUB_SHA'],
                'run_id': int(os.environ['GITHUB_RUN_ID']), 'assets': {}}
    policy = Path('.github/release-policy.json')
    if policy.exists():
        supersedes = json.loads(policy.read_text())['supersedes_tag']
        if not re.fullmatch(r'v\d+\.\d+\.\d+', supersedes):
            raise ValueError('invalid superseded test-release tag')
        manifest['supersedes_tag'] = supersedes
    for path in sorted(files):
        if path.stat().st_size < 1_000_000:
            raise ValueError(f'package is unexpectedly small: {path}')
        manifest['assets'][path.name] = {'size': path.stat().st_size, 'sha256': sha256(path)}
        shutil.copyfile(path, destination / path.name)
    (destination / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')


def verify_source(commit):
    # Publication metadata can be repaired without rebuilding the packages.
    # Runtime, platform build scripts and data must match the tested source.
    changed = subprocess.check_output(['git', 'diff', '--name-only', commit, 'HEAD'], text=True).splitlines()
    allowed = {'.github/release.json', 'README.md', '.github/workflows/release.yml',
               'tools/release_packages.py'}
    if any(path not in allowed and not path.startswith('docs/') for path in changed):
        raise ValueError('runtime or build source changed after candidate validation')


def verify(directory, request):
    wanted = json.loads(request.read_text())
    actual = json.loads((directory / 'manifest.json').read_text())
    if actual != wanted:
        raise ValueError('candidate artifact differs from the approved release manifest')
    expected = names(actual['version'])
    if actual['release_version'] != release_version(actual['version']):
        raise ValueError('release name differs from package version')
    assets = actual['assets']
    if len(assets) != 3 or not set(expected).issubset(assets):
        raise ValueError('missing installable packages')
    if set(path.name for path in directory.iterdir()) != {*assets, 'manifest.json'}:
        raise ValueError('unexpected files in release artifact')
    for name, metadata in assets.items():
        if name not in expected and not android_name(actual['version'], name):
            raise ValueError('unexpected asset name')
        path = directory / name
        if path.stat().st_size != metadata['size'] or sha256(path) != metadata['sha256']:
            raise ValueError(f'asset digest differs: {name}')
    verify_source(actual['source_commit'])
    if os.environ.get('GITHUB_OUTPUT'):
        with open(os.environ['GITHUB_OUTPUT'], 'a') as output:
            output.write(f"version={actual['version']}\nrelease_version={actual['release_version']}\n")
    print(f"Verified three installables for {actual['version']} from {actual['source_commit']}")


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest='action', required=True)
    for command in ('collect', 'verify'):
        p = sub.add_parser(command)
        p.add_argument('first', type=Path)
        p.add_argument('second', type=Path)
    args = parser.parse_args()
    (collect if args.action == 'collect' else verify)(args.first, args.second)
