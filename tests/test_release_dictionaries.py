"""Release packages must use one verified community dictionary snapshot."""
import hashlib
import json
from pathlib import Path
import sys
import tempfile
import zipfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
sys.path.insert(0, str(ROOT / 'nvda'))
from release_packages import release_version, verify_packaged_dictionaries
from _reloq_dictionaries import FILES


def blob(data):
    return hashlib.sha1(b'blob ' + str(len(data)).encode() + b'\0' + data).hexdigest()


def main():
    assert release_version('1.0.0') == '1.0'
    assert release_version('1.2.0') == '1.2.0'
    assert release_version('1.1.3') == '1.1.3'
    with tempfile.TemporaryDirectory() as temp:
        root = Path(temp)
        snapshot = root / 'snapshot'
        snapshot.mkdir()
        records = {}
        for name in FILES:
            data = ('dictionary for ' + name).encode()
            (snapshot / name).write_bytes(data)
            records[name] = {'sha': blob(data), 'size': len(data)}
        manifest = {'repository': 'eigencrow/IBMTTSDictionaries', 'branch': 'master',
                    'commit': 'a' * 40, 'files': records}
        (snapshot / 'manifest.json').write_text(json.dumps(manifest), encoding='utf-8')
        prefixes = ('assets/community/', 'synthDrivers/reloqC/communityDictionaries/',
                    'dictionaries/', 'dictionaries/')
        archives = []
        for i, prefix in enumerate(prefixes):
            path = root / f'{i}.zip'
            with zipfile.ZipFile(path, 'w') as archive:
                for name in (*FILES, 'manifest.json'):
                    archive.writestr(prefix + name, (snapshot / name).read_bytes())
            archives.append((path, prefix))
        assert verify_packaged_dictionaries(archives, snapshot) == manifest
        # A single stale file in any one platform must block publication.
        for path, prefix in archives:
            with zipfile.ZipFile(path, 'w') as archive:
                for name in (*FILES, 'manifest.json'):
                    data = b'stale' if name == FILES[0] else (snapshot / name).read_bytes()
                    archive.writestr(prefix + name, data)
            try:
                verify_packaged_dictionaries(archives, snapshot)
            except ValueError as exc:
                assert 'dictionary differs' in str(exc)
            else:
                raise AssertionError('a stale packaged dictionary was accepted')
            with zipfile.ZipFile(path, 'w') as archive:
                for name in (*FILES, 'manifest.json'):
                    archive.writestr(prefix + name, (snapshot / name).read_bytes())
    print('Release dictionaries: verified identical snapshot in all four payloads')


if __name__ == '__main__':
    main()
