"""Build and package a native x86 or x64 SAPI adapter and matching C core."""
from pathlib import Path
import argparse
import hashlib
import json
import re
import subprocess
import sys
import zipfile
from version import VERSION

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_dictionaries import FILES, validate_snapshot


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--arch', choices=('x64', 'x86'), default='x64')
    parser.add_argument('--cc')
    parser.add_argument('--objdump')
    parser.add_argument('--core', type=Path)
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    prefix = 'i686' if args.arch == 'x86' else 'x86_64'
    args.cc = args.cc or prefix + '-w64-mingw32-gcc'
    args.objdump = args.objdump or prefix + '-w64-mingw32-objdump'
    args.core = args.core or ROOT / ('eci/eci-x86.dll' if args.arch == 'x86' else 'eci/eci.dll')
    args.output = args.output or ROOT / f'dist/reloq-c-sapi5-{args.arch}-{VERSION}-payload.zip'
    build = ROOT / 'sapi/build' / args.arch
    subprocess.run([sys.executable, str(ROOT / 'sapi/generate_catalogue.py'), '--check'], check=True)
    subprocess.run([sys.executable, str(ROOT / 'sapi/generate_ui.py'), '--check'], check=True)
    subprocess.run(['make', '-C', str(ROOT / 'sapi'), '-j2', 'CC=' + args.cc,
                    'ARCH=' + args.arch, 'BUILD=' + str(build), 'VERSION=' + VERSION], check=True)
    dictionary = validate_snapshot(ROOT / 'data/community')
    names = ['ReLoqSapi.dll', 'ReLoqSettings.exe', 'ReLoqUpdate.exe', 'Test-Update.exe', 'Test-SAPI.exe', 'Test-Dictionary.exe']
    entries = {name: (build / name).read_bytes() for name in names}
    entries.update({'eci.dll': args.core.read_bytes(), 'README.txt': (ROOT / 'sapi/README.txt').read_bytes(),
                    'LICENSE': (ROOT / 'LICENSE').read_bytes(), 'NOTICE': (ROOT / 'NOTICE').read_bytes()})
    for name, source in {
        'OpenEVV-LICENSE.txt': 'vendor/openevv61/LICENSE',
        'OpenEVV-NOTICE.txt': 'vendor/openevv61/NOTICE',
        'OpenEVV-wordseg-LICENSE.txt': 'nvda/wordseg-data/LICENSE',
        'OpenEVV-wordseg-NOTICE.txt': 'nvda/wordseg-data/NOTICE',
        'Dictionaries-CC0.txt': 'data/community/LICENSE.md',
        'Unicode-LICENSE.txt': 'reference-apple/behavior-0.2.82/unicode-input/LICENSE.txt',
        'SCOWL-Copyright.txt': 'reference-apple/behavior-0.2.83/input/SCOWL-Copyright.txt',
    }.items():
        entries['licenses/' + name] = (ROOT / source).read_bytes()
    for name in (*FILES, 'manifest.json'):
        entries['dictionaries/' + name] = (ROOT / 'data/community' / name).read_bytes()
    assert len(entries['eci.dll']) > 1000000
    for file in [args.core, *(build / name for name in names)]:
        description = subprocess.check_output([args.objdump, '-f', str(file)], text=True)
        assert ('file format ' + ('pei-i386' if args.arch == 'x86' else 'pei-x86-64')) in description, (file, description)
    subprocess.run([sys.executable, str(ROOT / 'eci/verify_pe_exports.py'), str(args.core),
                    '--arch', args.arch, '--objdump', args.objdump,
                    '--header', str(ROOT / 'include/reloq_eci.h')], check=True)
    pe = subprocess.check_output([args.objdump, '-p', str(build / 'ReLoqSapi.dll')], text=True)
    names = pe.split('[Ordinal/Name Pointer] Table\n', 1)[1].split('\n\n', 1)[0]
    exports = re.findall(r'\[\s*\d+\]\s+(\w+)', names)
    required = {'DllCanUnloadNow', 'DllGetClassObject', 'DllRegisterServer', 'DllUnregisterServer', 'ReLoqSettings'}
    assert set(exports) == required, exports
    hashes = {name: {'bytes': len(data), 'sha256': hashlib.sha256(data).hexdigest()} for name, data in entries.items()}
    entries['package.json'] = (json.dumps({'version': VERSION + '-payload', 'architecture': args.arch,
        'voices': 14, 'core_copies': 1, 'dictionary_commit': dictionary['commit'], 'files': hashes}, indent=2) + '\n').encode()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(args.output, 'w', compression=zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
        for name, data in sorted(entries.items()):
            info = zipfile.ZipInfo(name, (2026, 10, 31, 0, 0, 0))
            info.compress_type = zipfile.ZIP_DEFLATED
            archive.writestr(info, data, compresslevel=9)
    with zipfile.ZipFile(args.output) as archive:
        assert archive.testzip() is None
        for name, data in entries.items():
            assert archive.read(name) == data
    print(args.output)


if __name__ == '__main__':
    main()
