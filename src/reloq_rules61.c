#include "reloq_rules61.h"
#include "reloq61.h"
#include "reloq_frame_adapter.h"
#include "reloq_german_input.h"
#include "reloq_german_voice_model.h"
#include "reloq_german_lexicon.h"
#include "reloq_german_spr.h"
#include "reloq_inline_tags.h"
#include "reloq_character.h"
#include "reloq_cp1252.h"
#include "reloq_utf8.h"
#include "reloq_voice_controls.h"
#include "reloq_voice_filter.h"
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "reloq_inline_word_controls.h"

static int dash_space(uint32_t c)
{return c==' '||c=='\t'||c=='\r'||c=='\n'||c==0xa0;}
static int dash_word_edge(uint32_t c)
{
    if(c<128)return (c>='A'&&c<='Z')||(c>='a'&&c<='z')||(c>='0'&&c<='9')||
        c=='_'||c=='\''||c=='"'||c=='('||c==')'||c=='['||c==']';
    /* Avoid classifying Unicode punctuation or symbols as word material. */
    return (c>=0xc0&&c<0x2000)||(c>=0x2c00&&c<=0xd7ff)||
        c==0x2018||c==0x2019||c==0x201c||c==0x201d;
}
static int dash_between_words(const unsigned char *text,size_t length,size_t begin,size_t end)
{
    uint32_t left=0,right=0;size_t p=begin;
    while(p){size_t at=p-1;while(at&&(text[at]&0xc0)==0x80)--at;
        size_t next=at;if(reloq_utf8_next(text,p,&next,&left)!=1||next!=p)return 0;
        p=at;
        if(!dash_space(left))break;
        left=0;}
    p=end;
    while(p<length){if(reloq_utf8_next(text,length,&p,&right)!=1)return 0;
        if(!dash_space(right))break;
        right=0;}
    if(!dash_word_edge(left)||!dash_word_edge(right))return 0;
    /* Keep the native handling of numeric ranges such as 2–4. */
    return !(left>='0'&&left<='9'&&right>='0'&&right<='9');
}
/* A dash before whitespace and a number in prose is a sentence
 * separator. Preserve numeric subtraction, a leading minus and -1. */
static int dash_before_number(const unsigned char *text,size_t length,size_t begin,size_t end)
{
    uint32_t right=0,left=0;size_t p=end;
    if(p>=length||reloq_utf8_next(text,length,&p,&right)!=1||!dash_space(right))return 0;
    while(p<length) {
        if(reloq_utf8_next(text,length,&p,&right)!=1)return 0;
        if(!dash_space(right))break;
    }
    if(right<'0'||right>'9')return 0;
    p=begin;
    if(!p)return 0;
    while(p) {
        size_t at=p-1;while(at&&(text[at]&0xc0)==0x80)--at;
        size_t next=at;if(reloq_utf8_next(text,p,&next,&left)!=1||next!=p)return 0;
        p=at;
        if(!dash_space(left))break;
        left=0;
    }
    if((left>='0'&&left<='9')||left=='('||left=='[')return 0;
    return dash_word_edge(left)||left=='.'||left==','||left==':'||left==';'||left=='!'||left=='?';
}

/* The native English parser normally treats a lone letter and one following
 * space as a person's initial, dropping the period. Two spaces retain a
 * sentence boundary. Only apply this when the following token is a likely
 * sentence start; names, sequences of initials, domains and abbreviations
 * remain in the native parser's care. */
static int english_initial_sentence_space(const unsigned char *text,size_t n,size_t at)
{
    if(at<2||at+1>=n||text[at]!=' '||text[at-1]!='.')return 0;
    unsigned char letter=text[at-2];
    if(letter<'A'||letter>'Z')return 0;
    size_t start=at-2;
    if(start&&text[start-1]!=' '&&text[start-1]!='\t'&&text[start-1]!='\n')return 0;
    size_t previous=start;
    while(previous&&(text[previous-1]==' '||text[previous-1]=='\t'||text[previous-1]=='\n'))--previous;
    if(previous&&text[previous-1]=='.')return 0;
    size_t next=at+1,end=next;
    if(text[next]<'A'||text[next]>'Z')return 0;
    while(end<n&&((text[end]>='A'&&text[end]<='Z')||(text[end]>='a'&&text[end]<='z')))++end;
    if(end==next || (end<n&&text[end]=='.'&&end-next==1))return 0;
    int acronym=end-next>=2;
    for(size_t i=next;i<end;++i)if(text[i]<'A'||text[i]>'Z')acronym=0;
    if(acronym)return 1;
    static const char *const starters[]={"A","After","Before","For","He","Here","However",
        "I","In","It","Next","Now","Please","She","That","The","These","They",
        "This","Those","We","You"};
    for(size_t i=0;i<sizeof starters/sizeof *starters;++i)
        if(strlen(starters[i])==end-next&&!memcmp(text+next,starters[i],end-next))return 1;
    return 0;
}

/* Preserve admitted text until the native sentence frontend sees it. The
 * former word-by-word SPR conversion ran a second, older language analysis
 * and forced every ordinary token through its historical dictionaries. */
static int english_input(const unsigned char *text,size_t length,int tags,int unicode,int english,
    unsigned char **out,size_t **source,size_t *written)
{
    if(length==SIZE_MAX||memchr(text,0,length))return -1;
    unsigned char *bytes=NULL;size_t *map=NULL,n=0,capacity=0;int spelling=0;
    for(size_t at=0;at<length;) {
        size_t begin=at,end=at;uint32_t cp;
        if(reloq_utf8_next(text,length,&end,&cp)!=1)goto invalid;
        unsigned char encoded[8];size_t count=0;
        if(cp<128) {
            if(cp=='`') {
                struct reloq_inline_tag t=reloq_inline_tag_read(text+at,length-at,0);
                if(t.length&&(tags||t.kind==RELOQ_TAG_SPR||t.kind==RELOQ_TAG_PROMINENCE)) {
                    count=t.length;end=at+count;
                    if(t.kind==RELOQ_TAG_SPELL)spelling=t.value;
                }
            }
            if(!count){
                /* Never let an invalid VWPM control reach the native
                 * divisor. Unknown text remains available to be spoken. */
                int invalid_wpm=cp=='`'&&tags&&end+4<=length&&
                    !memcmp(text+end,"vwpm",4);
                encoded[0]=cp=='`'&&(!tags||invalid_wpm)?' ':
                    cp=='-'&&!spelling&&dash_before_number(text,length,at,end)?',':(unsigned char)cp;
                count=1;
                if(english&&!spelling&&cp==' '&&english_initial_sentence_space(text,length,at))
                    encoded[count++]=' ';
            }
        } else if(cp==0x2013&&!spelling&&dash_between_words(text,length,at,end)) {
            /* The native long-dash rule supplies the pause, including the
             * agreed short-dash behavior, without inserting a spoken word. */
            if(unicode){encoded[0]=0xe2;encoded[1]=0x80;encoded[2]=0x94;count=3;}
            else {encoded[0]=0x97;count=1;}
        } else if(unicode) {
            count=end-at;memcpy(encoded,text+at,count);
        } else if(reloq_german_input_encode(text+at,end-at,encoded,sizeof encoded,&count,NULL)) {
            unsigned char base;
            if(reloq_character_latin_base(cp,&base)!=1)goto invalid;
            encoded[0]=base;count=1;
        }
        if(count>SIZE_MAX-n-1)goto memory;
        if(n+count+1>capacity) {
            size_t cap=capacity?capacity:256;
            while(cap<n+count+1){if(cap>SIZE_MAX/2){cap=n+count+1;break;}cap*=2;}
            if(cap>SIZE_MAX/sizeof *map)goto memory;
            void *p=realloc(bytes,cap);if(!p)goto memory;bytes=p;
            p=realloc(map,cap*sizeof *map);if(!p)goto memory;map=p;capacity=cap;
        }
        const unsigned char *part=cp=='`'&&count>1?text+at:encoded;
        memcpy(bytes+n,part,count);
        for(size_t k=0;k<count;++k)map[n+k]=begin;
        n+=count;at=end;
    }
    if(!bytes) {
        bytes=malloc(1);map=malloc(sizeof *map);
        if(!bytes||!map)goto memory;
    }
    bytes[n]=0;map[n]=length;*out=bytes;*source=map;*written=n;return 0;
memory:free(bytes);free(map);return -2;
invalid:free(bytes);free(map);return -1;
}

/* The shared frontend consumes whole sentences. Remove only presentation
 * controls and discretionary layout breaks before that sentence reaches
 * the native German grammar. Source offsets remain attached to each byte. */
static int german_letter(unsigned char c)
{
    return (c>='a'&&c<='z')||(c>='A'&&c<='Z')||
        (c>=0xc0&&c!=0xd7&&c!=0xf7)||c==0x8c||c==0x9c;
}
static int german_word_byte(unsigned char c)
{return german_letter(c)||(c>='0'&&c<='9')||c=='_'||c=='\'';}

/* German low quotes and guillemets can disrupt native compound and question
 * analysis. Normalize matched quoted passages containing a written hyphenated
 * word or a question mark; leave isolated and explicitly spelled symbols alone. */
static int german_quote_close(unsigned char open,unsigned char close)
{
    switch(open) {
    case 0x84: return close==0x93||close==0x94;
    case 0x93: return close==0x94||close==0x93;
    case 0xab: return close==0xbb;
    case 0xbb: return close==0xab;
    case 0x82: return close==0x91||close==0x92;
    case 0x91: return close==0x92||close==0x91;
    case '\'': return close=='\'';
    default: return 0;
    }
}
static int german_compound_alnum(unsigned char c)
{return german_letter(c)||(c>='0'&&c<='9')||c=='_';}
static void german_normalize_quoted_text(unsigned char *a,size_t n,size_t i)
{
    unsigned char opening=a[i];
    if(opening!=0x84&&opening!=0x93&&opening!=0xab&&opening!=0xbb&&
       opening!=0x82&&opening!=0x91&&opening!='\'')return;
    if(i+1>=n || (i&&(a[i-1]=='\\'||german_compound_alnum(a[i-1]))))return;
    size_t end=i+1,limit=n-i>4096?i+4096:n;int hyphen=0,question=0;
    while(end<limit) {
        unsigned char c=a[end];
        if(german_quote_close(opening,c)) {
            if(hyphen||question)a[i]=a[end]='"';
            return;
        }
        if(c=='\n'||c=='\r')return;
        if(c=='?')question=1;
        if(c=='-'&&end>i+1&&end+1<n&&
           german_compound_alnum(a[end-1])&&german_compound_alnum(a[end+1]))hyphen=1;
        ++end;
    }
}

static void german_compact_input(unsigned char *a,size_t *map,size_t *length,int tags)
{
    const size_t n=*length;size_t out=0;int spelling=0,in_spr=0;
    struct reloq61_text_options options=reloq61_text_options_current();
    for(size_t i=0;i<n;) {
        struct reloq_inline_tag t=tags?reloq_inline_tag_read(a+i,n-i,0):(struct reloq_inline_tag){0};
        if(!tags&&!in_spr&&a[i]=='`'&&i+1<n&&a[i+1]=='[')in_spr=1;
        if(!spelling&&!in_spr&&!t.length)german_normalize_quoted_text(a,n,i);
        int end_spr=in_spr&&a[i]==']';
        size_t count=t.length?t.length:1;int omit=0;
        if(t.kind==RELOQ_TAG_SPELL)spelling=t.value;
        if(t.kind==RELOQ_TAG_DICTIONARY) {
            int next=!t.value;omit=next==options.dictionary_disabled;options.dictionary_disabled=next;
        } else if(t.kind==RELOQ_TAG_PREDICTION) {
            omit=t.value==options.phrase_prediction;options.phrase_prediction=t.value;
        } else if(t.kind==RELOQ_TAG_PAUSE && !t.value)omit=1;
        if(omit) {
            if((t.kind==RELOQ_TAG_DICTIONARY||t.kind==RELOQ_TAG_PREDICTION) &&
               i+count<n && a[i+count]==' ')++count;
            if(i+count<n && strchr(",.:;?!)",a[i+count]))
                while(out&&(a[out-1]==' '||a[out-1]=='\t'))--out;
        } else if(!spelling && !t.length && a[i]==0xad && out && german_letter(a[out-1])) {
            size_t end=i;while(end<n&&a[end]==0xad)++end;
            if(end<n&&german_letter(a[end])){i=end;continue;}
            memmove(a+out,a+i,count);memmove(map+out,map+i,count*sizeof *map);out+=count;
        } else {
            memmove(a+out,a+i,count);memmove(map+out,map+i,count*sizeof *map);out+=count;
        }
        i+=count;if(end_spr)in_spr=0;
    }
    map[out]=map[n];a[out]=0;*length=out;
}

struct german_shared_buffer {unsigned char *bytes;size_t *map,used,capacity;};
extern const struct reloq_lexicon_override *reloq61_dictionary_long_override(
    const unsigned char *,size_t);
static int german_shared_append(struct german_shared_buffer *b,const unsigned char *bytes,
    const size_t *map,size_t count,size_t origin)
{
    if(count>SIZE_MAX-b->used-1)return -1;
    size_t needed=b->used+count+1;
    if(needed>b->capacity) {
        size_t cap=b->capacity?b->capacity:256;
        while(cap<needed){if(cap>SIZE_MAX/2){cap=needed;break;}cap*=2;}
        if(cap>SIZE_MAX/sizeof *b->map)return -1;
        void *p=realloc(b->bytes,cap);if(!p)return -1;b->bytes=p;
        p=realloc(b->map,cap*sizeof *b->map);if(!p)return -1;b->map=p;b->capacity=cap;
    }
    memcpy(b->bytes+b->used,bytes,count);
    for(size_t j=0;j<count;++j)b->map[b->used+j]=map?map[j]:origin;
    b->used+=count;return 0;
}

/* Main text aliases are resolved once before grammar, including components
 * of a hyphenated word. Root and phonetic Main entries remain native. */
static int german_main_input(unsigned char **text,size_t **source,size_t *length,int tags)
{
    const unsigned char *a=*text;const size_t *map=*source,n=*length;
    struct german_shared_buffer b={0};int spelling=0,changed=0;
    for(size_t i=0;i<n;) {
        struct reloq_inline_tag t=reloq_inline_tag_read(a+i,n-i,0);
        if(t.length&&(tags||t.kind==RELOQ_TAG_SPR)) {
            if(t.kind==RELOQ_TAG_SPELL)spelling=t.value;
            if(t.kind==RELOQ_TAG_SPELL&&(!b.used||b.bytes[b.used-1]!=' ')) {
                if(german_shared_append(&b,(const unsigned char *)" ",NULL,1,map[i]))goto fail;
                changed=1;
            }
            if(german_shared_append(&b,a+i,map+i,t.length,0))goto fail;
            if(t.kind==RELOQ_TAG_SPELL&&(i+t.length==n||a[i+t.length]!=' ')) {
                if(german_shared_append(&b,(const unsigned char *)" ",NULL,1,map[i+t.length]))goto fail;
                changed=1;
            }
            i+=t.length;continue;
        }
        if(spelling||!german_word_byte(a[i])) {
            if(german_shared_append(&b,a+i,map+i,1,0))goto fail;
            ++i;continue;
        }
        size_t end=i;
        while(end<n&&(german_word_byte(a[end])||
            (a[end]=='-'&&end+1<n&&german_word_byte(a[end+1]))))++end;
        while(i<end) {
            size_t stop=end;const char *value=reloq61_dictionary_main_text(a+i,stop-i);
            if(!value){stop=i;while(stop<end&&a[stop]!='-')++stop;
                value=reloq61_dictionary_main_text(a+i,stop-i);}
            size_t value_length=value?strlen(value):0,utf8_length=0;
            unsigned char utf8[1537];
            if(value&&value_length<=512&&!reloq_cp1252_decode((const unsigned char *)value,
                    value_length,utf8,sizeof utf8,&utf8_length,NULL)) {
                if(german_shared_append(&b,(const unsigned char *)value,NULL,value_length,map[i]))goto fail;
                changed=1;
            } else if(german_shared_append(&b,a+i,map+i,stop-i,0))goto fail;
            i=stop;
            if(i<end){if(german_shared_append(&b,a+i,map+i,1,0))goto fail;++i;}
        }
    }
    if(!changed){free(b.bytes);free(b.map);return 0;}
    if(german_shared_append(&b,(const unsigned char *)"",NULL,0,n))goto fail;
    b.bytes[b.used]=0;b.map[b.used]=map[n];
    free(*text);free(*source);*text=b.bytes;*source=b.map;*length=b.used;return 0;
fail:free(b.bytes);free(b.map);return -1;
}

extern const struct reloq_lexicon_resolver *reloq61_dictionary_resolver(void);
extern void reloq61_dictionary_resolver_release(const struct reloq_lexicon_resolver *);
extern const struct reloq_lexicon *reloq61_dictionary_overlay_current(void);

/* Compare the complete native pronunciation with the resolved compound,
 * even when an initialism makes it span several SPR groups. Compound stress
 * is chosen by the native sentence planner, so compare phones rather than
 * copying the isolated token's stress or its phrase marks into the sentence. */
struct compound_phone_cursor {
    const unsigned char *text;
    size_t length,scan,begin,end;
    struct reloq_german_spr_cursor group;
    int inside;
};
static int compound_phone_next(struct compound_phone_cursor *c,enum reloq_german_phone *phone)
{
    for(;;) {
        if(c->inside) {
            struct reloq_german_spr_event event;
            int rc=reloq_german_spr_next(c->text+c->begin,c->end-c->begin,&c->group,&event);
            if(rc<0)return -1;
            if(!rc){c->inside=0;c->scan=c->end;continue;}
            if(event.kind==RELOQ_DE_SPR_PHONE){*phone=event.phone;return 1;}
            continue;
        }
        while(c->scan+1<c->length&&!(c->text[c->scan]=='`'&&c->text[c->scan+1]=='['))++c->scan;
        if(c->scan+1>=c->length)return 0;
        c->begin=c->scan;
        const unsigned char *close=memchr(c->text+c->begin+2,']',c->length-c->begin-2);
        if(!close)return -1;
        c->end=(size_t)(close-c->text)+1;
        memset(&c->group,0,sizeof c->group);c->inside=1;
    }
}
static int german_native_compound_matches(const unsigned char *word,size_t length,
    const unsigned char *preferred,size_t preferred_length)
{
    if(!length||length>=256)return 0;
    char input[256],native[8192]={0};memcpy(input,word,length);input[length]=0;
    struct reloq61_request q={.language=0x40000,.annotations=1,.phonemes_only=1,
        .voice={0,50,65,30,0,0,50,100},.text=input,
        .phonemes=native,.phonemes_capacity=sizeof native};
    struct reloq61_result result={0};
    if(reloq61_generate(&q,&result)||result.error||result.unsupported||
       result.phonemes_length>=sizeof native)return 0;
    struct compound_phone_cursor a={.text=(const unsigned char *)native,.length=result.phonemes_length};
    struct compound_phone_cursor b={.text=preferred,.length=preferred_length};
    int count=0;
    for(;;) {
        enum reloq_german_phone x,y;
        int ax=compound_phone_next(&a,&x),by=compound_phone_next(&b,&y);
        if(ax<0||by<0||ax!=by)return 0;
        if(!ax)return count>0;
        if(x!=y)return 0;
        ++count;
    }
}

/* An initialism may already consist of several native words. If its
 * dictionary constituent still needs an explicit pronunciation, retain the
 * original components and replace just that constituent. A written hyphen
 * adjacent to an SPR group becomes a word separator, otherwise the lexer
 * would speak the hyphen as a separate punctuation symbol. */
static int german_append_compound_parts(struct german_shared_buffer *b,
    const unsigned char *a,const size_t *map,size_t begin,size_t end,
    const struct reloq_lexicon *overlay)
{
    struct part {size_t from,to;const struct reloq_lexicon_override *entry;} parts[128];
    size_t count=0;
    for(size_t at=begin;at<end;) {
        if(count==sizeof parts/sizeof *parts)return -1;
        size_t stop=at;while(stop<end&&a[stop]!='-')++stop;
        const struct reloq_lexicon_override *entry=NULL;
        (void)reloq_lexicon_find_override(overlay,a+at,stop-at,&entry);
        parts[count++]=(struct part){at,stop,entry};
        at=stop<end?stop+1:end;
    }
    for(size_t j=0;j<count;++j) {
        if(j) {
            unsigned char separator=parts[j-1].entry||parts[j].entry?' ':'-';
            if(german_shared_append(b,&separator,NULL,1,map[parts[j].from-1]))return -1;
        }
        if(parts[j].entry) {
            if(german_shared_append(b,parts[j].entry->spr,NULL,
                parts[j].entry->spr_length,map[parts[j].from]))return -1;
        } else if(german_shared_append(b,a+parts[j].from,map+parts[j].from,
                    parts[j].to-parts[j].from,0))return -1;
    }
    return 0;
}

/* Ask the native grammar for the phones of the compound with the caller's
 * explicit components substituted. This supplies only a comparison target:
 * actual surrounding sentence analysis always stays in the native path. */
static int german_native_compound_preferred(const unsigned char *text,const size_t *map,
    size_t begin,size_t end,const struct reloq_lexicon *overlay,
    unsigned char **out,size_t *out_length)
{
    struct german_shared_buffer input={0};
    if(german_append_compound_parts(&input,text,map,begin,end,overlay)) {
        free(input.bytes);free(input.map);return -2;
    }
    if(!input.bytes){free(input.map);return 1;}
    input.bytes[input.used]=0;
    unsigned char *phones=malloc(8192);
    if(!phones){free(input.bytes);free(input.map);return -2;}
    struct reloq61_request q={.language=0x40000,.annotations=1,.phonemes_only=1,
        .voice={0,50,65,30,0,0,50,100},.text=(char *)input.bytes,
        .phonemes=(char *)phones,.phonemes_capacity=8192};
    struct reloq61_result result={0};
    int failed=reloq61_generate(&q,&result)||result.error||result.unsupported||
        result.phonemes_length>=8192;
    free(input.bytes);free(input.map);
    if(failed){free(phones);return 1;}
    phones[result.phonemes_length]=0;
    /* If the original compound is one native lexical word, preserve that
     * shape when a component needed explicit replacement. Separate SPR
     * groups can otherwise change the word focus and index positions. */
    if(end-begin<256) {
        char original[256],native[8192]={0};
        memcpy(original,text+begin,end-begin);original[end-begin]=0;
        q.text=original;q.phonemes=native;q.phonemes_capacity=sizeof native;
        struct reloq61_result original_result={0};
        if(!reloq61_generate(&q,&original_result)&&!original_result.error&&
           !original_result.unsupported&&original_result.phonemes_length<sizeof native) {
            const char *first=strstr(native,"`[");
            const char *next=first?strstr(first+2,"`["):NULL;
            size_t candidate_groups=0;
            for(const char *p=(const char *)phones;(p=strstr(p,"`["))!=NULL;p+=2)
                ++candidate_groups;
            int merge=first&&(!next||candidate_groups==2);
            if(first&&!merge&&candidate_groups>2) {
                /* Main entries can make the raw grammar spell out the
                 * hyphens, so its group count is not a word count. A
                 * component that is itself a multiword initialism must
                 * stay separate; otherwise retain one compound group. */
                merge=1;
                for(size_t at=begin;at<end&&merge;) {
                    size_t stop=at;while(stop<end&&text[stop]!='-')++stop;
                    /* A later isolated letter is an initialism boundary,
                     * even though its own pronunciation has one group. */
                    if(at>begin&&stop-at==1&&german_letter(text[at])){merge=0;break;}
                    const struct reloq_lexicon_override *entry=NULL;
                    (void)reloq_lexicon_find_override(overlay,text+at,stop-at,&entry);
                    if(entry) {
                        size_t groups=0;
                        for(size_t k=0;k+1<entry->spr_length;++k)
                            if(entry->spr[k]=='`'&&entry->spr[k+1]=='[')++groups;
                        if(groups!=1)merge=0;
                    } else {
                        char part[256];size_t n=stop-at;
                        if(!n||n>=sizeof part){merge=0;break;}
                        memcpy(part,text+at,n);part[n]=0;
                        memset(native,0,sizeof native);q.text=part;
                        struct reloq61_result part_result={0};
                        if(reloq61_generate(&q,&part_result)||part_result.error||
                           part_result.unsupported||part_result.phonemes_length>=sizeof native) {
                            merge=0;break;
                        }
                        const char *one=strstr(native,"`[");
                        if(!one||strstr(one+2,"`["))merge=0;
                    }
                    at=stop<end?stop+1:end;
                }
            }
            if(merge) {
                unsigned char *merged=malloc(8192);size_t used=2,groups=0;
                if(!merged){free(phones);return -2;}
                merged[0]='`';merged[1]='[';
                const char *scan=(const char *)phones,*open;
                while((open=strstr(scan,"`["))!=NULL) {
                    const char *close=strchr(open+2,']');
                    if(!close||used+(size_t)(close-open)>=8192){groups=0;break;}
                    for(const char *p=open+2;p<close;++p) {
                        unsigned char c=(unsigned char)*p;
                        if(groups&&c=='1'&&p>open+2&&p[-1]=='.')c='2';
                        merged[used++]=c;
                    }
                    ++groups;scan=close+1;
                }
                if(groups>1) {
                    merged[used++]=']';merged[used]=0;
                    free(phones);phones=merged;result.phonemes_length=used;
                } else free(merged);
            }
        }
    }
    *out=phones;*out_length=result.phonemes_length;return 0;
}

/* A native compound can split a final dictionary constituent as an inflected
 * stem before Root lookup. When a supplied entry is missing from the complete
 * pronunciation, resolve just that lexical token with the existing compound
 * rules. The surrounding sentence remains in the native sentence planner. */
static int german_dictionary_compound_input(unsigned char **text,size_t **source,size_t *length,int tags)
{
    const struct reloq_lexicon_resolver *resolver=reloq61_dictionary_resolver();
    const struct reloq_lexicon *overlay=reloq61_dictionary_overlay_current();
    if(!resolver&&!overlay)return 0;
    struct reloq_lexicon empty={0};if(!overlay)overlay=&empty;
    const struct reloq_lexicon_resolver *previous=reloq_lexicon_resolver_scope(resolver);
    const unsigned char *a=*text;const size_t *map=*source,n=*length;
    struct german_shared_buffer b={0};int changed=0,spelling=0,status=0;
    struct reloq61_text_options options=reloq61_text_options_current();
    for(size_t i=0;i<n;) {
        struct reloq_inline_tag t=reloq_inline_tag_read(a+i,n-i,0);
        if(t.length&&(tags||t.kind==RELOQ_TAG_SPR||t.kind==RELOQ_TAG_PROMINENCE)) {
            if(tags&&t.kind==RELOQ_TAG_SPELL)spelling=t.value;
            if(tags&&t.kind==RELOQ_TAG_DICTIONARY)options.dictionary_disabled=!t.value;
            if(changed&&german_shared_append(&b,a+i,map+i,t.length,0)){status=-1;break;}
            i+=t.length;continue;
        }
        if(spelling||!german_word_byte(a[i])||(i&&german_word_byte(a[i-1]))) {
            if(changed&&german_shared_append(&b,a+i,map+i,1,0)){status=-1;break;}
            ++i;continue;
        }
        size_t end=i;while(end<n&&(german_word_byte(a[end])||
            (a[end]=='-'&&end+1<n&&german_word_byte(a[end+1]))))++end;
        int has_hyphen=memchr(a+i,'-',end-i)!=NULL,lexical=0;
        const struct reloq_lexicon_override *entry=NULL,*whole=NULL;
        /* The abbreviation switch does not disable Main/Root entries. */
        if(has_hyphen&&end-i<256)
            (void)reloq_lexicon_find_override(overlay,a+i,end-i,&whole);
        if(has_hyphen&&!whole&&end-i<256) {
            for(size_t at=i;at<end;) {
                size_t stop=at;while(stop<end&&a[stop]!='-')++stop;
                if(reloq_lexicon_find_override(overlay,a+at,stop-at,&entry)==1)lexical=1;
                at=stop<end?stop+1:end;
            }
        }
        unsigned char *preferred=NULL;size_t preferred_n=0;
        unsigned char native[4096];size_t native_n=0;
        if(lexical) {
            struct reloq61_text_options previous_options=reloq61_text_options_scope(options);
            int result=german_native_compound_preferred(a,map,i,end,overlay,&preferred,&preferred_n);
            reloq61_text_options_scope(previous_options);
            if(result==-2){free(preferred);status=-1;break;}
            if(result) {free(preferred);preferred=NULL;}
        }
        /* The native lexer splits at the hyphen before its Root lookup. A
         * complete explicit entry must therefore win before composition. */
        const unsigned char *pron=whole?whole->spr:NULL;
        size_t pron_n=whole?whole->spr_length:0;
        if(preferred) {
            /* If the real native pronunciation already contains the
             * supplied constituent phones, keep the original spelling for
             * sentence analysis. Replacing it with isolated SPR groups can
             * move the sentence focus onto an acronym in a longer compound. */
            struct reloq61_text_options previous_options=reloq61_text_options_scope(options);
            int matches=german_native_compound_matches(a+i,end-i,preferred,preferred_n);
            reloq61_text_options_scope(previous_options);
            if(matches) {
                free(preferred);preferred=NULL;
            }
        }
        if(preferred) {
            const unsigned char *open=(const unsigned char *)strstr((char *)preferred,"`[");
            const unsigned char *close=open?(const unsigned char *)strchr((char *)open,']'):NULL;
            if(open&&close) {
                if(!strstr((char *)close+1,"`[")) {
                    pron=open;pron_n=(size_t)(close+1-open);
                    if(reloq_rules61_word(0x40000,a+i,end-i,native,sizeof native,&native_n)==1&&
                       native_n==pron_n&&!memcmp(native,pron,pron_n))pron=NULL;
                } else {
                    /* Keep the native letter/word structure visible to
                     * sentence analysis, while retaining entries the native
                     * compound did not itself use. */
                    if(!changed){if(german_shared_append(&b,a,map,i,0)){free(preferred);status=-1;break;}changed=1;}
                    if(german_append_compound_parts(&b,a,map,i,end,overlay)){free(preferred);status=-1;break;}
                    free(preferred);i=end;continue;
                }
            }
        }
        if(pron) {
            if(!changed){if(german_shared_append(&b,a,map,i,0)){free(preferred);status=-1;break;}changed=1;}
            for(size_t j=0;j<pron_n;++j) {
                /* Separate adjacent generated annotations. Without this,
                 * an initialism followed by another compound member can
                 * be interpreted as literal annotation characters. */
                if(j&&pron[j]=='`'&&pron[j-1]==']'&&
                   german_shared_append(&b,(const unsigned char *)" ",NULL,1,map[i])) {status=-1;break;}
                if(german_shared_append(&b,pron+j,NULL,1,map[i])) {status=-1;break;}
            }
            if(status){free(preferred);break;}
        } else if(changed&&german_shared_append(&b,a+i,map+i,end-i,0)){free(preferred);status=-1;break;}
        free(preferred);i=end;
    }
    reloq_lexicon_resolver_scope(previous);reloq61_dictionary_resolver_release(resolver);
    if(status){free(b.bytes);free(b.map);return -1;}
    if(!changed){free(b.bytes);free(b.map);return 0;}
    if(german_shared_append(&b,(const unsigned char *)"",NULL,0,n))goto fail_compound;
    b.bytes[b.used]=0;b.map[b.used]=map[n];free(*text);free(*source);
    *text=b.bytes;*source=b.map;*length=b.used;return 0;
fail_compound:free(b.bytes);free(b.map);return -1;
}

/* Only an explicitly supplied pronunciation longer than the native lexer's
 * 100-letter token limit is projected early. All other text, compound parts
 * and sentence analysis still go through the ordinary native Root stage. */
static int german_long_overlay_input(unsigned char **text,size_t **source,size_t *length,int tags)
{
    const unsigned char *a=*text;const size_t *map=*source,n=*length;
    struct german_shared_buffer b={0};int spelling=0,changed=0;
    for(size_t i=0;i<n;) {
        struct reloq_inline_tag t=reloq_inline_tag_read(a+i,n-i,0);
        if(t.length&&(tags||t.kind==RELOQ_TAG_SPR||t.kind==RELOQ_TAG_PROMINENCE)) {
            if(tags&&t.kind==RELOQ_TAG_SPELL)spelling=t.value;
            if(changed&&german_shared_append(&b,a+i,map+i,t.length,0))goto fail;
            i+=t.length;continue;
        }
        size_t end=i;
        if(!spelling&&german_word_byte(a[i])&&(!i||!german_word_byte(a[i-1])))
            while(end<n&&german_word_byte(a[end]))++end;
        const struct reloq_lexicon_override *entry=end>i+100?
            reloq61_dictionary_long_override(a+i,end-i):NULL;
        if(entry) {
            if(!changed){if(german_shared_append(&b,a,map,i,0))goto fail;changed=1;}
            if(german_shared_append(&b,entry->spr,NULL,entry->spr_length,map[i]))goto fail;
            i=end;continue;
        }
        if(changed&&german_shared_append(&b,a+i,map+i,1,0))goto fail;
        ++i;
    }
    if(!changed){free(b.bytes);free(b.map);return 0;}
    if(german_shared_append(&b,(const unsigned char *)"",NULL,0,n))goto fail;
    b.bytes[b.used]=0;b.map[b.used]=map[n];
    free(*text);free(*source);*text=b.bytes;*source=b.map;*length=b.used;return 0;
fail:free(b.bytes);free(b.map);return -1;
}

static int german_currency_amount(const unsigned char *a,size_t start,size_t end)
{
    while(start<end&&(a[start]==' '||a[start]=='\t'||a[start]==0xa0))++start;
    if(start==end || (start&&german_word_byte(a[start-1])))return 0;
    size_t digits=0,group=0,groups=0;
    unsigned char first=a[start];
    for(size_t i=start;i<end;++i) {
        unsigned char ch=a[i];
        if(ch>='0'&&ch<='9'){++digits;++group;continue;}
        if(ch==',') {
            if(i+3!=end||a[i+1]<'0'||a[i+1]>'9'||a[i+2]<'0'||a[i+2]>'9')return 0;
            break;
        }
        if(ch!='.'&&ch!=' '&&ch!='\t'&&ch!=0xa0)return 0;
        if(!group || (groups?group!=3:group>3))return 0;
        ++groups;group=0;
        while(i+1<end&&(a[i+1]==' '||a[i+1]=='\t'||a[i+1]==0xa0))++i;
    }
    if(!group||(groups&&group!=3)||digits>12||(digits>1&&first=='0'))return 0;
    return 1;
}

/* Native German currency grammar recognizes a spaced euro sign as the
 * monetary form, while equivalent EUR/Euro spellings otherwise take its
 * ordinary decimal branch. Canonicalize only a unit directly following a
 * numeric token, after caller Main aliases and outside character spelling.
 * Keep source offsets for public index and word-control projection. */
static int german_currency_input(unsigned char **text,size_t **source,size_t *length,int tags)
{
    const unsigned char *a=*text;const size_t *map=*source,n=*length;
    struct german_shared_buffer b={0};int spelling=0,changed=0;
    for(size_t i=0;i<n;) {
        struct reloq_inline_tag t=tags?reloq_inline_tag_read(a+i,n-i,0):(struct reloq_inline_tag){0};
        if(t.length) {
            if(t.kind==RELOQ_TAG_SPELL)spelling=t.value;
            if(changed&&german_shared_append(&b,a+i,map+i,t.length,0))goto fail;
            i+=t.length;continue;
        }
        size_t unit=0;
        if(!spelling && a[i]==0x80)unit=1;
        else if(!spelling && n-i>=3 && (a[i]=='e'||a[i]=='E') &&
                (a[i+1]=='u'||a[i+1]=='U') && (a[i+2]=='r'||a[i+2]=='R')) {
            unit=3;
            if(i+unit<n && (a[i+unit]=='o'||a[i+unit]=='O'))++unit;
            if(i+unit<n && german_word_byte(a[i+unit]))unit=0;
        }
        size_t prior=i;
        while(prior && (a[prior-1]==' '||a[prior-1]=='\t'||a[prior-1]==0xa0))--prior;
        if(unit && prior && a[prior-1]>='0' && a[prior-1]<='9') {
            size_t start=prior;
            while(start && ((a[start-1]>='0'&&a[start-1]<='9')||
                            a[start-1]=='.'||a[start-1]==','))--start;
            while(start<prior&&a[start]=='.')++start;
            /* A group of exactly three digits can have another numeric group
             * before a space. Do not absorb an unrelated preceding number. */
            for(;;) {
                if(prior-start<3||a[start]<'0'||a[start]>'9'||
                   a[start+1]<'0'||a[start+1]>'9'||a[start+2]<'0'||a[start+2]>'9'||
                   (start+3<prior&&a[start+3]!=','&&a[start+3]!='.'))break;
                size_t before=start;
                while(before&&(a[before-1]==' '||a[before-1]=='\t'||a[before-1]==0xa0))--before;
                if(before==start||!before||a[before-1]<'0'||a[before-1]>'9')break;
                while(before&&((a[before-1]>='0'&&a[before-1]<='9')||a[before-1]=='.'))--before;
                if(!german_currency_amount(a,before,prior))break;
                start=before;
            }
            if(!german_currency_amount(a,start,prior))unit=0;
        } else unit=0;
        const int needs_space=unit && prior==i;
        const int convert=unit && (a[i]!=0x80 || needs_space);
        if(convert) {
            if(!changed) {
                if(german_shared_append(&b,a,map,i,0))goto fail;
                changed=1;
            }
            if(needs_space && german_shared_append(&b,(const unsigned char *)" ",NULL,1,map[i]))goto fail;
            const unsigned char euro=0x80;
            if(german_shared_append(&b,&euro,NULL,1,map[i]))goto fail;
            i+=unit;continue;
        }
        if(changed&&german_shared_append(&b,a+i,map+i,1,0))goto fail;
        ++i;
    }
    if(!changed){free(b.bytes);free(b.map);return 0;}
    if(german_shared_append(&b,(const unsigned char *)"",NULL,0,n))goto fail;
    b.bytes[b.used]=0;b.map[b.used]=map[n];
    free(*text);free(*source);*text=b.bytes;*source=b.map;*length=b.used;return 0;
fail:free(b.bytes);free(b.map);return -1;
}

int reloq_rules61_phonemes(int language,const unsigned char *text,size_t length,
    int tags,const int voice[8],const struct reloq61_mark *indices,size_t count,
    reloq61_phoneme_sink sink,reloq61_index_sink index_sink,void *context)
{
    unsigned char *input=NULL;size_t *map=NULL,n=0;
    int family=language>>16;
    int status=english_input(text,length,tags,
        family==6||family==8||family==10||family==18,family==1,&input,&map,&n);
    if(status)return status;
    if(language==0x40000) {
        german_compact_input(input,map,&n,tags);
        if(german_main_input(&input,&map,&n,tags)){free(input);free(map);return -2;}
        if(german_dictionary_compound_input(&input,&map,&n,tags)){free(input);free(map);return -2;}
        if(german_long_overlay_input(&input,&map,&n,tags)){free(input);free(map);return -2;}
        if(german_currency_input(&input,&map,&n,tags)){free(input);free(map);return -2;}
    }
    struct reloq61_mark *marks=count?malloc(count*sizeof *marks):NULL;
    if(count&&!marks){free(input);free(map);return -2;}
    size_t at=0;
    for(size_t i=0;i<count;++i) {
        if(indices[i].offset>length||(i&&indices[i].offset<indices[i-1].offset)) {
            free(input);free(map);free(marks);return -1;
        }
        while(at<n&&map[at]<indices[i].offset)++at;
        marks[i]=indices[i];marks[i].offset=at;
    }
    struct reloq61_request q={.language=language,.annotations=1,.phonemes_only=1,
        .text=(char *)input,.marks=marks,.mark_count=count,
        .phoneme_indices=1,.index=index_sink,.context=context};
    memcpy(q.voice,voice,sizeof q.voice);
    struct reloq61_result result={0};
    status=reloq61_generate_phonemes(&q,&result,sink,context);
    free(input);free(map);free(marks);return status;
}

/* Each word is analyzed for this request. No word or pronunciation result
 * survives in thread-local storage. */
int reloq_rules61_word(int language,const unsigned char *word,size_t length,
    unsigned char *out,size_t capacity,size_t *written)
{
    if(!word||!written||(!out&&capacity))return -1;
    if(!length||length>=256)return 0;
    unsigned char bytes[256],spr[2048];size_t bytes_n=0,n=0;char phonemes[4096];
    if(language==0x40000) {memcpy(bytes,word,length);bytes[length]=0;bytes_n=length;}
    else if(reloq_german_input_encode(word,length,bytes,sizeof bytes,&bytes_n,NULL))return 0;
    struct reloq61_request q={.language=language,.annotations=1,.phonemes_only=1,
        .voice={0,50,65,30,0,0,50,100},.text=(char *)bytes,
        .phonemes=phonemes,.phonemes_capacity=sizeof phonemes};
    struct reloq61_result r;
    if(reloq61_generate(&q,&r))return 0;
    char *a=strstr(phonemes,"`["),*b=a?strchr(a,']'):NULL;
    if(!a||!b||strstr(b+1,"`[")||(size_t)(b+1-a)>=sizeof spr)return 0;
    n=(size_t)(b+1-a);memcpy(spr,a,n);spr[n]=0;
    int status;
    if(language==0x40000)status=reloq_german_lexicon_validate_spr(spr,n,NULL)==1;
    else {
        struct reloq_english_phone *p=NULL;size_t count=0;
        status=!reloq_english_parse(spr,n,&p,&count,NULL)&&count>0;free(p);
    }
    if(!status)return 0;
    *written=n;if(capacity<=n)return -2;
    memcpy(out,spr,n+1);return 1;
}

struct rule_span {
    struct reloq61_curve curve[62];
    struct reloq61_curve male_af;
    int af_marked;
    int32_t from,to,volume,step,pitch,native_pitch,speed,slow_wpm;
    int64_t tick,pitch_tick;size_t pitch_source;
    struct reloq_german_voice_model model;
};
struct quiet_cut { int64_t from,to,after; };
struct rule_control { int64_t tick;size_t ordinal;int parameter,value; };
extern int reloq61_generate_with_pauses(const struct reloq61_request *,struct reloq61_result *,
    int,void (*)(void *,int64_t,int64_t,int64_t),void *);
struct rule_playback;
struct rule_plan {
    struct rule_span *spans;size_t count,capacity;
    struct quiet_cut *cuts;size_t cut_count,cut_capacity;
    int64_t ticks,*indices;size_t index_count;
    int64_t pitch_tick;
    const size_t *control_sources;size_t pitch_source;
    struct rule_control *controls;size_t control_count,control_capacity;
    struct reloq_session *session;
    int memory,retracted_filter,retracted_st,word_volume,cjk_volume,managed_voice_controls,word_pitch,native_pitch,word_speed,german_af_markers;
    struct reloq_german_voice_model word_model;
    int base_voice[8];
    struct rule_playback *playback;
};
static int process_spans(struct rule_plan *,int final);
static void release_spans(struct rule_plan *);
static struct rule_span *new_span(struct rule_plan *s)
{
    if(reloq_session_cancelled(s->session))return NULL;
    release_spans(s);
    if(s->count==s->capacity) {
        size_t cap=s->capacity?s->capacity*2:16;
        if(cap<s->capacity||cap>SIZE_MAX/sizeof *s->spans){s->memory=1;return NULL;}
        void *n=realloc(s->spans,cap*sizeof *s->spans);
        if(!n){s->memory=1;return NULL;}s->spans=n;s->capacity=cap;
    }
    return s->spans+s->count++;
}
static int take_span(void *context,int32_t from,int32_t to,
    const struct reloq61_curve curves[62],int32_t volume)
{
    struct rule_plan *s=context;struct rule_span *f=new_span(s);
    if(!f)return 1;
    memcpy(f->curve,curves,sizeof f->curve);
    f->af_marked=0;
    if(s->german_af_markers) {
        struct reloq61_curve *c=f->curve+8;f->male_af=*c;
        if(c->val_l>=RELOQ61_DE_AF_MARK){c->val_l-=RELOQ61_DE_AF_MARK;f->male_af.val_l=c->val_l>2?c->val_l-2:0;f->af_marked=1;}
        if(c->val_r>=RELOQ61_DE_AF_MARK){c->val_r-=RELOQ61_DE_AF_MARK;f->male_af.val_r=c->val_r>2?c->val_r-2:0;f->af_marked=1;}
    }
    f->from=from;f->to=to;f->volume=s->cjk_volume&&volume>=10000?
        (volume-10000)/100:s->word_volume;f->step=1;f->tick=s->ticks;f->pitch=s->word_pitch;f->native_pitch=s->native_pitch;
    f->slow_wpm=s->cjk_volume&&volume>=10000?(volume-10000)%100:0;
    f->model=s->word_model;f->speed=s->word_speed;f->pitch_tick=s->pitch_tick;f->pitch_source=s->pitch_source;
    s->ticks+=to-from;return s->playback?process_spans(s,0):0;
}
/* General fallback for a provider window whose step is not one ms. */
static int take_frame(void *context,const int32_t p[62],int32_t volume)
{
    struct rule_plan *s=context;struct rule_span *f=new_span(s);
    if(!f)return 1;
    for(unsigned k=0;k<62;++k)f->curve[k]=(struct reloq61_curve){0,p[k],p[0],p[k]};
    f->af_marked=0;
    if(s->german_af_markers&&p[8]>=RELOQ61_DE_AF_FRAME) {
        int packed=p[8]-RELOQ61_DE_AF_FRAME;
        int ordinary=packed/RELOQ61_DE_AF_PACK,male=packed%RELOQ61_DE_AF_PACK;
        f->curve[8]=(struct reloq61_curve){0,ordinary,p[0],ordinary};
        f->male_af=(struct reloq61_curve){0,male,p[0],male};f->af_marked=1;
    }
    f->from=0;f->to=p[0];f->step=p[0];f->volume=s->cjk_volume&&volume>=10000?
        (volume-10000)/100:s->word_volume;f->tick=s->ticks;f->pitch=s->word_pitch;f->native_pitch=s->native_pitch;
    f->slow_wpm=s->cjk_volume&&volume>=10000?(volume-10000)%100:0;
    f->model=s->word_model;f->speed=s->word_speed;f->pitch_tick=s->pitch_tick;f->pitch_source=s->pitch_source;
    s->ticks+=p[0];return s->playback?process_spans(s,0):0;
}
static void take_index(void *context,int32_t index,int64_t tick)
{
    struct rule_plan *s=context;
    if(index>=0&&(size_t)index<s->index_count && s->indices[index]!=INT64_MAX &&
       s->indices[index]!=0)s->indices[index]=tick;
}
static void take_control(void *context,int parameter,int value,int64_t tick,size_t ordinal)
{
    struct rule_plan *s=context;
    if(s->control_count>=s->control_capacity||ordinal>=s->control_capacity){s->memory=1;return;}
    size_t at=s->control_count++;
    while(at&&(s->controls[at-1].tick>tick ||
        (s->controls[at-1].tick==tick&&s->controls[at-1].ordinal>ordinal))) {
        s->controls[at]=s->controls[at-1];--at;
    }
    s->controls[at]=(struct rule_control){tick,ordinal,parameter,value};
}
static int32_t curve_value(const struct reloq61_curve *c,int32_t at)
{
    if(c->val_l==c->val_r || c->at_l==c->at_r)return c->val_l;
    float offset=(float)(at-c->at_l);
    offset*=(float)(c->val_r-c->val_l);offset/=(float)(c->at_r-c->at_l);
    return (int32_t)((float)c->val_l+offset);
}
/* A native span has immutable endpoints. Prepare its constant controls once,
 * then evaluate only changing fields at the original integer frame times. */
struct span_controls {
    float fixed[62];
    unsigned char moving[61];
    unsigned count;
};
static void prepare_controls(const struct rule_span *span,struct span_controls *p)
{
    p->count=0;p->fixed[0]=(float)span->step;
    for(unsigned k=1;k<62;++k) {
        const struct reloq61_curve *c=span->curve+k;
        p->fixed[k]=(float)c->val_l;
        if(c->val_l!=c->val_r&&c->at_l!=c->at_r)p->moving[p->count++]=(unsigned char)k;
    }
}
static void read_controls(const struct span_controls *p,const struct rule_span *span,
    int32_t at,float controls[62])
{
    memcpy(controls,p->fixed,sizeof p->fixed);
    for(unsigned j=0;j<p->count;++j) {
        unsigned k=p->moving[j];controls[k]=(float)curve_value(span->curve+k,at);
    }
}
static void take_pause(void *context,int64_t from,int64_t to,int64_t after)
{
    struct rule_plan *s=context;
    if(to-from<=3)return;
    if(s->cut_count==s->cut_capacity) {
        size_t cap=s->cut_capacity?s->cut_capacity*2:8;
        if(cap<s->cut_capacity||cap>SIZE_MAX/sizeof *s->cuts){s->memory=1;return;}
        void *p=realloc(s->cuts,cap*sizeof *s->cuts);if(!p){s->memory=1;return;}
        s->cuts=p;s->cut_capacity=cap;
    }
    s->cuts[s->cut_count++]=(struct quiet_cut){from+3,to,after};
}
/* Map original UTF-8 offsets to a complete SPR boundary. Inserting an
 * index inside a phoneme group would change the token stream. */
static size_t annotation_boundary(const unsigned char *a,size_t n,const size_t *map,size_t offset,size_t *cursor)
{
    if(!offset){*cursor=0;return 0;}
    size_t boundary=n;
    /* Source positions normally arrive in increasing order. Resume at the
     * unadjusted match, so each annotation token is scanned only once while
     * preserving the original punctuation and inside-word adjustments. */
    for(size_t j=*cursor;j<n;) {
        if(map[j]>=offset){boundary=j;break;}
        struct reloq_inline_tag t=reloq_inline_tag_read(a+j,n-j,1);
        j+=t.length?t.length:1;
    }
    *cursor=boundary;
    /* An AddText piece ending in punctuation lets the private ECI parser
     * finish a stretch before the following index arrives. Put the mark
     * at the preceding complete word edge, leaving punctuation and the
     * next text in one parser piece. This keeps the PCM independent of
     * source fragment/index positions. EOF is delivered after all audio. */
    while(boundary&&strchr(".,!?;:\n() \t",a[boundary-1]))--boundary;
    /* A source index within ordinary text belongs to the complete token.
       Native SPR groups were already skipped atomically above. */
    if(boundary&&boundary<n) {
        unsigned char previous=a[boundary-1],current=a[boundary];
        int left=(previous>='A'&&previous<='Z')||(previous>='a'&&previous<='z')||
            (previous>='0'&&previous<='9')||previous>=128||previous=='\'';
        int right=(current>='A'&&current<='Z')||(current>='a'&&current<='z')||
            (current>='0'&&current<='9')||current>=128||current=='\'';
        if(left&&right)while(boundary<n) {
            unsigned char c=a[boundary];
            if(!((c>='A'&&c<='Z')||(c>='a'&&c<='z')||
                 (c>='0'&&c<='9')||c>=128||c=='\''))break;
            ++boundary;
        }
    }
    return boundary;
}
struct rule_delivery {
    reloq_session_pcm_sink sink;void *context;double gain;
    int16_t pending[1024];size_t used,limit,next_limit;
};
static int rule_flush(struct rule_delivery *d)
{
    if(!d->used)return 0;
    int status=d->sink(d->context,d->pending,d->used);
    d->used=0;d->limit=d->next_limit;return status;
}
static int rule_pcm(void *context,const int16_t *pcm,size_t count)
{
    struct rule_delivery *d=context;
    while(count) {
        size_t n=d->limit-d->used;if(n>count)n=count;
        for(size_t i=0;i<n;++i) {
            long x=lround(pcm[i]*d->gain);
            d->pending[d->used+i]=(int16_t)(x>32767?32767:x< -32768?-32768:x);
        }
        d->used+=n;
        if(d->used==d->limit&&rule_flush(d))return -1;
        pcm+=n;count-=n;
    }
    return 0;
}
/* Compact windows can be rendered as soon as their native word rules
 * finish. Only a source-silent run waits for the requested pause policy. */
struct rule_playback {
    struct reloq_german_voice_model model;
    const struct reloq_german_generic_settings *settings;
    const struct reloq_german_generic_pitch_mark *pitches;
    size_t index_count,pitch_count,next,next_pitch,next_cut,read;
    size_t next_control;
    struct rule_span voice;
    int32_t at;
    int64_t last_pitch_tick;
    size_t last_pitch_source;
    int pause_mode,final,result;
    double pitch_scale,base_pitch,time_fraction;
    int slow_wpm;
    struct rule_delivery delivery;
    reloq_german_generic_index_sink index_sink;
    void *context;
};
static void release_spans(struct rule_plan *s)
{
    /* Absolute ticks and marks are independent of these consumed windows.
     * Retain a window only while playback still holds part of a pause. */
    if(s->playback&&s->playback->read==s->count) {
        s->count=0;s->playback->read=0;
    }
}
static int32_t neutral_endpoint(const struct reloq_voice_filter *model,unsigned parameter,int32_t value)
{
    if(model->head_size==50&&!model->female)return value;
    /* The native anatomy map is monotone, with integer endpoint rounding.
       Recover the nearest neutral endpoint before applying the new word's
       anatomy. Unchanged words never enter this conversion. */
    int32_t lo=0,hi=20000;
    while(lo<hi) {
        int32_t mid=lo+(hi-lo)/2;
        if(reloq_voice_filter_endpoint(model,parameter,mid)<value)lo=mid+1;
        else hi=mid;
    }
    return lo;
}
static void word_voice_frame(const struct rule_plan *s,const struct rule_span *f,float c[62])
{
    const int *v=s->base_voice;
    const struct reloq_german_voice_model *m=&f->model;
    if(m->gender!=v[0]||m->head_size!=v[1]) {
        struct reloq_voice_filter old={v[1],v[0],m->sample_rate,(250-v[1])/2,24};
        struct reloq_voice_filter next={m->head_size,m->gender,m->sample_rate,(250-m->head_size)/2,24};
        const unsigned char centres[]={9,13,15,17,19};
        for(unsigned k=0;k<5;++k) {
            unsigned j=centres[k];
            c[j]=(float)reloq_voice_filter_endpoint(&next,k+1,neutral_endpoint(&old,k+1,(int32_t)c[j]));
            static const int male[]={70,100,150,330,260};
            static const int female[3][5]={{300,200,150,300,500},{200,100,350,500,500},{350,100,350,1000,1000}};
            int before=v[0]?female[v[1]<30?1:v[5]==100?2:0][k]:male[k];
            int after=m->gender?female[m->head_size<30?1:m->breathiness==100?2:0][k]:male[k];
            if(k>=2) {
                before=reloq_voice_filter_endpoint(&old,k+6,before);
                after=reloq_voice_filter_endpoint(&next,k+6,after);
            }
            c[j+1]*=(float)after/before;
        }
    }
    if(m->gender!=v[0]||m->head_size!=v[1]||m->breathiness!=v[5]) {
        int old_open=v[0]?(v[1]<=40?85:100):56+43*v[5]/100;
        int new_open=m->gender?(m->head_size<=40?85:100):56+43*m->breathiness/100;
        if(c[2]>0)c[3]=fminf(100,c[3]*new_open/old_open);
        if(c[2]>0) {
            int old_tilt=(v[0]?36:35)*v[5]/100+(v[0]&&v[1]<30?18:0);
            int new_tilt=(m->gender?36:35)*m->breathiness/100+(m->gender&&m->head_size<30?18:0);
            c[4]=fmaxf(c[4]-old_tilt,0)+(float)new_tilt;
        }
    }
    if(m->roughness!=v[4]&&c[2]>0) {
        float native=4*v[4]/10;
        if(c[6]<=native)c[6]=(float)(4*m->roughness/10);
    }
    if(m->breathiness!=v[5]&&c[2]>0) {
        c[7]=fmaxf(c[7]-50*v[5]/100,0)+(float)(50*m->breathiness/100);
        if(m->breathiness==100)c[2]=0;
    }
}
static double pitch_ratio(int target,int native_baseline)
{
    double baseline=reloq_voice_pitch_f0_scale(native_baseline);
    if(reloq_pitch_is_hz(target)) {
        /* The provider emits its baseline in whole tenths of a hertz.
         * Undo that rounding before applying the absolute Hz request. */
        double tenths=trunc(1189.15*baseline);
        return 10.0*(target-RELOQ_PITCH_HZ_OFFSET)/tenths;
    }
    return reloq_voice_pitch_f0_scale(target)/baseline;
}
static int render_until(struct rule_plan *s,int64_t limit)
{
    struct rule_playback *p=s->playback;
    struct span_controls prepared={0};const struct rule_span *previous=NULL;
    while(p->read<s->count) {
        const struct rule_span *f=s->spans+p->read;
        if(p->at<f->from)p->at=f->from;
        if(p->at>=f->to){++p->read;p->at=0;continue;}
        int32_t at=p->at;int64_t tick=f->tick+at-f->from;
        if(tick>=limit)break;
        p->at+=f->step;
        while(p->next_cut<s->cut_count&&tick>=s->cuts[p->next_cut].to)++p->next_cut;
        if(p->final&&p->next_cut<s->cut_count&&
           s->cuts[p->next_cut].after==s->ticks&&tick>=s->cuts[p->next_cut].from)continue;
        while(p->next<p->index_count && s->indices[p->next]>=0 && s->indices[p->next]<=tick) {
            size_t ordinal=p->next++;
            if(rule_flush(&p->delivery)){p->result=RELOQ_DE_UTTERANCE_SINK;return 1;}
            if(p->index_sink&&p->index_sink(p->context,ordinal)) {
                p->result=RELOQ_DE_UTTERANCE_SINK;return 1;
            }
        }
        while(p->next_pitch<p->pitch_count && s->indices[p->index_count+p->next_pitch]>=0 &&
              s->indices[p->index_count+p->next_pitch]<=tick) {
            p->last_pitch_tick=s->indices[p->index_count+p->next_pitch];
            p->last_pitch_source=p->pitches[p->next_pitch].annotation_offset;
            p->pitch_scale=reloq_voice_pitch_f0_scale(p->pitches[p->next_pitch++].native_pitch)/p->base_pitch;
        }
        if(previous!=f){prepare_controls(f,&prepared);previous=f;}
        float controls[62];read_controls(&prepared,f,at,controls);
        const struct rule_span *voice=f;
        if(s->control_capacity) {
            while(p->next_control<s->control_count&&s->controls[p->next_control].tick<=tick) {
                const struct rule_control *change=s->controls+p->next_control++;
                int parameter=change->parameter,value=change->value;
                if(parameter==7)p->voice.volume=value;
                else if(parameter==2){p->voice.pitch=value;p->voice.pitch_tick=change->tick;
                    p->voice.pitch_source=s->control_sources[change->ordinal];}
                else if(parameter==6)p->voice.speed=value;
                else if(parameter==0)p->voice.model.gender=value;
                else if(parameter==1)p->voice.model.head_size=value;
                else if(parameter==4)p->voice.model.roughness=value;
                else if(parameter==5)p->voice.model.breathiness=value;
                else if(parameter==8)p->slow_wpm=value;
            }
            if(s->managed_voice_controls)voice=&p->voice;
        }
        if(f->af_marked&&!voice->model.gender)controls[8]=(float)curve_value(&f->male_af,at);
        word_voice_frame(s,voice,controls);
        double pitch_scale=voice->pitch>=0&&(voice->pitch_tick>p->last_pitch_tick||
            (voice->pitch_tick==p->last_pitch_tick&&voice->pitch_source>=p->last_pitch_source))?
            pitch_ratio(voice->pitch,voice->native_pitch):p->pitch_scale;
        if(pitch_scale!=1)controls[1]=(float)(controls[1]*pitch_scale);
        if(memcmp(&p->model,&voice->model,sizeof p->model)) {
            p->model=voice->model;
            int head=p->model.head_size,gender=p->model.gender;
            long source=gender?(head<30?5:3):4;
            if(p->model.sample_rate==8000&&head<=(gender?40:30)&&!(gender&&head<30))source=7;
            unsigned reference=p->model.sample_rate>=16000?(head<=(gender?40:30)?4u:5u):0;
            if(reloq_session_source_levels(s->session,source,gender?-3:-5,6,gender?-7:0,reference)) {
                p->result=RELOQ_DE_UTTERANCE_INVALID;return 1;
            }
        }
        reloq_german_voice_model_frame(&p->model,controls);
        struct reloq_core_voiced_frame frame;
        if(reloq_frame_adapter_map(controls,reloq_german_voice_model_formants(&p->model),&frame)) {
            p->result=RELOQ_DE_UTTERANCE_INVALID;return 1;
        }
        frame.noise.s_frication=controls[8]>0&&(controls[38]>controls[37]+10||controls[39]>controls[37]+10);
        if(s->retracted_filter) {
            /* Castilian and Greek share an acoustic core. Their /s/ can have
             * A4F-A3F == 10 dB, below the older generic
             * classifier's strict >10 test. Hold its bandwidth limit over
             * the AF release, rather than switching at each rounded tail. */
            if(controls[8]<=0)s->retracted_st=0;
            else if(controls[38]>controls[37]+5||controls[39]>controls[37]+5)
                s->retracted_st=1;
            frame.noise.limit_frication_extension=s->retracted_st;
        }
        p->delivery.gain=p->settings->volume/100.0*reloq_voice_volume_gain(voice->volume);
        if(reloq_session_float_volume(s->session)) {
            if(reloq_session_linear_gain(s->session,(float)p->delivery.gain)) {
                p->result=RELOQ_DE_UTTERANCE_INVALID;return 1;
            }
            p->delivery.gain=1;
        }
        int slow_wpm=s->cjk_volume?f->slow_wpm:p->slow_wpm;
        if((voice->speed>=0&&voice->speed!=s->base_voice[6])||slow_wpm) {
            double ratio=1;
            if(voice->speed>=0&&voice->speed!=s->base_voice[6])
                ratio=reloq_voice_speed_time_scale(voice->speed)/reloq_voice_speed_time_scale(s->base_voice[6]);
            if(slow_wpm>=21&&slow_wpm<=39)ratio*=20.0/(slow_wpm-20);
            else if(slow_wpm)ratio*=5.0/slow_wpm;
            p->time_fraction+=f->step*ratio;
            int milliseconds=(int)p->time_fraction;p->time_fraction-=milliseconds;
            if(!milliseconds)continue;
            frame.noise.duration_ms=(float)milliseconds;
        }
        size_t completed=0;
        int status=reloq_session_render_frames(s->session,&frame,1,rule_pcm,&p->delivery,&completed);
        if(status) {
            p->result=status==RELOQ_SESSION_CANCELLED?RELOQ_DE_UTTERANCE_CANCELLED:RELOQ_DE_UTTERANCE_SINK;
            return 1;
        }
    }
    return 0;
}
static int process_spans(struct rule_plan *s,int final)
{
    struct rule_playback *p=s->playback;
    if(p->result)return 1;
    if(!p->pause_mode)return render_until(s,s->ticks);
    p->final=final;
    int64_t limit=s->ticks;
    if(!final&&s->cut_count) {
        const struct quiet_cut *tail=s->cuts+s->cut_count-1;
        /* A following spoken span proves this was an internal pause.
           Until then hold only its known quiet body, never stop closures. */
        if(s->ticks<=tail->after&&limit>tail->from)limit=tail->from;
    }
    return render_until(s,limit);
}
static int rule_cancelled(void *context)
{return reloq_session_cancelled(context);}

int reloq_language_rules_render(int language,const unsigned char *text,size_t length,int tags,int native_volume,int default_wpm,
    const struct reloq_german_generic_settings *settings,
    const struct reloq_german_generic_index *indices,size_t index_count,
    const struct reloq_german_generic_pitch_mark *pitches,size_t pitch_count,
    struct reloq_session *session,reloq_session_pcm_sink sink,
    reloq_german_generic_index_sink index_sink,void *context)
{
    if(!text||!settings||!session||!sink||(index_count&&!indices)||(pitch_count&&!pitches)||
       index_count>INT32_MAX||pitch_count>(size_t)INT32_MAX-index_count)return RELOQ_DE_UTTERANCE_INVALID;
    if(settings->native_pitch_plus_one<0)return RELOQ_DE_UTTERANCE_INVALID;
    int configured_pitch=settings->native_pitch_plus_one?settings->native_pitch_plus_one-1:
        language==0x40000?65:settings->pitch;
    if(!reloq_pitch_valid(configured_pitch))return RELOQ_DE_UTTERANCE_INVALID;
    /* The provider accepts only ECI 0..100. Its curves are rescaled in the
     * renderer below when the requested baseline is expressed in Hz. */
    int provider_pitch=reloq_pitch_is_hz(configured_pitch)?65:configured_pitch;
    unsigned char *a=NULL;size_t *map=NULL,n=0;
    struct rule_plan plan={.session=session,.index_count=index_count+pitch_count,
        .retracted_filter=language==0x20000||language==0x120000,.word_volume=native_volume,.word_pitch=-1,
        .native_pitch=provider_pitch,
        .word_speed=-1};
    plan.word_model=(struct reloq_german_voice_model){
        settings->head_size_plus_one?settings->head_size_plus_one-1:50,
        settings->roughness,settings->breathiness,settings->gender,reloq_session_sample_rate(session)};
    struct reloq61_mark *marks=NULL;int result=RELOQ_DE_UTTERANCE_INVALID;
    struct reloq61_mark *volumes=NULL;size_t volume_count=0;size_t *control_sources=NULL;
    int family=language>>16;
    plan.cjk_volume=family==6||family==8||family==10;
    int input_status=english_input(text,length,tags,family==6||family==8||family==10||family==18,family==1,&a,&map,&n);
    if(input_status==-2)goto memory;
    if(input_status)goto done;
    if(language==0x40000)german_compact_input(a,map,&n,tags);
    if(tags&&!plan.cjk_volume&&normalize_word_voice_controls(&a,&map,&n,family==18))goto memory;
    if(language==0x40000&&german_main_input(&a,&map,&n,tags))goto memory;
    if(language==0x40000&&german_dictionary_compound_input(&a,&map,&n,tags))goto memory;
    if(language==0x40000&&german_long_overlay_input(&a,&map,&n,tags))goto memory;
    if(language==0x40000&&german_currency_input(&a,&map,&n,tags))goto memory;
    if(default_wpm) {
        /* Input has already been sanitized according to the caller's tag
         * setting. Add only this trusted control afterwards so WPM works
         * even when ordinary text tags are disabled. Map it to source 0. */
        char prefix[16];int count=snprintf(prefix,sizeof prefix,"`vwpm%d ",default_wpm);
        if(count<=0||(size_t)count>=sizeof prefix||n>SIZE_MAX-(size_t)count-1||
           n+(size_t)count+1>SIZE_MAX/sizeof *map)goto memory;
        size_t shift=(size_t)count;
        unsigned char *new_a=malloc(n+shift+1);size_t *new_map=malloc((n+shift+1)*sizeof *new_map);
        if(!new_a||!new_map){free(new_a);free(new_map);goto memory;}
        memcpy(new_a,prefix,shift);memcpy(new_a+shift,a,n+1);
        for(size_t i=0;i<shift;++i)new_map[i]=0;
        memcpy(new_map+shift,map,(n+1)*sizeof *map);
        free(a);free(map);a=new_a;map=new_map;n+=shift;tags=1;
    }
    int has_wpm=0,has_very_slow_wpm=0;
    if(tags)for(size_t i=0;i<n;) {
        struct reloq_inline_tag t=reloq_inline_tag_read(a+i,n-i,1);
        if(t.kind==RELOQ_TAG_WPM){has_wpm=1;if(t.value<5)has_very_slow_wpm=1;}
        i+=t.length?t.length:1;
    }
    int cjk_exact_pitch=plan.cjk_volume && reloq_pitch_is_hz(configured_pitch);
    if(plan.cjk_volume && tags)for(size_t i=0;i<n;) {
        struct reloq_inline_tag t=reloq_inline_tag_read(a+i,n-i,1);
        if(t.kind==RELOQ_TAG_PITCH && reloq_pitch_is_hz(t.value))cjk_exact_pitch=1;
        i+=t.length?t.length:1;
    }
    int managed_controls=!plan.cjk_volume || cjk_exact_pitch;
    plan.managed_voice_controls=managed_controls;
    if(has_wpm&&managed_controls&&native_preset_speed_tags(&a,&map,&n))goto memory;
    if(plan.cjk_volume) {
        /* CJK lexers can treat even a redundant command as a token break.
         * No-op voice controls must leave native segmentation untouched. Actual
         * changes retain the module's acoustic-word parameter schedule. */
        int values[8]={settings->gender,
            settings->head_size_plus_one?settings->head_size_plus_one-1:50,
            configured_pitch,
            settings->native_inflection_plus_one?settings->native_inflection_plus_one-1:30,
            settings->roughness,settings->breathiness,
            settings->native_speed?(settings->native_speed<0?0:settings->native_speed):settings->rate,
            native_volume};
        size_t out=0;
        for(size_t i=0;i<n;) {
            struct reloq_inline_tag t=reloq_inline_tag_read(a+i,n-i,1);
            size_t count=t.length?t.length:1;
            int parameter=-1;
            switch(t.kind) {
            case RELOQ_TAG_GENDER:parameter=0;break;
            case RELOQ_TAG_HEAD_SIZE:parameter=1;break;
            case RELOQ_TAG_PITCH:if(!pitch_count)parameter=2;break;
            case RELOQ_TAG_INFLECTION:parameter=3;break;
            case RELOQ_TAG_ROUGHNESS:parameter=4;break;
            case RELOQ_TAG_BREATHINESS:parameter=5;break;
            case RELOQ_TAG_SPEED:if(!has_wpm)parameter=6;break;
            case RELOQ_TAG_VOLUME:parameter=7;break;
            default:break;
            }
            int omit=parameter>=0&&values[parameter]==t.value;
            if(parameter>=0)values[parameter]=t.value;
            if(t.kind==RELOQ_TAG_PRESET) {
                struct reloq_inline_word v={0};reloq_inline_preset(&v,t.value);
                int preset[8]={v.gender,v.head_size,v.pitch,v.inflection,v.roughness,v.breathiness,v.speed,v.volume};
                omit=!memcmp(values,preset,sizeof values)&&!pitch_count;
                memcpy(values,preset,sizeof values);
            }
            if(!omit){memmove(a+out,a+i,count);memmove(map+out,map+i,count*sizeof *map);out+=count;}
            i+=count;
        }
        n=out;a[n]=0;map[n]=length;
    }
    if(managed_controls&&native_preset_inflection_tags(&a,&map,&n))goto memory;
    /* Voice tags travel beside text, and are applied at the native word
     * callback. Native analysis must see the unchanged lexical sentence. */
    for(size_t i=0;i<n;) {
        struct reloq_inline_tag t=reloq_inline_tag_read(a+i,n-i,1);
        if(has_very_slow_wpm&&!plan.cjk_volume&&(t.kind==RELOQ_TAG_WPM||t.kind==RELOQ_TAG_SPEED||
                              t.kind==RELOQ_TAG_PRESET))++volume_count;
        if(managed_controls) {
            if(t.kind==RELOQ_TAG_PITCH||(t.kind==RELOQ_TAG_SPEED&&!has_wpm)||t.kind==RELOQ_TAG_VOLUME||
               t.kind==RELOQ_TAG_GENDER||t.kind==RELOQ_TAG_HEAD_SIZE||
               t.kind==RELOQ_TAG_ROUGHNESS||t.kind==RELOQ_TAG_BREATHINESS)++volume_count;
            else if(t.kind==RELOQ_TAG_PRESET)volume_count+=has_wpm?6:7;
        }
        i+=t.length?t.length:1;
    }
    if(volume_count) {
        if(volume_count>SIZE_MAX/sizeof *volumes)goto memory;
        volumes=malloc(volume_count*sizeof *volumes);if(!volumes)goto memory;
        control_sources=malloc(volume_count*sizeof *control_sources);if(!control_sources)goto memory;
        if(volume_count>SIZE_MAX/sizeof *plan.controls)goto memory;
        plan.controls=malloc(volume_count*sizeof *plan.controls);if(!plan.controls)goto memory;
        plan.control_capacity=volume_count;
        size_t out=0,control=0;
        for(size_t i=0;i<n;) {
            struct reloq_inline_tag t=reloq_inline_tag_read(a+i,n-i,1);
            size_t count=t.length?t.length:1;
            /* At 1--4 WPM the native 16-bit duration cells can wrap, even in
             * the reference. Generate at 5 and stretch its control frames as
             * they are synthesized, retaining the pitch and word boundaries. */
            if(has_very_slow_wpm&&!plan.cjk_volume&&(t.kind==RELOQ_TAG_WPM||t.kind==RELOQ_TAG_SPEED||
                                   t.kind==RELOQ_TAG_PRESET)) {
                int slow=t.kind==RELOQ_TAG_WPM&&t.value<5?t.value:0;
                control_sources[control]=map[i];
                volumes[control++]=(struct reloq61_mark){out+(t.kind==RELOQ_TAG_WPM?count:0),slow,8};
                if(slow) {
                    for(size_t j=i+5;j<i+count;++j)a[j]='0';
                    a[i+count-1]='5';
                }
            }
            int parameter=t.kind==RELOQ_TAG_GENDER?0:t.kind==RELOQ_TAG_HEAD_SIZE?1:
                t.kind==RELOQ_TAG_PITCH?2:
                t.kind==RELOQ_TAG_ROUGHNESS?4:t.kind==RELOQ_TAG_BREATHINESS?5:
                t.kind==RELOQ_TAG_SPEED&&!has_wpm?6:t.kind==RELOQ_TAG_VOLUME?7:-1;
            if(parameter>=0) {
                control_sources[control]=map[i];volumes[control++]=(struct reloq61_mark){out,t.value,parameter};
            }
            else if(t.kind==RELOQ_TAG_PRESET) {
                struct reloq_inline_word v={0};reloq_inline_preset(&v,t.value);
                int preset[]={v.gender,v.head_size,v.pitch,v.inflection,v.roughness,v.breathiness,v.speed,v.volume};
                for(int k=0;k<8;++k) {
                    if(k==3||(has_wpm&&k==6))continue;
                    control_sources[control]=map[i];volumes[control++]=(struct reloq61_mark){out,preset[k],k};
                }
            } else {
                memmove(a+out,a+i,count);memmove(map+out,map+i,count*sizeof *map);out+=count;
            }
            i+=count;
        }
        n=out;a[n]=0;map[n]=length;
        for(size_t i=0;i<volume_count;++i)
            while(volumes[i].offset<n&&strchr(" \t\r\n",a[volumes[i].offset]))++volumes[i].offset;
    }
    /* Leading punctuation has no preceding spoken word to pause after. */
    for(size_t j=0;j<n;) {
        struct reloq_inline_tag t=reloq_inline_tag_read(a+j,n-j,1);
        if(t.kind==RELOQ_TAG_SPR || (t.kind==RELOQ_TAG_SPELL&&t.value))break;
        if(!t.length&&!strchr(".,!?;:\n() \t",a[j]))break;
        if(!t.length&&strchr(".,!?;:\n()",a[j]))a[j]=' ';
        j+=t.length?t.length:1;
    }
    if(plan.index_count) {
        if(plan.index_count>SIZE_MAX/sizeof *plan.indices)goto memory;
        plan.indices=malloc(plan.index_count*sizeof *plan.indices);if(!plan.indices)goto memory;
        for(size_t i=0;i<plan.index_count;++i)plan.indices[i]=-1;
    }
    size_t mark_count=index_count+pitch_count;
    if(mark_count) {
        if(mark_count>SIZE_MAX/sizeof *marks)goto memory;
        marks=malloc(mark_count*sizeof *marks);if(!marks)goto memory;
        size_t i=0,p=0,cursor=0,previous_offset=0;
        for(size_t j=0;j<mark_count;++j) {
            int is_index=p==pitch_count||(i<index_count&&indices[i].annotation_offset<=pitches[p].annotation_offset);
            size_t offset=is_index?indices[i].annotation_offset:pitches[p].annotation_offset;
            if(offset>length)goto done;
            if(!is_index&&(pitches[p].native_pitch<0||pitches[p].native_pitch>100))goto done;
            /* Preserve behaviour for callers with out-of-order inputs. */
            if(j&&offset<previous_offset)cursor=0;
            marks[j]=(struct reloq61_mark){annotation_boundary(a,n,map,offset,&cursor),
                (int32_t)(is_index?i:index_count+p),
                is_index&&plan.cjk_volume?RELOQ61_SOURCE_INDEX_ONLY:-1};
            previous_offset=offset;
            if(j&&marks[j].offset<marks[j-1].offset)goto done;
            if(is_index)++i;else ++p;
        }
    }
    /* Preserve the existing frontend's Unicode, numeric and index ownership;
     * all connected phonetic timing and targets now come from native rules. */
    struct reloq61_request q={.language=language,.annotations=1,.text=(char *)a,
        .marks=marks,.mark_count=mark_count,.frame=take_frame,.index=take_index,.context=&plan,
        .span=take_span,.sample_rate=reloq_session_sample_rate(session)==8000?8000:0,
        .output_rate=(int)reloq_session_sample_rate(session)};
    q.german_affricate_markers=plan.german_af_markers=language==0x40000;
    q.cjk_slow_wpm=plan.cjk_volume&&has_very_slow_wpm;
    q.cjk_has_wpm=plan.cjk_volume&&has_wpm;
    q.controls=volumes;q.control_count=volume_count;q.control=take_control;plan.control_sources=control_sources;
    q.voice[0]=settings->gender;
    q.voice[1]=settings->head_size_plus_one?settings->head_size_plus_one-1:50;
    q.voice[2]=provider_pitch;
    q.voice[3]=settings->native_inflection_plus_one?settings->native_inflection_plus_one-1:30;
    q.voice[4]=settings->roughness;q.voice[5]=settings->breathiness;
    q.voice[6]=settings->native_speed?(settings->native_speed<0?0:settings->native_speed):settings->rate;
    q.voice[7]=managed_controls?100:native_volume;
    memcpy(plan.base_voice,q.voice,sizeof plan.base_voice);
    /* Only an explicit trailing pause protects the final silence. A p0
       reset or an earlier pause must not disable the entire policy. */
    int explicit_pause=0;
    for(size_t j=0;j<n;) {
        struct reloq_inline_tag t=reloq_inline_tag_read(a+j,n-j,1);
        if(t.kind==RELOQ_TAG_PAUSE)explicit_pause=t.value>0;
        else if(t.kind==RELOQ_TAG_SPR || (!t.length &&
            !strchr(".,!?;:\n()[]{}\"' \t",a[j])))explicit_pause=0;
        j+=t.length?t.length:1;
    }
    for(size_t i=0;i<index_count;++i) {
        if(!indices[i].annotation_offset)plan.indices[i]=0;
        else if(indices[i].annotation_offset>=length)plan.indices[i]=INT64_MAX;
    }
    for(size_t i=0;i<pitch_count;++i) {
        if(!pitches[i].annotation_offset)plan.indices[index_count+i]=0;
        else if(pitches[i].annotation_offset>=length)plan.indices[index_count+i]=INT64_MAX;
    }
    struct rule_playback playback={
        .voice={.volume=native_volume,.pitch=-1,.native_pitch=q.voice[2],.speed=-1,
            .model={q.voice[1],q.voice[4],q.voice[5],q.voice[0],reloq_session_sample_rate(session)}},
        .model={q.voice[1],q.voice[4],q.voice[5],q.voice[0],reloq_session_sample_rate(session)},
        .settings=settings,.pitches=pitches,.index_count=index_count,.pitch_count=pitch_count,
        .pause_mode=!explicit_pause&&settings->pause_mode==1,
        .pitch_scale=reloq_pitch_is_hz(configured_pitch)?pitch_ratio(configured_pitch,provider_pitch):
            language==0x40000&&!settings->native_pitch_plus_one?
            (150.0+settings->pitch)/200.0:1,
        .base_pitch=reloq_voice_pitch_f0_scale(q.voice[2]),.last_pitch_tick=INT64_MIN,
        .delivery={.sink=sink,.context=context,.gain=1,
            .limit=(size_t)reloq_session_sample_rate(session)/100,
            .next_limit=(size_t)reloq_session_sample_rate(session)/50},
        .index_sink=index_sink,.context=context};
    plan.playback=&playback;
    struct reloq61_result r;
    /* A refused PCM/span callback need not stop the frontend's remaining
     * input. Let native text rules observe the same owner cancellation so
     * a following NVDA utterance does not wait for the old text's EOF. */
    struct reloq61_cancel_check previous_cancel=reloq61_cancel_scope(
        (struct reloq61_cancel_check){rule_cancelled,session});
    int previous_dictionary_scope=language==0x40000?reloq61_dictionary_text_scope(1):0;
    int status=reloq61_generate_with_pauses(&q,&r,settings->pause_mode,
        playback.pause_mode?take_pause:NULL,&plan);
    if(language==0x40000)reloq61_dictionary_text_scope(previous_dictionary_scope);
    reloq61_cancel_scope(previous_cancel);
    if(reloq_session_cancelled(session)){result=RELOQ_DE_UTTERANCE_CANCELLED;goto done;}
    if(plan.memory)goto memory;
    if(playback.result){result=playback.result;goto done;}
    /* Every production phonetic rule is native C. A provider error stops
     * the stream; it never switches acoustic implementations mid-sentence. */
    if(status) {
        if(getenv("RELOQ_RULE_DEBUG"))fprintf(stderr,"rules61 status=%d unsupported=%d error=%d spans=%zu\n",status,r.unsupported,r.error,plan.count);
        goto done;
    }
    for(size_t i=0;i<plan.index_count;++i)
        if(plan.indices[i]<0 || plan.indices[i]==INT64_MAX)plan.indices[i]=plan.ticks;
    if(process_spans(&plan,1)) {
        if(plan.memory)goto memory;
        result=playback.result;goto done;
    }
    if(rule_flush(&playback.delivery)){result=RELOQ_DE_UTTERANCE_SINK;goto done;}
    while(playback.next<index_count) {
        size_t ordinal=playback.next++;
        if(index_sink&&index_sink(context,ordinal)){result=RELOQ_DE_UTTERANCE_SINK;goto done;}
    }
    result=RELOQ_DE_UTTERANCE_OK;goto done;
memory:result=RELOQ_DE_UTTERANCE_MEMORY;
done:free(a);free(map);free(marks);free(volumes);free(control_sources);free(plan.controls);free(plan.spans);free(plan.cuts);free(plan.indices);return result;
}

int reloq_english_rules_render(const unsigned char *text,size_t length,int tags,int native_volume,
    const struct reloq_german_generic_settings *settings,
    const struct reloq_german_generic_index *indices,size_t index_count,
    const struct reloq_german_generic_pitch_mark *pitches,size_t pitch_count,
    struct reloq_session *session,reloq_session_pcm_sink sink,
    reloq_german_generic_index_sink index_sink,void *context)
{
    return reloq_language_rules_render(0x10000,text,length,tags,native_volume,0,
        settings,indices,index_count,pitches,pitch_count,session,sink,index_sink,context);
}
