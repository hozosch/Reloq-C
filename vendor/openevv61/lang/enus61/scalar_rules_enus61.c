/* Authored scalar state rules. The layout constants only adapt the
 * existing language-global record; decisions and defaults use ordinary C.
 * No generated-body, rule-frame or diagnostic fallback is retained here. */
#include "reloq61_scalar_state.h"
#define DG_c14 (DG_BASE+3196)
#define DG_f20 (DG_BASE+2172)
#define DG_f42 (DG_BASE+3612)
#define DG_f43 (DG_BASE+3620)
#define DG_s18 (DG_BASE+346)
#define DG_s19 (DG_BASE+350)
#define DG_s20 (DG_BASE+354)
#define DG_s4 (DG_BASE+210)
#define DG_s7 (DG_BASE+222)
#define DG_s236 (DG_BASE+1886)
#define DG_s479 (DG_BASE+3474)
#define DG_s22 (DG_BASE+362)
#define DG_s246 (DG_BASE+1926)
#define DG_s25 (DG_BASE+374)
#define DG_s26 (DG_BASE+378)
#define DG_s27 (DG_BASE+382)
#define DG_s28 (DG_BASE+386)
#define DG_s318 (DG_BASE+2282)
#define DG_s319 (DG_BASE+2286)
#define DG_s322 (DG_BASE+2298)
#define DG_s340 (DG_BASE+2506)
#define DG_s341 (DG_BASE+2510)
#define DG_s342 (DG_BASE+2514)
#define DG_s343 (DG_BASE+2518)
#define DG_s344 (DG_BASE+2522)
#define DG_s349 (DG_BASE+2550)
#define DG_s355 (DG_BASE+2622)
#define DG_s458 (DG_BASE+3162)
#define DG_s473 (DG_BASE+3450)
#define DG_s474 (DG_BASE+3454)
#define DG_s476 (DG_BASE+3462)
#define DG_s498 (DG_BASE+3670)
#define DG_s5 (DG_BASE+214)
#define DG_s579 (DG_BASE+3994)
#define DG_s580 (DG_BASE+3998)
#define DG_s581 (DG_BASE+4002)
#define DG_s582 (DG_BASE+4006)
#define DG_s587 (DG_BASE+4026)
#define DG_s592 (DG_BASE+4046)
#define DG_s593 (DG_BASE+4050)
#define DG_s594 (DG_BASE+4054)
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
#define DG_w70 (DG_BASE+792)
#define DG_w71 (DG_BASE+800)
#define DG_w72 (DG_BASE+808)
#define DG_w73 (DG_BASE+816)
#define DG_w74 (DG_BASE+824)
#define DG_w75 (DG_BASE+832)
#define DG_w76 (DG_BASE+840)
#define DG_w77 (DG_BASE+848)
#define DG_w78 (DG_BASE+856)
#define DG_w79 (DG_BASE+864)
#define DG_w80 (DG_BASE+872)
#define DG_w81 (DG_BASE+880)
#define DG_w91 (DG_BASE+2144)
#define DG_w93 (DG_BASE+2576)
#define DG_w95 (DG_BASE+2592)
#define S(key) (*(int16_t *)((unsigned char *)state+DG_##key))
#define W(key) (*(int32_t *)((unsigned char *)state+DG_##key))
#define SD(key,offset) (*(int16_t *)((unsigned char *)state+DG_##key+(offset)))
#define WD(key,offset) (*(int32_t *)((unsigned char *)state+DG_##key+(offset)))

int32_t reloq_enus61_stretch_focus(void *state,int32_t p0,int32_t p1)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    return reloq_scalar_focus_duration(state,p1,S(s479)==S(s473),
        S(s236)<S(s7)&&S(s25)==S(s20)&&(S(s476)!=S(s473)||S(s4)==1));
}

int32_t reloq_enus61_init_gcat_vars(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    S(s246)=0; if(S(s474)==S(s473))S(s343)=S(s344)=1;
    return reloq_scalar_finish(state,0);
}

int32_t reloq_enus61_init_current_word_ptrs(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    W(w44)=W(w45)=W(w64)=W(w65)=W(w91);
    return reloq_scalar_finish(state,0);
}

int32_t reloq_enus61_increment_output_file_number(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    S(s355)=(int16_t)reloq_scalar_next_file((uint16_t)S(s355));
    return reloq_scalar_finish(state,0);
}

int32_t reloq_enus61_init_adjacent_word_ptrs(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    W(w70)=W(w71)=W(w72)=W(w73)=W(w74)=W(w75)=W(w76)=W(w77)=
        W(w78)=W(w79)=W(w80)=W(w81)=W(w91);
    return reloq_scalar_finish(state,0);
}

int32_t reloq_enus61_set_eng_global_constants(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    S(s319)=0; S(s322)=S(s593)=1; S(s594)=2;
    return reloq_scalar_finish(state,0);
}

int32_t reloq_enus61_set_syll_stream_word_ptrs(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    if(S(s476)!=S(s473)) { W(w21)=W(w19); W(w22)=W(w20); }
    return reloq_scalar_finish(state,0);
}

int32_t reloq_enus61_print_streamarrays_from_delta(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    
    return reloq_scalar_finish(state,0);
}

int32_t reloq_enus61_init_eng_vars(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    S(s344)=0; S(s473)=S(s474); S(s318)=S(s319);
    return reloq_scalar_finish(state,0);
}

int32_t reloq_enus61_reset_eng_sent_vars(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    
    return reloq_scalar_finish(state,0);
}

int32_t reloq_enus61_eng_ph_F_dur(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    W(f20)=200000; S(s458)=5;
    return reloq_scalar_finish(state,0);
}

int32_t reloq_enus61_is_target_syll(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    return reloq_scalar_finish(state,S(s18)==1 && (S(s25)!=S(s5) || S(s25)==S(s20))?0:94);
}

int32_t reloq_enus61_init_syll_acoustic_vars(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    S(s22)=S(s28);
    return reloq_scalar_finish(state,0);
}

int32_t reloq_enus61_init_syll_inton_vars(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    W(w34)=W(w35)=W(w91);
    S(s19)=reloq_scalar_next_accent(S(s19),S(s25),S(s26),S(s27),S(s5),S(s20));
    return reloq_scalar_finish(state,0);
}

int32_t reloq_enus61_init_eng_morph_vars(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    S(s340)=S(s587)=S(s349)=S(s581)=S(s580)=S(s592)=0;
    S(s341)=S(s579)=S(s582)=1;
    return reloq_scalar_finish(state,0);
}

int32_t reloq_enus61_set_voice_param_constants(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    reloq_scalar_voice_defaults((unsigned char *)state+DG_s498);
    return reloq_scalar_finish(state,0);
}

int32_t reloq_enus61_print_prompt(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    
    return reloq_scalar_finish(state,0);
}

int32_t reloq_enus61_reset_lang_sent_vars(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    
    return reloq_scalar_finish(state,0);
}

int32_t reloq_enus61_DeltaProc_getInputCharCount(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    
    return reloq_scalar_finish(state,0);
}

int32_t reloq_enus61_set_f0_wind_and_base(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    reloq_scalar_pitch_window(SD(c14,28),WD(c14,36),&W(f43),&W(f42));
    return reloq_scalar_finish(state,0);
}

int32_t reloq_enus61_DeltaProc_main(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    
    return reloq_scalar_finish(state,0);
}

int32_t reloq_enus61_extend_vals(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    
    return reloq_scalar_finish(state,0);
}

int32_t reloq_enus61_init_symbolic_vars(void *state,int32_t p0)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    S(s342)=0; W(w93)=W(w95)=W(w38);
    return reloq_scalar_finish(state,0);
}
