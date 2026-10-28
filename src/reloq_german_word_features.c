#include "reloq_german_word_features.h"
#include "reloq_german_spr.h"
#include "reloq_german_annotation.h"
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

struct lexical_features {
    const char *word, *spr;
    uint64_t glottal_syllables;
    unsigned syllable_count;
};
static const struct lexical_features entries[]={
#include "reloq_german_glottal_lexicon.inc"
};
static const struct {const char *word,*spr;uint64_t full;} quantities[]={
#include "reloq_german_lexical_quantity.inc"
    };
/* Fixed indexes order the existing language data, never request results. */
struct spr_key { uint16_t entry,length; };
struct number_shape { const char *body; uint16_t length; uint8_t modes; };
#include "reloq_german_feature_index.inc"
static int bytes_compare(const unsigned char *s,size_t n,const char *key,size_t length)
{
    size_t common=n<length?n:length;int order=memcmp(s,key,common);
    return order?order:n<length?-1:n>length?1:0;
}
static const char *spr_text(const struct spr_key *key,int quantity)
{return quantity?quantities[key->entry].spr:entries[key->entry].spr;}
static size_t spr_first(const unsigned char *s,size_t n,
    const struct spr_key *keys,size_t count,int quantity)
{
    size_t lo=0,hi=count;
    while(lo<hi) {
        size_t mid=lo+(hi-lo)/2;
        if(bytes_compare(s,n,spr_text(keys+mid,quantity),keys[mid].length)>0)lo=mid+1;
        else hi=mid;
    }
    return lo;
}
static int cardinal_shape(const unsigned char *body,size_t length,unsigned mode)
{
    size_t lo=0,hi=sizeof cardinal_shapes/sizeof *cardinal_shapes;
    while(lo<hi) {
        size_t mid=lo+(hi-lo)/2;const struct number_shape *shape=cardinal_shapes+mid;
        int order=bytes_compare(body,length,shape->body,shape->length);
        if(!order)return (shape->modes&mode)!=0;
        if(order>0)lo=mid+1;else hi=mid;
    }
    return 0;
}

static unsigned char lower(unsigned char c)
{
    return (c>='A' && c<='Z') || c==0xc4 || c==0xd6 || c==0xdc ?
        (unsigned char)(c+32) : c;
}
static int compare(const unsigned char *word,size_t length,const char *key)
{
    size_t i=0;
    for (;i<length && key[i];++i) {
        int d=(int)lower(word[i])-(unsigned char)key[i];
        if (d) return d;
    }
    return i<length?1:key[i]?-1:0;
}
static size_t first(const unsigned char *word,size_t length)
{
    size_t lo=0,hi=sizeof entries/sizeof *entries;
    while (lo<hi) {
        size_t mid=lo+(hi-lo)/2;
        int c=compare(word,length,entries[mid].word);
        if (c>0) lo=mid+1;
        else hi=mid;
    }
    return lo;
}

static int phone(const unsigned char *spr,size_t length,
    struct reloq_german_spr_cursor *cursor,struct reloq_german_spr_event *event)
{
    int status;
    while ((status=reloq_german_spr_next(spr,length,cursor,event))==1)
        if (event->kind==RELOQ_DE_SPR_PHONE) return 1;
    return status;
}

static int full_vowel(enum reloq_german_phone p)
{
    return (p>=RELOQ_DE_PHONE_a && p<=RELOQ_DE_PHONE_Y) ||
        (p>=RELOQ_DE_PHONE_Q_E_COLON && p<=RELOQ_DE_PHONE_Q_oe_TILDE);
}

int reloq_german_word_features_root_hiatus(const unsigned char *spr,size_t length,
    struct reloq_german_word_features *out)
{
    if (!spr || !out) return -1;
    struct reloq_german_spr_cursor cursor={0};
    struct reloq_german_spr_event e;
    struct reloq_german_word_features f={.glottal_syllables=UINT64_C(1)};
    enum reloq_german_phone previous=RELOQ_DE_PHONE_NONE;
    size_t previous_syllable=0;
    int status,hiatus=0;
    while ((status=reloq_german_spr_next(spr,length,&cursor,&e))==1) {
        if (e.syllable>=64) return 0;
        if (e.kind==RELOQ_DE_SPR_STRESS && e.stress)
            f.glottal_syllables|=UINT64_C(1)<<e.syllable;
        if (e.kind!=RELOQ_DE_SPR_PHONE) continue;
        if (e.syllable!=previous_syllable && full_vowel(previous) && full_vowel(e.phone)) {
            /* Stress alone does not establish a lexical boundary. Loan
             * roots can retain secondary stress inside a continuous hiatus
             * (Download). Observed readings and real compound heads are
             * resolved separately, before/after this conservative default. */
            f.glottal_syllables&=~(UINT64_C(1)<<e.syllable);
            hiatus=1;
        }
        f.syllable_count=(unsigned)e.syllable+1;
        previous=e.phone;previous_syllable=e.syllable;
    }
    if (status || !hiatus) return 0;
    *out=f;
    return 1;
}

int reloq_german_word_features_final_a(const unsigned char *spr,size_t length,
    struct reloq_german_word_features *inout)
{
    if (!spr || !inout) return -1;
    struct reloq_german_spr_cursor cursor={0};
    struct reloq_german_spr_event e,last={0};
    struct reloq_german_word_features f=*inout;
    const unsigned known=f.syllable_count;
    int status;
    while ((status=reloq_german_spr_next(spr,length,&cursor,&e))==1) {
        if (e.syllable>=64) return 0;
        if (e.kind==RELOQ_DE_SPR_STRESS && e.syllable>=known &&
            (!e.syllable || e.stress)) f.glottal_syllables|=UINT64_C(1)<<e.syllable;
        if (e.kind==RELOQ_DE_SPR_PHONE) last=e;
    }
    if (status || last.phone!=RELOQ_DE_PHONE_A || last.stress || !last.syllable) return 0;
    f.syllable_count=(unsigned)last.syllable+1;
    f.full_a_syllables=UINT64_C(1)<<last.syllable;
    *inout=f;return 1;
}

/* A suffix can move the stem's last consonant into a new syllable. Only
 * inherit onset metadata when every stem phone and vowel syllable still
 * matches; changed pronunciations and guesses about a root are rejected. */
static int unchanged_stem(const char *stem,const unsigned char *spr,size_t n)
{
    struct reloq_german_spr_cursor a={0},b={0};
    struct reloq_german_spr_event x,y;
    size_t length=strlen(stem);
    int state;
    while ((state=phone((const unsigned char *)stem,length,&a,&x))==1) {
        if (phone(spr,n,&b,&y)!=1 || x.phone!=y.phone) return 0;
        int vowel=(x.phone>=RELOQ_DE_PHONE_a && x.phone<=RELOQ_DE_PHONE_AT) ||
            x.phone>=RELOQ_DE_PHONE_Q_E_COLON;
        if (vowel && x.syllable!=y.syllable) return 0;
    }
    return state==0;
}

int reloq_german_word_features_lookup(const unsigned char *word,size_t length,
    const unsigned char *spr,size_t spr_length,
    struct reloq_german_word_features *out)
{
    if ((!word && length) || !spr || !out) return -1;
    size_t lo=first(word,length);
    for (;lo<sizeof entries/sizeof *entries && !compare(word,length,entries[lo].word);++lo) {
        const struct lexical_features *e=entries+lo;
        if (strlen(e->spr)!=spr_length || memcmp(e->spr,spr,spr_length)) continue;
        *out=(struct reloq_german_word_features){.glottal_syllables=e->glottal_syllables,.syllable_count=e->syllable_count};
        return 1;
    }
    static const char *const endings[]={"e","n","s","t","en","er","es","te",
        "ten","ter","tes","est","et","in","ung","ungen","isch","ische",
        "ischen","ischer","isches","lich","liche","lichen","licher","liches"};
    for (size_t j=0;j<sizeof endings/sizeof *endings;++j) {
        size_t suffix=strlen(endings[j]);
        if (length<=suffix+2 || compare(word+length-suffix,suffix,endings[j])) continue;
        size_t root=length-suffix;
        for (lo=first(word,root);lo<sizeof entries/sizeof *entries &&
            !compare(word,root,entries[lo].word);++lo) {
            const struct lexical_features *e=entries+lo;
            if (!unchanged_stem(e->spr,spr,spr_length)) continue;
            *out=(struct reloq_german_word_features){.glottal_syllables=e->glottal_syllables,.syllable_count=e->syllable_count};
            return 1;
        }
    }
    return 0;
}

int reloq_german_word_features_quantity(const unsigned char *word,size_t length,
    const unsigned char *spr,size_t spr_length,struct reloq_german_word_features *inout)
{

    if ((!word && length) || !spr || !inout) return -1;
    size_t lo=0,hi=sizeof quantities/sizeof *quantities;
    while (lo<hi) {
        size_t mid=lo+(hi-lo)/2;
        if (compare(word,length,quantities[mid].word)>0)lo=mid+1;
        else hi=mid;
    }
    if (lo==sizeof quantities/sizeof *quantities || compare(word,length,quantities[lo].word) ||
        strlen(quantities[lo].spr)!=spr_length || memcmp(quantities[lo].spr,spr,spr_length)) return 0;
    struct reloq_german_spr_cursor cursor={0};
    struct reloq_german_spr_event e;
    unsigned known=inout->syllable_count;int status;
    while ((status=reloq_german_spr_next(spr,spr_length,&cursor,&e))==1) {
        if (e.syllable>=64) return 0;
        if (e.kind==RELOQ_DE_SPR_STRESS) {
            if (e.syllable>=known && (!e.syllable || e.stress))
                inout->glottal_syllables|=UINT64_C(1)<<e.syllable;
            inout->syllable_count=(unsigned)e.syllable+1;
        }
    }
    if (status) return -1;
    inout->full_quantity_syllables=quantities[lo].full;
    return 1;
}

static uint64_t mask_for(unsigned n)
{return n>=64?UINT64_MAX:(UINT64_C(1)<<n)-1;}

static int default_features(const unsigned char *spr,size_t n,
    struct reloq_german_word_features *out,int *explicit_header)
{
    struct reloq_german_spr_cursor c={0};
    struct reloq_german_spr_event e;
    struct reloq_german_word_features f={0},header={0};
    int rc,has=0;
    while((rc=reloq_german_spr_next(spr,n,&c,&e))==1) {
        if(e.kind==RELOQ_DE_SPR_FEATURES) {
            has=1;header=(struct reloq_german_word_features){
                .syllable_count=e.feature_syllables,.glottal_syllables=e.glottal_syllables,
                .full_quantity_syllables=e.full_quantity_syllables,
                .full_a_syllables=e.full_a_syllables};
        }
        if(e.kind==RELOQ_DE_SPR_SYLLABLE) {
            if(e.syllable>=64)continue;
            f.syllable_count=(unsigned)e.syllable+1;
            if(!e.syllable)f.glottal_syllables|=UINT64_C(1);
        }
        if(e.kind==RELOQ_DE_SPR_STRESS && e.stress && e.syllable<64)
            f.glottal_syllables|=UINT64_C(1)<<e.syllable;
    }
    if(rc || (has && header.syllable_count!=f.syllable_count))return -1;
    if(explicit_header)*explicit_header=has;
    *out=has?header:f;return 1;
}

/* Reuse the measured cardinal syllables to recognize numeric groups.
 * No complete text word is introduced as a pronunciation exception. */
static int numeric_features(const unsigned char *spr,size_t n,
    struct reloq_german_word_features *out)
{
    if(n<5 || spr[0]!='`' || spr[1]!='[' || spr[n-1]!=']')return 0;
    const size_t body=n-3;
    int found=cardinal_shape(spr+2,body,1);
    static const struct {const char *text;size_t length;unsigned mode;} tails[]={
        {".2hUn.0dRt",sizeof ".2hUn.0dRt"-1,2},
        {".2t'aw'.0z@nt",sizeof ".2t'aw'.0z@nt"-1,4}
    };
    for(size_t j=0;j<sizeof tails/sizeof *tails && !found;++j)
        if(body>=tails[j].length &&
           !memcmp(spr+2+body-tails[j].length,tails[j].text,tails[j].length))
            found=cardinal_shape(spr+2,body-tails[j].length,tails[j].mode);
    if(!found)return 0;
    unsigned syll=0;uint64_t mask=1;
    for(size_t k=2;k+1<n;++k)if(spr[k]=='.') {
        if(syll>=64)return 0;
        if(k+9<n && !memcmp(spr+k+2,"Ax.0TIX",7))mask|=UINT64_C(1)<<syll;
        if(k+5<n && !memcmp(spr+k+2,"Unt",3) &&
           ((k>=4&&!memcmp(spr+k-4,"fYnf",4)) ||
            (k>=4&&!memcmp(spr+k-4,"zEks",4)) ||
            (k>=3&&!memcmp(spr+k-3,"Axt",3))))mask|=UINT64_C(1)<<syll;
        ++syll;
    }
    out->glottal_syllables=mask;out->syllable_count=syll;return 1;
}

int reloq_german_word_features_native_onsets(const unsigned char *spr,size_t n,
    uint64_t mask,unsigned syllables,struct reloq_german_word_features *out)
{
    struct reloq_german_word_features shape={0};int explicit_header=0;
    if(!out||!syllables||syllables>64||
       default_features(spr,n,&shape,&explicit_header)!=1||explicit_header||
       shape.syllable_count!=syllables)return 0;
    if(!out->syllable_count)*out=shape;
    out->syllable_count=syllables;
    out->glottal_syllables=mask;
    (void)numeric_features(spr,n,out);return 1;
}

int reloq_german_word_features_from_spr(const unsigned char *spr,size_t n,
    struct reloq_german_word_features *out)
{
    if(!spr || !out)return -1;
    struct reloq_german_word_features f={0};int explicit_header=0;
    int rc=default_features(spr,n,&f,&explicit_header);
    if(rc!=1)return rc;
    if(explicit_header){*out=f;return 2;}
    int found=0,conflict=0;uint64_t glottal=0;
    const size_t lexical_count=sizeof lexical_spr_order/sizeof *lexical_spr_order;
    for(size_t k=spr_first(spr,n,lexical_spr_order,lexical_count,0);k<lexical_count;++k) {
        const struct spr_key *key=lexical_spr_order+k;
        if(key->length!=n || memcmp(entries[key->entry].spr,spr,n))break;
        uint64_t mask=entries[key->entry].glottal_syllables;
        if(found && glottal!=mask)conflict=1;
        glottal=mask;found=1;
    }
    if(found && !conflict)f.glottal_syllables=glottal;
    (void)numeric_features(spr,n,&f);
    const size_t quantity_count=sizeof quantity_spr_order/sizeof *quantity_spr_order;
    for(size_t k=spr_first(spr,n,quantity_spr_order,quantity_count,1);k<quantity_count;++k) {
        const struct spr_key *key=quantity_spr_order+k;
        if(key->length!=n || memcmp(quantities[key->entry].spr,spr,n))break;
        f.full_quantity_syllables|=quantities[key->entry].full;
    }
    (void)reloq_german_word_features_final_a(spr,n,&f);
    *out=f;return 1;
}

int reloq_german_word_features_resolve(const unsigned char *s,size_t n,
    const struct reloq_german_word_features *known,size_t known_count,
    struct reloq_german_word_features **out,size_t *count,
    int (*cancelled)(void *),void *context)
{
    if(!s || !out || !count || (known_count&&!known))return -1;
    for(size_t j=0;j<known_count;++j)
        if(known[j].syllable_count>64 || (j&&known[j-1].word_index>=known[j].word_index))return -1;
    struct reloq_german_annotation_cursor cursor={0};
    struct reloq_german_annotation_event e;
    struct reloq_german_word_features *rows=NULL;
    size_t used=0,cap=0,start=0,word=0,k=0;int rc;
    while((rc=reloq_german_annotation_next(s,n,&cursor,UINT32_MAX,&e))==1) {
        if(cancelled&&cancelled(context)){free(rows);return 1;}
        if(e.kind!=RELOQ_DE_ANNOTATION_SPR)continue;
        if(e.spr.kind==RELOQ_DE_SPR_WORD_OPEN)start=e.offset;
        if(e.spr.kind!=RELOQ_DE_SPR_WORD_CLOSE)continue;
        struct reloq_german_word_features f={0};
        int state=reloq_german_word_features_from_spr(s+start,e.offset+e.length-start,&f);
        if(state<0){free(rows);return -1;}
        while(k<known_count && known[k].word_index<word)++k;
        if(state && state!=2 && k<known_count && known[k].word_index==word) {
            const struct reloq_german_word_features *old=known+k;
            uint64_t bits=mask_for(old->syllable_count);
            f.glottal_syllables=(f.glottal_syllables&~bits)|(old->glottal_syllables&bits);
            f.full_quantity_syllables=old->full_quantity_syllables;
            f.full_a_syllables|=old->full_a_syllables;
        }
        if(state) {
            if(used==cap) {
                size_t next=cap?2*cap:16;
                if(next<cap || next>SIZE_MAX/sizeof *rows){free(rows);return -2;}
                void *p=realloc(rows,next*sizeof *rows);
                if(!p){free(rows);return -2;}rows=p;cap=next;
            }
            f.word_index=word;rows[used++]=f;
        }
        ++word;
    }
    if(rc){free(rows);return -1;}
    *out=rows;*count=used;return 0;
}

int reloq_german_word_features_annotate(const unsigned char *s,size_t n,
    const struct reloq_german_word_features *features,size_t feature_count,
    unsigned char **out,size_t *written)
{
    if(!s || !out || !written || n==SIZE_MAX)return -1;
    struct reloq_german_word_features *all=NULL;size_t count=0;
    int rc=reloq_german_word_features_resolve(s,n,features,feature_count,&all,&count,NULL,NULL);
    if(rc)return rc;
    if(count>(SIZE_MAX-n-1)/80){free(all);return -2;}
    unsigned char *result=malloc(n+count*80+1);
    if(!result){free(all);return -2;}
    struct reloq_german_annotation_cursor cursor={0};
    struct reloq_german_annotation_event e;
    size_t offset=0,used=0,word=0,k=0;
    while((rc=reloq_german_annotation_next(s,n,&cursor,UINT32_MAX,&e))==1) {
        if(e.kind!=RELOQ_DE_ANNOTATION_SPR)continue;
        if(e.spr.kind==RELOQ_DE_SPR_WORD_OPEN) {
            size_t end=e.offset+e.length;
            memcpy(result+used,s+offset,end-offset);used+=end-offset;offset=end;
            while(k<count&&all[k].word_index<word)++k;
            if(k<count&&all[k].word_index==word) {
                const struct reloq_german_word_features *f=all+k;
                int size=snprintf((char*)result+used,80,"!%u,%" PRIx64 ",%" PRIx64 ",%" PRIx64 ";",
                    f->syllable_count,f->glottal_syllables,f->full_quantity_syllables,f->full_a_syllables);
                if(size<0||size>=80){rc=-1;break;}used+=(size_t)size;
            }
            ++word;
        } else if(e.spr.kind==RELOQ_DE_SPR_FEATURES)offset=e.offset+e.length;
    }
    free(all);
    if(rc){free(result);return -1;}
    memcpy(result+used,s+offset,n-offset);used+=n-offset;result[used]=0;
    *out=result;*written=used;return 0;
}
