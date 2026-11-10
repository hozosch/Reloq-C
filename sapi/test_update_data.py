"""Exercise actual release metadata parsing and trust boundaries on the host."""
import ctypes
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent

class Update(ctypes.Structure):
    _fields_ = [('version',ctypes.c_char*48),('name',ctypes.c_char*96),
                ('url',ctypes.c_char*512),('sha256',ctypes.c_char*65),('size',ctypes.c_uint32)]

with tempfile.TemporaryDirectory(prefix='reloq-update-test-') as folder:
    libpath = Path(folder)/'update.so'
    subprocess.run(['cc','-std=c11','-O2','-Wall','-Wextra','-Werror',
                    '-Wno-misleading-indentation','-shared','-fPIC',str(HERE/'update_data.c'),
                    '-o',str(libpath)],check=True)
    lib=ctypes.CDLL(str(libpath));parse=lib.reloq_update_parse
    parse.argtypes=[ctypes.c_char_p,ctypes.c_size_t,ctypes.c_char_p,ctypes.POINTER(Update)]
    parse.restype=ctypes.c_int
    def asset(version):
        name=f'reloq-c-sapi5-{version}-setup.exe'
        return {'name':name,'browser_download_url':f'https://github.com/hozosch/Reloq-C/releases/download/v{version}/{name}',
                'size':12345,'digest':'sha256:'+hashlib.sha256(b'fixture').hexdigest()}
    def call(data,installed='1.3.0'):
        encoded=(json.dumps(data,ensure_ascii=True,separators=(',',':')).encode() if not isinstance(data,bytes) else data)
        output=Update();result=parse(encoded,len(encoded),installed.encode(),ctypes.byref(output))
        return result,output
    def feed(*assets):return {'draft':False,'prerelease':False,'ignored':{'quoted':'x\\y\"z'},'assets':list(assets)}
    assert call(feed(asset('1.3.0')))[0]==0
    assert call(feed(asset('1.2.9')))[0]==0
    result,update=call(feed(asset('1.3.1'),asset('1.4.0'),asset('1.3.10')))
    assert result==1 and update.version==b'1.4.0' and update.size==12345
    assert call(feed(asset('1.3.10')),installed='1.3.9')[0]==1
    for change in ({'draft':True},{'prerelease':True},{'draft':None},{'assets':None}):
        data=feed(asset('1.3.1'));data.update(change);assert call(data)[0]==-1
    a=asset('1.3.1')
    for bad in [a['browser_download_url'].replace('https:', 'http:'),
                a['browser_download_url'].replace('github.com','example.com'),
                a['browser_download_url']+'?x=1',a['browser_download_url'].replace('/v1.3.1/','/../'),
                a['browser_download_url'].replace('github.com','github.com@evil.test')]:
        assert call(feed({**a,'browser_download_url':bad}))[0]==-1,bad
    for bad in [0,-1,128*1024*1024+1,'123',None]:assert call(feed({**a,'size':bad}))[0]==-1
    for bad in ['sha256:'+('z'*64),'sha256:'+('a'*63),None,'']:
        assert call(feed({**a,'digest':bad}))[0]==-1
    upper={**a,'digest':a['digest'].upper().replace('SHA256:', 'sha256:')}
    assert call(feed(upper))[1].sha256==a['digest'][7:].encode()
    assert call(b'{"draft":false,"draft":false,"prerelease":false,"assets":[]}')[0]==-1
    assert call(b'{"draft":false,"prerelease":false,"assets":[]', '1.3.0')[0]==-1
    assert call(b'{"draft":false,"prerelease":false,"assets":[]} trailing')[0]==-1
    assert call(feed(a),installed='1.3.0-dev1')[0]==-1
    print('SAPI update selection: version ordering, malformed metadata, URL, size and SHA-256 checks passed')
