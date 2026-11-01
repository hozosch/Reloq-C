"""Read-only, isolated observations of the pinned original x86 Syn modules.

This research tool downloads reference binaries to a temporary directory. It
does not copy them into source, builds or artifacts. Only the JSON observations
survive. Run under 32-bit Python on Windows, independently of the C renderer.
"""
from pathlib import Path
import argparse
import ctypes as C
import hashlib
import io
import json
import os
import re
import subprocess
import sys
import tempfile
import urllib.request
import zipfile

URL = ('https://github.com/pumper42nickel/eloquence_threshold/releases/'
       'download/v0.20210417.01/eloquence.nvda-addon')
ARCHIVE_SHA256 = '10e4f2ad321afaf2f4edf41a114f881261ce3768d23ceaf647cc715a71e3a521'
FILES = {
    'ECI.DLL': 'c1653e61e9fd599421d2f7faee21eda171d3a8ff4329343ddb24a2dd565fba5f',
    'ENU.SYN': '1a5f225f2d7c6fe1deff7df57eb285600430ac98dc3509e613051d20715996e9',
    'ENG.SYN': '4437a8b089b6205b21b255e21ddd62e8149400f5a0bc6dc696100ead5b0723ef',
    'DEU.SYN': '97c2069b4c782b0f52d72bd9f5be7120945b1cec1eff0104e2b97ee8d56f448d',
}
TIMES = ('13:28', '13:28:25', '00:20', '00:20:48', '00:20:48.526',
         '1:00', '1:00:03', '13:00', '13:00:25', '00:00:00',
         '12:00:30 pm', '1:02:03', '13:28:00', '25', '48.526',
         'thirteen twenty eight twenty five', 'one zero zero zero three')


def load(dll_path):
    dll = C.WinDLL(str(dll_path))
    signatures = {
        'eciNewEx': ([C.c_int], C.c_void_p),
        'eciDelete': ([C.c_void_p], None),
        'eciAddText': ([C.c_void_p, C.c_char_p], C.c_int),
        'eciSetVoiceParam': ([C.c_void_p, C.c_int, C.c_int, C.c_int], C.c_int),
        'eciSetParam': ([C.c_void_p, C.c_int, C.c_int], C.c_int),
        'eciSetOutputBuffer': ([C.c_void_p, C.c_int, C.POINTER(C.c_short)], C.c_int),
        'eciSynthesize': ([C.c_void_p], C.c_int),
        'eciSynchronize': ([C.c_void_p], C.c_int),
        'eciGeneratePhonemes': ([C.c_void_p, C.c_int, C.c_void_p], C.c_int),
        'eciVersion': ([C.c_char_p], None),
    }
    for name, (args, result) in signatures.items():
        f = getattr(dll, name)
        f.argtypes, f.restype = args, result
    return dll


def observe(dll, language, text, phonemes=False):
    callback_type = C.WINFUNCTYPE(C.c_int, C.c_void_p, C.c_int, C.c_int, C.c_void_p)
    buffer = (C.c_short * 2048)()
    chunks = []
    @callback_type
    def callback(handle, message, count, user):
        if message == 0:
            if not 0 <= count <= len(buffer):
                return 0
            chunks.append(C.string_at(buffer, count * 2))
        return 1
    dll.eciRegisterCallback.argtypes = [C.c_void_p, callback_type, C.c_void_p]
    dll.eciRegisterCallback.restype = None
    handle = dll.eciNewEx(language)
    if not handle:
        raise RuntimeError(f'Cannot initialize original language {language:#x}')
    try:
        dll.eciRegisterCallback(handle, callback, None)
        assert dll.eciSetOutputBuffer(handle, len(buffer), buffer)
        dll.eciSetParam(handle, 1, 1)
        for parameter, value in enumerate((0, 50, 65, 30, 0, 0, 75, 100)):
            assert dll.eciSetVoiceParam(handle, 0, parameter, value) != -1
        assert dll.eciAddText(handle, text.encode('cp1252'))
        if phonemes:
            out = C.create_string_buffer(65536)
            status = dll.eciGeneratePhonemes(handle, len(out), out)
            return dict(status=status, phonemes=out.value.decode('cp1252', errors='replace'))
        assert dll.eciSynthesize(handle)
        assert dll.eciSynchronize(handle)
        pcm = b''.join(chunks)
        assert pcm, (language, text, 'No original PCM')
        return dict(samples=len(pcm)//2, sha256=hashlib.sha256(pcm).hexdigest())
    finally:
        dll.eciDelete(handle)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--engine-dir', type=Path, help=argparse.SUPPRESS)
    args = p.parse_args()
    assert os.name == 'nt' and C.sizeof(C.c_void_p) == 4, 'Use Windows x86 Python'
    output = args.output.resolve()
    if args.engine_dir:
        # Syn modules stay loaded even after the last legacy ECI handle is
        # deleted. A child process owns all those mappings, so the parent can
        # remove the temporary binaries reliably when the child exits.
        os.chdir(args.engine_dir)
        dll = load(args.engine_dir/'ECI.DLL')
        version = C.create_string_buffer(256)
        dll.eciVersion(version)
        rows = []
        for language in (0x10000, 0x10001):
            for text in TIMES:
                row = dict(language=language, text=text,
                           pcm=observe(dll, language, text),
                           spr=observe(dll, language, text, phonemes=True))
                rows.append(row)
                print(json.dumps(row), flush=True)
        output.write_text(json.dumps(dict(url=URL, archive_sha256=ARCHIVE_SHA256,
            files=FILES, engine=version.value.decode('ascii'), observations=rows), indent=2)+'\n')
        return
    archive = urllib.request.urlopen(URL, timeout=60).read()
    assert hashlib.sha256(archive).hexdigest() == ARCHIVE_SHA256
    with tempfile.TemporaryDirectory(prefix='reloq-syn-reference-') as tmp:
        folder = Path(tmp)
        with zipfile.ZipFile(io.BytesIO(archive)) as z:
            assert z.testzip() is None
            for name, digest in FILES.items():
                data = z.read('synthDrivers/eloquence/' + name)
                assert hashlib.sha256(data).hexdigest() == digest
                (folder/name).write_bytes(data)
            ini = z.read('synthDrivers/eloquence/ECI.INI').decode('latin1')
        ini = re.sub(r'(?im)^(\s*Path\s*=\s*).*?[\\/]?([^\\/\r\n]+\.syn)\s*$',
                     lambda m: m[1] + str(folder/m[2]), ini)
        (folder/'ECI.INI').write_bytes(ini.encode('ascii'))
        subprocess.run([sys.executable, str(Path(__file__).resolve()),
                        '--engine-dir', str(folder), '--output', str(output)],
                       check=True, timeout=180)


if __name__ == '__main__':
    main()
