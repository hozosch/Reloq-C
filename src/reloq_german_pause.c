#include "reloq_german_pause.h"
#include <limits.h>

int reloq_german_pause_plan(const struct reloq_german_phone_node *node,
    int carry, enum reloq_german_terminal_mark mark, int continuing,
    struct reloq_german_pause *out)
{
    struct reloq_german_terminal_timing timing;
    if (!node || !out || node->phone<=RELOQ_DE_PHONE_NONE ||
        node->phone>RELOQ_DE_PHONE_Q_oe_TILDE ||
        (carry!=0 && carry!=1) || (continuing!=0 && continuing!=1) ||
        reloq_german_terminal_timing(mark,&timing)) return -1;
    const enum reloq_german_phone p=node->phone;
    const int soft=continuing && reloq_german_terminal_continues(mark);
    struct reloq_german_pause next={30,timing.window_ticks-60,-1,-1,0};
    switch (p) {
    case RELOQ_DE_PHONE_p: case RELOQ_DE_PHONE_b:
        next.release_ticks=60;next.av=0;next.ah=35;break;
    case RELOQ_DE_PHONE_t: case RELOQ_DE_PHONE_d:
        next.release_ticks=60;next.av=0;next.ah=48;break;
    case RELOQ_DE_PHONE_k: case RELOQ_DE_PHONE_g:
        next.release_ticks=60;next.av=0;
        next.ah=node->left_phone==RELOQ_DE_PHONE_I?53:
            node->left_phone==RELOQ_DE_PHONE_e || node->left_phone==RELOQ_DE_PHONE_i?51:48;
        break;
    case RELOQ_DE_PHONE_m:
        next.release_ticks=15;next.av=node->stress?43:41;
        if (node->left_phone==RELOQ_DE_PHONE_a || node->left_phone==RELOQ_DE_PHONE_A ||
            node->left_phone==RELOQ_DE_PHONE_R) next.av-=3;
        next.ah=next.av+4;next.voiced_ticks=7;break;
    case RELOQ_DE_PHONE_n:
        next.av=node->stress?40:38;next.ah=node->stress?38:36;
        next.voiced_ticks=15;break;
    case RELOQ_DE_PHONE_G:
        next.av=0;next.ah=38;break;
    case RELOQ_DE_PHONE_Q_aw: case RELOQ_DE_PHONE_w:
        next.release_ticks=40;next.ah=soft?-1:35;break;
    case RELOQ_DE_PHONE_Q_aj: case RELOQ_DE_PHONE_Q_oj:
    case RELOQ_DE_PHONE_l: case RELOQ_DE_PHONE_r: case RELOQ_DE_PHONE_j:
        next.ah=soft?-1:35;break;
    default:
        if ((p>=RELOQ_DE_PHONE_a && p<=RELOQ_DE_PHONE_AT) ||
            p==RELOQ_DE_PHONE_R || p>=RELOQ_DE_PHONE_Q_E_COLON) {
            next.release_ticks=carry || soft?30:60;
            next.ah=soft?-1:35;
        }
        break;
    }
    /* A final nasal after an obstruent owns a complete nasal source and
     * its observed release, rather than the postvocalic quiet tail. */
    if(node->same_syllable_left &&
       (node->left_phone==RELOQ_DE_PHONE_p || node->left_phone==RELOQ_DE_PHONE_t ||
        node->left_phone==RELOQ_DE_PHONE_k || node->left_phone==RELOQ_DE_PHONE_b ||
        node->left_phone==RELOQ_DE_PHONE_d || node->left_phone==RELOQ_DE_PHONE_g ||
        node->left_phone==RELOQ_DE_PHONE_f || node->left_phone==RELOQ_DE_PHONE_v ||
        node->left_phone==RELOQ_DE_PHONE_s || node->left_phone==RELOQ_DE_PHONE_z ||
        node->left_phone==RELOQ_DE_PHONE_S || node->left_phone==RELOQ_DE_PHONE_Z ||
        node->left_phone==RELOQ_DE_PHONE_x || node->left_phone==RELOQ_DE_PHONE_X)) {
        if(p==RELOQ_DE_PHONE_m || p==RELOQ_DE_PHONE_n)next.av=next.ah=45;
        else if(p==RELOQ_DE_PHONE_G){next.av=0;next.ah=39;}
    }
    *out=next;
    return 0;
}

int reloq_german_pause_release(const struct reloq_german_pause *p,
    int64_t begin,size_t phone,reloq_german_pause_emit emit,void *context)
{
    if (!p || !emit || begin<0 || p->release_ticks<=0 ||
        begin>INT64_MAX-p->release_ticks || p->voiced_ticks<0 ||
        p->voiced_ticks>=p->release_ticks || p->av< -1 || p->ah< -1) return -1;
    const int64_t end=begin+p->release_ticks;
    if (p->ah>=0 && (emit(context,RELOQ_TARGET_AH,begin,p->ah,phone) ||
                     emit(context,RELOQ_TARGET_AH,end,p->ah,phone))) return -1;
    if (p->av>=0) {
        const int64_t change=begin+p->voiced_ticks;
        if (emit(context,RELOQ_TARGET_AV,begin,p->av,phone)) return -1;
        if (p->voiced_ticks && (emit(context,RELOQ_TARGET_AV,change,p->av,phone) ||
                               emit(context,RELOQ_TARGET_AV,change,0,phone))) return -1;
        if (emit(context,RELOQ_TARGET_AV,end,0,phone)) return -1;
    }
    return 0;
}
