/* Authored scalar state rules. The layout constants only adapt the
 * existing language-global record; decisions and defaults use ordinary C.
 * No generated-body, rule-frame or diagnostic fallback is retained here. */
#include "reloq61_scalar_state.h"
#define DG_c14 (DG_BASE+3104)
#define DG_f42 (DG_BASE+3520)
#define DG_f43 (DG_BASE+3528)
#define DG_s18 (DG_BASE+346)
#define DG_s19 (DG_BASE+350)
#define DG_s20 (DG_BASE+354)
#define DG_s4 (DG_BASE+210)
#define DG_s7 (DG_BASE+222)
#define DG_s237 (DG_BASE+1794)
#define DG_s480 (DG_BASE+3382)
#define DG_s22 (DG_BASE+362)
#define DG_s247 (DG_BASE+1834)
#define DG_s25 (DG_BASE+374)
#define DG_s26 (DG_BASE+378)
#define DG_s260 (DG_BASE+1928)
#define DG_s261 (DG_BASE+1932)
#define DG_s27 (DG_BASE+382)
#define DG_s28 (DG_BASE+386)
#define DG_s343 (DG_BASE+2422)
#define DG_s356 (DG_BASE+2530)
#define DG_s398 (DG_BASE+2794)
#define DG_s399 (DG_BASE+2798)
#define DG_s407 (DG_BASE+2830)
#define DG_s408 (DG_BASE+2834)
#define DG_s449 (DG_BASE+2998)
#define DG_s450 (DG_BASE+3002)
#define DG_s451 (DG_BASE+3006)
#define DG_s452 (DG_BASE+3010)
#define DG_s453 (DG_BASE+3014)
#define DG_s454 (DG_BASE+3018)
#define DG_s455 (DG_BASE+3022)
#define DG_s456 (DG_BASE+3026)
#define DG_s457 (DG_BASE+3030)
#define DG_s458 (DG_BASE+3034)
#define DG_s474 (DG_BASE+3358)
#define DG_s477 (DG_BASE+3370)
#define DG_s499 (DG_BASE+3578)
#define DG_s5 (DG_BASE+214)
#define DG_s581 (DG_BASE+4002)
#define DG_s590 (DG_BASE+4062)
#define DG_s591 (DG_BASE+4066)
#define DG_w19 (DG_BASE+160)
#define DG_w20 (DG_BASE+168)
#define DG_w21 (DG_BASE+176)
#define DG_w22 (DG_BASE+184)
#define DG_w34 (DG_BASE+484)
#define DG_w35 (DG_BASE+492)
#define DG_w38 (DG_BASE+536)
#define DG_w44 (DG_BASE+584)
#define DG_w45 (DG_BASE+592)
#define DG_w64 (DG_BASE+744)
#define DG_w65 (DG_BASE+752)
#define DG_w79 (DG_BASE+2052)
#define DG_w81 (DG_BASE+2484)
#define DG_w83 (DG_BASE+2500)
#define S(key) (*(int16_t *)((unsigned char *)state+DG_##key))
#define W(key) (*(int32_t *)((unsigned char *)state+DG_##key))
#define SD(key,offset) (*(int16_t *)((unsigned char *)state+DG_##key+(offset)))
#define WD(key,offset) (*(int32_t *)((unsigned char *)state+DG_##key+(offset)))

int32_t reloq_dede61_stretch_focus(void *state,int32_t p0,int32_t p1)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    return reloq_scalar_focus_duration(state,p1,S(s480)==S(s474),
        S(s237)<S(s7)&&S(s25)==S(s20)&&(S(s477)!=S(s474)||S(s4)==1));
}

int32_t reloq_dede61_print_prompt(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    
    return reloq_scalar_finish(state,0);
}

int32_t reloq_dede61_init_syll_acoustic_vars(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    S(s22)=S(s28);
    return reloq_scalar_finish(state,0);
}

int32_t reloq_dede61_stanger_ph_nasal_E(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    S(s449)=S(s450)=570; S(s451)=S(s452)=1850; S(s453)=S(s454)=2550;
    S(s455)=S(s456)=3500; S(s457)=S(s458)=4000; S(s407)=250; S(s408)=350;
    return reloq_scalar_finish(state,0);
}

int32_t reloq_dede61_set_f0_wind_and_base(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    reloq_scalar_pitch_window(SD(c14,28),WD(c14,36),&W(f43),&W(f42));
    return reloq_scalar_finish(state,0);
}

int32_t reloq_dede61_set_syll_stream_word_ptrs(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    if(S(s477)!=S(s474)) { W(w21)=W(w19); W(w22)=W(w20); }
    return reloq_scalar_finish(state,0);
}

int32_t reloq_dede61_print_streamarrays_from_delta(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    
    return reloq_scalar_finish(state,0);
}

int32_t reloq_dede61_init_symbolic_vars(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    S(s343)=0; W(w81)=W(w83)=W(w38);
    return reloq_scalar_finish(state,0);
}

int32_t reloq_dede61_increment_output_file_number(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    S(s356)=(int16_t)reloq_scalar_next_file((uint16_t)S(s356));
    return reloq_scalar_finish(state,0);
}

int32_t reloq_dede61_set_ger_global_constants(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    S(s591)=10; S(s590)=12; S(s260)=S(s261)=2;
    return reloq_scalar_finish(state,0);
}

int32_t reloq_dede61_init_word_ptrs(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    
    return reloq_scalar_finish(state,0);
}

int32_t reloq_dede61_set_voice_param_constants(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    reloq_scalar_voice_defaults((unsigned char *)state+DG_s499);
    return reloq_scalar_finish(state,0);
}

int32_t reloq_dede61_DeltaProc_main(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    
    return reloq_scalar_finish(state,0);
}

int32_t reloq_dede61_init_gcat_vars(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    S(s247)=0;
    return reloq_scalar_finish(state,0);
}

int32_t reloq_dede61_reset_gcat_vars(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    
    return reloq_scalar_finish(state,0);
}

int32_t reloq_dede61_stanger_ph_nasal_o(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    S(s449)=450; S(s451)=650; S(s453)=2800; S(s398)=S(s399)=200;
    return reloq_scalar_finish(state,0);
}

int32_t reloq_dede61_init_syll_inton_vars(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    W(w34)=W(w35)=W(w79);
    S(s19)=reloq_scalar_next_accent(S(s19),S(s25),S(s26),S(s27),S(s5),S(s20));
    return reloq_scalar_finish(state,0);
}

int32_t reloq_dede61_reset_ger_sent_vars(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    S(s581)=0;
    return reloq_scalar_finish(state,0);
}

int32_t reloq_dede61_is_target_syll(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    return reloq_scalar_finish(state,S(s18)==1?0:94);
}

int32_t reloq_dede61_extend_vals(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    
    return reloq_scalar_finish(state,0);
}

int32_t reloq_dede61_set_letter_name_wordcats(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    
    return reloq_scalar_finish(state,0);
}

int32_t reloq_dede61_init_current_word_ptrs(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    W(w44)=W(w45)=W(w64)=W(w65)=W(w79);
    return reloq_scalar_finish(state,0);
}

int32_t reloq_dede61_phrase_level_printouts(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    
    return reloq_scalar_finish(state,0);
}

int32_t reloq_dede61_stanger_ph_nasal_a(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    S(s449)=725; S(s451)=1025; S(s453)=3000; S(s398)=150; S(s407)=450; S(s408)=650;
    return reloq_scalar_finish(state,0);
}

int32_t reloq_dede61_DeltaProc_getInputCharCount(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    
    return reloq_scalar_finish(state,0);
}
