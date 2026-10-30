#include "reloq_rules61.h"
#include "reloq61.h"
#include "reloq_german_annotation.h"
#include "reloq_german_generic_prosody.h"
#include "reloq_german_lexicon.h"
#include "reloq_german_morphology.h"
#include "reloq_german_root_plan.h"
#include "reloq_inline_tags.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct group { size_t open,end; unsigned syllables; };
struct bytes { unsigned char *p;size_t n,capacity; };
static int same_character(const struct reloq_german_prosody_unit *a,
    const struct reloq_german_prosody_unit *b)
{return a->fixed_phrase>=RELOQ_DE_SOURCE_SYMBOL&&a->fixed_phrase==b->fixed_phrase&&
    a->text==b->text&&a->length==b->length;}
static int adjacent_characters(const struct reloq_german_prosody_unit *a,
    const struct reloq_german_prosody_unit *b)
{return a->fixed_phrase>=RELOQ_DE_SOURCE_SYMBOL&&a->fixed_phrase==b->fixed_phrase&&
    a->text+a->length==b->text;}
/* In an utterance consisting only of ampersands, a separately written
 * final pair names both words. An attached run retains conjunction stress.
 * All symbol sources refer to the same immutable admitted input buffer. */
static int final_ampersand_pair(const struct reloq_german_prosody_unit *u,size_t n)
{
    if(n<2)return 0;
    for(size_t i=0;i<n;++i)
        if(u[i].fixed_phrase!=RELOQ_DE_SOURCE_SYMBOL||u[i].length!=1||u[i].text[0]!='&')return 0;
    const unsigned char *p=u[n-2].text+1;
    uintptr_t first=(uintptr_t)p,last=(uintptr_t)u[n-1].text;
    if(last<=first||last-first>256)return 0;
    for(size_t i=0;i<last-first;++i)
        if(p[i]!=' '&&p[i]!='\t'&&p[i]!='\n'&&p[i]!='\r'&&p[i]!=0xa0)return 0;
    return 1;
}
static int put(struct bytes *b,const unsigned char *p,size_t n)
{
    if(n>SIZE_MAX-b->n-1)return -1;
    size_t need=b->n+n+1;
    if(need>b->capacity) {
        size_t cap=b->capacity?b->capacity:256;
        while(cap<need) {if(cap>SIZE_MAX/2){cap=need;break;}cap*=2;}
        void *next=realloc(b->p,cap);if(!next)return -1;b->p=next;b->capacity=cap;
    }
    memcpy(b->p+b->n,p,n);b->n+=n;b->p[b->n]=0;return 0;
}
static int append_phonemes(void *context,const char *data,size_t length)
{return put(context,(const unsigned char *)data,length);}
static int groups(const unsigned char *s,size_t n,struct group *g,size_t count)
{
    struct reloq_german_annotation_cursor c={0};
    struct reloq_german_annotation_event e;size_t seen=0;int state;
    while((state=reloq_german_annotation_next(s,n,&c,UINT32_MAX,&e))==1) {
        if(e.kind!=RELOQ_DE_ANNOTATION_SPR)continue;
        if(e.spr.kind==RELOQ_DE_SPR_WORD_OPEN) {
            if(seen>=count)return 0;
            g[seen++]=(struct group){.open=e.offset};
        } else if(e.spr.kind==RELOQ_DE_SPR_WORD_CLOSE) {
            if(!seen)return 0;
            g[seen-1].end=e.offset+e.length;
        } else if(e.spr.kind==RELOQ_DE_SPR_SYLLABLE && seen)++g[seen-1].syllables;
    }
    return !state&&seen==count;
}
static int phone_next(const unsigned char *s,size_t n,
    struct reloq_german_spr_cursor *c,struct reloq_german_spr_event *e)
{
    int state;
    while((state=reloq_german_spr_next(s,n,c,e))==1)
        if(e->kind==RELOQ_DE_SPR_PHONE)return 1;
    return state;
}
static int same_phones(const unsigned char *a,size_t an,const unsigned char *b,size_t bn)
{
    struct reloq_german_spr_cursor ac={0},bc={0};
    struct reloq_german_spr_event ae,be;int ar,br;
    do {
        ar=phone_next(a,an,&ac,&ae);br=phone_next(b,bn,&bc,&be);
        if(ar!=br||ar<0||(ar&&(ae.phone!=be.phone||ae.syllable!=be.syllable)))return 0;
    } while(ar);
    return 1;
}
static int voice_control(enum reloq_inline_kind kind)
{
    return (kind>=RELOQ_TAG_PITCH&&kind<=RELOQ_TAG_INFLECTION)||
        (kind>=RELOQ_TAG_GENDER&&kind<=RELOQ_TAG_PRESET);
}
static int retain_voice_controls(struct bytes *out,const unsigned char *s,size_t n)
{
    for(size_t at=0;at<n;) {
        struct reloq_inline_tag t=reloq_inline_tag_read(s+at,n-at,1);
        if(voice_control(t.kind) &&
           (put(out,s+at,t.length)||put(out,(const unsigned char *)" ",1)))return -1;
        at+=t.length?t.length:1;
    }
    return 0;
}
static int prefix(struct bytes *out,const unsigned char *s,size_t n,int force)
{
    for(size_t i=0;i<n;) {
        struct reloq_inline_tag t=reloq_inline_tag_read(s+i,n-i,1);
        if(t.kind>=RELOQ_TAG_PITCH && t.kind<=RELOQ_TAG_INFLECTION) {i+=t.length;continue;}
        if(t.kind==RELOQ_TAG_PROMINENCE && force) {i+=t.length;continue;}
        if(s[i]) {if(put(out,s+i,1))return -1;} /* callback NUL becomes a separator */
        else if(put(out,(const unsigned char *)" ",1))return -1;
        ++i;
    }
    if(force && put(out,(const unsigned char *)(force==1?"`00 ":"`0 "),force==1?4:3))return -1;
    return 0;
}
/* Phonetic ECI output omits an ellipsis although the native planner keeps
 * its suspended boundary internally. The compact German planner owns its
 * own boundary marks, so carry that source fact across the word alignment.
 * Never scan inside an SPR/control group or manufacture a new text token. */
static int ellipsis(const unsigned char *s,size_t n)
{
    for(size_t i=0;i<n;) {
        struct reloq_inline_tag t=reloq_inline_tag_read(s+i,n-i,1);
        if(t.length){i+=t.length;continue;}
        if(s[i]==0x85 || (s[i]=='.'&&i+2<n&&s[i+1]=='.'&&s[i+2]=='.'))return 1;
        ++i;
    }
    return 0;
}
static int boundary(struct bytes *out,const unsigned char *source,size_t source_n,
    const unsigned char *native,size_t native_n)
{
    static const unsigned char suspended=0x85;
    /* Public phoneme output omits these written clause delimiters. Keep
     * their identity instead of inventing a comma before native analysis. */
    for(size_t i=0;i<source_n;++i)
        if((source[i]=='('||source[i]==')'||source[i]=='-'||source[i]==0x96||source[i]==0x97)&&put(out,source+i,1))return -1;
    return ellipsis(source,source_n)&&!ellipsis(native,native_n)?put(out,&suspended,1):0;
}
static int pronoun(const struct reloq_german_prosody_unit *u)
{
    static const char *const words[]={"ich","du","er","sie","es","wir","ihr"};
    for(size_t i=0;i<sizeof words/sizeof *words;++i) {
        if(strlen(words[i])!=u->length)continue;
        size_t j=0;
        for(;j<u->length;++j) {
            unsigned char c=u->text[j];if(c>='A'&&c<='Z')c+=32;
            if(c!=(unsigned char)words[i][j])break;
        }
        if(j==u->length)return 1;
    }
    return 0;
}
static size_t final_tail(const struct reloq_german_prosody_unit *u,size_t count,
    const unsigned char *s,const struct group *g,size_t n)
{
    size_t candidate=count,before=0;
    for(size_t i=0;i<count;++i) {
        for(size_t at=before;at<g[i].open;++at) {
            if(s[at]==',')candidate=i;
            else if(strchr(".?!:;",s[at]))candidate=count;
        }
        before=g[i].end;
    }
    if(candidate==count || count-candidate<2 || count-candidate>4 ||
       !pronoun(u+count-1))return count;
    for(size_t at=g[count-1].end;at<n;++at)if(s[at]=='?'||s[at]=='!'||s[at]==',')return count;
    before=candidate?g[candidate-1].end:0;
    for(size_t at=before;at<g[count-1].end;) {
        struct reloq_inline_tag t=reloq_inline_tag_read(s+at,g[count-1].end-at,1);
        /* The full native frontend deliberately withheld a new nucleus.
           A clause with an explicit 2 remains an ordinary accent phrase. */
        if(t.kind==RELOQ_TAG_PROMINENCE && t.value>=2 && t.value<=4)return count;
        at+=t.length?t.length:1;
    }
    for(size_t i=candidate;i<count;++i)
        if(u[i].preserve||u[i].fixed_phrase||u[i].force_weak||u[i].force_deaccent||u[i].source_emphasis)return count;
    return candidate;
}
static size_t source_offset(const struct reloq_german_context_source *source,
    const struct reloq_german_prosody_unit *unit)
{
    if(!source)return SIZE_MAX;
    const unsigned char *buffers[]={source->encoded,source->original,source->words};
    uintptr_t at=(uintptr_t)unit->text;
    for(size_t i=0;i<sizeof buffers/sizeof *buffers;++i) {
        uintptr_t base=(uintptr_t)buffers[i];
        if(base&&at>=base&&at-base<=source->length&&unit->length<=source->length-(at-base))
            return (size_t)(at-base);
    }
    return SIZE_MAX;
}
static int native_german_word(const unsigned char *word,size_t length,
    unsigned char *spr,size_t capacity,size_t *written)
{return reloq_rules61_word(0x40000,word,length,spr,capacity,written);}

int reloq_rules61_german_context_features(const struct reloq_german_prosody_unit *units,size_t count,
    const struct reloq_lexicon *lexicon,
    const unsigned char *annotation,size_t length,unsigned char **out,size_t *written,
    struct reloq61_german_word_facts *facts,int (*cancelled)(void *),void *context,
    const struct reloq_german_context_source *source)
{
    if(facts&&count>SIZE_MAX/sizeof *facts)return -1;
    if(facts)memset(facts,0,count*sizeof *facts);
    if(!units||!count||!annotation||!out||!written)return 0;
    if(cancelled&&cancelled(context))return 2;
    int prediction=reloq61_text_options_current().phrase_prediction;
    for(size_t at=0;at<length;) {
        struct reloq_inline_tag t=reloq_inline_tag_read(annotation+at,length-at,1);
        if(t.kind==RELOQ_TAG_PREDICTION && t.value)prediction=1;
        at+=t.length?t.length:1;
    }
    int supplied=1;
    for(size_t i=0;i<count;++i) {
        /* Composition needs real phones before it can become a fixed
           phrase. Refuse provisional groups so the caller retries its
           complete constituent analysis, without speaking placeholders. */
        if(units[i].unresolved &&
           (units[i].preserve||units[i].fixed_phrase||!units[i].length))return 0;
        if(!units[i].preserve)supplied=0;
    }
    /* A single original character already has its complete native reading.
     * Keep navigation latency independent of another language-analysis pass. */
    if(units[0].fixed_phrase==RELOQ_DE_SOURCE_CHARACTER) {
        size_t i=1;
        while(i<count&&same_character(units,units+i))++i;
        if(i==count)supplied=1;
    }
    if(supplied) {
        /* Fully supplied phonetic input already owns phones and accents.
           Character navigation must not run another language analysis. */
        size_t begin=0,end=length;
        while(begin<end&&annotation[begin]==' ')++begin;
        while(end>begin&&annotation[end-1]==' ')--end;
        unsigned char *copy=malloc(end-begin+1);if(!copy)return -1;
        memcpy(copy,annotation+begin,end-begin);copy[end-begin]=0;
        *out=copy;*written=end-begin;return 1;
    }
    if(count>SIZE_MAX/(2*sizeof(struct group)))return -1;
    /* groups() initializes each admitted record before any field is read. */
    struct group *old=malloc(2*count*sizeof *old),*native=old?old+count:NULL;
    struct bytes input={0},output={0},spoken={0};unsigned char *phonemes=NULL;int result=0;
    struct reloq61_german_phrase *phrases=NULL;
    if(!old||!native){result=-1;goto done;}
    if(!groups(annotation,length,old,count))goto done;
    /* Acoustic controls retain their exact word boundary. They do not
       disable the complete sentence analysis or become lexical input. */
    for(size_t at=0;at<length;) {
        if(annotation[at]=='`') {
            struct reloq_inline_tag t=reloq_inline_tag_read(annotation+at,length-at,1);
            if(t.kind!=RELOQ_TAG_SPR && t.kind!=RELOQ_TAG_PROMINENCE && t.kind!=RELOQ_TAG_DICTIONARY &&
               t.kind!=RELOQ_TAG_PREDICTION && !voice_control(t.kind))goto done;
            if(!t.length)goto done;
            at+=t.length;
        } else ++at;
    }
    size_t before=0,inserted_space=SIZE_MAX;
    for(size_t i=0;i<count;++i) {
        if(cancelled&&cancelled(context)){result=2;goto done;}
        int repeated=i&&same_character(units+i-1,units+i);
        int joined=repeated||(i&&adjacent_characters(units+i-1,units+i));
        size_t gap_begin=SIZE_MAX,gap_end=SIZE_MAX;
        if(i&&source) {
            gap_begin=source_offset(source,units+i-1);gap_end=source_offset(source,units+i);
            if(gap_begin!=SIZE_MAX)gap_begin+=units[i-1].length;
            if(gap_begin>gap_end || gap_end==SIZE_MAX)gap_begin=SIZE_MAX;
            if(gap_begin!=SIZE_MAX)for(size_t k=gap_begin;k<gap_end;++k)
                if(!strchr(" \t\r\n",source->encoded[k])&&source->encoded[k]!=0xa0){gap_begin=SIZE_MAX;break;}
        }
        if(gap_begin!=SIZE_MAX) {
            if(inserted_space!=SIZE_MAX&&inserted_space+1==input.n)input.p[--input.n]=0;
            if(put(&input,source->encoded+gap_begin,gap_end-gap_begin))goto memory;
        } else if(!repeated)for(size_t at=before;at<old[i].open;) {
            struct reloq_inline_tag t=reloq_inline_tag_read(annotation+at,old[i].open-at,1);
            if(t.kind==RELOQ_TAG_PROMINENCE || voice_control(t.kind)) {at+=t.length;continue;}
            if(!(joined&&annotation[at]==' ')&&put(&input,annotation+at,1))goto memory;
            ++at;
        }
        inserted_space=SIZE_MAX;
        if(units[i].source_emphasis) {
            static const char *const controls[]={"","`00 ","`0 ","`1 ","`2 ","`3 ","`4 "};
            int value=units[i].source_emphasis;
            if(value<1||value>6)goto done;
            if(put(&input,(const unsigned char *)controls[value],strlen(controls[value])))goto memory;
        }
        if(units[i].fixed_phrase>=RELOQ_DE_SOURCE_SYMBOL) {
            if(!repeated) {
                size_t next=i+1;
                while(next<count&&same_character(units+i,units+next))++next;
                int continues=next<count&&adjacent_characters(units+i,units+next);
                if(units[i].fixed_phrase==RELOQ_DE_SOURCE_CHARACTER&&!joined&&put(&input,(const unsigned char *)" `ts2 ",6))goto memory;
                if(put(&input,units[i].text,units[i].length))goto memory;
                if(units[i].fixed_phrase==RELOQ_DE_SOURCE_CHARACTER&&!continues&&put(&input,(const unsigned char *)" `ts0 ",6))goto memory;
            }
        } else if(units[i].preserve || units[i].fixed_phrase || !units[i].length) {
            if(put(&input,annotation+old[i].open,old[i].end-old[i].open))goto memory;
        } else if(put(&input,units[i].text,units[i].length))goto memory;
        unsigned char following=old[i].end<length?annotation[old[i].end]:0;
        int attached=following=='('||following==')'||following=='-'||following==0x96||following==0x97;
        if(units[i].fixed_phrase<RELOQ_DE_SOURCE_SYMBOL&&!attached) {
            inserted_space=input.n;
            if(put(&input,(const unsigned char *)" ",1))goto memory;
        }
        before=old[i].end;
    }
    for(size_t at=before;at<length;) {
        struct reloq_inline_tag t=reloq_inline_tag_read(annotation+at,length-at,1);
        if(voice_control(t.kind)){at+=t.length;continue;}
        if(put(&input,annotation+at,1))goto memory;
        ++at;
    }
    struct reloq61_request q={.language=0x40000,.annotations=1,.phonemes_only=1,
        .voice={0,50,65,30,0,0,50,92},.text=(char *)input.p};
    struct reloq61_result r;
    if(prediction&&facts) {
        if(count>SIZE_MAX/sizeof *phrases)goto memory;
        phrases=calloc(count,sizeof *phrases);if(!phrases)goto memory;
    }
    struct reloq61_german_capture capture={.words=facts,.capacity=count,
        .phrases=phrases,.phrase_capacity=prediction?count:0};
    struct reloq61_german_capture *previous=reloq61_german_capture_scope(facts?&capture:NULL);
    int generated=reloq61_generate_phonemes(&q,&r,append_phonemes,&spoken);
    reloq61_german_capture_scope(previous);
    /* Native input is no longer needed while the final annotation grows. */
    free(input.p);input=(struct bytes){0};
    phonemes=spoken.p;
    if(generated){if(r.error==1)goto memory;goto done;}
    if(facts&&(capture.overflow||capture.count!=count))memset(facts,0,count*sizeof *facts);
    if(cancelled&&cancelled(context)){result=2;goto done;}
    if(!groups(phonemes,r.phonemes_length,native,count))goto done;
    size_t tail=final_tail(units,count,phonemes,native,r.phonemes_length);
    if(boundary(&output,annotation,old[0].open,NULL,0))goto memory;
    int named_pair=final_ampersand_pair(units,count);
    before=0;
    for(size_t i=0;i<count;++i) {
        if(cancelled&&cancelled(context)){result=2;goto done;}
        int force=units[i].source_emphasis?0:units[i].force_weak?1:units[i].force_deaccent?2:0;
        size_t original_before=i?old[i-1].end:0;
        if(i&&boundary(&output,annotation+original_before,old[i].open-original_before,
            phonemes+before,native[i].open-before))goto memory;
        /* The native SPR callback omits inferred intonation boundaries.
         * Project the original phrase relation alongside each word, rather
         * than guessing breaks from word counts or editing the input text. */
        if(i && prediction && facts && facts[i].phrase_before) {
            int explicit_boundary=0;
            for(size_t at=original_before;at<old[i].open;) {
                struct reloq_inline_tag t=reloq_inline_tag_read(annotation+at,old[i].open-at,1);
                if(!t.length && (strchr(",.?!:;()",annotation[at])||annotation[at]==0x85))explicit_boundary=1;
                at+=t.length?t.length:1;
            }
            for(size_t at=before;at<native[i].open;) {
                struct reloq_inline_tag t=reloq_inline_tag_read(phonemes+at,native[i].open-at,1);
                if(!t.length && strchr(",.?!:;()",phonemes[at]))explicit_boundary=1;
                at+=t.length?t.length:1;
            }
            if(!explicit_boundary) {
                static const unsigned char marks[]={'.',',',';','?','?','!','!','?','?'};
                unsigned kind=facts[i].phrase_before-1;
                if(kind<sizeof marks && put(&output,marks+kind,1))goto memory;
            }
        }
        if(retain_voice_controls(&output,annotation+original_before,old[i].open-original_before))goto memory;
        if(named_pair&&i>=count-2&&!units[i].source_emphasis) {
            if(count>2&&i==count-2&&put(&output,(const unsigned char *)"`2 ",3))goto memory;
        } else if(prefix(&output,phonemes+before,native[i].open-before,force))goto memory;
        if(i==tail && put(&output,(const unsigned char *)"`10 ",4))goto memory;
        size_t start=output.n;
        /* The requested voiced reading of "samt" and authored/expanded
           pronunciations take precedence over the contextual dictionary. */
        int samt=units[i].fixed_phrase!=RELOQ_DE_SOURCE_DICTIONARY && units[i].length==4 &&
            (units[i].text[0]=='s'||units[i].text[0]=='S') &&
            !memcmp(units[i].text+1,"amt",3);
        const struct reloq_lexicon_override *entry=NULL;
        int explicit_entry=units[i].fixed_phrase!=RELOQ_DE_SOURCE_DICTIONARY && lexicon && reloq_lexicon_find_override(lexicon,
            units[i].text,units[i].length,&entry)==1;
        int preserve=units[i].preserve||units[i].fixed_phrase==RELOQ_DE_SOURCE_SPR||
            units[i].fixed_phrase==RELOQ_DE_SOURCE_NUMBER||samt ||
            (explicit_entry && !same_phones(annotation+old[i].open,old[i].end-old[i].open,
                                           phonemes+native[i].open,native[i].end-native[i].open));
        if(preserve&&units[i].unresolved)goto done;
        const unsigned char *word=preserve?annotation+old[i].open:phonemes+native[i].open;
        size_t n=preserve?old[i].end-old[i].open:native[i].end-native[i].open;
        unsigned char compound[4096];size_t compound_n=0;
        /* Joined hyphens already passed the constituent override guard in
         * admission. Keep their native whole-word pronunciation and stress. */
        if(!units[i].preserve && !units[i].fixed_phrase && !explicit_entry &&
           !memchr(units[i].text,'-',units[i].length) &&
           reloq_german_morphology_user_compound_resolved(lexicon,units[i].text,units[i].length,
               compound,sizeof compound,&compound_n,native_german_word)==1) {word=compound;n=compound_n;}
        if(!units[i].preserve && !units[i].fixed_phrase && !explicit_entry &&
           reloq_german_root_plan_os_adjective(units[i].text,units[i].length) &&
           reloq_german_root_plan_word(lexicon,units[i].text,units[i].length,
               compound,sizeof compound,&compound_n)==1) {word=compound;n=compound_n;}
        if(reloq_german_lexicon_validate_spr(word,n,NULL)!=1)goto done;
        /* Caller-owned phones and authored departures keep their own facts.
         * Never apply onset bits to a different syllable/phone alignment. */
        if(facts&&(units[i].preserve||units[i].fixed_phrase==RELOQ_DE_SOURCE_SPR||
           (explicit_entry&&reloq_lexicon_override_from_resolver(entry))||
           !same_phones(word,n,phonemes+native[i].open,native[i].end-native[i].open)))
            facts[i]=(struct reloq61_german_word_facts){0};
        if(put(&output,word,n))goto memory;
        if(named_pair&&i>=count-2)
            for(size_t k=start;k+1<start+n;++k)
                if(output.p[k]=='.'&&output.p[k+1]=='0'){output.p[k+1]='1';break;}
        if(units[i].fixed_phrase>=RELOQ_DE_SOURCE_SYMBOL&&units[i].length==1&&
           (units[i].text[0]=='+'||units[i].text[0]==0xb1))
            for(size_t k=start;k<start+n;++k)if(output.p[k]=='u')output.p[k]='U';
        if(!units[i].preserve && !units[i].fixed_phrase) {
            reloq_german_morphology_nominal_stress(lexicon,units[i].text,units[i].length,
                output.p+start,n);
            reloq_german_morphology_repair_stem(units[i].text,units[i].length,
                output.p+start,n);
            /* This authored departure belongs after native word analysis,
               just like the requested nominal prefix stress. */
            int plus=units[i].length==4;
            for(size_t k=0;plus&&k<4;++k) {
                unsigned char ch=units[i].text[k];if(ch>='A'&&ch<='Z')ch+=32;
                if(ch!=(unsigned char)"plus"[k])plus=0;
            }
            if(plus)for(size_t k=start;k<start+n;++k)if(output.p[k]=='u')output.p[k]='U';
        }
        if(force==1 && native[i].syllables==1)
            for(size_t k=start;k+1<output.n;++k)
                if(output.p[k]=='.' && output.p[k+1]>='0' && output.p[k+1]<='2')output.p[k+1]='0';
        before=native[i].end;
    }
    if(boundary(&output,annotation+old[count-1].end,length-old[count-1].end,
        phonemes+before,r.phonemes_length-before))goto memory;
    if(prefix(&output,phonemes+before,r.phonemes_length-before,0))goto memory;
    if(retain_voice_controls(&output,annotation+old[count-1].end,length-old[count-1].end))goto memory;
    /* A phoneme callback terminator is not part of ordinary text output. */
    while(output.n && output.p[output.n-1]==' ')--output.n;
    output.p[output.n]=0;
    size_t leading=0;while(leading<output.n&&output.p[leading]==' ')++leading;
    memmove(output.p,output.p+leading,output.n-leading+1);output.n-=leading;
    *out=output.p;*written=output.n;output.p=NULL;result=1;goto done;
memory:result=-1;
done:free(phrases);free(old);free(input.p);free(output.p);free(spoken.p);return result;
}

int reloq_rules61_german_context(const struct reloq_german_prosody_unit *units,size_t count,
    const struct reloq_lexicon *lexicon,const unsigned char *annotation,size_t length,
    unsigned char **out,size_t *written,int (*cancelled)(void *),void *context)
{
    return reloq_rules61_german_context_features(units,count,lexicon,annotation,length,
        out,written,NULL,cancelled,context,NULL);
}
