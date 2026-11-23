"""Compare the private provider to the listener-approved timing experiment.

This Linux development check uses the hidden provider symbol in unstripped
local ELF builds. It is not a public ECI interface or a packaged test.
"""
import argparse
import ctypes as ct
import hashlib
import os
from pathlib import Path
import struct
import subprocess

U = ct.c_int32
Frame = ct.CFUNCTYPE(ct.c_int, ct.c_void_p, ct.POINTER(U), U)

class Request(ct.Structure):
    _fields_ = [('language',ct.c_int),('annotations',ct.c_int),('phonemes_only',ct.c_int),
                ('voice',ct.c_int*8),('text',ct.c_char_p),('marks',ct.c_void_p),
                ('mark_count',ct.c_size_t),('frame',Frame),('index',ct.c_void_p),
                ('context',ct.c_void_p),('phonemes',ct.c_void_p),('phonemes_capacity',ct.c_size_t),
                ('span',ct.c_void_p),('sample_rate',ct.c_int),('output_rate',ct.c_int),
                ('phoneme_indices',ct.c_int),('controls',ct.c_void_p),('control_count',ct.c_size_t),
                ('control',ct.c_void_p),('german_affricate_markers',ct.c_int),
                ('cjk_slow_wpm',ct.c_int),('cjk_has_wpm',ct.c_int)]

class Result(ct.Structure):
    _fields_=[('unsupported',ct.c_int),('error',ct.c_int),('frames',ct.c_size_t),
              ('phonemes_length',ct.c_size_t),('ticks',ct.c_int64)]

def function(path):
    library=ct.CDLL(str(path.resolve()))
    lines=subprocess.check_output(['nm','-S',str(path)]).decode().splitlines()
    targets=[line.split() for line in lines if line.split()[-1:] == ['reloq61_generate']]
    assert len(targets)==1, targets
    offset=int(targets[0][0],16)
    with open('/proc/self/maps') as maps:
        rows=[line for line in maps if str(path.resolve()) in line and line.split()[2]=='00000000']
    assert len(rows)==1,rows
    base=int(rows[0].split('-')[0],16)
    return library,ct.CFUNCTYPE(ct.c_int,ct.POINTER(Request),ct.POINTER(Result))(base+offset)

def speak(f,language,text,rate,speed,gender):
    digest=hashlib.sha256()
    def sink(_,values,volume):
        digest.update(ct.string_at(values,62*4))
        digest.update(struct.pack('i',volume))
        return 0
    callback=Frame(sink)
    buf=ct.create_string_buffer(1048576)
    req=Request(language,1,0,(ct.c_int*8)(gender,50,65,30,0,0,speed,100),
                text.encode(),None,0,callback,None,None,ct.addressof(buf),len(buf),
                None,0,rate,0,None,0,None,0,0,0)
    result=Result()
    rc=f(ct.byref(req),ct.byref(result))
    assert rc==0 and not result.error and not result.unsupported and result.ticks,(language,text,rc,result.error,result.unsupported,result.ticks)
    return digest.digest(),result.ticks,buf.value

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library',type=Path,required=True)
    parser.add_argument('--timing',type=Path,required=True,
                        help='private timing-probe library from build_native_probe.py timing')
    args=parser.parse_args()
    os.environ.update(RELOQ_ARAGON_PROBE_AU_PERCENT='40', RELOQ_ARAGON_PROBE_SH_PERCENT='65',
                      RELOQ_ARAGON_PROBE_TIMING_MODE='2', RELOQ_ARAGON_PROBE_TIMING_SCOPE='1',
                      RELOQ_ARAGON_PROBE_TIMING_PERCENT='70')
    current,current_call=function(args.library)
    previous,previous_call=function(args.timing)
    words=('auca.','auto.','la auca.','pauto.','pauca.','cudiau.',
           'a hueso.','la huerta.','casa.','hola.','`[.1Sa.0da].','`[.1Ca.0da].','`[.1Ta.0da].')
    checks=0
    for rate in (11025,16000,48000):
     for speed in (40,80):
      for gender in (0,1):
       for word in words:
        expected=speak(previous_call,0x20000,word,rate,speed,gender)
        actual=speak(current_call,0x130000,word,rate,speed,gender)
        assert expected==actual,(rate,speed,gender,word,'ticks',expected[1],actual[1],
                                 'phones',expected[2],actual[2])
        checks+=1
    print(f'{checks} private AU, SH, affricate, TH and boundary frame comparisons passed')

if __name__=='__main__':main()
