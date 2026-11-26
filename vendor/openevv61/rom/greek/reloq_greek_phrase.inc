/* A sentence-level English analysis, split back into the source words so
 * the Greek frontend keeps its own character map and sound inventory. */
enum { LATIN_PHRASE_WORDS=128, LATIN_PHRASE_GROUPS=512,
       LATIN_PHRASE_BYTES=2048, LATIN_PHONEMES=16384 };
struct latin_phrase_group {size_t begin,end;char control[4];};
struct latin_phrase_word {size_t begin,end,first,count;};
struct latin_phrase {
    size_t end,words,groups;
    int valid;
    struct latin_phrase_word word[LATIN_PHRASE_WORDS];
    struct latin_phrase_group group[LATIN_PHRASE_GROUPS];
    char phonemes[LATIN_PHONEMES];
};

static size_t latin_groups(struct latin_phrase *p,const char *s)
{
    size_t n=0,length=strlen(s);char control[4]={0};
    for(size_t at=0;at<length;) {
        if(s[at]!='`'){++at;continue;}
        struct reloq_inline_tag tag=reloq_inline_tag_read((const unsigned char *)s+at,length-at,0);
        if(!tag.length){++at;continue;}
        if(tag.kind==RELOQ_TAG_PROMINENCE) {
            if(tag.length>=sizeof control)return SIZE_MAX;
            memcpy(control,s+at,tag.length);control[tag.length]=0;
        } else if(tag.kind==RELOQ_TAG_SPR) {
            if(p) {
                if(n==LATIN_PHRASE_GROUPS)return SIZE_MAX;
                p->group[n]=(struct latin_phrase_group){at,at+tag.length,{0}};
                memcpy(p->group[n].control,control,sizeof control);
            }
            ++n;control[0]=0;
        }
        at+=tag.length;
    }
    return n;
}

static int latin_phrase_build(struct latin_phrase *p,const char *text,size_t start,
    reloq_greek_english english,void *context)
{
    memset(p,0,sizeof *p);p->end=start+1;
    if(!english)return 0;
    struct buffer phrase={0};size_t at=start;
    while(text[at] && phrase.length<LATIN_PHRASE_BYTES) {
        uint32_t cp;size_t z=decode(text+at,&cp);if(!z)break;
        if(reloq61_latin_byte(cp)) {
            if(p->words==LATIN_PHRASE_WORDS)break;
            size_t begin=at;
            do {
                int c=reloq61_latin_byte(cp);
                if(!c&&cp!='\'')break;
                char byte=c?(char)c:(char)cp;
                if(add(&phrase,&byte,1))goto fallback;
                at+=z;if(!text[at])break;z=decode(text+at,&cp);
            } while(z);
            p->word[p->words++]=(struct latin_phrase_word){begin,at,0,0};
        } else if(cp>='0'&&cp<='9') {
            /* Keep numbers in the English sentence analysis, while the main
               frontend still renders them through the Greek number rules. */
            if(p->words==LATIN_PHRASE_WORDS)break;
            size_t begin=at;
            while(text[at]>='0'&&text[at]<='9') {
                if(add(&phrase,text+at,1))goto fallback;
                ++at;
            }
            p->word[p->words++]=(struct latin_phrase_word){begin,at,0,0};
        } else if(cp<128 && strchr(" \t\r\n.,!?;:()\"-",(int)cp)) {
            char c=(char)cp;if(add(&phrase,&c,1))goto fallback;at+=z;
        } else break;
    }
    p->end=at;
    if(p->words<2 || !phrase.length || english(context,phrase.data,p->phonemes,sizeof p->phonemes))goto fallback;
    p->groups=latin_groups(p,p->phonemes);
    if(p->groups==SIZE_MAX||!p->groups)goto fallback;
    size_t first=0;
    for(size_t i=0;i<p->words;++i) {
        struct buffer word={0};size_t pos=p->word[i].begin;
        while(pos<p->word[i].end) {
            uint32_t cp;size_t z=decode(text+pos,&cp);if(!z){free(word.data);goto fallback;}
            int c=reloq61_latin_byte(cp);char byte=c?(char)c:(char)cp;
            if(add(&word,&byte,1)){free(word.data);goto fallback;}pos+=z;
        }
        char isolated[LATIN_PHONEMES];
        int status=english(context,word.data,isolated,sizeof isolated);free(word.data);
        if(status)goto fallback;
        size_t count=latin_groups(NULL,isolated);
        if(!count||count==SIZE_MAX||count>p->groups-first)goto fallback;
        p->word[i].first=first;p->word[i].count=count;first+=count;
    }
    if(first!=p->groups)goto fallback;
    p->valid=1;free(phrase.data);return 1;
fallback:
    free(phrase.data);p->valid=0;if(p->end<=start)p->end=start+1;return 0;
}

static int latin_phrase_phones(const struct latin_phrase *p,size_t start,
    struct buffer *out)
{
    if(!p->valid)return 0;
    for(size_t i=0;i<p->words;++i)if(p->word[i].begin==start) {
        const struct latin_phrase_word *w=p->word+i;
        for(size_t j=w->first;j<w->first+w->count;++j) {
            const struct latin_phrase_group *g=p->group+j;
            if((g->control[0]&&(say(out," ")||say(out,g->control)||say(out," ")))||
               add(out,p->phonemes+g->begin,g->end-g->begin))return -1;
        }
        return 1;
    }
    return 0;
}
