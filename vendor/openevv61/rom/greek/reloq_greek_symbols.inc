/* Expand symbols which the native grammar normally reads as words. This
 * lets their names take the same Greek letter and connected-word path as
 * ordinary text. Sentence punctuation stays with the native grammar after
 * selecting the Greek or English meaning of a semicolon. */
static int greek_symbol_is_spoken(uint32_t cp)
{
    switch(cp) {
    case '@':case '+':case '_':case '=':case '/':case '\\':case '%':
    case '#':case '$':case '*':case '&':case '|':case '^':case '~':
    case '<':case '>':
    case 0x20ac:case 0xa3:case 0xa5:case 0xa7:case 0xb0:case 0xb1:
    case 0xd7:case 0xf7:case 0xa9:case 0xae:case 0x2122:case 0x2022:
    case 0x2030:case 0xa1:case 0xa2:case 0xa4:case 0xa6:case 0xa8:
    case 0xaa:case 0xac:case 0xaf:case 0xb2:case 0xb3:case 0xb4:
    case 0xb6:case 0xb8:case 0xb9:case 0xba:case 0xbc:case 0xbd:
    case 0xbe:case 0xbf:case 0x2dc:return 1;
    default:return 0;
    }
}

/* A Latin title inside Greek text does not end its surrounding sentence. */
static int greek_symbol_title_period(const char *text,size_t at)
{
    size_t first=at;
    while(first&&((text[first-1]>='A'&&text[first-1]<='Z')||
                  (text[first-1]>='a'&&text[first-1]<='z')))--first;
    size_t n=at-first;if(!n||n>4)return 0;
    char word[5];for(size_t i=0;i<n;++i)word[i]=lower_ascii(text[first+i]);word[n]=0;
    if(strcmp(word,"dr")&&strcmp(word,"mr")&&strcmp(word,"mrs")&&
       strcmp(word,"ms")&&strcmp(word,"prof")&&strcmp(word,"st"))return 0;
    size_t next=at+1;while(text[next]==' '||text[next]=='\t')++next;
    uint32_t cp=0;if(!text[next]||!decode(text+next,&cp))return 0;
    return letter(cp).base||reloq61_latin_byte(cp);
}

int reloq_greek_symbol_text(const char *text,int annotations,
    int (*protected_symbol)(void *,uint32_t),void *context,
    int (*valid_spr)(void *,const char *,size_t),void *spr_context,
    char **output,size_t **offsets)
{
    if(!text||!output||!offsets)return -1;
    *output=NULL;*offsets=NULL;
    size_t length=strlen(text);if(length>SIZE_MAX/sizeof(size_t)-1)return -1;
    size_t *map=malloc((length+1)*sizeof *map);if(!map)return -1;
    struct buffer b={0};int spelling=0,changed=0,greek_clause=0,latin_clause=0;
    for(size_t at=0;at<length;) {
        uint32_t cp;size_t z=decode(text+at,&cp),start=b.length;
        if(!z){free(map);free(b.data);return -1;}
        if(cp=='`'&&annotations) {
            struct reloq_inline_tag tag=reloq_inline_tag_read((const unsigned char *)text+at,length-at,0);
            size_t end=at+1;
            if(tag.kind==RELOQ_TAG_SPELL){spelling=tag.value;end=at+tag.length;}
            else if(text[end]=='['){while(text[end]&&text[end]!=']')++end;if(text[end])++end;}
            else while(text[end]&&(unsigned char)text[end]>32&&(unsigned char)text[end]<128)++end;
            size_t slash=at;while(slash&&text[slash-1]=='\\')--slash;
            if(!spelling&&text[at+1]=='['&&valid_spr&&!((at-slash)&1)&&
               !memchr(text+at+1,'`',end-at-1)&&
               valid_spr(spr_context,text+at,end-at)==0) {
                /* The native parser spells a malformed SPR token. Send that
                   spelling through the same Greek character-name frontend
                   as explicit character review instead of its legacy names. */
                if(say(&b,"βαρεία `ts2 "))goto fail;
                map[at]=start;
                for(size_t j=at+1;j<end;++j) {
                    map[j]=b.length;if(add(&b,text+j,1))goto fail;
                }
                if(say(&b," `ts0 "))goto fail;
                at=end;map[at]=b.length;changed=1;continue;
            }
            if(add(&b,text+at,end-at))goto fail;
            for(size_t j=at;j<end;++j)map[j]=start+j-at;
            at=end;map[at]=b.length;continue;
        }
        if(!spelling) {
            /* A semicolon is a Greek question mark in Greek or mixed-script
               text. Keep its English punctuation role in Latin-only text. */
            if(cp==';'&&(greek_clause||!latin_clause)) {
                if(say(&b,"?"))goto fail;
                map[at]=start;map[at+z]=b.length;at+=z;changed=1;
                greek_clause=latin_clause=0;continue;
            }
            if(letter(cp).base)greek_clause=1;
            else if(reloq61_latin_byte(cp))latin_clause=1;
            int decimal=cp=='.'&&at&&text[at-1]>='0'&&text[at-1]<='9'&&
                text[at+z]>='0'&&text[at+z]<='9';
            if((cp=='.'&&!decimal&&!greek_symbol_title_period(text,at))||
               cp=='?'||cp=='!'||cp==';'||
               cp==0x37e||cp=='\n'||cp=='\r')greek_clause=latin_clause=0;
        }
        const char *name=!spelling&&greek_symbol_is_spoken(cp)?character_name(cp):NULL;
        if(name&&protected_symbol&&protected_symbol(context,cp))name=NULL;
        /* The native parser treats <word> as markup-like punctuation, but
         * reads a comparison sign when it is separated or appears alone. */
        if(cp=='<'&&at+z<length&&text[at+z]!=' '&&text[at+z]!='\t'&&
           text[at+z]!='\r'&&text[at+z]!='\n')name=NULL;
        if(cp=='>') {
            size_t p=at;
            while(p&&text[p-1]!=' '&&text[p-1]!='\t'&&text[p-1]!='\r'&&text[p-1]!='\n') {
                if(text[--p]=='<'){name=NULL;break;}
            }
        }
        /* A backslash immediately before a caller's tag is syntax, not a
         * character name. The main frontend retains its escape handling. */
        if(cp=='\\'&&annotations&&at+z<length&&text[at+z]=='`')name=NULL;
        if(name) {
            int before=b.length&&b.data[b.length-1]!=' '&&b.data[b.length-1]!='\t'&&
                b.data[b.length-1]!='\r'&&b.data[b.length-1]!='\n';
            unsigned char next=(unsigned char)text[at+z];
            int after=next&&next!=' '&&next!='\t'&&next!='\r'&&next!='\n'&&
                !strchr(".,:;!?)]}",next);
            if((before&&say(&b," "))||say(&b,name)||(after&&say(&b," ")))goto fail;
            map[at]=start;for(size_t j=1;j<=z;++j)map[at+j]=b.length;
            changed=1;
        } else {
            if(add(&b,text+at,z))goto fail;
            for(size_t j=0;j<z;++j)map[at+j]=start+j;
            map[at+z]=b.length;
        }
        at+=z;
    }
    if(!changed){free(map);free(b.data);return 0;}
    if(add(&b,"",0))goto fail;
    map[length]=b.length;*output=b.data;*offsets=map;return 1;
fail:free(map);free(b.data);return -1;
}
