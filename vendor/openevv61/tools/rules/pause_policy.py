"""Connect native phrase pauses to the shared duration/playback policy.

The source rules already distinguish authored pauses from punctuation. Their
existing pause token is shortened after that distinction; quiet boundaries
come from the native silence call, before the acoustic output is submitted.
No input rewriting, tags, new envelope points or waveform analysis is used.
"""
import re
FUNCTION=re.compile(r'int32_t reloq_\w+\(void \*state, int32_t p0(?:, int32_t p\d+)*\)\n\{.*?\n\}',re.S)

def connect(tag,name,body):
    if name=='insert_phrase_pause'and'reloq61_shorten_phrase_pause('not in body:
        body=re.sub(r'    \{ extern void reloq61_shorten_default_pause\(void \*,void \*\);\n      reloq61_shorten_default_pause\(state,[^\n]+\); \}\n','',body)
        call=re.search(r'    r0 = reloq61_typed_insert_2ptv\(DELTA_ENTRY_insert_2ptv, state, \(uint8_t\)\(\d+\), (\(delta_loc \*\)\(base \+ \(-?\d+\)\)), \(uint8_t\)\(0\)\);',body)
        assert call,(tag,name,'pause insertion shape')
        before=body[:call.start()]
        assert re.search(r'reloq61_typed_copyvar\([^\n]+\);\n$',before), (tag,name,'copy before insertion')
        cell=int(re.search(r'base \+ \((-?\d+)\)',call[1])[1])
        fields=re.findall(r'AT\(int16_t, '+str(cell+2)+r'\) = \((\d+)\);',before)
        assert fields,(tag,name,'native duration field')
        field=int(fields[-1]);assert 0<field<32,(tag,name,field)
        hook='    { extern void reloq61_shorten_phrase_pause(void *,void *,int);\n      reloq61_shorten_phrase_pause(state,'+call[1]+','+str(field)+'); }\n'
        body=before+hook+body[call.start():]
    if name=='handle_pause'and'reloq61_note_pause('not in body:
        call=re.search(r'reloq_call_'+re.escape(tag)+r'_define_all_params_for_silence\([^\n]+',body)
        assert call,(tag,name,'native silence bounds')
        args=re.findall(r'STATE_AT\((\w+), -4\)',call[0])
        assert len(args)==4,(tag,name,args)
        begin,end=args[-2:]
        writes=list(re.finditer(r'    STATE\(int32_t, (\w+)\) = [^\n]+;\n',body[:call.start()]))
        assert writes,(tag,name,'native transition end')
        write=writes[-1];after=write[1]
        assert len({begin,end,after})==3 and all(re.fullmatch(r'f\d+',x)for x in [begin,end,after]),(tag,name,begin,end,after)
        hook='    { extern void reloq61_note_pause(void *,int32_t,int32_t,int32_t);\n      reloq61_note_pause(state,'+','.join('STATE(int32_t,'+x+')'for x in [begin,end,after])+'); }\n'
        body=body[:write.end()]+hook+body[write.end():]
    return body

def apply(tag,units):
    return [(name,connect(tag,name,body))for name,body in units]

def update_files(directory,tag):
    found=set()
    def replace(match):
        body=match[0];name=re.search(r'int32_t reloq_'+re.escape(tag)+r'_(\w+)\(',body)[1]
        if name in ['insert_phrase_pause','handle_pause']:found.add(name)
        return connect(tag,name,body)
    for path in sorted(directory.glob('delta_rules_c??_*.c')):
        old=path.read_text();new=FUNCTION.sub(replace,old)
        if old!=new:path.write_text(new)
    assert found=={'insert_phrase_pause','handle_pause'},(tag,found)
