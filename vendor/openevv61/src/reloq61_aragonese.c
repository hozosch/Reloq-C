/* Initial Aragonese backend: shares immutable Castilian resources but owns
 * its language identity, pronunciation rules, and dictionaries.
 */
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
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

/* Native x/ix spellings. This cannot be a completely mechanical rule:
 * established and learned words can have the same spelling but /sh/ or /ks/.
 * Preserve common learned fix-/mixt- spellings and the Spanish fallback for
 * ambiguous cases, rather than turning every written x into /sh/. */
int reloq61_an_x_pronunciation(void *state,int32_t begin_ref,int32_t end_ref)
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
            const uint8_t vowels[]={1,2,3,4,5,6,7,8,9,10};
            for(size_t i=0;i<sizeof vowels;++i)
                if(!testFldeq(d,1,0,vowels[i])){match=1;break;}
            /* Greek xeno-, xero- and xilo- are pronounced /ks/ even
             * though native initial xe-/xi- can be pronounced /sh/. */
            int e=!testFldeq(d,1,0,2)||!testFldeq(d,1,0,7);
            int i=!testFldeq(d,1,0,3)||!testFldeq(d,1,0,8);
            if((e||i)&&!advance_tok(d)) {
                int greek=(e&&(!testFldeq(d,1,0,21)||!testFldeq(d,1,0,42)||
                               !testFldeq(d,1,0,24)||!testFldeq(d,1,0,45)))||
                          (i&&(!testFldeq(d,1,0,19)||!testFldeq(d,1,0,40)));
                if(greek&&!advance_tok(d)&&
                   (!testFldeq(d,1,0,4)||!testFldeq(d,1,0,9)))match=2;
            }
        }
    } else {
        lpta_loadp(d,EVV_AT(const delta_token *,begin_ref));
        if(setscan_l(d,1))goto done;
        if(!testFldeq(d,1,0,21)||!testFldeq(d,1,0,42)) {
            /* enxargar: x immediately after n. */
            match=1;
        } else if(!testFldeq(d,1,0,3)||!testFldeq(d,1,0,8)) {
            /* ix at the start of a word or following a vowel/consonant.
             * fixar and its family, and mixto, retain /ks/. */
            if(advance_tok(d))match=1;
            else if(!testFldeq(d,1,0,14)||!testFldeq(d,1,0,35))match=0;
            else if(!testFldeq(d,1,0,20)||!testFldeq(d,1,0,41)) {
                lpta_loadp(d,EVV_AT(const delta_token *,end_ref));
                if(!setscan_r(d,1)&&
                   (!testFldeq(d,1,0,26)||!testFldeq(d,1,0,47)))match=0;
                else match=1;
            } else match=1;
        } else if(!testFldeq(d,1,0,5)||!testFldeq(d,1,0,10)) {
            /* engauxar and the same -gaux- root; do not misread auxilio. */
            if(!advance_tok(d)&&(!testFldeq(d,1,0,1)||!testFldeq(d,1,0,6))&&
               !advance_tok(d)&&(!testFldeq(d,1,0,15)||!testFldeq(d,1,0,36)))match=1;
        }
    }
done:
    restore_cursor(d,&saved);
    return match;
}

/* The inherited q-rule consumes u in every context. In qua/quo it is a
 * pronounced glide; in que/qui it remains the silent part of the digraph. */
int reloq61_an_qu_glide(void *state,int32_t end_ref)
{
    delta_state *d=state;struct saved_cursor saved;
    if(!end_ref||!save_cursor(d,&saved))return 0;
    word_fence(d);
    lpta_loadp(d,EVV_AT(const delta_token *,end_ref));
    int match=0;
    if(!setscan_r(d,1)&&(!testFldeq(d,1,0,5)||!testFldeq(d,1,0,10))&&
       !advance_tok(d)) {
        const uint8_t open[]={1,4,6,9,107,110,112,115};
        for(size_t i=0;i<sizeof open;++i)
            if(!testFldeq(d,1,0,open[i])){match=1;break;}
    }
    restore_cursor(d,&saved);
    return match;
}

/* Read the spelling projection of a word without modifying its tokens. */
static int32_t an_input_node(delta_state *d,int32_t position,int back)
{
    if(!position)return 0;
    int32_t point=back?EVV_REF(vmovel(d,EVV_AT(delta_node *,position),1)):
        EVV_REF(vmover(d,EVV_AT(int32_t *,position),1));
    int32_t node=vgetsc(d,back,0,point,1);
    return node&&!(*EVV_AT(int32_t *,node)&2)?node:0;
}
static unsigned an_input_name(int32_t node)
{
    return node?*(const unsigned char *)delta_record_field(&vstmtbl[1],0,
        TFLDS(EVV_AT(void *,node))):0;
}
static int32_t an_right(delta_state *d,int32_t at,unsigned type)
{return at?EVV_REF(vmover(d,EVV_AT(void *,at),(uint8_t)type)):0;}

static size_t an_word_spelling(delta_state *d,int32_t begin,int32_t next,
                               char *text,size_t capacity,int *accented)
{
    static const char alphabet[]="\0aeiouAEIOUbcdfghjklmnpqrstvwxyzBCDFGHJKLMNPQRSTVWXYZ";
    size_t used=0;
    if(accented)*accented=0;
    int32_t at=begin,end=an_right(d,next,1);
    for(unsigned guard=0;at&&an_right(d,at,1)!=end&&guard<256;++guard) {
        int32_t node=an_input_node(d,at,0);unsigned name=an_input_name(node);
        if(!node||!name||used+1>=capacity)return 0;
        char c='?';
        if(name<sizeof alphabet-1)c=(char)tolower((unsigned char)alphabet[name]);
        else if(name>=97&&name<=116) {
            c="aeiou"[(name-97)%5];
            if(accented)*accented=1;
        }
        text[used++]=c;
        int32_t n=EVV_AT(int32_t *,node)[1]&~3;
        if(!n||n==at)return 0;
        at=n;
    }
    if(an_right(d,at,1)!=end)return 0;
    text[used]=0;
    return used;
}

/* Pa is an Aragonese preposition, not the parent's Spanish abbreviation
 * for por autorización. User abbreviation dictionaries are consulted
 * first in the existing caller. */
int reloq61_an_suppress_spanish_abbr(void *state,int32_t from,int32_t to)
{
    if(!from||!to)return 0;
    delta_state *d=state;
    const delta_loc *left=EVV_AT(const delta_loc *,from),*right=EVV_AT(const delta_loc *,to);
    char text[16];int accented=0;
    size_t n=an_word_spelling(d,left->value,right->value,text,sizeof text,&accented);
    /* At this stage the native input still contains the apostrophe. Keep
     * the original d token as provenance when the parser joins d'o/d'a,
     * including their plural forms. Later, da and dos must remain distinct
     * when they did not contain an apostrophe. */
    if(!accented&&(n==3||n==4)&&text[0]=='d'&&text[1]=='?'&&
       (text[2]=='o'||text[2]=='a')&&(n==3||text[3]=='s')) {
        int32_t first=an_input_node(d,left->value,0);
        int32_t position=first?(EVV_AT(int32_t *,first)[1]&~3):0;
        int32_t apostrophe=an_input_node(d,position,0);
        if(first&&an_input_name(apostrophe)==73)
            (void)reloq61_an_note_elision(state,first);
    }
    return !accented&&n==2&&!strcmp(text,"pa");
}

static int an_enclitic_word(const char *text)
{
    static const char *const clitics[]={
        "me","te","se","mos","nos","vos","lo","la","los","las",
        "le","les","li","ne","en","n","ie","i","bi",
        /* Apostrophized combinations share one native word record. */
        "mone","tone","sene","lin","lien","lsie"
    };
    for(size_t i=0;i<sizeof clitics/sizeof clitics[0];++i)
        if(!strcmp(text,clitics[i]))return 1;
    return 0;
}

static int an_weak_word(const char *text)
{
    static const char *const words[]={
        "pa","ta","enta","per","por","con","sin","de","en",
        "o","a","os","as","lo","la","los","las","ro","ra","ros","ras",
        "me","te","se","mos","nos","vos","le","les","li","ne","ie","bi","hu",
        "i","e","y","u","que"
    };
    for(size_t i=0;i<sizeof words/sizeof words[0];++i)
        if(!strcmp(text,words[i]))return 1;
    return 0;
}

static int32_t an_phone_node(delta_state *d,int32_t at)
{
    int32_t point=an_right(d,at,4),node=point?vgetsc(d,0,0,point,4):0;
    return node&&!(*EVV_AT(int32_t *,node)&2)?node:0;
}

/* Hyphenated, postverbal pronouns are unstressed and belong to the verb's
 * prosodic group. Adjust the existing native word records before the phrase
 * chooses its nuclear accent; spelling and source positions remain intact. */
void reloq61_an_adjust_clitics(void *state,int32_t from,int32_t to)
{
    if(delta_lang_now()->id!=RELOQ61_ARAGONESE||!from||!to)return;
    delta_state *d=state;
    delta_loc *left=EVV_AT(delta_loc *,from),*right=EVV_AT(delta_loc *,to);
    if(!left->value||!right->value)return;
    int32_t at=left->value,end=an_right(d,right->value,2);
    int previous_word=0,previous_annotated=0;
    for(unsigned guard=0;at&&an_right(d,at,2)!=end&&guard<256;++guard) {
        int32_t point=an_right(d,at,2),word=point?vgetsc(d,0,0,point,2):0;
        if(!word||(*EVV_AT(int32_t *,word)&2))break;
        unsigned char *body=TFLDS(EVV_AT(void *,word));
        unsigned char *level=delta_record_field(&vstmtbl[2],6,body);
        unsigned annotated=*(const unsigned char *)delta_record_field(&vstmtbl[2],9,body);
        int32_t begin=*EVV_AT(int32_t *,word)&~3,next=EVV_AT(int32_t *,word)[1]&~3;
        char text[128];int accented=0;
        size_t length=an_word_spelling(d,begin,next,text,sizeof text,&accented);
        int elision=length&&reloq61_an_take_elision(state,an_input_node(d,begin,0));
        if(length&&!annotated&&!accented&&(elision||an_weak_word(text))&&
           next&&an_right(d,next,2)!=end&&an_phone_node(d,next)) {
            int16_t unstressed=-1;
            memcpy(level,&unstressed,sizeof unstressed);
        }
        if(previous_word&&!previous_annotated&&!annotated&&
           an_input_name(an_input_node(d,begin,1))==63&&
           length&&an_enclitic_word(text)) {
            int16_t unstressed=-1;
            memcpy(level,&unstressed,sizeof unstressed);
        }
        previous_word=1;previous_annotated=annotated!=0;
        if(!next||next==at)break;
        at=next;
    }
}
