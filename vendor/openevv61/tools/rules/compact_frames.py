"""Bounded C workspaces and scalar-only execution for closed helper blocks.

The source pass only accepts blocks with no nested rule/record operations,
address-taken locals, parameters or failing primitives. Scalar locals become
ordinary C variables. The diagnostic/error path keeps its established scope.
Other functions reserve their actual declared C workspace, not a language-wide
maximum. This is an offline source conversion, not a runtime instruction set.
"""
import re
FUNCTION=re.compile(r'(int32_t reloq_\w+\(void \*state, int32_t p0[^\n]*\)\n\{)(.*?)(\n\})',re.S)
LOCAL=re.compile(r'(?<!\w)AT\((int32_t|int16_t), (-?\d+)\)')

def scalar_core(body,head):
    if head.splitlines()[0].count('int32_t p')!=1:return None
    if re.search(r'\bCALL\(|reloq_call_',body):return None
    if not set(re.findall(r'reloq61_typed_(\w+)\(',body))<={'ventproc','fence','vretproc','succeed'}:return None
    start=re.search(r'    r0 = reloq61_typed_fence[^\n]+\n',body)
    end=re.search(r'\n(L\d+):;\n    r0 = reloq61_typed_vretproc',body)
    success=re.search(r'\n(L\d+):;\n    r0 = reloq61_typed_succeed',body)
    if not start or not end or not success:return None
    core=re.sub(r'^    r0 = reloq61_typed_fence[^\n]+\n','',body[start.start():end.start()],flags=re.M)
    if re.search(r'\b(?:SLOT|PARAM|PARAMAT|FIELD|CALL|ARG|DROP)\(',core):return None
    if re.search(r'(?<!\w)AT\((?!int32_t|int16_t)',core):return None
    slots=sorted({int(o)for _,o in LOCAL.findall(core)})
    if any(o%4 for o in slots):return None
    for m in LOCAL.finditer(core):
        if m[1]=='int16_t'and re.match(r'\s*=(?!=)',core[m.end():]):return None
    # Every local has one full-width storage cell; narrow reads cast that cell.
    core=LOCAL.sub(lambda m:(''if m[1]=='int32_t'else'(int16_t)')+'local_'+str(-int(m[2])),core)
    # No reference into the old packed local record remains.
    if re.search(r'(?<!\w)AT\(',core):return None
    labels=set(re.findall(r'\bL\d+\b',core));core=re.sub(r'\bL(\d+)\b',r'C_L\1',core)
    declarations=''.join('        int32_t local_'+str(-o)+'=0;\n'for o in slots)
    code='\n    /* Closed scalar block: no rule frame, landing, fence or record stack. */\n'
    code+='    if(reloq61_scalar_ready(state)) {\n'+declarations
    code+='        int32_t r0=0,r1=0,r2=0,r3=0,r4=0,r5=0,r6=0,r7=0;\n'
    code+='        ((delta_state *)state)->unknown_3c=0;\n'
    code+='\n'.join('    '+line for line in core.splitlines())+'\n'
    if end[1]in labels:code+='    C_'+end[1]+':;\n        return reloq61_scalar_done(state,94);\n'
    code+='    C_'+success[1]+':;\n        return reloq61_scalar_done(state,0);\n    }\n'
    return code

def bind(text):
    if '/* Compact C workspaces, scalar-only helpers. */'in text:return text
    def convert(m):
        head,body,tail=m.groups()
        size=re.search(r'memset\(frame, 0, (\d+)\);',body)
        if not size:return m[0]
        assert body.count('evv_frame_push(DELTA_RULE_FRAME_MAX)')==1
        base=int(re.search(r'base = frame \+ (\d+);',body)[1]);n=int(size[1])
        # Literal local references, landing names and parameter cells must all
        # start inside the workspace. Dynamic accesses remain to external data.
        for value in re.findall(r'\bSLOT\((-?\d+)\)',body):assert 0<=base+int(value)<n
        for typ,value in LOCAL.findall(body):assert 0<=base+int(value)<=n-(4 if typ=='int32_t'else 2)
        fast=scalar_core(body,head)
        body=body.replace('evv_frame_push(DELTA_RULE_FRAME_MAX)','evv_frame_push('+str(n)+')',1)
        return head+(fast or'')+body+tail
    text=FUNCTION.sub(convert,text)
    return text.replace('#include "evv_arena.h"','#include "evv_arena.h"\n/* Compact C workspaces, scalar-only helpers. */\n#include "reloq61_scalar_blocks.h"',1)

def convert_bodies(units):
    # Body-only generation supplies its common includes separately.
    return [(name,bind(text).replace('/* Compact C workspaces, scalar-only helpers. */',''))for name,text in units]

if __name__=='__main__':
    from pathlib import Path
    root=Path(__file__).resolve().parents[2]
    frames=blocks=0
    for tag in ['enus61','dede61']:
        for path in sorted((root/'lang'/tag).glob('delta_rules_c??_*.c')):
            before=path.read_text();after=bind(before)
            frames+=before.count('evv_frame_push(DELTA_RULE_FRAME_MAX)')-after.count('evv_frame_push(DELTA_RULE_FRAME_MAX)')
            blocks+=after.count('if(reloq61_scalar_ready(state))')
            path.write_text(after)
    print(frames,'bounded C workspaces;',blocks,'scalar-only complete helper blocks')
