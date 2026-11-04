"""Measure speech spectra from the user's 32-bit Eloquence reference.
Only numeric measurements are retained; the engine and audio stay in the runner.
"""
import array
import cmath
import hashlib
import json
import math
import os
from pathlib import Path
import sys
import tempfile
import urllib.request
import zipfile

UPSTREAM = 'https://github.com/pumper42nickel/eloquence_threshold/releases/download/v0.20210417.01/eloquence.nvda-addon'
FAST = 'https://raw.githubusercontent.com/fastfinge/eloquence_64/b24a6c88446581e50867abac23807233829b367f/addon/synthDrivers/'
def fetch(url, path):
    req=urllib.request.Request(url,headers={'User-Agent':'ReLoq-reference-probe'})
    with urllib.request.urlopen(req,timeout=90) as response:
        path.write_bytes(response.read())

with tempfile.TemporaryDirectory() as td:
    base=Path(td)
    fetch(UPSTREAM,base/'upstream.zip')
    with zipfile.ZipFile(base/'upstream.zip') as package:
        for name in ('ECI.DLL','DEU.SYN','ENU.SYN'):
            (base/name).write_bytes(package.read('synthDrivers/eloquence/'+name))
    fetch(FAST+'eloquence/ECI.INI',base/'ECI.INI')
    fetch(FAST+'_eci_engine.py',base/'_eci_engine.py')
    sys.path.insert(0,str(base))
    import _eci_engine as ec
    pcm=bytearray()
    def sink(event,**kw):
        if event=='audio':
            pcm.extend(kw.get('data',b''))
    config=ec.EngineConfig(eci_path=str(base/'ECI.DLL'),data_directory='',
        language_code='deu',enable_abbrev_dict=False,enable_phrase_prediction=False,
        voice_variant=1,sample_rate=11025)
    engine=ec.EciEngine(sink,config)
    engine.start()
    for param,value in ((ec.HSZ,50),(ec.PITCH,65),(ec.FLUCTUATION,30),
                        (ec.RGH,0),(ec.BTH,0),(ec.RATE,75),(ec.VLM,100)):
        engine.set_voice_param(param,value)
    data={'eci_sha256':hashlib.sha256((base/'ECI.DLL').read_bytes()).hexdigest(),
          'deu_sha256':hashlib.sha256((base/'DEU.SYN').read_bytes()).hexdigest(),
          'rate':engine.sample_rate,'samples':{}}
    def lpc_peaks(frame,rate):
        n=len(frame)
        x=[(frame[i]-(0.97*frame[i-1] if i else 0))*(0.54-0.46*math.cos(2*math.pi*i/(n-1))) for i in range(n)]
        order=14
        r=[sum(x[i]*x[i-k] for i in range(k,n)) for k in range(order+1)]
        if r[0]<1e-6:return []
        a=[1.0]; error=r[0]
        for k in range(1,order+1):
            reflection=-(r[k]+sum(a[j]*r[k-j] for j in range(1,k)))/max(error,1e-10)
            reflection=max(-0.999,min(0.999,reflection))
            a=a+[0.0]
            a=[a[j]+reflection*a[k-j] if 0<j<k else a[j] for j in range(k+1)]
            a[k]=reflection
            error*=1-reflection*reflection
        powers=[]
        for freq in range(150,5001,25):
            z=cmath.exp(-2j*math.pi*freq/rate)
            den=sum(a[j]*z**j for j in range(len(a)))
            powers.append((freq,1/max(abs(den),1e-10)))
        peaks=[(powers[i][1],powers[i][0]) for i in range(1,len(powers)-1)
               if powers[i][1]>powers[i-1][1] and powers[i][1]>=powers[i+1][1]]
        return sorted(freq for _,freq in sorted(peaks,reverse=True)[:5])
    for name,phrase in (('homepage','Homepage'),('page','Page'),('drei','drei'),
                        ('kann_er','Kann er'),('er','er')):
        pcm.clear()
        engine.add_text(phrase.encode('cp1252'))
        engine.insert_index(ec.FINAL_INDEX)
        engine.synthesize()
        samples=array.array('h')
        samples.frombytes(pcm)
        assert len(samples)>1000,(name,len(samples))
        rate=engine.sample_rate
        window=int(rate*0.030)
        hop=int(rate*0.010)
        rows=[]
        for start in range(0,len(samples)-window,hop):
            frame=samples[start:start+window]
            rms=math.sqrt(sum(s*s for s in frame)/len(frame))
            rows.append({'ms':round(start*1000/rate,1),'rms':round(rms,1),
                         'peaks':lpc_peaks(frame,rate)})
        data['samples'][name]={'count':len(samples),'frames':rows}
    engine.delete()
    Path('reference-metrics.json').write_text(json.dumps(data,indent=2))
    print('Original ECI voice measured:',{name:row['count'] for name,row in data['samples'].items()})
