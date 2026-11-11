"""Observe Italian apostrophes with the pinned external x86 ECI reference.

Only phoneme strings and sample counts are recorded. Original binaries stay
in a temporary directory and are never bundled with the implementation.
"""
import argparse
import ctypes as C
import hashlib
import io
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import urllib.request
import zipfile

from observe_syn_times import URL, ARCHIVE_SHA256, load, observe

FILES = {
    'ECI.DLL': 'c1653e61e9fd599421d2f7faee21eda171d3a8ff4329343ddb24a2dd565fba5f',
    'ITA.SYN': '6965a5704dc42c215a9c3e3a2a82096d3d555cd685d28366c6c140a38ac4f7bf',
    'ENU.SYN': '1a5f225f2d7c6fe1deff7df57eb285600430ac98dc3509e613051d20715996e9',
}
CASES = ('po', "po'", 'po\u2019', 'p\u00f2', "Aspetta un po'", 'Aspetta un po\u2019',
         "un po' di latte", "mo'", "to'", "va'", "sta'", "da'", "di'", "be'",
         "l'amico", "perche'", "qual'e'", "'po'", "po'chi")


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--engine-dir',type=Path,help=argparse.SUPPRESS)
    args=parser.parse_args()
    assert os.name=='nt' and C.sizeof(C.c_void_p)==4, 'Use Windows x86 Python'
    output=args.output.resolve()
    if args.engine_dir:
        os.chdir(args.engine_dir)
        dll=load(args.engine_dir/'ECI.DLL')
        rows=[]
        for text in CASES:
            row={'text':text,'spr':observe(dll,0x50000,text,phonemes=True),
                 'pcm':observe(dll,0x50000,text)}
            rows.append(row)
            print(json.dumps(row,ensure_ascii=True),flush=True)
        output.write_text(json.dumps({'archive_sha256':ARCHIVE_SHA256,'files':FILES,
            'observations':rows},ensure_ascii=True,indent=2)+'\n',encoding='utf8')
        return
    archive=urllib.request.urlopen(URL,timeout=60).read()
    if hashlib.sha256(archive).hexdigest()!=ARCHIVE_SHA256:
        raise ValueError('Unexpected original reference archive')
    with tempfile.TemporaryDirectory(prefix='reloq-italian-reference-') as tmp:
        folder=Path(tmp)
        with zipfile.ZipFile(io.BytesIO(archive)) as z:
            for name,digest in FILES.items():
                data=z.read('synthDrivers/eloquence/'+name)
                if hashlib.sha256(data).hexdigest()!=digest:
                    raise ValueError('Unexpected original reference: '+name)
                (folder/name).write_bytes(data)
            ini=z.read('synthDrivers/eloquence/ECI.INI').decode('latin1')
        ini=re.sub(r'(?im)^(\s*Path\s*=\s*).*?[\\/]?([^\\/\r\n]+\.syn)\s*$',
                   lambda m:m[1]+str(folder/m[2]),ini)
        (folder/'ECI.INI').write_bytes(ini.encode('latin1'))
        subprocess.run([sys.executable,__file__,'--output',str(output),
                        '--engine-dir',str(folder)],check=True,timeout=150)


if __name__=='__main__':main()
