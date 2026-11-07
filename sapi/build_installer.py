"""Compile one Inno Setup installer from the verified x86 and x64 payloads."""
from pathlib import Path
import argparse
import hashlib
import json
import subprocess
import zipfile

ROOT = Path(__file__).resolve().parents[1]
VERSION = '1.2.0-dev6'


def stage(build, packages):
    payload = build / 'payload'
    payload.mkdir(parents=True, exist_ok=True)
    common = None
    snapshots = {}
    for arch, package in packages.items():
        target = payload / arch
        target.mkdir(exist_ok=True)
        with zipfile.ZipFile(package) as archive:
            assert archive.testzip() is None, package
            manifest = json.loads(archive.read('package.json'))
            assert manifest['architecture'] == arch, package
            assert manifest['version'] == VERSION + '-payload', package
            for name, row in manifest['files'].items():
                data = archive.read(name)
                assert hashlib.sha256(data).hexdigest() == row['sha256'], (package, name)
            for name in ('eci.dll', 'ReLoqSapi.dll', 'ReLoqSettings.exe', 'Test-SAPI.exe'):
                (target / name).write_bytes(archive.read(name))
            shared = {n: archive.read(n) for n in archive.namelist()
                      if n.startswith(('dictionaries/', 'licenses/')) or n in ('LICENSE', 'README.txt')}
            if common is None:
                common = shared
            else:
                assert shared == common, 'Architecture payloads disagree about dictionaries/licenses'
            snapshots[arch] = {'package': package.name, 'sha256': hashlib.sha256(package.read_bytes()).hexdigest(),
                               'core_sha256': manifest['files']['eci.dll']['sha256']}
    for name, data in common.items():
        file = payload / 'common' / name
        file.parent.mkdir(parents=True, exist_ok=True)
        file.write_bytes(data)
    return payload, snapshots


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--iscc', required=True, type=Path)
    parser.add_argument('--build-dir', type=Path, default=ROOT / 'sapi/build/installer')
    parser.add_argument('--x64', type=Path)
    parser.add_argument('--x86', type=Path)
    parser.add_argument('--output-dir', type=Path, default=ROOT / 'dist')
    args = parser.parse_args()
    packages = {arch: (getattr(args, arch) or sorted((ROOT / 'dist').glob(f'reloq-c-sapi5-{arch}-{VERSION}-payload.zip'))[-1]).resolve()
                for arch in ('x64', 'x86')}
    build = args.build_dir.resolve()
    payload, snapshots = stage(build, packages)
    output = args.output_dir.resolve()
    output.mkdir(parents=True, exist_ok=True)
    script = ROOT / 'sapi/installer/reloq-sapi.iss'
    subprocess.run([str(args.iscc.resolve()), '/DAppVersion=' + VERSION, '/DPayload=' + str(payload),
                    '/DOutput=' + str(output), str(script)], check=True)
    installer = output / f'reloq-c-sapi5-{VERSION}-setup.exe'
    report = {'version': VERSION, 'architectures': ['x86', 'x64'], 'languages': 14,
              'sha256': hashlib.sha256(installer.read_bytes()).hexdigest(),
              'bytes': installer.stat().st_size, 'payloads': snapshots,
              'shared_dictionary_copies': 1, 'uninstall': 'Inno Setup, both COM registry views'}
    (output / f'reloq-c-sapi5-{VERSION}-installer.json').write_text(json.dumps(report, indent=2) + '\n')
    print(installer)


if __name__ == '__main__':
    main()
