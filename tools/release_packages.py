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
import sys
import zipfile

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'nvda'))
from _reloq_dictionaries import FILES as DICTIONARY_FILES, validate_snapshot


def sha256(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def names(version):
    if not re.fullmatch(r'\d+\.\d+\.\d+', version):
        raise ValueError('invalid release version')
    return (f'reloq-c-sapi5-{version}-setup.exe',
            f'reloq-c-synth-x64-{version}.nvda-addon')


def release_version(version):
    """Stable 1.0 is packaged as SemVer 1.0.0 for existing update clients."""
    names(version)
    return '1.0' if version == '1.0.0' else version


def android_name(version, name):
    return name in android_names(version)


def android_names(version):
    names(version)  # Validate before interpolating into a package name.
    archs = ('arm64', 'x86_64') if tuple(map(int, version.split('.'))) >= (1, 4, 0) else ('arm64',)
    return tuple(f'reloq-c-android-{arch}-{version}.apk' for arch in archs)


def verify_packaged_dictionaries(archives, directory):
    """All platform packages must contain exactly the resolved snapshot."""
    directory = Path(directory)
    manifest = validate_snapshot(directory)
    expected = {name: (directory / name).read_bytes()
                for name in (*DICTIONARY_FILES, 'manifest.json')}
    for path, prefix in archives:
        with zipfile.ZipFile(path) as archive:
            for name, data in expected.items():
                if archive.read(prefix + name) != data:
                    raise ValueError(f'dictionary differs: {path.name}: {name}')
    return manifest


def verify_packaged_build_epoch(addon, androids, payloads):
    """All installers must identify the exact same candidate build."""
    with zipfile.ZipFile(addon) as archive:
        epoch = json.loads(archive.read('doc/source.json')).get('build_epoch')
    if type(epoch) is not int or not 0 < epoch <= 253402300799:
        raise ValueError('invalid NVDA build timestamp')
    for path, name in [*((p, 'assets/reloq-build-epoch.txt') for p in androids),
                       *((p, 'build-epoch.txt') for p in payloads)]:
        with zipfile.ZipFile(path) as archive:
            if archive.read(name) != f'{epoch}\n'.encode('ascii'):
                raise ValueError(f'package build timestamp differs: {path.name}')
    return epoch


def release_body(notes, epoch):
    """The invisible marker identifies a build, not the asset upload time."""
    if epoch is None:  # Historic release manifests predate build timestamps.
        return notes
    if type(epoch) is not int or not 0 < epoch <= 253402300799:
        raise ValueError('invalid release build timestamp')
    if '<!-- ReLoq-C build:' in notes:
        raise ValueError('release notes already contain a build marker')
    return notes.rstrip() + f'\n\n<!-- ReLoq-C build:{epoch} -->\n'


def collect(source, destination):
    version = re.search(r'^version\s*=\s*(\S+)', Path('addon/synth/manifest.ini').read_text(), re.M)[1].strip('"')
    required = (*names(version), *android_names(version))
    files = [path for path in source.rglob('*') if path.is_file() and
             path.name in required]
    if len(files) != len(required) or set(required) != {path.name for path in files}:
        raise ValueError('expected one SAPI installer, one NVDA add-on and every Android ABI')
    addon = next(p for p in files if p.name.endswith('.nvda-addon'))
    androids = [p for p in files if p.name.endswith('.apk')]
    payloads = []
    for arch in ('x64', 'x86'):
        matches = list(source.rglob(f'reloq-c-sapi5-{arch}-{version}-payload.zip'))
        if len(matches) != 1:
            raise ValueError(f'expected one {arch} SAPI payload')
        payloads.append(matches[0])
    dictionary = verify_packaged_dictionaries(
        [(addon, 'synthDrivers/reloqC/communityDictionaries/'),
         *((p, 'assets/community/') for p in androids),
         *((p, 'dictionaries/') for p in payloads)], Path('data/community'))
    epoch = verify_packaged_build_epoch(addon, androids, payloads)
    destination.mkdir(parents=True, exist_ok=False)
    manifest = {'version': version, 'release_version': release_version(version),
                'source_commit': os.environ['GITHUB_SHA'],
                'run_id': int(os.environ['GITHUB_RUN_ID']),
                'dictionary_snapshot': dictionary, 'build_epoch': epoch, 'assets': {}}
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
    expected = (*names(actual['version']), *android_names(actual['version']))
    if actual['release_version'] != release_version(actual['version']):
        raise ValueError('release name differs from package version')
    assets = actual['assets']
    if set(assets) != set(expected):
        raise ValueError('missing installable packages')
    if set(path.name for path in directory.iterdir()) != {*assets, 'manifest.json'}:
        raise ValueError('unexpected files in release artifact')
    for name, metadata in assets.items():
        if name not in expected:
            raise ValueError('unexpected asset name')
        path = directory / name
        if path.stat().st_size != metadata['size'] or sha256(path) != metadata['sha256']:
            raise ValueError(f'asset digest differs: {name}')
    verify_source(actual['source_commit'])
    if os.environ.get('GITHUB_OUTPUT'):
        with open(os.environ['GITHUB_OUTPUT'], 'a') as output:
            output.write(f"version={actual['version']}\nrelease_version={actual['release_version']}\n")
    print(f"Verified {len(expected)} installables for {actual['version']} from {actual['source_commit']}")


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest='action', required=True)
    for command in ('collect', 'verify'):
        p = sub.add_parser(command)
        p.add_argument('first', type=Path)
        p.add_argument('second', type=Path)
    args = parser.parse_args()
    (collect if args.action == 'collect' else verify)(args.first, args.second)
