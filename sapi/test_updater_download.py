"""Exercise WinHTTP streaming and Windows SHA-256 against a known Git blob."""
import argparse
import hashlib
import os
from pathlib import Path
import subprocess
import tempfile

parser=argparse.ArgumentParser()
parser.add_argument('--directory',type=Path,required=True)
parser.add_argument('--commit',required=True)
args=parser.parse_args()
assert len(args.commit)==40 and all(c in '0123456789abcdef' for c in args.commit)
exe=args.directory.resolve()/'Test-Update.exe';assert exe.is_file()
data=(Path(__file__).resolve().parents[1]/'LICENSE').read_bytes()
digest=hashlib.sha256(data).hexdigest()
url=f'https://raw.githubusercontent.com/hozosch/Reloq-C/{args.commit}/LICENSE'
with tempfile.TemporaryDirectory(prefix='reloq-download-test-') as folder:
    env={**os.environ,'TEMP':folder,'TMP':folder}
    for size,sha,success in [(len(data),digest,True),(len(data),('0'*64),False),
                             (len(data)-1,digest,False),(len(data)+1,digest,False)]:
        result=subprocess.run([str(exe),'--test-download',url,str(size),sha],env=env,timeout=120)
        assert (result.returncode==0)==success,(size,sha,result.returncode)
        assert not list(Path(folder).iterdir()),'Downloaded temporary file was not removed'
print('SAPI updater: real WinHTTP download, SHA-256, size rejection and temporary-file cleanup passed')
