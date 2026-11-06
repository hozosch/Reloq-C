"""Verify that an NVDA add-on updates an existing installation in place."""
import argparse
import ast
import configparser
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
    assert re.fullmatch(r'\d+\.\d+\.\d+', text), text
    return tuple(map(int, text.split('.')))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--previous', type=Path, required=True)
    parser.add_argument('--candidate', type=Path, required=True)
    args = parser.parse_args()
    old, _ = manifest(args.previous)
    new, summaries = manifest(args.candidate)
    assert old['name'] == new['name'] == 'reloqC'
    assert version(new['version']) > version(old['version'])
    for summary in summaries:
        assert not re.search(r'\b\d+\.\d+(?:\.\d+)?\b', summary), summary
    print(f'NVDA upgrade: same add-on identity, {old["version"]} -> {new["version"]}; version-free names')


if __name__ == '__main__':
    main()
