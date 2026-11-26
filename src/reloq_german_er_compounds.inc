/* An -e constituent followed by a known r-initial noun can be mistaken for
 * an -er constituent plus a vowel-initial remainder. Check the actual native
 * analyses before supplying a boundary: already-correct words and genuine
 * -er compounds keep their original spelling, stress and dictionaries. */
static unsigned char german_er_lower(unsigned char c)
{
    return ((c>='A'&&c<='Z')||(c>=0xc0&&c<=0xde&&c!=0xd7))?
        (unsigned char)(c+32):c;
}

static int german_er_head(const unsigned char *word,size_t length)
{
    /* Productive noun families; ordinary case and inflection are handled by
     * the same native grammar after the boundary, not by whole-word rules. */
    static const char *const heads[]={"rampe","rand","rate","ratte","rechnung",
        "referenz","regal","regelung","register","regler","reihe","reihenfolge",
        "reise","resultat","riegel","richtung","riss","rohr","r\366hre",
        "rolle","runde","rundgang","r\374ckmeldung","ruhe"};
    static const char *const endings[]={"","e","en","er","ern","es","n","s"};
    for(size_t i=0;i<sizeof heads/sizeof *heads;++i) {
        size_t n=strlen(heads[i]);if(n>length)continue;
        size_t j=0;while(j<n&&german_er_lower(word[j])==(unsigned char)heads[i][j])++j;
        if(j!=n)continue;
        for(size_t k=0;k<sizeof endings/sizeof *endings;++k) {
            size_t m=strlen(endings[k]);if(n+m!=length)continue;
            size_t z=0;while(z<m&&german_er_lower(word[n+z])==(unsigned char)endings[k][z])++z;
            if(z==m)return 1;
        }
    }
    return 0;
}

/* The isolated plans must differ only in exactly the faulty boundary:
 * ...R.2V... becomes ...@.2rV..., with stress and all other phones intact.
 * This rejects already-correct compounds and changes unrelated to the split. */
static int german_er_same_except_boundary(const unsigned char *old,
                                           const unsigned char *joined)
{
    int changed=0;
    while(*old&&*joined) {
        if(!changed&&old[0]=='R'&&joined[0]=='@'&&old[1]=='.'&&joined[1]=='.'&&
           old[2]>='0'&&old[2]<='2'&&old[2]==joined[2]&&joined[3]=='r') {
            old+=3;joined+=4;changed=1;continue;
        }
        if(*old++!=*joined++)return 0;
    }
    return changed&&!*old&&!*joined;
}

static int german_er_possible(const unsigned char *word,size_t length)
{
    if(length<7||length>=254)return 0;
    for(size_t split=3;split+3<length;++split)
        if(german_er_lower(word[split-1])=='e'&&german_er_lower(word[split])=='r'&&
           german_er_head(word+split,length-split))return 1;
    return 0;
}

static size_t german_er_boundary(const unsigned char *word,size_t length)
{
    if(length<7||length>=254)return 0;
    for(size_t split=3;split+3<length;++split) {
        if(german_er_lower(word[split-1])!='e'||german_er_lower(word[split])!='r'||
           !german_er_head(word+split,length-split))continue;
        unsigned char separate[256],original[4096],candidate[4096];size_t a=0,b=0;
        memcpy(separate,word,split);separate[split]='-';
        memcpy(separate+split+1,word+split,length-split);
        if(reloq_rules61_word(0x40000,word,length,original,sizeof original,&a)==1&&
           reloq_rules61_word(0x40000,separate,length+1,candidate,sizeof candidate,&b)==1&&
           german_er_same_except_boundary(original,candidate))return split;
    }
    return 0;
}

static int german_er_compound_input(unsigned char **text,size_t **source,size_t *length,int tags)
{
    const struct reloq_lexicon_resolver *resolver=reloq61_dictionary_resolver();
    const struct reloq_lexicon *overlay=reloq61_dictionary_overlay_current();
    struct reloq_lexicon empty={0};if(!overlay)overlay=&empty;
    const struct reloq_lexicon_resolver *previous=reloq_lexicon_resolver_scope(resolver);
    const unsigned char *a=*text;const size_t *map=*source,n=*length;
    struct german_shared_buffer b={0};int changed=0,spelling=0,status=0;
    for(size_t i=0;i<n;) {
        struct reloq_inline_tag tag=reloq_inline_tag_read(a+i,n-i,0);
        if(tag.length&&(tags||tag.kind==RELOQ_TAG_SPR||tag.kind==RELOQ_TAG_PROMINENCE)) {
            if(tags&&tag.kind==RELOQ_TAG_SPELL)spelling=tag.value;
            if(changed&&german_shared_append(&b,a+i,map+i,tag.length,0)){status=-1;break;}
            i+=tag.length;continue;
        }
        if(spelling||!german_letter(a[i])||(i&&german_word_byte(a[i-1]))) {
            if(changed&&german_shared_append(&b,a+i,map+i,1,0)){status=-1;break;}
            ++i;continue;
        }
        size_t end=i;while(end<n&&german_word_byte(a[end]))++end;
        size_t size=end-i,split=0;int letters=1;
        for(size_t j=i;j<end;++j)if(!german_letter(a[j])){letters=0;break;}
        const struct reloq_lexicon_override *entry=NULL;
        if(letters&&german_er_possible(a+i,size)&&!reloq61_dictionary_exact(a+i,size)&&
           reloq_lexicon_find_override(overlay,a+i,size,&entry)!=1)
            split=german_er_boundary(a+i,size);
        if(split) {
            if(!changed){if(german_shared_append(&b,a,map,i,0)){status=-1;break;}changed=1;}
            if(german_shared_append(&b,a+i,map+i,split,0)||
               german_shared_append(&b,(const unsigned char *)"-",NULL,1,map[i+split])||
               german_shared_append(&b,a+i+split,map+i+split,size-split,0)){status=-1;break;}
        } else if(changed&&german_shared_append(&b,a+i,map+i,size,0)){status=-1;break;}
        i=end;
    }
    reloq_lexicon_resolver_scope(previous);reloq61_dictionary_resolver_release(resolver);
    if(status){free(b.bytes);free(b.map);return -1;}
    if(!changed){free(b.bytes);free(b.map);return 0;}
    if(german_shared_append(&b,(const unsigned char *)"",NULL,0,n)) {
        free(b.bytes);free(b.map);return -1;
    }
    b.bytes[b.used]=0;b.map[b.used]=map[n];free(*text);free(*source);
    *text=b.bytes;*source=b.map;*length=b.used;return 0;
}
