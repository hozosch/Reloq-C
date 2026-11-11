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
from release_packages import android_names, release_version, verify_packaged_dictionaries, verify_packaged_build_epoch, release_body
from _reloq_dictionaries import FILES


def blob(data):
    return hashlib.sha1(b'blob ' + str(len(data)).encode() + b'\0' + data).hexdigest()


def main():
    assert release_version('1.0.0') == '1.0'
    assert release_version('1.2.0') == '1.2.0'
    assert release_version('1.1.3') == '1.1.3'
    assert android_names('1.3.1') == ('reloq-c-android-arm64-1.3.1.apk',)
    assert set(android_names('1.4.0')) == {'reloq-c-android-arm64-1.4.0.apk',
                                           'reloq-c-android-x86_64-1.4.0.apk'}
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
        prefixes = ('synthDrivers/reloqC/communityDictionaries/', 'assets/community/',
                    'assets/community/', 'dictionaries/', 'dictionaries/')
        archives = []
        for i, prefix in enumerate(prefixes):
            path = root / f'{i}.zip'
            with zipfile.ZipFile(path, 'w') as archive:
                for name in (*FILES, 'manifest.json'):
                    archive.writestr(prefix + name, (snapshot / name).read_bytes())
            archives.append((path, prefix))
        assert verify_packaged_dictionaries(archives, snapshot) == manifest
        epoch=1700000001
        epoch_files=(('doc/source.json',json.dumps({'build_epoch':epoch})),
                     ('assets/reloq-build-epoch.txt',f'{epoch}\n'),
                     ('assets/reloq-build-epoch.txt',f'{epoch}\n'),
                     ('build-epoch.txt',f'{epoch}\n'),('build-epoch.txt',f'{epoch}\n'))
        for (path,_),(name,data) in zip(archives,epoch_files):
            with zipfile.ZipFile(path,'a') as archive:archive.writestr(name,data)
        assert verify_packaged_build_epoch(archives[0][0],[archives[1][0],archives[2][0]],
                    [archives[3][0],archives[4][0]])==epoch
        bad=root/'bad.zip'
        with zipfile.ZipFile(bad,'w') as archive:archive.writestr('build-epoch.txt',f'{epoch+1}\n')
        try:verify_packaged_build_epoch(archives[0][0],[archives[1][0],archives[2][0]],[archives[3][0],bad])
        except ValueError:pass
        else:raise AssertionError('mismatched package timestamp was accepted')
        assert release_body('notes\n',epoch)=='notes\n\n<!-- ReLoq-C build:1700000001 -->\n'
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
    print('Release dictionaries: verified identical snapshot in all five payloads')


if __name__ == '__main__':
    main()
