"""Move closed scalar entrypoints to authored C language modules.

This pass removes complete former bodies. It does not translate or retain
their instruction-shaped statements. Native entry shims keep their names.
"""
from pathlib import Path
import re
FUNCTION=re.compile(r'int32_t (reloq_\w+)\(void \*state, int32_t p0(?:, int32_t p\d+)*\)\n\{.*?\n\}',re.S)
def authored_names(tag):
    root=Path(__file__).resolve().parents[2]
    source='\n'.join(p.read_text()for p in (root/'lang'/tag).glob('*_rules_'+tag+'.c')
        if not p.name.startswith('delta_'))
    return set(re.findall(r'int32_t (reloq_\w+)\(void \*state,int32_t p0(?:,int32_t p\d+)*\)',source))
def move(tag,units):
    names=authored_names(tag)
    def replace(m):
        if m[1]not in names:return m[0]
        module='segment_rules_'if m[1].endswith(('_addStreamArrayPtVal','_doAV','_doAF','_doFNP','_doFNZ','_apply_speed_anno','_insert_dur'))else \
            'input_rules_'if m[1].endswith(('_count_cumulative_chars','_count_chars','_count_sylls','_count_normalized_inp','_count_unnormalized_inp','_count_input','_single_chars_count_inp','_set_min_token_vals','_num_words_in_range','_num_tokens_in_range','_space','_inside_orig_inp_token','_mark_uppercase'))else \
            'acoustic_rules_'if m[1].endswith(('_addStreamArraySsVal','_add_filtered_val','_run_filters','_doF'))else \
            'spelling_rules_'if m[1].endswith(('_print_eng_SPR','_print_ger_SPR','_lang_specific_SPR'))else \
            'text_rules_'if m[1].endswith('_convert_to_lowercase')else \
            'token_rules_'if m[1].endswith('_reset_token_vars')else \
            'record_rules_'if m[1].endswith(('_in_syllable','_test_stress_annot','_find_most_stressed_syll'))else \
            'phonotactic_rules_'if ('eng_legal_'in m[1]or m[1]=='reloq_enus61_legal_initial_consonants')else'scalar_rules_'
        return '/* '+m[1]+' is implemented in '+module+tag+'.c. */'
    converted=[(name,FUNCTION.sub(replace,text))for name,text in units]
    # This entry was authored before scalar language-call boundaries were
    # introduced. Keep its compatibility shim on the same C implementation.
    if tag=='enus61':
        adapter='''extern int32_t reloq_enus61_run_filters(void *,int32_t,int32_t,int32_t);
static int32_t evv_run_filters(void *state, const int32_t *args, int nargs)
{
    return reloq_enus61_run_filters(state,nargs>0?args[0]:0,nargs>1?args[1]:0,nargs>2?args[2]:0);
}'''
        pattern=r'(?:extern int(?:16|32)_t reloq_enus61_(?:filter_endpoint|run_filters)\([^\n]+\);\n)?static int32_t evv_run_filters\(void \*state, const int32_t \*args, int nargs\)\n\{.*?\n\}'
        converted=[(name,re.sub(pattern,lambda _:adapter,text,flags=re.S))for name,text in converted]
    return converted
if __name__=='__main__':
    root=Path(__file__).resolve().parents[2]
    for tag in ['enus61','dede61']:
        paths=sorted((root/'lang'/tag).glob('delta_rules_c??_*.c'))
        for name,text in move(tag,[(p,p.read_text())for p in paths]):
            if name.read_text()!=text:name.write_text(text)
