/* The inflected wachsen stem and Gewächs have /ks/. The ambiguous wachst
 * is corrected only next to ihr, which identifies the plural verb form.
 * Keep the native plan when the alternative changes more than that phone. */
static int german_waechs_phone_only(const unsigned char *old,const unsigned char *candidate)
{
    int changed=0;
    while(*old&&*candidate) {
        if(!changed&&(old[0]=='X'||old[0]=='x')&&candidate[0]=='k'&&old[1]=='s'&&candidate[1]=='s') {
            ++old;++candidate;changed=1;continue;
        }
        if(*old++!=*candidate++)return 0;
    }
    return changed&&!*old&&!*candidate;
}

static int german_wachsen_possible(const unsigned char *text,size_t length)
{
    for(size_t j=0;j+5<=length;++j) {
        unsigned char vowel=german_er_lower(text[j+1]);
        if(german_er_lower(text[j])=='w'&&(vowel==0xe4||vowel=='a')&&
           german_er_lower(text[j+2])=='c'&&german_er_lower(text[j+3])=='h'&&
           german_er_lower(text[j+4])=='s')return 1;
    }
    return 0;
}

static int german_wachsen_subject_tag(enum reloq_inline_kind kind)
{
    switch(kind) {
    case RELOQ_TAG_PROMINENCE:case RELOQ_TAG_PITCH:case RELOQ_TAG_SPEED:
    case RELOQ_TAG_VOLUME:case RELOQ_TAG_INFLECTION:case RELOQ_TAG_GENDER:
    case RELOQ_TAG_HEAD_SIZE:case RELOQ_TAG_ROUGHNESS:case RELOQ_TAG_BREATHINESS:
    case RELOQ_TAG_PRESET:case RELOQ_TAG_WPM:case RELOQ_TAG_F0_GLIDE:return 1;
    default:return 0;
    }
}

static int german_wachsen_ihr(const unsigned char *text,size_t length,size_t begin,size_t end,int tags,int allow_inverted)
{
    size_t at=begin;
    for(unsigned n=0;n<8&&at;++n) {
        while(at&&(text[at-1]==' '||text[at-1]=='\t'))--at;
        size_t stop=at;while(at&&text[at-1]>32)--at;
        if(stop-at==3&&german_er_lower(text[at])=='i'&&
           german_er_lower(text[at+1])=='h'&&german_er_lower(text[at+2])=='r')return 1;
        /* An explicit preceding du wins over a later possessive ihr:
         * "du bewachst ihr Haus" belongs to bewachen, not bewachsen. */
        if(stop-at==2&&german_er_lower(text[at])=='d'&&german_er_lower(text[at+1])=='u')return 0;
        struct reloq_inline_tag tag=reloq_inline_tag_read(text+at,stop-at,0);
        if(!tags||!tag.length||tag.length!=stop-at||!german_wachsen_subject_tag(tag.kind))break;
    }
    /* With a prefix, a following ihr can introduce the object of bewachen,
     * e.g. "bewachst ihr kleines Haus". Only use inversion for bare wachst. */
    if(!allow_inverted)return 0;
    at=end;
    for(unsigned n=0;n<8&&at<length;++n) {
        while(at<length&&(text[at]==' '||text[at]=='\t'))++at;
        if(at+3<=length&&german_er_lower(text[at])=='i'&&
           german_er_lower(text[at+1])=='h'&&german_er_lower(text[at+2])=='r'&&
           (at+3==length||!german_word_byte(text[at+3]))) {
            size_t next=at+3;while(next<length&&(text[next]==' '||text[next]=='\t'))++next;
            unsigned char c=next<length?text[next]:0;
            /* "bewachst ihr Haus" is also ambiguous as a fragment: prefer
             * the unchanged reading when ihr can modify a following noun. */
            if((c>='A'&&c<='Z')||(c>=0xc0&&c<=0xde&&c!=0xd7))return 0;
            return 1;
        }
        struct reloq_inline_tag tag=reloq_inline_tag_read(text+at,length-at,0);
        if(!tags||!tag.length||!german_wachsen_subject_tag(tag.kind))break;
        at+=tag.length;
    }
    return 0;
}

static int german_wachsen_input(unsigned char *text,size_t length,int tags)
{
    if(!german_wachsen_possible(text,length))return 0;
    const struct reloq_lexicon_resolver *resolver=reloq61_dictionary_resolver();
    const struct reloq_lexicon *overlay=reloq61_dictionary_overlay_current();
    struct reloq_lexicon empty={0};if(!overlay)overlay=&empty;
    const struct reloq_lexicon_resolver *previous=reloq_lexicon_resolver_scope(resolver);
    int spelling=0;
    for(size_t i=0;i<length;) {
        struct reloq_inline_tag tag=reloq_inline_tag_read(text+i,length-i,0);
        if(tag.length&&(tags||tag.kind==RELOQ_TAG_SPR||tag.kind==RELOQ_TAG_PROMINENCE)) {
            if(tags&&tag.kind==RELOQ_TAG_SPELL)spelling=tag.value;
            i+=tag.length;continue;
        }
        if(spelling||!german_letter(text[i])||(i&&german_word_byte(text[i-1]))) {++i;continue;}
        size_t end=i;while(end<length&&german_word_byte(text[end]))++end;
        size_t size=end-i;int letters=size<256;
        for(size_t j=i;j<end&&letters;++j)if(!german_letter(text[j]))letters=0;
        const struct reloq_lexicon_override *entry=NULL;
        if(letters&&german_wachsen_possible(text+i,size)&&!reloq61_dictionary_exact(text+i,size)&&
           reloq_lexicon_find_override(overlay,text+i,size,&entry)!=1) {
            for(size_t j=0;j+5<=size;++j) {
                unsigned char *word=text+i;
                unsigned char vowel=german_er_lower(word[j+1]);
                if(german_er_lower(word[j])!='w'||(vowel!=0xe4&&vowel!='a')||
                   german_er_lower(word[j+2])!='c'||german_er_lower(word[j+3])!='h'||
                   german_er_lower(word[j+4])!='s')continue;
                int ends_verb=j+6==size&&german_er_lower(word[j+5])=='t';
                int verb=ends_verb&&(vowel==0xe4||german_wachsen_ihr(text,length,i,end,tags,j==0));
                int noun=vowel==0xe4&&j>=2&&german_er_lower(word[j-2])=='g'&&german_er_lower(word[j-1])=='e';
                if(!verb&&!noun)continue;
                size_t root=noun?j-2:j,root_length=noun?7:6;
                if(reloq61_dictionary_exact(word+root,root_length)||
                   reloq_lexicon_find_override(overlay,word+root,root_length,&entry)==1)continue;
                unsigned char alternative[256],original[4096],candidate[4096];size_t a=0,b=0;
                /* A lowercase k also keeps a fully uppercase known word from
                   being misidentified as a newly spelled-out acronym. */
                memcpy(alternative,word,size);alternative[j+3]='k';
                if(reloq_rules61_word(0x40000,word,size,original,sizeof original,&a)==1&&
                   reloq_rules61_word(0x40000,alternative,size,candidate,sizeof candidate,&b)==1&&
                   german_waechs_phone_only(original,candidate))word[j+3]=alternative[j+3];
                break;
            }
        }
        i=end;
    }
    reloq_lexicon_resolver_scope(previous);reloq61_dictionary_resolver_release(resolver);
    return 0;
}
