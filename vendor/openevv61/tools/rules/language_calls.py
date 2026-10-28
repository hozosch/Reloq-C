"""Ordinary C calls between complete language functions.

Each body receives scalar C parameters; only the outer compatibility entry
adapts an argument vector. Internal calls bypass the generic entry table,
language shim, argument vector, and rule lookup. Landing cleanup belongs to
the C function that planted the landing. Diagnostics retain the old route.
Compact record references inside the rules are a separate remaining boundary.
"""
from pathlib import Path
import re
from direct_calls import pieces,immutable

FUNCTION=re.compile(r'static int32_t evv_(\w+)\(void \*state, const int32_t \*args, int nargs\)\n\{(.*?)\n\}',re.S)
COPY=re.compile(r'    for \(i = 0; i < nargs && i < (\d+); i\+\+\)\n        memcpy\(base \+ (\d+) \+ 4 \* i, &args\[i\], 4\);')
REFERENCE=re.compile(r'    const int32_t reloq_machine_ref=\n        nargs>0 && EVV_AT\(void \*,args\[0\]\)==state\?args\[0\]:REG_REF\(state\);')

def catalogue(texts):
    result={}
    for text in texts:
        for m in FUNCTION.finditer(text):
            name,body=m.groups();copy=COPY.search(body)
            if not copy or not REFERENCE.search(body):continue
            rest=COPY.sub('',REFERENCE.sub('',body))
            if re.search(r'\b(?:args|nargs)\b',rest):continue
            assert name not in result,name
            result[name]=int(copy[1])
    return result

def declaration(tag,name,n,variables=False):
    return 'void *'+('state'if variables else'')+''.join(', int32_t'+(' p'+str(i)if variables else'')for i in range(n))

def header(tag,specs):
    root=Path(__file__).resolve().parents[2]
    entries=set(re.findall(r'\bDELTA_ENTRY_(\w+)\s*=',(root/'lang'/tag/('delta_rules_'+tag+'.h')).read_text()))
    guard='RELOQ61_RULE_CALLS_'+tag.upper()+'_H'
    rows=['/* Direct C language calls; record values remain compact until their own port. */',
        '#ifndef '+guard,'#define '+guard,'#include "delta_rules_c.h"',
        'extern int delta_rule_trace;',
        '#ifndef RELOQ_RULE_RETURN',
        '#define RELOQ_RULE_RETURN(x) do { int32_t out_=(x); evv_frame_pop(frame); \\',
        '    evv_land_release(reloq_landing_mark); return out_; } while (0)',
        '#endif']
    for name,n in sorted(specs.items()):
        function='reloq_'+tag+'_'+name;call='reloq_call_'+tag+'_'+name
        rows.append('int32_t '+function+'('+declaration(tag,name,n)+');')
        if name not in entries:continue
        rows.append('static inline int32_t '+call+'('+declaration(tag,name,n,True)+')\n{')
        rows.append('    if(delta_rule_trace) {')
        rows.append('        const int32_t args[]={'+', '.join('p'+str(i)for i in reversed(range(n)))+'};')
        rows.append(f'        return delta_rule_called(DELTA_ENTRY_{name},args,{n},{n});\n    }}')
        rows.append('    return '+function+'(state'+''.join(', p'+str(i)for i in range(n))+');\n}')
    rows.append('#endif\n');return '\n'.join(rows)

def bodies(text,tag,specs):
    def convert(m):
        name,body=m.groups()
        if name not in specs:return m[0]
        n=specs[name];copy=COPY.search(body);assert copy and int(copy[1])==n
        offset=int(copy[2]);assert n>0
        body=REFERENCE.sub('    const int32_t reloq_machine_ref=\n        EVV_AT(void *,p0)==state?p0:REG_REF(state);\n'
            '    const int reloq_landing_mark=evv_land_mark();',body)
        body=COPY.sub('\n'.join(f'    memcpy(base + {offset+4*i}, &p{i}, 4);'for i in range(n)),body)
        if not re.search(r'\bi\b',body.replace('    int i;','')):body=body.replace('    int i;\n','')
        assert not re.search(r'\b(?:args|nargs)\b',body),name
        assert 'EVV_LAND_SAVE'in body and 'RETURN('in body,name
        body=body.replace('RETURN(','RELOQ_RULE_RETURN(')
        native='int32_t reloq_'+tag+'_'+name+'('+declaration(tag,name,n,True)+')\n{'+body+'\n}'
        adapter='static int32_t evv_'+name+'(void *state, const int32_t *args, int nargs)\n{\n'
        adapter+='    return reloq_'+tag+'_'+name+'(state'+''.join(f', nargs>{i}?args[{i}]:0'for i in range(n))+');\n}'
        return native+'\n\n'+adapter
    return FUNCTION.sub(convert,text)

def calls(text,tag,specs):
    def convert(m):
        fields=pieces(m[1])
        if not fields:return m[0]
        target=re.fullmatch(r'CALL\((\w+),\s*(\d+)\)',fields[-1])
        if not target or target[1]not in specs:return m[0]
        name=target[1];n=specs[name]
        if int(target[2])!=n or int(m[2])!=n or len(fields)!=n+1:return m[0]
        values=[]
        for field in fields[:-1]:
            if not field.startswith('ARG(')or not field.endswith(')'):return m[0]
            value=field[4:-1]
            if not immutable(value):return m[0]
            values.append(value)
        if values[-1]!='FIELD(0)':return m[0]
        return '    r0 = reloq_call_'+tag+'_'+name+'(state'+''.join(', (int32_t)('+value+')'for value in reversed(values))+');'
    return re.sub(r'^    r0 = \(([^\n]+)\);\n    DROP\((\d+)\);',convert,text,flags=re.M)

def bind(text,tag,specs):
    if '#include "reloq61_rule_calls_'+tag+'.h"'in text:return text
    text=bodies(calls(text,tag,specs),tag,specs)
    return text.replace('#include "evv_arena.h"','#include "evv_arena.h"\n#include "reloq61_rule_calls_'+tag+'.h"',1)

def convert_bodies(tag,units):
    specs=catalogue([text for _,text in units]);assert specs
    root=Path(__file__).resolve().parents[2]
    (root/'lang'/tag/('reloq61_rule_calls_'+tag+'.h')).write_text(header(tag,specs))
    return [(name,bodies(calls(text,tag,specs),tag,specs))for name,text in units]

if __name__=='__main__':
    root=Path(__file__).resolve().parents[2]
    for tag in ['enus61','dede61']:
        paths=sorted((root/'lang'/tag).glob('delta_rules_c??_*.c'))
        texts=[p.read_text()for p in paths];specs=catalogue(texts)
        assert specs,'Input is already converted or has no supported functions'
        removed=0;sites=0
        for path,before in zip(paths,texts):
            after=bind(before,tag,specs);removed+=before.count('ARG(')-after.count('ARG(')
            sites+=after.count('r0 = reloq_call_'+tag+'_')
            path.write_text(after)
        (root/'lang'/tag/('reloq61_rule_calls_'+tag+'.h')).write_text(header(tag,specs))
        print(tag,len(specs),'native C functions;',sites,'direct sites;',removed,'argument pushes removed')
