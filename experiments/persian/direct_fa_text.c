/* Complete-word Persian text probe, not a production NVDA frontend. */
#include "direct_fa_text.h"
#include "direct_fa_lexicon.h"
#include "direct_fa_normalize.h"
#include "direct_fa_numbers.h"
#include "direct_fa_characters.h"
#include "reloq_utf8.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct output_buffer { char *data; size_t used,capacity; };

static int append(struct output_buffer *b,const char *s,size_t n)
{
    if(n>SIZE_MAX-b->used-1)return -1;
    size_t needed=b->used+n+1;
    if(needed>b->capacity) {
        size_t next=b->capacity?b->capacity:128;
        while(next<needed)next=next>SIZE_MAX/2?needed:next*2;
        char *data=realloc(b->data,next);
        if(!data)return -1;
        b->data=data;b->capacity=next;
    }
    memcpy(b->data+b->used,s,n);b->used+=n;b->data[b->used]=0;
    return 0;
}

/* Two independently generated ECI phoneme tags must not touch: the
 * provider otherwise reads their punctuation and spelling as ordinary
 * Finnish. This can occur when a Persian combining mark follows a digit,
 * or when a word and a number are adjacent in the source. */
static int separate_adjacent_tags(struct output_buffer *b)
{
    return b->used && b->data[b->used-1]==']' ? append(b," ",1) : 0;
}

static int word_character(uint32_t c)
{
    /* The lexical lookup makes the final decision. Digits and symbols in
     * this range are therefore rejected instead of being guessed. */
    return (c>=0x600 && c<=0x6ff && c!=0x60c && c!=0x61b && c!=0x61f) || c==0x200c;
}

/* A half-space after the preposition may join any independently pronounceable
 * following word. Preserve independently classified fused lexemes, including
 * derived compounds whose first fused element is followed by a verb stem.
 * The class lookup is evidence about morphology, not an allowlist of phrases. */
static int joined_be_preposition(const char *word,size_t bytes)
{
    static const char prefix[]="به\xe2\x80\x8c";
    static const char be[]="به";
    const size_t prefix_bytes=sizeof prefix-1,be_bytes=sizeof be-1;
    if(bytes<=prefix_bytes || memcmp(word,prefix,prefix_bytes))return 0;
    const char *tail=word+prefix_bytes;
    size_t tail_bytes=bytes-prefix_bytes;
    if(tail_bytes>1024-be_bytes)return 0;
    char reading[2048],fused[1025];
    if(reloq_fa_direct_context_word(tail,tail_bytes,0,reading,sizeof reading)!=1)return 0;
    memcpy(fused,be,be_bytes);memcpy(fused+be_bytes,tail,tail_bytes);
    size_t main_end=0;
    while(main_end<tail_bytes) {
        size_t next=main_end;uint32_t cp;
        if(reloq_utf8_next((const unsigned char *)tail,tail_bytes,&next,&cp)!=1)return 0;
        if(cp==0x200c)break;
        main_end=next;
    }
    if(reloq_fa_direct_word_classes(fused,be_bytes+main_end)&1u)return 0;
    /* A fused lexical prefix followed by a separately classified verb can
     * be the base of a derived word such as به‌روزرسانی. */
    for(size_t at=0;at<main_end;) {
        uint32_t cp;
        if(reloq_utf8_next((const unsigned char *)tail,main_end,&at,&cp)!=1)return 0;
        if(at==main_end)break;
        if((reloq_fa_direct_word_classes(fused,be_bytes+at)&1u) &&
           (reloq_fa_direct_word_classes(tail+at,main_end-at)&2u))return 0;
    }
    return 1;
}

/* Phrase-final function words receive the reference's final stress.  Keep
 * punctuation and line breaks as boundaries; a following word in the same
 * phrase must be separated by spaces or tabs. */
static int follows_word_in_phrase(const char *text,size_t length,size_t at)
{
    int saw_space=0;
    while(at<length && (text[at]==' ' || text[at]=='\t')){++at;saw_space=1;}
    if(!saw_space || at==length)return 0;
    uint32_t cp;size_t next=at;
    if(reloq_utf8_next((const unsigned char *)text,length,&next,&cp)!=1)return 0;
    return word_character(cp) || (cp>='0' && cp<='9') ||
           ((cp=='+' || cp=='-' || cp==0x2212) && next<length &&
            text[next]>='0' && text[next]<='9');
}

/* Spacing is a common spelling of the same verbal prefix otherwise joined
 * with U+200C. Require a recognized following verb before moving stress. */
static size_t following_prefixed_verb(const char *text,size_t length,size_t at)
{
    size_t start=at;
    while(at<length && (text[at]==' ' || text[at]=='\t'))++at;
    if(at==start || at==length)return SIZE_MAX;
    size_t first=at;
    while(at<length) {
        size_t next=at;uint32_t cp;
        if(reloq_utf8_next((const unsigned char *)text,length,&next,&cp)!=1 || !word_character(cp))break;
        at=next;
    }
    return at>first && reloq_fa_direct_is_verb(text+first,at-first)?first:SIZE_MAX;
}

static void prefixed_tag_stress(char *tag,int prefix)
{
    size_t n=strlen(tag),syllable=0;
    if(n<4 || tag[0]!='`' || tag[1]!='[' || tag[n-1]!=']')return;
    for(size_t i=2;i+1<n;++i)
        if(tag[i]==']' || tag[i]=='[' || tag[i]=='`' || tag[i]=='~')return;
    for(size_t i=2;i+1<n;++i) {
        if(tag[i]=='.' && tag[i+1]>='0' && tag[i+1]<='2') {
            tag[i+1]=prefix && !syllable?'1':'0';
            ++syllable;
        }
    }
}

/* An all-zero syllable sequence is a deliberately unstressed component,
 * not a request for the provider to select a new sentence accent there.
 * Pass the native weak-word control as well as the syllable marks. Restrict
 * this to one generated word; explicit user tags are handled elsewhere. */
static int tag_is_weak_word(const char *tag)
{
    if(tag[0]!='`' || tag[1]!='[')return 0;
    int syllables=0;
    for(size_t i=2;tag[i];++i) {
        if(tag[i]==']')return syllables &&
            (tag[i+1]==0 || (tag[i+1]==' ' && tag[i+2]=='`' && tag[i+3]=='['));
        if(tag[i]=='`' || tag[i]=='[' || tag[i]=='~')return 0;
        if(tag[i]=='.' && tag[i+1]>='0' && tag[i+1]<='2') {
            if(tag[i+1]!='0')return 0;
            ++syllables;++i;
        }
    }
    return 0;
}

static int followed_by_dar(const char *text,size_t length,size_t at)
{
    size_t start=at;
    while(at<length && (text[at]==' ' || text[at]=='\t'))++at;
    static const char dar[]="دار";
    if(at==start || length-at<sizeof dar-1 || memcmp(text+at,dar,sizeof dar-1))return 0;
    at+=sizeof dar-1;
    if(at==length)return 1;
    uint32_t cp;size_t next=at;
    return reloq_utf8_next((const unsigned char *)text,length,&next,&cp)==1 && !word_character(cp);
}

/* Restore the native phrase prediction of verified -دار compounds such as
 * the NVDA checkbox state علامت‌دار. Both halves must independently reproduce
 * the entire original phone and stress sequence; otherwise keep one word. */
static void retain_compound_prosody(const char *word,size_t bytes,int follows,
                                    char *tag,size_t capacity)
{
    static const char joiner[]="\xe2\x80\x8c";
    size_t at=0;
    while(at+sizeof joiner-1<=bytes && memcmp(word+at,joiner,sizeof joiner-1))++at;
    if(!at || at+sizeof joiner-1>=bytes)return;
    size_t right_at=at+sizeof joiner-1;
    static const char dar[]="\xd8\xaf\xd8\xa7\xd8\xb1";
    if(bytes-right_at!=sizeof dar-1 || memcmp(word+right_at,dar,sizeof dar-1))return;
    for(size_t i=right_at;i+sizeof joiner-1<=bytes;++i)
        if(!memcmp(word+i,joiner,sizeof joiner-1))return;
    char left[2048],right[2048];
    if(reloq_fa_direct_context_word(word,at,1,left,sizeof left)!=1 ||
       reloq_fa_direct_context_word(word+right_at,bytes-right_at,follows,right,sizeof right)!=1 ||
       !strstr(left,".1") || !strstr(right,".1"))return;
    size_t ln=strlen(left),rn=strlen(right);
    if(ln<4 || rn<4 || left[ln-1]!=']' || right[0]!='`' || right[1]!='[' ||
       strlen(tag)!=ln+rn-3)return;
    /* The compound's newly assigned accent can differ from its separate
     * constituents. Verify every phone, then carry the compound's stress
     * into the equivalent two-tag representation. */
    for(size_t i=0;i<ln-1;++i) {
        if(i && left[i-1]=='.' && left[i]>='0' && left[i]<='2' &&
           tag[i]>='0' && tag[i]<='2')continue;
        if(left[i]!=tag[i])return;
    }
    for(size_t i=2;i<rn;++i) {
        size_t at=ln-1+i-2;
        if(i && right[i-1]=='.' && right[i]>='0' && right[i]<='2' &&
           tag[at]>='0' && tag[at]<='2')continue;
        if(right[i]!=tag[at])return;
    }
    for(size_t i=1;i<ln-1;++i)
        if(left[i-1]=='.' && left[i]>='0' && left[i]<='2')left[i]=tag[i];
    for(size_t i=2;i<rn;++i)
        if(right[i-1]=='.' && right[i]>='0' && right[i]<='2')right[i]=tag[ln-1+i-2];
    if(ln+1+rn>=capacity)return;
    memcpy(tag,left,ln);tag[ln]=' ';
    memcpy(tag+ln+1,right,rn+1);
}

void reloq_fa_direct_free(void *pointer) { free(pointer); }

static int direct_normalized_text(const char *text,size_t length,char **output,
                                  size_t **source_map,size_t *error_offset,int complete)
{
    if(output)*output=NULL;
    if(source_map)*source_map=NULL;
    if(error_offset)*error_offset=0;
    if(!text || !output || !source_map || !error_offset || length==SIZE_MAX ||
       length+1>SIZE_MAX/sizeof(size_t) || memchr(text,0,length))return -1;
    size_t *map=malloc((length+1)*sizeof *map);
    if(!map)return -2;
    struct output_buffer b={0};
    if(append(&b,"",0)){free(map);return -2;}
    int status=0,preceding_word=0;
    size_t at=0,prefixed_verb=SIZE_MAX,previous_start=SIZE_MAX,previous_end=0;
    while(at<length) {
        size_t start=at,next=at;
        uint32_t cp;
        if(reloq_utf8_next((const unsigned char *)text,length,&next,&cp)!=1) {status=-1;break;}
        int sign=(cp=='+' || cp=='-' || cp==0x2212) && next<length &&
                 text[next]>='0' && text[next]<='9';
        if(sign && start && text[start-1]!=' ' && text[start-1]!='\t' &&
           text[start-1]!='\r' && text[start-1]!='\n' && text[start-1]!='(' &&
           text[start-1]!='[' && text[start-1]!='=') {status=2;break;}
        if((cp>='0' && cp<='9') || sign) {
            char tag[2048];size_t count=0;
            size_t number_start=sign?next:start;
            int found=reloq_fa_direct_number(text+number_start,length-number_start,&count,tag,sizeof tag);
            if(found){status=found;break;}
            if(separate_adjacent_tags(&b)){status=-2;break;}
            size_t tag_start=b.used;
            if(sign) {
                const char *name=cp=='+'?"`[.0mos.1bAt] ":"`[.0mAn.1fi] ";
                if(append(&b,name,strlen(name))){status=-2;break;}
            }
            if(append(&b,tag,strlen(tag))){status=-2;break;}
            size_t end=number_start+count;
            for(size_t i=start;i<end;++i)map[i]=tag_start;
            map[end]=b.used;at=end;
            preceding_word=1;
            previous_start=SIZE_MAX;
        } else if(cp==0x066b || cp==0x066c) {
            status=2;break;
        } else if(cp>=0x064e && cp<=0x0650) {
            /* A mark at the start of a token has no base letter. Speak the
             * short character name even when another mark follows; marks
             * attached to a preceding Persian letter remain inside its
             * normal word and are never split here. */
            char tag[128];int kind=0;
            int found=reloq_fa_character_resolve(cp,tag,sizeof tag,&kind);
            if(found!=1 || kind){status=found<0?-1:1;break;}
            if(separate_adjacent_tags(&b)){status=-2;break;}
            size_t tag_start=b.used;
            if(append(&b,tag,strlen(tag))){status=-2;break;}
            for(size_t i=start;i<next;++i)map[i]=tag_start;
            map[next]=b.used;at=next;preceding_word=0;previous_start=SIZE_MAX;
        } else if(word_character(cp)) {
            size_t end=next;
            while(end<length) {
                size_t probe=end;uint32_t c;
                if(reloq_utf8_next((const unsigned char *)text,length,&probe,&c)!=1){at=end;status=-1;break;}
                if(!word_character(c))break;
                end=probe;
            }
            if(status)break;
            /* The documented prepositional expressions take the same weak
             * form and timing as the corresponding spaced phrases. */
            const size_t be_bytes=sizeof "به"-1, join_bytes=3;
            int joined_be=joined_be_preposition(text+start,end-start);
            if(joined_be)end=start+be_bytes;
            char tag[2048];
            int follows=joined_be || follows_word_in_phrase(text,length,end);
            int found=start==prefixed_verb
                ? reloq_fa_direct_prefixed_past(text+start,end-start,tag,sizeof tag) : 0;
            if(found==0)found=reloq_fa_direct_context_word(text+start,end-start,follows,tag,sizeof tag);
            if(found==0 && start==prefixed_verb)
                found=reloq_fa_direct_guess_prefixed_verb(text+start,end-start,tag,sizeof tag);
            if(found==0 && complete)
                found=reloq_fa_direct_guess_word(text+start,end-start,tag,sizeof tag);
            if(found!=1){status=found==0?1:found==-2?-2:-1;break;}
            if(preceding_word && previous_start!=SIZE_MAX && previous_end<start)
                reloq_fa_direct_stress_after_word(text+previous_start,previous_end-previous_start,
                                                  text+start,end-start,tag);
            int weak_component=0;
            /* The object marker and ordinary copula are enclitic even at
             * the end of a phrase. Their isolated citation forms remain
             * accented; punctuation resets preceding_word below. */
            int enclitic=(end-start==sizeof "را"-1 && !memcmp(text+start,"را",end-start)) ||
                         (end-start==sizeof "است"-1 && !memcmp(text+start,"است",end-start));
            if(preceding_word && enclitic) {
                prefixed_tag_stress(tag,0);
                weak_component=1;
            }
            if(end-start==sizeof "علامت"-1 && !memcmp(text+start,"علامت",end-start) &&
               followed_by_dar(text,length,end)) {
                prefixed_tag_stress(tag,0);
                weak_component=1;
            }
            if(start==prefixed_verb) {
                prefixed_tag_stress(tag,0);
                weak_component=1;
                prefixed_verb=SIZE_MAX;
            } else if((end-start==sizeof "می"-1 && !memcmp(text+start,"می",end-start)) ||
                      (end-start==sizeof "نمی"-1 && !memcmp(text+start,"نمی",end-start))) {
                size_t next=following_prefixed_verb(text,length,end);
                if(next!=SIZE_MAX) {
                    prefixed_tag_stress(tag,1);
                    prefixed_verb=next;
                } else if(end-start==sizeof "می"-1 && !memcmp(text+start,"می",end-start)) {
                    /* Independent می can be the noun "wine". A following
                     * non-verb is no reason to suppress its lexical accent. */
                    prefixed_tag_stress(tag,1);
                }
            }
            retain_compound_prosody(text+start,end-start,follows,tag,sizeof tag);
            reloq_fa_direct_normalize_durations(tag);
            if(separate_adjacent_tags(&b)){status=-2;break;}
            size_t tag_start=b.used;
            /* Only intentionally weak forms receive the phrase control.
             * A zero digit copied from a reference entry alone is not
             * evidence that an arbitrary content word is a clitic. */
            if(tag_is_weak_word(tag) &&
               (weak_component || (follows && reloq_fa_direct_is_proclitic(text+start,end-start)) ||
                strstr(tag,"] `[")!=NULL) && append(&b,"`00 ",4)){status=-2;break;}
            if(append(&b,tag,strlen(tag))){status=-2;break;}
            for(size_t i=start;i<end;++i)map[i]=tag_start;
            map[end]=b.used;at=end;
            preceding_word=1;
            previous_start=start;previous_end=end;
            if(joined_be) {
                size_t point=b.used;
                if(append(&b," ",1)){status=-2;break;}
                for(size_t i=end;i<end+join_bytes;++i)map[i]=point;
                at=end+join_bytes;
                map[at]=b.used;
            }
        } else {
            char punctuation=0;
            if(cp==' ' || cp=='\t' || cp=='\r' || cp=='\n' ||
               cp=='.' || cp==',' || cp=='?' || cp=='!' || cp==':' || cp==';' || cp=='-' ||
               cp=='(' || cp==')' || cp=='[' || cp==']' || cp=='{' || cp=='}')
                punctuation=cp=='-'?' ':(char)cp;
            else if(cp==0x60c)punctuation=',';
            else if(cp==0x61b)punctuation=';';
            else if(cp==0x61f)punctuation='?';
            else if(cp==0x00ab)punctuation='(';
            else if(cp==0x00bb)punctuation=')';
            else if(complete) {
                char name[2048];int kind=0;
                int named=reloq_fa_character_resolve(cp,name,sizeof name,&kind);
                if(named==1 && kind)named=reloq_fa_character_code_name(cp,name,sizeof name);
                if(named!=1){status=named<0?-1:1;break;}
                if(b.used && b.data[b.used-1]!=' ' && append(&b," ",1)){status=-2;break;}
                for(size_t i=start;i<next;++i)map[i]=b.used;
                if(append(&b,name,strlen(name))){status=-2;break;}
                map[next]=b.used;at=next;preceding_word=0;previous_start=SIZE_MAX;
                continue;
            } else {status=1;break;}
            size_t point=b.used;
            if(append(&b,&punctuation,1)){status=-2;break;}
            for(size_t i=start;i<next;++i)map[i]=point;
            map[next]=b.used;at=next;
            if(cp!=' ' && cp!='\t'){preceding_word=0;previous_start=SIZE_MAX;}
        }
    }
    if(status) {
        *error_offset=at;free(map);free(b.data);return status;
    }
    map[length]=b.used;*source_map=map;*output=b.data;return 0;
}

static int direct_text_mode(const char *text,size_t length,char **output,
                            size_t **source_map,size_t *error_offset,int complete)
{
    if(output)*output=NULL;
    if(source_map)*source_map=NULL;
    if(error_offset)*error_offset=0;
    if(!text || !output || !source_map || !error_offset || length==SIZE_MAX ||
       length+1>SIZE_MAX/sizeof(size_t) || memchr(text,0,length))return -1;
    /* Validate UTF-8 before normalizing. Numeric separators remain distinct
     * through text normalization so they cannot become clause commas. */
    size_t at=0;
    while(at<length) {
        size_t start=at;uint32_t cp;
        if(reloq_utf8_next((const unsigned char *)text,length,&at,&cp)!=1) {
            *error_offset=start;return -1;
        }
    }
    char *normalized=NULL,*tagged=NULL;
    size_t *source_to_normalized=NULL,*normalized_to_tagged=NULL,offset=0;
    int status=reloq_fa_direct_normalize_text(text,length,&normalized,&source_to_normalized,&offset);
    if(status){*error_offset=offset;return status;}
    /* A source containing only deleted tatweel is a symbol-only request,
     * not a successful silent utterance. Defer its spoken name to the
     * eventual symbol-processing path. Within a word it remains removable. */
    if(normalized[strspn(normalized," \t\r\n")]==0) {
        size_t first=0;
        while(first<length && (text[first]==' ' || text[first]=='\t' ||
                               text[first]=='\r' || text[first]=='\n'))++first;
        if(first<length) {
            *error_offset=first;free(source_to_normalized);free(normalized);return 1;
        }
    }
    status=direct_normalized_text(normalized,strlen(normalized),&tagged,&normalized_to_tagged,&offset,complete);
    if(status) {
        size_t original=0;
        while(original<length && source_to_normalized[original]<offset)++original;
        *error_offset=original;free(source_to_normalized);free(normalized);return status;
    }
    for(size_t i=0;i<=length;++i)
        source_to_normalized[i]=normalized_to_tagged[source_to_normalized[i]];
    free(normalized_to_tagged);free(normalized);
    *output=tagged;*source_map=source_to_normalized;return 0;
}

int reloq_fa_direct_text(const char *text,size_t length,char **output,
                         size_t **source_map,size_t *error_offset)
{return direct_text_mode(text,length,output,source_map,error_offset,0);}

int reloq_fa_direct_text_complete(const char *text,size_t length,char **output,
                                  size_t **source_map,size_t *error_offset)
{return direct_text_mode(text,length,output,source_map,error_offset,1);}

static int direct_project_mode(const char *text,size_t length,
    const size_t *source_offsets,const unsigned char *is_index,size_t event_count,
    char **output,size_t *target_offsets,size_t *error_offset,int complete)
{
    if(output)*output=NULL;
    if(error_offset)*error_offset=0;
    if(!output || !error_offset || (event_count && (!source_offsets || !is_index || !target_offsets)) ||
       event_count>SIZE_MAX/sizeof(size_t))return -1;
    size_t *map=NULL,*mapped=event_count?malloc(event_count*sizeof *mapped):NULL;
    if(event_count&&!mapped)return -2;
    char *tagged=NULL;
    int status=direct_text_mode(text,length,&tagged,&map,error_offset,complete);
    if(status){free(mapped);return status;}
    size_t tagged_length=strlen(tagged),previous=0,cursor=0,tag_start=0,tag_end=0;
    if(map[length]!=tagged_length){status=-1;goto done;}
    for(size_t i=0;i<event_count;++i) {
        size_t offset=source_offsets[i];
        if(offset>length || (i && offset<previous) || is_index[i]>1) {status=3;goto done;}
        size_t target=map[offset];
        if(target>tagged_length || (i && target<mapped[i-1])){status=-1;goto done;}
        if(!is_index[i] && offset>0 && offset<length && map[offset]==map[offset-1]) {
            status=3;*error_offset=offset;goto done;
        }
        while(cursor<tagged_length) {
            if(tag_end>target)break;
            while(cursor<tagged_length && !(tagged[cursor]=='`' && cursor+1<tagged_length && tagged[cursor+1]=='['))++cursor;
            if(cursor==tagged_length)break;
            tag_start=cursor;
            while(cursor<tagged_length && tagged[cursor]!=']')++cursor;
            if(cursor==tagged_length){status=-1;goto done;}
            tag_end=++cursor;
        }
        if(tag_start<target && target<tag_end){status=3;*error_offset=offset;goto done;}
        mapped[i]=target;previous=offset;
    }
    if(event_count)memcpy(target_offsets,mapped,event_count*sizeof *mapped);
    *output=tagged;tagged=NULL;
done:
    free(tagged);free(map);free(mapped);return status;
}

int reloq_fa_direct_project(const char *text,size_t length,
    const size_t *source_offsets,const unsigned char *is_index,size_t event_count,
    char **output,size_t *target_offsets,size_t *error_offset)
{return direct_project_mode(text,length,source_offsets,is_index,event_count,output,target_offsets,error_offset,0);}

int reloq_fa_direct_project_complete(const char *text,size_t length,
    const size_t *source_offsets,const unsigned char *is_index,size_t event_count,
    char **output,size_t *target_offsets,size_t *error_offset)
{return direct_project_mode(text,length,source_offsets,is_index,event_count,output,target_offsets,error_offset,1);}
