"""Compare requested live heap bytes through public ECI; PCM stays in RAM.

Linux/glibc development probe. Allocator overhead, untracked mmap arenas,
stacks and RSS are excluded. Interposition changes timings, so this is not a
CPU/latency benchmark. No shim or measurement code enters an install package.
"""
from pathlib import Path
import argparse,ctypes,hashlib,json,os,subprocess,sys,tempfile
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'nvda'))
from _reloq_eci_direct import DirectEciEngine,EciIndex,NATIVE_SPEED


class Stats(ctypes.Structure):
    _fields_=[(n,ctypes.c_size_t) for n in ('peak','live','allocations','reallocations','total_requested')]+[('overflow',ctypes.c_int)]


def worker(library,shim):
    watch=ctypes.CDLL(str(shim));watch.heap_watch_begin.argtypes=[]
    watch.heap_watch_end.argtypes=[ctypes.POINTER(Stats)]
    engine=DirectEciEngine(library,dialect=0x40000,encoding='utf8',experimental_generic=True)
    engine.set_parameter(NATIVE_SPEED,98)
    rows=[]
    try:
        assert engine.speak(['Hallo.'],lambda _:None,lambda _:None)
        for words in (80,800,4000):
            text='Die Sprachausgabe ist klar. '*(words//4)
            digest=hashlib.sha256();samples=[0];marks=[]
            def pcm(data):digest.update(data);samples[0]+=len(data)//2
            stats=Stats();watch.heap_watch_begin()
            try:ok=engine.speak([text,EciIndex(1)],pcm,marks.append)
            finally:watch.heap_watch_end(ctypes.byref(stats))
            assert ok and marks==[1] and samples[0] and not stats.overflow
            rows.append(dict(words=words,utf8_bytes=len(text.encode()),
                requested_peak_bytes=stats.peak,requested_live_bytes=stats.live,
                allocations=stats.allocations,reallocations=stats.reallocations,
                total_requested_bytes=stats.total_requested,samples=samples[0],pcm_sha256=digest.hexdigest()))
    finally:engine.close()
    return dict(version=engine.native_version,library=str(library),measurements=rows)


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--library',type=Path,default=ROOT/'eci/libreloq_eci.so')
    p.add_argument('--baseline',type=Path)
    p.add_argument('--output',type=Path)
    p.add_argument('--worker',type=Path,help=argparse.SUPPRESS)
    a=p.parse_args()
    if a.worker:print(json.dumps(worker(a.library.resolve(),a.worker.resolve())));return
    with tempfile.TemporaryDirectory(prefix='reloq-heap-') as folder:
        shim=Path(folder)/'heap.so'
        subprocess.run(['cc','-std=c11','-O2','-Wall','-Wextra','-Werror','-fPIC','-shared',
            str(ROOT/'tools/eci_heap_watch.c'),'-o',str(shim)],check=True)
        rows=[]
        for library in ([a.baseline] if a.baseline else [])+[a.library]:
            result=subprocess.run([sys.executable,str(Path(__file__).resolve()),'--library',str(library.resolve()),
                '--worker',str(shim)],env={**os.environ,'LD_PRELOAD':str(shim)},
                check=True,capture_output=True,text=True,timeout=120)
            rows.append(json.loads(result.stdout))
    if len(rows)==2:
        for before,after in zip(rows[0]['measurements'],rows[1]['measurements']):
            assert (before['samples'],before['pcm_sha256'])==(after['samples'],after['pcm_sha256']),before['words']
    report=dict(method='Requested live malloc/calloc/realloc sizes during warmed public German ECI synthesis; Linux/glibc; not RSS or a timing benchmark',no_audio_files=True,pcm_equal_when_comparing=len(rows)==2,builds=rows)
    if a.output:a.output.write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2))


if __name__=='__main__':main()
