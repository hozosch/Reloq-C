"""Slower A/B/C hearing trials: 1.0.1 colour, native-16 AF, and gentle voice harmonics.

No model or release package is changed. Native rate WAVs are primary; only
announced comparison compilations use polyphase conversion to 48 kHz.
"""
from pathlib import Path
import argparse
import ctypes as C
import hashlib
import json
import math
import os
import sys
import wave
import zipfile

import numpy as np
from scipy.signal import resample_poly, welch

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'nvda'))
from _reloq_eci_direct import (DirectEciEngine,SAMPLE_RATE,NATIVE_SPEED,NATIVE_PITCH,
    NATIVE_INFLECTION,NATIVE_VOLUME,GENDER,HEAD_SIZE,EciIndex)
RATES=((1,11025),(3,16000),(2,22050),(4,32000),(5,44100),(6,48000))
CASES={
 'Deutsch':(0x40000,(('01_Stimmloses_S','Es ist nass. Das Fass ist weiß. Klaus isst Reis.'),
 ('02_Stimmhaftes_S','Susi summt. Sie sehen sieben Segel.'),
 ('03_Andere_Konsonanten','Tina schaut nach. Karl kocht Tee. Peter findet frische Pflaumen.'),
 ('04_Z_und_ts','Zwei Züge ziehen. Die Katze sitzt auf dem Platz.'),
 ('05_Obere_Stimmanteile','Eine warme, ruhige Melodie am Meer.'))),
 'US_English':(0x10000,(('01_Unvoiced_S','Sue sips soup. Sam sits still beside a silent sea.'),
 ('02_Voiced_Z','Lazy bees buzz by the roses.'),
 ('03_Other_Consonants','Tim shuts the thick book. Peter finds fresh fruit.'),
 ('04_Affricates','Jack chooses two rich cheeses. The cats sit outside.'),
 ('05_Upper_Voice','A warm, mellow aroma.')))
}

def engine(path,dialect):
 e=DirectEciEngine(path,dialect=dialect,encoding='utf8',experimental_generic=True)
 for key,value in ((NATIVE_SPEED,75),(NATIVE_PITCH,65),(NATIVE_INFLECTION,30),
                   (NATIVE_VOLUME,100),(GENDER,0),(HEAD_SIZE,50)):
  e.set_parameter(key,value)
 return e

def capture(e,text):
 parts=[];marks=[];size=0
 def take(data):
  nonlocal size
  parts.append(data);size+=len(data)
 assert e.speak([text,EciIndex(71)],take,lambda n:marks.append((n,size)))
 assert marks==[(71,size)]
 return np.frombuffer(b''.join(parts),dtype='<i2').copy(),marks

def energy(x):return float(np.mean(np.asarray(x,dtype=float)**2))

def power(x,rate,low,high):
 f,p=welch(np.asarray(x,dtype=float),fs=rate,nperseg=min(len(x),4096))
 return float(np.trapezoid(p[(f>=low)&(f<=high)],f[(f>=low)&(f<=high)]))

def at48(x,rate):
 g=math.gcd(rate,48000)
 return resample_poly(x,48000//g,rate//g) if rate!=48000 else x

def write(path,data,rate):
 path.parent.mkdir(parents=True,exist_ok=True)
 assert np.isfinite(data).all() and np.max(np.abs(data))<1
 pcm=np.rint(data*32767).astype('<i2')
 with wave.open(str(path),'wb') as w:
  w.setnchannels(1);w.setsampwidth(2);w.setframerate(rate);w.writeframes(pcm.tobytes())
 return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument('--library',type=Path,required=True);p.add_argument('--reference',type=Path,required=True)
 p.add_argument('--output',type=Path,required=True);a=p.parse_args()
 a.output=a.output.resolve();a.output.mkdir(parents=True,exist_ok=False)
 dll=C.CDLL(str(a.library.resolve()))
 native=dll.reloq_native16_listening_set;native.argtypes=(C.c_uint,);native.restype=C.c_int
 source=dll.reloq_source_listening_set;source.argtypes=(C.c_uint,C.c_double);source.restype=C.c_int
 report={'listening_only':True,'native_speed':75,'nvda_rate_equivalent':30,
         'voice_harmonic_shelf_max_db':2,'languages':{}}
 for lang,(dialect,cases) in CASES.items():
  folder=a.output/('Reloq_Nativ16_Klarheit_'+lang);folder.mkdir()
  e=engine(a.library.resolve(),dialect);r=engine(a.reference.resolve(),dialect)
  captured={};gains={};metrics=[];marks={}
  def render(n,h,g,text):
   assert native(n)==0 and source(h,g)==0
   return capture(e,text)
  try:
   for code,hz in RATES:
    e.set_parameter(SAMPLE_RATE,code);r.set_parameter(SAMPLE_RATE,code)
    body=cases[-1][1]
    noise,_=render(1,0,0,body)
    voice,_=render(1,0,1,body)
    shaped,_=render(1,10,1,body)
    gain=1 if hz==11025 else math.sqrt(energy(voice.astype(float)-noise)/energy(shaped.astype(float)-noise))
    assert .5<gain<1.1
    gains[hz]=gain
    for name,text in cases:
     for variant,n,h,g in (('A',0,0,1),('B',1,0,1),('C',1,10,gain)):
      x,ix=render(n,h,g,text);captured[variant,hz,name]=x;marks[variant,hz,name]=ix
     original,ix=capture(r,text)
     captured['R',hz,name]=original;marks['R',hz,name]=ix
     base=captured['A',hz,name]
     assert all(len(captured[v,hz,name])==len(base) and marks[v,hz,name]==ix for v in ('A','B','C'))
     assert not any(np.any(np.abs(captured[v,hz,name].astype(np.int32))>=32767) for v in ('A','B','C'))
     if lang=='US_English' or name!='04_Z_und_ts':
      assert np.array_equal(original,base),(lang,hz,name,'unrelated PCM changed')
     if hz==11025:assert np.array_equal(base,captured['B',hz,name]) and np.array_equal(base,captured['C',hz,name])
    name=cases[-1][0]
    b=captured['B',hz,name].astype(float)-noise
    c=captured['C',hz,name].astype(float)-noise
    diff=10*math.log10((power(c,hz,2500,5000)/power(c,hz,80,800))/(power(b,hz,2500,5000)/power(b,hz,80,800)))
    metrics.append({'rate':hz,'voice_gain':gain,'upper_to_lower_change_db':diff,
                    'body_total_energy_difference_db':10*math.log10(energy(captured['C',hz,name])/energy(captured['B',hz,name]))})
    print(lang,hz,'upper/lower dB',round(diff,3),flush=True)
   # Noise isolation uses identical source seeds. Voice-only shaping must not
   # move AF/AH, including in voiced Z. Compare the PCM after muting AV.
   noise_checks=[]
   for code,hz in ((3,16000),(6,48000)):
    e.set_parameter(SAMPLE_RATE,code)
    for name,text in cases[:2]:
     b,_=render(1,0,0,text);c,_=render(1,10,0,text)
     assert np.array_equal(b,c),(lang,hz,name,'voice colour changed noise')
     noise_checks.append({'rate':hz,'case':name,'exact':True})
   # One shared file scale per rate and language; no per-variant gain or
   # compressor. Exact native-rate files remain at their original clock.
   scales={}
   for _,hz in RATES:
    body=captured['A',hz,cases[-1][0]].astype(float)
    active=body[np.abs(body)>=64]
    scales[hz]=2600/math.sqrt(energy(active))
   peak=max(float(np.max(np.abs(x.astype(float))))*scales[hz]/32768 for (v,hz,n),x in captured.items())
   trim=min(1,.8/max(peak,1e-12));scaled={};hashes={}
   for (v,hz,name),x in captured.items():
    if v=='R' and not (lang=='Deutsch' and name=='04_Z_und_ts'):continue
    if hz==11025 and v in ('B','C'):continue
    y=x.astype(float)/32768*scales[hz]*trim;scaled[v,hz,name]=y
    path=folder/'Einzeldateien'/name/f'{hz}_{v}.wav';hashes[str(path.relative_to(folder))]=write(path,y,hz)
   labeler=engine(a.reference.resolve(),dialect);labeler.set_parameter(SAMPLE_RATE,1)
   label_cache={};gap=np.zeros(14400)
   def announce(text):
    if text not in label_cache:
     x,_=capture(labeler,text);y=at48(x.astype(float)/32768,11025)
     label_cache[text]=y*.25/max(float(np.max(np.abs(y))),.001)
    return label_cache[text]
   def seq(filename,entries):
    pieces=[]
    for v,hz,name in entries:
     description=name[3:].replace('_',' ')
     text=(f'{description}. {hz} Hertz. Fassung {v}.' if lang=='Deutsch' else f'{description}. {hz} Hertz. Version {v}.')
     pieces.extend([announce(text),gap,at48(scaled[v,hz,name],hz),gap])
    return write(folder/filename,np.concatenate(pieces),48000)
   try:
    entries=[(v,hz,name) for name,_ in cases[:3]+cases[-1:] for hz in (16000,48000) for v in ('A','B','C')]
    seq('00_Direktvergleich.wav',entries)
    for v in ('A','B','C'):
     seq('01_Ratenfolge_'+v+'.wav',[(('A' if hz==11025 else v),hz,cases[-1][0]) for _,hz in RATES])
     seq('02_Konsonanten_'+v+'.wav',[(('A' if hz==11025 else v),hz,name) for _,hz in RATES for name,_ in cases[:3]])
    if lang=='Deutsch':seq('03_Z_Pegel_101_gegen_Kontext.wav',[(v,hz,'04_Z_und_ts') for hz in (11025,48000) for v in ('R','A')])
   finally:labeler.close()
   german=lang=='Deutsch'
   text=('''Neue Hörversuche: native 16-kHz-Konsonanten und dezente obere Stimmanteile

A: Klangabstimmung aus 1.0.1. Deutsch verwendet bereits wieder den Kontextpegel
für die Reibung von /ts/ statt der später hinzugefügten 65/55-Sonderbalance.
B: Native 16-kHz-Parallelresonanzen als Spektralreferenz für Konsonanten. Der
p/f-artige breite Pfad bleibt unverändert. Die höheren Bänder werden ab
6,5 kHz weich an die bisherige Erweiterung übergeben; oberhalb 8 kHz bleibt
sie erhalten. S-Gesamtenergie wird im Filterentwurf angeglichen.
C: B plus eine zurückhaltende Anreicherung der periodischen oberen Stimmanteile:
0,4 / 0,8 / 1,2 / 1,6 / 2 dB nominell von 16 bis 48 kHz vor Pegelabgleich.
Das ist deutlich weniger als die vorigen Versuche mit bis zu 8 bzw. 12 dB.
S-Rauschen und Behauchtheit werden durch diese Stimmquellenänderung nicht geformt.
R: Nur im gesonderten Z-Vergleich: veröffentlichte 1.0.1 vor der /ts/-Korrektur.

Zuerst 00_Direktvergleich.wav hören. 01 vergleicht den Stimmkörper über die
Raten; 02 enthält S, stimmhaftes S und weitere Konsonanten. 03 zeigt separat
die deutsche Z-/ts-Pegelkorrektur bei 11,025 und 48 kHz.
''') if german else ('''Listening trials: native 16-kHz consonant colour and gentle upper voice harmonics

A: Released 1.0.1 colour and source levels. US English is unchanged.
B: Native 16-kHz parallel resonances anchor consonant colour. The broad p/f
path is unchanged. A smooth transition above 6.5 kHz retains the established
extension above 8 kHz. Integrated S energy is matched in the filter design.
C: B with gentle periodic upper-harmonic enrichment: nominal 0.4 / 0.8 / 1.2 /
1.6 / 2 dB from 16 to 48 kHz before voice level matching. This is much less
than the previous trials at up to 8 or 12 dB. AF and AH noise stay separate.

Start with 00_Direktvergleich.wav. 01 compares voice body across sample rates;
02 contains unvoiced S, voiced Z, and other consonants.
''')
   text+=('''
Native speed 75, equivalent to NVDA 30; pitch 65, inflection 30, head 50,
male model, volume 100, no user dictionaries. All variants use identical
texts, clocks, timing, and one common file gain per rate. No limiter or
compression. C uses one voice-level calibration per rate, shared by all
texts. Native-rate WAVs are in Einzeldateien. Announced compilations use
48-kHz polyphase assembly only for easy comparison. 11.025 kHz is unchanged
except the separately documented German /ts/ balance. Native 16 kHz here
means the captured numeric bank implemented independently in C, rather
than an Apple recording. Studio speech and Vocalizer are listening goals;
this experiment does not claim to match their recordings or synthesizers.

These are unaccepted listening experiments, not a release or latency fix.
''')
   (folder/'Liesmich.txt').write_text(text,encoding='utf-8')
   (folder/'Messwerte.json').write_text(json.dumps({'voice':metrics,'noise_isolation':noise_checks,'wave_sha256':hashes},indent=2)+'\n')
   archive=a.output/(folder.name+'.zip')
   temporary=archive.with_suffix('.zip.tmp')
   with zipfile.ZipFile(temporary,'w',compression=zipfile.ZIP_DEFLATED,compresslevel=6) as z:
    for f in sorted(folder.rglob('*')):
     if f.is_file():z.write(f,f.relative_to(a.output))
   with temporary.open('rb') as stream:os.fsync(stream.fileno())
   with zipfile.ZipFile(temporary) as z:assert z.testzip() is None
   temporary.replace(archive)
   report['languages'][lang]={'voice':metrics,'noise_isolation':noise_checks,'files':len(list(folder.rglob('*.wav'))),'no_new_clipping':True}
  finally:e.close();r.close()
 (a.output/'validation.json').write_text(json.dumps(report,indent=2)+'\n')

if __name__=='__main__':main()
