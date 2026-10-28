"""Keep independently authored C rules when rebuilding the remaining module.

Only transitional entry names live here. The implementations and spelling
data are maintained by hand in src/, outside the rule-generation pipeline.
"""
GERMAN={
    'normalize_ger_letters':'reloq61_normalize_german_letters',
    'ger_legal_initial_cons':'reloq61_german_onset',
    'ger_legal_root_final_cons':'reloq61_german_root_coda',
    'ger_legal_word_final_cons':'reloq61_german_word_coda',
    'legal_root_cons_plus_t':'reloq61_german_dental_coda',
    'legal_root_cons_plus_son':'reloq61_german_sonorant_coda',
}
def binding(language,name):
    adapter=GERMAN.get(name)if language=='dede61'else None
    if adapter is None:return None
    return (f'/* Independently authored C: {name}. */\n'
        f'extern int32_t {adapter}(void *,const int32_t *,int);\n'
        f'static int32_t evv_{name}(void *state, const int32_t *args, int nargs)\n'
        '{\n'+f'    return {adapter}(state,args,nargs);\n'+'}\n\n')


def intonation_policy(language,name,body):
    """Apply product question policy after phonetic timing is established."""
    if language=='enus61' and name=='insert_f0':
        marker=('    DROP(3);\n    AT(int32_t, -52) = (r0);\n'
                '    if (AT(int32_t, -52) == 0) goto L1894;')
        assert body.count(marker)==1
        hook=('    DROP(3);\n'
              '    if (r0 && reloq61_english_question_time(state,\n'
              '        STATE(int32_t, w52), AT(int16_t, -146),\n'
              '        STATE(int16_t, s470), STATE(int32_t, f34),\n'
              '        &STATE(int32_t, f38))) r0=0;\n'
              '    AT(int32_t, -52) = (r0);\n'
              '    if (AT(int32_t, -52) == 0) goto L1894;')
        return ('extern int reloq61_english_question_time(void *, int32_t,\n'
            '    int16_t, int16_t, int32_t, int32_t *);\n'+body.replace(marker,hook))
    if language!='enus61' or name!='init_phrase_inton_vars':return body
    marker='    DROP(1);\n    if ((int16_t)(STATE(int16_t, s113))'
    assert body.count(marker)==1
    hook=('    DROP(1);\n'
          '    reloq61_english_question_tones(state, STATE(int32_t, w52),\n'
          '        STATE(int16_t, s280), &STATE(int16_t, s272),\n'
          '        &STATE(int16_t, s273));\n'
          '    if ((int16_t)(STATE(int16_t, s113))')
    declaration=('/* Question policy affects F0 only, after word analysis and timing. */\n'
        'extern void reloq61_english_question_tones(void *, int32_t, int16_t,\n'
        '    int16_t *, int16_t *);\n')
    return declaration+body.replace(marker,hook)
