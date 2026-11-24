"""Derive the variable and symbol correspondence between the US-English and
the Finnish-based (Persian) rule providers.

Both providers were built from one rule family: 410 rules exist in both, and
289 of them have the same number of lines. Lines that differ only in a state
variable or an authored-symbol index give the correspondence; identical lines
confirm an identity. Run with an output path to write the mapping as JSON.
This is groundwork for moving the US sentence melody into the Persian core;
see docs/PERSIAN-NATIVE-MELODY-PLAN.md.
"""
import re,glob,collections,json,sys
from pathlib import Path
V=str(Path(__file__).resolve().parents[2]/'vendor/openevv61/lang')+'/'
def funcs(lang,prefix):
    out={}
    for path in sorted(glob.glob(V+lang+'/delta_rules_c*.c')):
        text=open(path,encoding='utf8').read()
        for m in re.finditer(r'^int32_t reloq_'+prefix+r'_(\w+)\(',text,re.M):
            name=m.group(1);end=text.find('\nstatic int32_t evv_'+name+'(',m.start())
            if end<0:continue
            out[name]=(text[m.start():end],path.split('/')[-1])
    return out
FI=funcs('fifi','fifi');EN=funcs('enus61','enus61')
def norm(body,prefix):
    return body.replace('reloq_call_'+prefix+'_','reloq_call_L_').replace('reloq_'+prefix+'_','reloq_L_')
tok=re.compile(r'DG_\w+|STATE(?:_D|_AT)?\(\s*(?:\w+\s*,\s*)?(\w+)|delta_sym_ref\[(\d+)\]|vstmtbl\[(\d+)\]')
state=collections.defaultdict(collections.Counter);sym=collections.defaultdict(collections.Counter)
same=0;used=0
for name in sorted(set(FI)&set(EN)):
    a=norm(FI[name][0],'fifi').split('\n');b=norm(EN[name][0],'enus61').split('\n')
    if len(a)!=len(b):continue
    used+=1
    for x,y in zip(a,b):
        if x==y:continue
        # same shape after masking identifiers?
        mx=re.sub(r'\b[swfcbd]\d+\b|delta_sym_ref\[\d+\]','@',x);my=re.sub(r'\b[swfcbd]\d+\b|delta_sym_ref\[\d+\]','@',y)
        if mx!=my:continue
        ix=re.findall(r'\b([swfcbd]\d+)\b|delta_sym_ref\[(\d+)\]',x);iy=re.findall(r'\b([swfcbd]\d+)\b|delta_sym_ref\[(\d+)\]',y)
        for (s1,d1),(s2,d2) in zip(ix,iy):
            if s1 and s2:state[s2][s1]+=1
            if d1 and d2:sym[d2][d1]+=1
    # identical lines: identity evidence
    for x,y in zip(a,b):
        if x==y:
            for s,d in re.findall(r'STATE(?:_D|_AT)?\(\s*(?:\w+\s*,\s*)?([swfcbd]\d+)|delta_sym_ref\[(\d+)\]',x):
                if s:state[s][s]+=1
                if d:sym[d][d]+=1
print('common',len(set(FI)&set(EN)),'equal-length pairs used',used)
conf=[(k,dict(v)) for k,v in state.items() if len(v)>1]
print('state slots mapped',len(state),'conflicts',len(conf));print(conf[:12])
conf=[(k,dict(v)) for k,v in sym.items() if len(v)>1]
print('symbols mapped',len(sym),'conflicts',len(conf));print(conf[:12])
if __name__=='__main__' and len(sys.argv)>1:
    json.dump({'state':{k:v.most_common(1)[0][0] for k,v in state.items()},'sym':{k:v.most_common(1)[0][0] for k,v in sym.items()},
 'state_conf':{k:dict(v) for k,v in state.items() if len(v)>1},'sym_conf':{k:dict(v) for k,v in sym.items() if len(v)>1}},open(sys.argv[1],'w'),indent=0)
