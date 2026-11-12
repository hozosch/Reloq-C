/* Initial Aragonese backend: shares immutable Castilian resources but owns
 * its language identity, pronunciation rules, and dictionaries.
 */
#include <string.h>
#include <stdlib.h>
#include "delta.h"
#include "delta_lang.h"
#include "reloq61_aragonese.h"

extern delta_language delta_lang_eses61;
delta_language delta_lang_anar61;
static char settings[4096];

void delta_lang_bind_anar61(void)
{
    delta_lang_anar61=delta_lang_eses61;
    delta_lang_anar61.tag="anar61";
    delta_lang_anar61.name="Aragonese";
    delta_lang_anar61.id=RELOQ61_ARAGONESE;
    delta_lang_anar61.library_name="Static Engine Aragonese";
    const char *source=delta_lang_eses61.ini;
    size_t used=0;
    for(int i=0;i<delta_lang_eses61.ini_size;) {
        if(i+5<=delta_lang_eses61.ini_size&&!memcmp(source+i,"[2.0]",5)) {
            if(used>sizeof settings-6)abort();
            memcpy(settings+used,"[19.0]",6);used+=6;i+=5;
        } else {
            if(used==sizeof settings)abort();
            settings[used++]=source[i++];
        }
    }
    delta_lang_anar61.ini=settings;
    delta_lang_anar61.ini_size=(int32_t)used;
}

/* Save every cursor and fence cell we alter; these rules run in a provider
 * which can be reused by other languages on the same worker thread. */
struct saved_cursor {
    delta_tpos left,right;
    unsigned char scan[8],index[256],marks[256],chars[256];
    int8_t compared,fences,testing;
};
static int save_cursor(delta_state *d,struct saved_cursor *saved)
{
    if(!d||!d->vars)return 0;
    delta_vars *v=EVV_AT(delta_vars *,d->vars);
    saved->left=d->lpta;saved->right=d->rpta;
    saved->compared=v->compared_equal;saved->fences=v->fence_count;saved->testing=v->testing;
    memcpy(saved->scan,&v->scan_ptr,sizeof saved->scan);
    memcpy(saved->index,EVV_AT(const void *,d->fence_index),d->nstmts);
    memcpy(saved->marks,EVV_AT(const void *,d->fence_marks),d->nstmts);
    memcpy(saved->chars,EVV_AT(const void *,d->fence_chars),d->nstmts);
    v->testing=0;
    return 1;
}
static void restore_cursor(delta_state *d,const struct saved_cursor *saved)
{
    delta_vars *v=EVV_AT(delta_vars *,d->vars);
    d->lpta=saved->left;d->rpta=saved->right;
    memcpy(&v->scan_ptr,saved->scan,sizeof saved->scan);
    v->compared_equal=saved->compared;v->fence_count=saved->fences;v->testing=saved->testing;
    memcpy(EVV_AT(void *,d->fence_index),saved->index,d->nstmts);
    memcpy(EVV_AT(void *,d->fence_marks),saved->marks,d->nstmts);
    memcpy(EVV_AT(void *,d->fence_chars),saved->chars,d->nstmts);
}
static void word_fence(delta_state *d)
{
    const uint8_t word[]={2};
    fence(d,1,word);
    memset(EVV_AT(void *,d->fence_marks),0,d->nstmts);
}

/* Token points immediately before /a/. Test /aw/ inside a single word and
 * require no earlier phone in that word. This is the 70% listening candidate. */
int reloq61_an_initial_aw(void *state,int32_t token_ref)
{
    delta_state *d=state;struct saved_cursor saved;
    if(!token_ref||!save_cursor(d,&saved))return 0;
    delta_vars *v=EVV_AT(delta_vars *,d->vars);
    word_fence(d);lpta_loadp(d,EVV_AT(const delta_token *,token_ref));
    int aw=!setscan_r(d,4)&&!testFldeq(d,4,0,31)&&!advance_tok(d)&&
           !testFldeq(d,4,0,28)&&!advance_tok(d);
    lpta_loadp(d,EVV_AT(const delta_token *,token_ref));
    int initial;
    if(setscan_l(d,4))initial=1;
    else {v->scan_held=0;initial=advance_tok(d)!=0;}
    restore_cursor(d,&saved);
    return aw&&initial;
}

/* Token points immediately before /w/. Restrict rounded targets to /aw/. */
int reloq61_an_aw(void *state,int32_t token_ref)
{
    delta_state *d=state;struct saved_cursor saved;
    if(!token_ref||!save_cursor(d,&saved))return 0;
    word_fence(d);lpta_loadp(d,EVV_AT(const delta_token *,token_ref));
    int after_a=!setscan_l(d,4)&&!testFldeq(d,4,0,31)&&!advance_tok(d);
    lpta_loadp(d,EVV_AT(const delta_token *,token_ref));
    int at_w=!setscan_r(d,4)&&!testFldeq(d,4,0,28);
    restore_cursor(d,&saved);
    return after_a&&at_w;
}

/* This isolates the approved /sh/ from the tested /tsh/ affricates. It is
 * deliberately provisional: a real /t/ + /sh/ sequence also needs a more
 * precise affricate marker before an Aragonese voice is exposed. */
int reloq61_an_standalone_sh(void *state,int32_t token_ref)
{
    delta_state *d=state;struct saved_cursor saved;
    if(!token_ref||!save_cursor(d,&saved))return 0;
    delta_vars *v=EVV_AT(delta_vars *,d->vars);
    v->fence_count=0;
    const uint8_t phone=16,stop=2;
    lpta_loadp(d,EVV_AT(const delta_token *,token_ref));
    int found=!setscan_r(d,4)&&!test_string_s(d,4,1,&phone);
    lpta_loadp(d,EVV_AT(const delta_token *,token_ref));
    int affricate=!setscan_l(d,4)&&!test_string_s(d,4,1,&stop);
    restore_cursor(d,&saved);
    return found&&!affricate;
}

/* Conservative native spelling cases: vowel+ix and word-initial xa/xo/xu.
 * Other x spellings can be /ks/ or /sh/ and remain with the inherited rule
 * until the lexicon can resolve them. The word fence prevents a match across
 * adjacent words. Nothing is changed in the caller's scanner. */
int reloq61_an_x_is_sh(void *state,int32_t begin_ref,int32_t end_ref)
{
    delta_state *d=state;struct saved_cursor saved;
    if(!begin_ref||!end_ref||!save_cursor(d,&saved))return 0;
    word_fence(d);
    lpta_loadp(d,EVV_AT(const delta_token *,begin_ref));
    int left=setscan_l(d,1),match=0;
    if(!left) {
        /* setscan only establishes a cursor; it does not itself test the
         * word fence. Try a step to tell the start of a word from an
         * ordinary preceding letter. */
        EVV_AT(delta_vars *,d->vars)->scan_held=0;
        left=advance_tok(d)!=0;
    }
    if(left) {
        lpta_loadp(d,EVV_AT(const delta_token *,end_ref));
        if(!setscan_r(d,1)) {
            const uint8_t vowels[]={1,4,5,6,9,10};
            for(size_t i=0;i<sizeof vowels;++i)
                if(!testFldeq(d,1,0,vowels[i])){match=1;break;}
        }
    } else {
        lpta_loadp(d,EVV_AT(const delta_token *,begin_ref));
        if(setscan_l(d,1))goto done;
        if(testFldeq(d,1,0,3)&&testFldeq(d,1,0,8))goto done;
        if(!advance_tok(d)) {
            const uint8_t vowels[]={1,2,4,5,6,7,9,10};
            for(size_t i=0;i<sizeof vowels;++i)
                if(!testFldeq(d,1,0,vowels[i])){match=1;break;}
        }
    }
done:
    restore_cursor(d,&saved);
    return match;
}
