"""Verify that an NVDA add-on updates an existing installation in place."""
import argparse
import ast
import configparser
import json
from pathlib import Path
import re
import zipfile


def manifest(path):
    with zipfile.ZipFile(path) as archive:
        assert archive.testzip() is None, path
        parser = configparser.ConfigParser(interpolation=None)
        parser.read_string('[addon]\n' + archive.read('manifest.ini').decode('utf-8'))
        values = {key: ast.literal_eval(value) for key, value in parser['addon'].items()}
        driver = f'synthDrivers/{values["name"]}/__init__.py'
        assert driver in archive.namelist(), (path, driver)
        summaries = [values['summary']]
        for name in archive.namelist():
            if re.fullmatch(r'locale/[^/]+/manifest\.ini', name):
                translated = configparser.ConfigParser(interpolation=None)
                translated.read_string('[addon]\n' + archive.read(name).decode('utf-8'))
                summaries.append(ast.literal_eval(translated['addon']['summary']))
        return values, summaries


def version(text):
    match = re.fullmatch(r'(\d+)\.(\d+)\.(\d+)(?:-(dev|rc)(\d+))?', text)
    assert match, text
    major, minor, patch = map(int, match.groups()[:3])
    stage, number = match.groups()[3:]
    return (major, minor, patch, {'dev': 0, 'rc': 1, None: 2}[stage], int(number or 0))


def build_epoch(path):
    with zipfile.ZipFile(path) as archive:
        value = json.loads(archive.read('doc/source.json'))['build_epoch']
    assert type(value) is int and value > 0
    return value


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--previous', type=Path, required=True)
    parser.add_argument('--candidate', type=Path, required=True)
    parser.add_argument('--allow-rebuild', action='store_true',
                        help='also allow the same version with a newer build timestamp')
    args = parser.parse_args()
    old, _ = manifest(args.previous)
    new, summaries = manifest(args.candidate)
    assert old['name'] == new['name'] == 'reloqC'
    old_version, new_version = version(old['version']), version(new['version'])
    if args.allow_rebuild and new_version == old_version:
        assert build_epoch(args.candidate) > build_epoch(args.previous)
    else:
        assert new_version > old_version
    for summary in summaries:
        assert not re.search(r'\b\d+\.\d+(?:\.\d+)?\b', summary), summary
    print(f'NVDA upgrade: same add-on identity, {old["version"]} -> {new["version"]}; version-free names')


if __name__ == '__main__':
    main()
