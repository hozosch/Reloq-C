"""Release packages must use one verified community dictionary snapshot."""
import hashlib
import io
import json
from pathlib import Path
import sys
import tempfile
import tarfile
import zipfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
sys.path.insert(0, str(ROOT / 'nvda'))
from release_packages import android_names, linux_names, release_version, verify_packaged_dictionaries, verify_packaged_build_epoch, verify_linux_archive, release_body
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
    assert linux_names('2.0.0') == ()
    assert set(linux_names('2.1.0')) == {'reloq-c-linux-x64-2.1.0.tar.gz',
                                        'reloq-c-synth-linux-x64-2.1.0.nvda-addon'}
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
        linux = root/'reloq-c-linux-x64-2.1.0.tar.gz'
        def build_linux(stale=False):
            with tarfile.open(linux,'w:gz') as archive:
                contents={f'dictionaries/{n}': (b'stale' if stale and n==FILES[0] else (snapshot/n).read_bytes())
                          for n in FILES if n.endswith('.dic')}
                contents.update({n:b'test' for n in ('sd_reloq','libreloq_eci.so','install.sh',
                                  'licenses/SpeechDispatcher-module-LICENSE.txt')})
                for name, data in contents.items():
                    member=tarfile.TarInfo(f'reloq-c-linux-x64-2.1.0/{name}')
                    member.size,member.mtime=len(data),epoch
                    archive.addfile(member,io.BytesIO(data))
        build_linux()
        verify_linux_archive(linux,'2.1.0',snapshot,epoch)
        build_linux(stale=True)
        try:verify_linux_archive(linux,'2.1.0',snapshot,epoch)
        except ValueError:pass
        else:raise AssertionError('a stale Linux dictionary was accepted')
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
