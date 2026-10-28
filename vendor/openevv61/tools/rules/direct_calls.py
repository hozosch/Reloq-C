"""Use the C ABI for complete calls to shared record operations.

The signature ledger is maintained explicitly. Only complete, side-effect-free
argument lists ending in the checked current-state argument are lowered. Other
call shapes retain the existing adapter. No runtime signature decoding is added.
"""
import re

# Return convention and actual C parameters after delta_state *.
SIGNATURES={
 'backtrack_function':('i',['int32_t *']),
 'get_parm':('i',['delta_loc *','delta_loc *','int16_t']),
 'test_string_s':('i',['uint8_t','uint8_t','const uint8_t *']),
 'testFldeq':('i',['uint8_t','uint8_t','uint8_t']),
 'ventproc':('i',['delta_actrec *','uint8_t *','uint8_t *','uint8_t *','void *']),
 'vretproc':('i',['int32_t']),
 'setd_lookup':('i',['int32_t','int16_t']),
 'insert_2ptv':('i',['uint8_t','delta_loc *','uint8_t']),
 'read_tvar':('i',['int8_t','delta_loc *']),
 'forto_adv_upto_r':('i',['int16_t','int16_t','int16_t','uint8_t','delta_token *','const delta_token *']),
 'bspop_boa':('p',[]),
 'fence':('v',['int8_t','const uint8_t *']),
 'savescptr':('v',['int16_t','delta_loc *']),
 'settvar_s':('v',['delta_loc *','int32_t']),
 'lpta_rpta_loadp':('v',['const delta_token *','const delta_token *']),
 'copyvar':('v',['delta_loc *','delta_loc *']),
 'savetok':('i',['delta_loc *']),
 'lpta_mover':('v',['uint8_t']),
}
for name in ['advance_tok','succeed','compare_ptas','testeq','test_ptr','advanc']:
 SIGNATURES[name]=('i',[])
for name in ['setscan_r','setscan_l']:SIGNATURES[name]=('i',['uint8_t'])
for name in ['starttest','starttest_l','startloop','bspush_ca_scan','bspush_ca','bspush_ca_boa','bspush_ca_scan_boa']:
 SIGNATURES[name]=('v',['int16_t'])
for name in ['lpta_loadp','lpta_loadpn','rpta_loadpn']:SIGNATURES[name]=('v',['const delta_token *'])
for name in ['push_ptr_init','lpta_storep']:SIGNATURES[name]=('v',['delta_loc *'])
for name in ['testeq_tvars','forall_to_test']:SIGNATURES[name]=('i',['delta_loc *','delta_loc *'])
for name in ['inline61_loffset','inline61_lflags','inline61_lnode','inline61_roffset','inline61_rflags','inline61_rnode']:
 SIGNATURES[name]=('v',['int32_t'])

# Shared scans, dictionaries, comparisons and record updates (130).
SIGNATURES['actd_goto']=('i', [])
SIGNATURES['actd_lookup']=('i', ['int16_t', 'delta_token *', 'delta_token *'])
SIGNATURES['bspush_boa']=('v', [])
SIGNATURES['bspush_nboa']=('v', [])
SIGNATURES['c_assvar']=('v', ['delta_loc *'])
SIGNATURES['callUserDictLookup']=('i', ['const void *', 'const delta_token *', 'const delta_token *'])
SIGNATURES['chstream']=('i', ['int16_t', 'uint8_t'])
SIGNATURES['delete_1pt']=('v', ['uint8_t'])
SIGNATURES['delete_2pt']=('i', ['uint8_t', 'uint8_t'])
SIGNATURES['forall_cont_from']=('i', ['int16_t', 'int16_t', 'int32_t', 'delta_loc *', 'const delta_loc *'])
SIGNATURES['forto_adv_r']=('i', ['int16_t', 'int16_t', 'int16_t', 'uint8_t', 'delta_token *', 'const delta_token *'])
SIGNATURES['if_testeq']=('i', [])
SIGNATURES['if_testeq_v_i']=('i', ['delta_loc *', 'int32_t'])
SIGNATURES['if_testge']=('i', [])
SIGNATURES['if_testge_v_i']=('i', ['delta_loc *', 'int32_t'])
SIGNATURES['if_testgt']=('i', [])
SIGNATURES['if_testgt_v_i']=('i', ['delta_loc *', 'int32_t'])
SIGNATURES['if_testle']=('i', [])
SIGNATURES['if_testle_v_i']=('i', ['delta_loc *', 'int32_t'])
SIGNATURES['if_testlt']=('i', [])
SIGNATURES['if_testlt_v_i']=('i', ['delta_loc *', 'int32_t'])
SIGNATURES['if_testneq']=('i', [])
SIGNATURES['if_testneq_v_i']=('i', ['delta_loc *', 'int32_t'])
SIGNATURES['insert_2pt_i']=('i', ['uint8_t', 'uint8_t', 'const uint8_t *', 'uint8_t'])
SIGNATURES['insert_2pt_l']=('i', ['uint8_t', 'uint8_t', 'const uint8_t *', 'uint8_t'])
SIGNATURES['insert_2pt_lng']=('i', ['uint8_t', 'uint8_t', 'const uint8_t *', 'uint8_t'])
SIGNATURES['insert_2pt_s']=('i', ['uint8_t', 'uint8_t', 'const uint8_t *', 'uint8_t'])
SIGNATURES['insert_l']=('v', ['int8_t', 'uint8_t', 'const uint8_t *', 'uint8_t'])
SIGNATURES['insert_r']=('v', ['int8_t', 'uint8_t', 'const uint8_t *', 'uint8_t'])
SIGNATURES['lpta_ctxtl']=('v', ['uint8_t'])
SIGNATURES['lpta_ctxtr']=('v', ['uint8_t'])
SIGNATURES['lpta_movel']=('v', ['uint8_t'])
SIGNATURES['lpta_tstctxtl']=('i', ['uint8_t'])
SIGNATURES['lpta_tstctxtr']=('i', ['uint8_t'])
SIGNATURES['lpta_tstmovel']=('i', ['uint8_t'])
SIGNATURES['lpta_tstmover']=('i', ['uint8_t'])
SIGNATURES['mark_s']=('i', ['uint8_t', 'uint8_t', 'uint8_t', 'uint8_t'])
SIGNATURES['mark_v']=('i', ['uint8_t', 'uint8_t', 'delta_loc *', 'uint8_t'])
SIGNATURES['move_f']=('v', ['delta_loc *', 'int32_t'])
SIGNATURES['move_i']=('v', ['delta_loc *', 'int16_t'])
SIGNATURES['move_lng']=('v', ['delta_loc *', 'int32_t'])
SIGNATURES['ncompare']=('v', [])
SIGNATURES['ncompare_s']=('v', ['uint8_t'])
SIGNATURES['npop']=('v', ['delta_loc *'])
SIGNATURES['npush_f']=('v', ['int32_t'])
SIGNATURES['npush_fld']=('i', ['uint8_t', 'uint8_t'])
SIGNATURES['npush_i']=('v', ['int32_t'])
SIGNATURES['npush_l']=('v', ['int32_t'])
SIGNATURES['npush_lng']=('v', ['int32_t'])
SIGNATURES['npush_s']=('v', ['int32_t'])
SIGNATURES['npush_v']=('v', ['delta_loc *'])
SIGNATURES['npush_vf']=('v', ['delta_loc *'])
SIGNATURES['print_lit']=('v', ['int32_t', 'const char *'])
SIGNATURES['print_var']=('v', ['int32_t', 'delta_loc *'])
SIGNATURES['proj_def']=('v', ['uint8_t'])
SIGNATURES['rpta_ctxtl']=('v', ['uint8_t'])
SIGNATURES['rpta_ctxtr']=('v', ['uint8_t'])
SIGNATURES['rpta_movel']=('v', ['uint8_t'])
SIGNATURES['rpta_mover']=('v', ['uint8_t'])
SIGNATURES['rpta_tstctxtl']=('i', ['uint8_t'])
SIGNATURES['rpta_tstctxtr']=('i', ['uint8_t'])
SIGNATURES['rpta_tstmovel']=('i', ['uint8_t'])
SIGNATURES['rpta_tstmover']=('i', ['uint8_t'])
SIGNATURES['setscan_nof_l']=('i', ['uint8_t'])
SIGNATURES['setscan_nof_r']=('i', ['uint8_t'])
SIGNATURES['starttest_e']=('v', ['int16_t'])
SIGNATURES['test_string']=('i', ['uint8_t', 'uint8_t', 'const uint8_t *'])
SIGNATURES['test_string_i']=('i', ['uint8_t', 'uint8_t', 'const uint8_t *'])
SIGNATURES['test_string_l']=('i', ['uint8_t', 'uint8_t', 'const uint8_t *'])
SIGNATURES['test_string_lng']=('i', ['uint8_t', 'uint8_t', 'const uint8_t *'])
SIGNATURES['test_synch']=('i', ['int16_t', 'uint8_t', 'const uint8_t *'])
SIGNATURES['testge']=('i', [])
SIGNATURES['testgt']=('i', [])
SIGNATURES['testle']=('i', [])
SIGNATURES['testlt']=('i', [])
SIGNATURES['testneq']=('i', [])

def pieces(text):
    result=[];depth=0;start=0
    for i,c in enumerate(text):
        if c=='(':depth+=1
        elif c==')':
            depth-=1
            if depth<0:return None
        elif c==','and depth==0:result.append(text[start:i].strip());start=i+1
    if depth:return None
    result.append(text[start:].strip());return result

def immutable(expr):
    # Generated argument expressions read local values and form references.
    # Reject writes, nested calls, volatile helpers and any unknown construct.
    if re.search(r'=|\+\+|--|\?|;|"|\b(?:CALL|ARG|NATIVE_CALL)\b',expr):return False
    functions=re.findall(r'\b([A-Za-z_]\w*)\s*\(',expr)
    return all(n in ['SLOT','STATE_AT','STATE','STATE_D','AT','PARAM','FIELD',
        'REG_P','GLOBAL','GLOBAL_D','GLOBAL_AT','GLOBAL_VIA','RECORD','LOW','BYTE0','BYTE1']for n in functions)

LOCAL_POINTER=re.compile(r'EVV_AT\(([^,\n]+\*),\s*SLOT\((-?\d+)\)\)')

def pointer(kind,value):
    # SLOT is EVV_REF(base+n). A constant local address already belongs to
    # the live, bounded C workspace: keep that native pointer rather than
    # checking/encoding it and immediately decoding the same address.
    slot=re.fullmatch(r'SLOT\((-?\d+)\)',value)
    if slot:return '('+kind+')(base + ('+slot[1]+'))'
    return 'EVV_AT('+kind+', '+value+')'

def native_pointers(text):
    def replace(m):return pointer(m[1].strip(),'SLOT('+m[2]+')')
    return '\n'.join(LOCAL_POINTER.sub(replace,line)
        if line.startswith('    r0 = reloq61_typed_') else line
        for line in text.split('\n'))

def convert(match):
    expressions=pieces(match.group(1))
    if not expressions:return match.group(0)
    target=re.fullmatch(r'NATIVE_CALL\((\w+),\s*(\d+)\)',expressions[-1])
    if not target or target[1]not in SIGNATURES:return match.group(0)
    name=target[1];types=SIGNATURES[name][1];n=len(types)+1
    if int(target[2])!=n or int(match.group(2))!=n or len(expressions)!=n+1:return match.group(0)
    args=[]
    for item in expressions[:-1]:
        if not item.startswith('ARG(')or not item.endswith(')'):return match.group(0)
        value=item[4:-1]
        if not immutable(value):return match.group(0)
        args.append(value)
    if args[-1]!='FIELD(0)':return match.group(0)
    actual=['DELTA_ENTRY_'+name,'state']
    for kind,value in zip(types,reversed(args[:-1])):
        actual.append(pointer(kind,value)if '*'in kind else'('+kind+')('+value+')')
    return '    r0 = reloq61_typed_'+name+'('+', '.join(actual)+');'

def bind(text):
    return native_pointers(re.sub(r'^    r0 = \(([^\n]+)\);\n    DROP\((\d+)\);',convert,text,flags=re.M))

def declarations():
    return ''.join('int32_t reloq61_typed_'+name+'(int, delta_state *'+
        ''.join(', '+t for t in kinds)+');\n'for name,(_,kinds)in sorted(SIGNATURES.items()))

def definitions():
    bodies=[]
    for name,(returns,kinds)in sorted(SIGNATURES.items()):
        params=['int which','delta_state *d']+[t+' a'+str(i)for i,t in enumerate(kinds)]
        args=['d']+['a'+str(i)for i in range(len(kinds))]
        encoded=['EVV_REF(a'+str(i)+')'if '*'in t else'(int32_t)a'+str(i)for i,t in enumerate(kinds)]
        stack=list(reversed(encoded))+['EVV_REF(d)'];n=len(stack)
        call=name+'('+', '.join(args)+')'
        result=call+';return 0;'if returns=='v'else'return EVV_REF('+call+');'if returns=='p'else'return '+call+';'
        bodies.append('int32_t reloq61_typed_'+name+'('+', '.join(params)+')\n{\n'
            '    if(delta_rule_trace) {\n        const int32_t args[]={'+', '.join(stack)+'};\n'
            f'        return delta_rule_called(which,args,{n},{n});\n    }}\n'
            '    '+result+'\n}\n')
    return '\n'.join(bodies)
