/* Modern Greek text rules over the shared Castilian acoustic inventory. */
#include "reloq_greek.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "evv_arena.h"
#include "reloq_greek_letters.inc"
#include "reloq_greek_character_accents.inc"
#include "reloq_inline_tags.h"
#define COUNT(a) (sizeof(a)/sizeof *(a))
struct buffer {char *data;size_t length,capacity;};
static int add(struct buffer *b,const char *s,size_t n)
{
    if(n>SIZE_MAX-b->length-1)return -1;
    size_t need=b->length+n+1;
    if(need>b->capacity){size_t cap=b->capacity?b->capacity:256;while(cap<need){if(cap>SIZE_MAX/2){cap=need;break;}cap*=2;}void *p=realloc(b->data,cap);if(!p)return -1;b->data=p;b->capacity=cap;}
    memcpy(b->data+b->length,s,n);b->length+=n;b->data[b->length]=0;return 0;
}
static int say(struct buffer *b,const char *s){return add(b,s,strlen(s));}
static size_t decode(const char *s,uint32_t *cp)
{
    unsigned c=(unsigned char)*s,n=c<128?1:c<224?2:c<240?3:4;
    uint32_t value=n==1?c:c&((1u<<(7-n))-1);
    for(unsigned i=1;i<n;++i){if(!s[i]||((unsigned char)s[i]&0xc0)!=0x80)return 0;value=(value<<6)|((unsigned char)s[i]&63);}
    *cp=value;return n;
}
static struct greek_letter letter(uint32_t cp)
{
    size_t lo=0,hi=COUNT(greek_letters);
    while(lo<hi){size_t m=lo+(hi-lo)/2;if(greek_letters[m].cp<cp)lo=m+1;else hi=m;}
    if(lo<COUNT(greek_letters)&&greek_letters[lo].cp==cp){struct greek_letter v=greek_letters[lo];if(v.base==0x3c2)v.base=0x3c3;return v;}
    return (struct greek_letter){cp,0,0,0};
}
static int vowel(uint32_t cp){return cp==0x3b1||cp==0x3b5||cp==0x3b7||cp==0x3b9||cp==0x3bf||cp==0x3c5||cp==0x3c9;}
static char vowel_sound(uint32_t cp){return cp==0x3b1?'a':cp==0x3b5?'e':cp==0x3bf||cp==0x3c9?'o':'i';}
static int voiceless(uint32_t cp){return cp==0x3ba||cp==0x3c0||cp==0x3c4||cp==0x3c3||cp==0x3be||cp==0x3c8||cp==0x3b8||cp==0x3c6||cp==0x3c7;}
static int front_vowel(const struct greek_letter *g,size_t n,size_t at)
{
    if(at>=n)return 0;
    uint32_t a=g[at].base;
    if(a==0x3b5||a==0x3b7||a==0x3b9||a==0x3c5)return 1;
    return at+1<n&&(a==0x3b1||a==0x3bf)&&
        g[at+1].base==0x3b9&&!g[at+1].diaeresis;
}
struct sound {char value;unsigned char vowel,accent;};
static void phone(struct sound *s,size_t *n,char c,int v,int accent)
{s[(*n)++]=(struct sound){c,(unsigned char)v,(unsigned char)accent};}
/* One internal value represents the velar nasal's two-character SPR name. */
static int sound_text(struct buffer *b,const struct sound *s)
{return s->value=='N'?say(b,"ng"):add(b,&s->value,1);}
static size_t onset(const struct sound *s,size_t first,size_t next)
{
    size_t count=next-first;
    if(count<2)return count;
    char a=s[next-2].value,b=s[next-1].value;
    if(count>=3&&s[next-3].value=='s'&&
       ((strchr("ptk",a)&&b=='r')||(strchr("pk",a)&&b=='l')))return 3;
    if((b=='r'||b=='l')&&strchr("pbtdkgfvBTD",a))return 2;
    if(a=='s'&&strchr("tkpfmn",b))return 2;
    return 1;
}
static int weak_word(const struct greek_letter *g,size_t n)
{
    static const char *const words[]={"και","να","στο","στα","στη","στην","στον","στους","στις","τον","την","το","του","της","τους","τις","τα","των","μου","σου","μας","σας","ο","η","οι","σε","με","για","θα","δεν","μη","μην","που","ας"};
    for(size_t w=0;w<COUNT(words);++w){size_t j=0,i=0;while(words[w][i]&&j<n){uint32_t cp;size_t k=decode(words[w]+i,&cp);if(!k||letter(cp).base!=g[j].base)break;i+=k;++j;}if(j==n&&!words[w][i])return 1;}
    return 0;
}
static const char *character_name(uint32_t cp);
static int greek_words(struct buffer *out,const char *text);
static int greek_word(struct buffer *out,const char *text,size_t bytes,int context)
{
    if(bytes>SIZE_MAX/sizeof(struct greek_letter)-1)return -1;
    struct greek_letter *g=malloc((bytes+1)*sizeof *g);
    struct sound *s=malloc((bytes*2+4)*sizeof *s);
    if(!g||!s){free(g);free(s);return -1;}
    size_t n=0,m=0;int explicit_accent=0;
    for(size_t i=0;i<bytes;){uint32_t cp;size_t z=decode(text+i,&cp);if(!z){free(g);free(s);return -1;}g[n]=letter(cp);explicit_accent|=g[n].accent;++n;i+=z;}
    for(size_t i=0;i<n;++i){uint32_t a=g[i].base,b=i+1<n?g[i+1].base:0;int accent=g[i].accent;
        if(a==b&&!vowel(a)&&a!=0x3b3)continue;
        if(vowel(a)){
            if(i+1<n&&!g[i+1].diaeresis){
                if((a==0x3b1&&b==0x3b9)||(a==0x3b5&&b==0x3b9)||(a==0x3bf&&b==0x3b9)||(a==0x3c5&&b==0x3b9)||(a==0x3bf&&b==0x3c5)){
                    char c=a==0x3b1?'e':a==0x3bf&&b==0x3c5?'u':'i';phone(s,&m,c,1,accent||g[++i].accent);continue;
                }
                if((a==0x3b1||a==0x3b5||a==0x3b7)&&b==0x3c5){phone(s,&m,vowel_sound(a),1,accent||g[i+1].accent);phone(s,&m,voiceless(i+2<n?g[i+2].base:0)?'f':'B',0,0);++i;continue;}
            }
            phone(s,&m,vowel_sound(a),1,accent);continue;
        }
        if(a==0x3bc&&b==0x3c0){if(i)phone(s,&m,'m',0,0);phone(s,&m,'b',0,0);++i;continue;}
        if(a==0x3bd&&b==0x3c4){if(i)phone(s,&m,'n',0,0);phone(s,&m,'d',0,0);++i;continue;}
        if(a==0x3b3&&(b==0x3b3||b==0x3ba||b==0x3c7||b==0x3be)){
            if(i||b!=0x3ba)phone(s,&m,b==0x3ba?'n':'N',0,0);
            phone(s,&m,b==0x3c7?'j':b==0x3be?'k':b==0x3ba?'g':
                front_vowel(g,n,i+2)?'Y':i+2<n&&vowel(g[i+2].base)?'G':'g',0,0);
            if(b==0x3be)phone(s,&m,'s',0,0);++i;continue;
        }
        if(a==0x3c4&&(b==0x3c3||b==0x3b6)){phone(s,&m,'t',0,0);phone(s,&m,b==0x3b6?'z':'s',0,0);++i;continue;}
        if(a==0x3be){phone(s,&m,'k',0,0);phone(s,&m,'s',0,0);continue;}
        if(a==0x3c8){phone(s,&m,'p',0,0);phone(s,&m,'s',0,0);continue;}
        char c=0;
        switch(a){case 0x3b2:c='B';break;case 0x3b3:c=front_vowel(g,n,i+1)?'Y':'G';break;case 0x3b4:c='D';break;case 0x3b6:c='z';break;case 0x3b8:c='T';break;case 0x3ba:c='k';break;case 0x3bb:c='l';break;case 0x3bc:c='m';break;case 0x3bd:c='n';break;case 0x3c0:c='p';break;case 0x3c1:c=i?'r':'R';break;case 0x3c3:c='s';break;case 0x3c4:c='t';break;case 0x3c6:c='f';break;case 0x3c7:c='j';break;default:break;}
        if(c)phone(s,&m,c,0,0);
    }
    /* Before another vowel, gamma plus an unaccented iota spelling is
       one palatal onset (για, γεια, γιο-, γιου-), rather than an extra
       unstressed syllable. The native phoneme reader requires a nucleus
       in every multi-syllable word. Keep this phonotactic distinction
       before choosing stress, including for weak words in a sentence. */
    for(size_t j=1;j+1<m;++j)
        if(s[j-1].value=='Y'&&s[j].value=='i'&&s[j].vowel&&
           !s[j].accent&&s[j+1].vowel) {
            memmove(s+j,s+j+1,(m-j-1)*sizeof *s);--m;
        }
    size_t nuclei=0;for(size_t j=0;j<m;++j)nuclei+=s[j].vowel;
    /* A consonant-only initialism has letter names, not a vowel-less
       phoneme group that the acoustic parser would read as literal tags. */
    if(!nuclei) {
        int error=0;
        for(size_t j=0;j<n&&!error;++j) {
            const char *name=character_name(g[j].base);
            if(name)error=greek_words(out,name);
        }
        free(g);free(s);return error;
    }
    int unstressed=context&&!explicit_accent&&weak_word(g,n);
    if(!explicit_accent&&!unstressed&&nuclei){size_t target=nuclei>1?nuclei-2:0,k=0;for(size_t j=0;j<m;++j)if(s[j].vowel&&k++==target){s[j].accent=1;break;}}
    int error=say(out," `[");size_t begin=0;
    for(size_t j=0;j<m&&!error;++j)if(s[j].vowel){
        if(j>begin){/* consonants already belong to this syllable */}
        error=say(out,s[j].accent?".1":".0");
        for(size_t k=begin;k<=j&&!error;++k)error=sound_text(out,s+k);
        size_t next=j+1;while(next<m&&!s[next].vowel)++next;
        size_t tail=next==m?m:next-onset(s,j+1,next);
        for(size_t k=j+1;k<tail&&!error;++k)error=sound_text(out,s+k);
        begin=tail;j=tail?tail-1:j;
    }
    if(!nuclei)for(size_t j=0;j<m&&!error;++j)error=sound_text(out,s+j);
    if(!error)error=say(out,"] ");free(g);free(s);return error;
}
static int greek_words(struct buffer *out,const char *text)
{
    for(size_t i=0;text[i];){uint32_t c;size_t z=decode(text+i,&c);if(!z)return -1;
        if(letter(c).base){size_t end=i+z;while(text[end]){uint32_t v;size_t k=decode(text+end,&v);if(!k||!letter(v).base)break;end+=k;}if(greek_word(out,text+i,end-i,0))return -1;i=end;}
        else {if(add(out,text+i,z))return -1;i+=z;}}
    return 0;
}
static int cardinal(struct buffer *,uint64_t);
static int feminine(struct buffer *out,unsigned n)
{
    static const char *const hundreds[]={"","εκατό","διακόσιες","τριακόσιες","τετρακόσιες","πεντακόσιες","εξακόσιες","επτακόσιες","οκτακόσιες","εννιακόσιες"};
    if(n>=100){if(greek_words(out,n/100==1&&n%100?"εκατόν":hundreds[n/100]))return -1;n%=100;}
    if(!n)return 0;
    if(n>=20){if(cardinal(out,n/10*10))return -1;n%=10;}
    if(n==1)return greek_words(out,"μία");
    if(n==3)return greek_words(out,"τρεις");
    if(n==4)return greek_words(out,"τέσσερις");
    return n?cardinal(out,n):0;
}
static int cardinal(struct buffer *out,uint64_t n)
{
    static const char *const small[]={"μηδέν","ένα","δύο","τρία","τέσσερα","πέντε","έξι","επτά","οκτώ","εννέα","δέκα","έντεκα","δώδεκα","δεκατρία","δεκατέσσερα","δεκαπέντε","δεκαέξι","δεκαεπτά","δεκαοκτώ","δεκαεννέα"};
    static const char *const tens[]={"","","είκοσι","τριάντα","σαράντα","πενήντα","εξήντα","εβδομήντα","ογδόντα","ενενήντα"};
    static const char *const hundreds[]={"","εκατό","διακόσια","τριακόσια","τετρακόσια","πεντακόσια","εξακόσια","επτακόσια","οκτακόσια","εννιακόσια"};
    if(n<20)return greek_words(out,small[n]);
    if(n<100)return greek_words(out,tens[n/10])||(n%10&&cardinal(out,n%10));
    if(n<1000)return greek_words(out,n/100==1&&n%100?"εκατόν":hundreds[n/100])||(n%100&&cardinal(out,n%100));
    const uint64_t units[]={1000000000000000000ULL,1000000000000000ULL,1000000000000ULL,1000000000,1000000,1000};
    const char *const one[]={"ένα πεντάκις εκατομμύριο","ένα τετράκις εκατομμύριο","ένα τρισεκατομμύριο","ένα δισεκατομμύριο","ένα εκατομμύριο","χίλια"};
    const char *const many[]={"πεντάκις εκατομμύρια","τετράκις εκατομμύρια","τρισεκατομμύρια","δισεκατομμύρια","εκατομμύρια","χιλιάδες"};
    for(size_t i=0;i<COUNT(units);++i)if(n>=units[i])return (n/units[i]==1?greek_words(out,one[i]):(i==5?feminine(out,(unsigned)(n/units[i])):cardinal(out,n/units[i]))||greek_words(out,many[i]))||(n%units[i]&&cardinal(out,n%units[i]));
    return -1;
}
static int latin_vowel(char c){return c=='a'||c=='e'||c=='i'||c=='o'||c=='u'||c=='y';}
static char lower_ascii(char c){return c>='A'&&c<='Z'?(char)(c+32):c;}
/* A flap retains the spelling's first eligible dental stop. This cursor
   is independent of the vowel alignment: an earlier written d can supply
   the later spoken flap even when a stressed syllable has intervened. */
static char written_stop(const char *word,size_t *cursor)
{
    size_t n=strlen(word);
    for(size_t p=*cursor;p<n;++p) {
        char c=lower_ascii(word[p]);
        if(c!='d'&&c!='t')continue;
        size_t end=p+1;
        while(end<n&&lower_ascii(word[end])==c)++end;
        if(p&&end<n&&strchr("aeiouyrnl",lower_ascii(word[p-1]))&&
           strchr("aeiouyl",lower_ascii(word[end]))) {
            *cursor=end;return c;
        }
    }
    *cursor=n;return 'd';
}
struct written_vowel {size_t at;char value;};
static size_t written_vowels(const char *word,size_t wanted,struct written_vowel *out)
{
    size_t n=0;
    for(size_t i=0;word[i];++i) {
        char c=lower_ascii(word[i]);
        if(!latin_vowel(c)||(c=='u'&&i&&lower_ascii(word[i-1])=='q'))continue;
        out[n++]=(struct written_vowel){i,c=='y'?'i':c};
    }
    /* Retain hiatus when the pronunciation has separate nuclei. Collapse
       written vowel pairs only when spelling has more vowels than speech. */
    for(size_t i=0;i+1<n&&n>wanted;) {
        char a=lower_ascii(word[out[i].at]),b=lower_ascii(word[out[i+1].at]);
        int pair=out[i+1].at==out[i].at+1&&
            ((a=='a'&&strchr("iuyw",b))||(a=='e'&&strchr("aeiy",b))||
             (a=='i'&&b=='e')||(a=='o'&&strchr("aeiouyw",b))||
             (a=='u'&&strchr("ei",b)));
        if(pair){memmove(out+i+1,out+i+2,(n-i-2)*sizeof *out);--n;}
        else ++i;
    }
    if(n>wanted&&n&&out[n-1].value=='e'&&!word[out[n-1].at+1])--n;
    return n;
}
static int latin(struct buffer *out,const char *word,reloq_greek_english english,void *context)
{
    char phonemes[16384];if(english(context,word,phonemes,sizeof phonemes))return -1;
    size_t wanted=0;
    for(size_t i=0;phonemes[i];++i)if(phonemes[i]=='[') {
        while(phonemes[++i]&&phonemes[i]!=']')wanted+=strchr("aAeEiIoOuUxXHYWc",phonemes[i])!=NULL;
        if(!phonemes[i])break;
    }
    size_t length=strlen(word);
    if(length>SIZE_MAX/sizeof(struct written_vowel)-1)return -1;
    struct written_vowel *vowels=malloc((length+1)*sizeof *vowels);
    if(!vowels)return -1;
    size_t count=written_vowels(word,wanted,vowels),position=0,stop=0;int found=0;
    for(size_t i=0;phonemes[i];++i){if(phonemes[i]!='[')continue;++i;found=1;if(say(out," `[") )goto error;
        /* Castilian SPR has primary/unstressed syllables. English's
           secondary stress marker is not part of that grammar: a single
           .2 invalidates the whole group and makes it spoken as text. */
        size_t end=i;int primary=0;
        while(phonemes[end]&&phonemes[end]!=']') {
            if(phonemes[end]=='.'&&phonemes[end+1]=='1')primary=1;
            ++end;
        }
        while(phonemes[i]&&phonemes[i]!=']'){
            char c=phonemes[i++],mapped[4]={0};
            if(c=='.'||(c>='0'&&c<='2')){
                if(c=='2'){c=primary?'0':'1';primary=1;}
                if(add(out,&c,1))goto error;continue;
            }
            char written=0;
            if(strchr("aAeEiIoOuUxXHYWc",c)&&position<count)
                written=vowels[position++].value;
            switch(c){case 'A':mapped[0]='a';break;case 'E':mapped[0]='e';break;case 'I':mapped[0]='i';break;case 'O':strcpy(mapped,"oy");break;case 'c':mapped[0]='o';break;case 'U':mapped[0]='u';break;case 'H':mapped[0]='a';break;case 'a':mapped[0]=written=='o'?'o':'a';break;case 'e':strcpy(mapped,"ey");break;case 'o':strcpy(mapped,"ow");break;case 'x':case 'X':mapped[0]=written=='a'?'a':written=='i'?'i':written=='o'?'o':written=='u'?'u':'e';break;case 'Y':strcpy(mapped,"ay");break;case 'W':strcpy(mapped,"aw");break;case 'S':mapped[0]='s';break;case 'Z':mapped[0]='z';break;case 'C':strcpy(mapped,"ts");break;case 'J':strcpy(mapped,"dz");break;case 'T':mapped[0]='T';break;case 'D':mapped[0]='D';break;case 'G':strcpy(mapped,"ng");break;case 'R':strcpy(mapped,"er");break;case 'F':mapped[0]=written_stop(word,&stop);break;case '?':break;case 'N':mapped[0]='n';break;default:if(strchr("bdfghjklmnprstuvwyzeiou",c))mapped[0]=c=='v'?'B':c=='h'?'j':c;break;}
            if(*mapped&&say(out,mapped))goto error;
        }
        if(say(out,"] "))goto error;if(!phonemes[i])break;
    }
    free(vowels);return found?0:say(out," ");
error:
    free(vowels);return -1;
}
static int latin_character(struct buffer *out,char value,int spelling,
    reloq_greek_english english,void *context)
{
    /* Character navigation uses the same English-to-Greek phone mapping
       as an isolated Latin letter in ordinary text. Punctuation and Greek
       accent names remain owned by the native character grammar. */
    if(!english)return add(out,&value,1);
    char word[2]={value,0};
    return say(out," `ts0 ")||latin(out,word,english,context)||
        say(out,spelling==1?" `ts1 ":" `ts2 ");
}
static const char *character_name(uint32_t cp)
{
    static const char *const names[]={"άλφα","βήτα","γάμμα","δέλτα","έψιλον","ζήτα","ήτα","θήτα","γιώτα","κάππα","λάμδα","μι","νι","ξι","όμικρον","πι","ρο","σίγμα","σίγμα","ταυ","ύψιλον","φι","χι","ψι","ωμέγα"};
    struct greek_letter g=letter(cp);if(g.base>=0x3b1&&g.base<=0x3c9)return names[g.base-0x3b1];
    switch(cp){case 0x384:return "τόνος";case 0x385:return "διαλυτικά τόνος";case '.':return "τελεία";case ',':return "κόμμα";case '?':case ';':case 0x37e:return "ερωτηματικό";case '!':return "θαυμαστικό";case ':':return "άνω και κάτω τελεία";case 0xb7:case 0x387:return "άνω τελεία";case '(':return "ανοιχτή παρένθεση";case ')':return "κλειστή παρένθεση";case '[':return "ανοιχτή αγκύλη";case ']':return "κλειστή αγκύλη";case '{':return "ανοιχτό άγκιστρο";case '}':return "κλειστό άγκιστρο";case '@':return "παπάκι";case '+':return "συν";case '-':case 0x2010:case 0x2011:case 0x2012:case 0x2013:case 0x2014:case 0x2015:return "παύλα";case 0x2026:return "αποσιωπητικά";case 0x2212:return "μείον";case '_':return "κάτω παύλα";case '=':return "ίσον";case '/':return "κάθετος";case '\\':return "ανάποδη κάθετος";case '%':return "τοις εκατό";case '#':return "δίεση";case ' ':return "διάστημα";case '`':return "βαρεία";case '"':return "εισαγωγικά";case '\'':return "απόστροφος";case '<':return "μικρότερο από";case '>':return "μεγαλύτερο από";default:return NULL;}
}
/* Preserve punctuation as syntax in running text. Unicode typography uses
   the same native phrase rules as its ordinary punctuation counterpart. */
static int western_symbol(uint32_t cp)
{
    switch(cp) {
    case 0xa1:case 0xa2:case 0xa3:case 0xa4:case 0xa5:case 0xa6:
    case 0xa7:case 0xa8:case 0xa9:case 0xaa:case 0xac:case 0xad:case 0xae:case 0xaf:
    case 0xb0:case 0xb1:case 0xb2:case 0xb3:case 0xb4:case 0xb5:case 0xb6:
    case 0xb8:case 0xb9:case 0xba:case 0xbc:case 0xbd:case 0xbe:case 0xbf:
    case 0xab:case 0xbb:case 0xb7:case 0xd7:case 0xf7:case 0x2dc:
    case 0x2013:case 0x2014:case 0x201c:case 0x201d:case 0x2026:
    case 0x2022:case 0x20ac:case 0x2122:return 1;
    default:return 0;
    }
}
static int western_character(struct buffer *out,uint32_t cp)
{
    unsigned char value=(unsigned char)cp;
    switch(cp){case 0x20ac:value=0x80;break;case 0x2022:value=0x95;break;case 0x2122:value=0x99;break;case 0x2dc:value=0x98;break;case 0x2013:value=0x96;break;case 0x2014:value=0x97;break;case 0x201c:value=0x93;break;case 0x201d:value=0x94;break;case 0x2026:value=0x85;break;}
    return add(out,(const char *)&value,1);
}
static const char *punctuation(uint32_t cp)
{
    switch(cp) {
    case 0x37e:return "?";
    case 0xb7:case 0x387:return ";";
    case 0xab:case 0xbb:case 0x201c:case 0x201d:case 0x201e:case 0x201f:return "\"";
    case 0x2018:case 0x2019:case 0x201a:case 0x201b:return "'";
    case 0x2012:case 0x2013:case 0x2014:case 0x2015:return ",";
    case 0x2010:case 0x2011:case 0x2212:return "-";
    case 0x2026:return "...";
    case 0xff08:return "(";case 0xff09:return ")";
    case 0xff3b:return "[";case 0xff3d:return "]";
    case 0xff5b:return "{";case 0xff5d:return "}";
    case 0xa0:case 0x1680:case 0x202f:case 0x205f:case 0x3000:return " ";
    case 0x200b:case 0x200c:case 0x200d:case 0x2060:case 0xfeff:return "";
    default:return cp>=0x2000&&cp<=0x200a?" ":NULL;
    }
}
int reloq_greek_text(const char *text,int annotations,reloq_greek_english english,void *context,char **output,size_t **offsets)
{
    size_t length=strlen(text);if(length>SIZE_MAX/sizeof(size_t)-1)return -1;
    size_t *map=malloc((length+1)*sizeof *map);if(!map)return -1;
    struct buffer b={0};int spelling=0,previous=0,failed=0;
    for(size_t i=0;i<length&&!failed;){size_t begin=i,start=b.length;int expanded=0;uint32_t cp;size_t z=decode(text+i,&cp);if(!z){failed=1;break;}
        if(cp>=0xff01&&cp<=0xff5e&&
           strchr("!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~",(int)(cp-0xfee0)))cp-=0xfee0;
        if(cp=='`'&&annotations&&z==1){
            struct reloq_inline_tag tag=reloq_inline_tag_read((const unsigned char *)text+i,length-i,0);
            if(tag.kind==RELOQ_TAG_SPELL){spelling=tag.value;failed=say(&b,spelling==2?" `ts2 ":spelling==1?" `ts1 ":" `ts0 ");i+=tag.length;}
            else {size_t end=i+1;if(text[end]=='['){while(text[end]&&text[end]!=']')++end;if(text[end])++end;}else while(text[end]&&(unsigned char)text[end]>32&&(unsigned char)text[end]<128)++end;failed=add(&b,text+i,end-i);i=end;}
        } else if(spelling){
            if((cp>='A'&&cp<='Z')||(cp>='a'&&cp<='z'))
                failed=latin_character(&b,(char)cp,spelling,english,context);
            else if(cp<128){char value=(char)cp;failed=add(&b,&value,1);}
            else {
                const char *mark=punctuation(cp);
                const char *name=character_name(cp);
                if(western_symbol(cp))failed=western_character(&b,cp);
                else {
                    size_t lo=0,hi=COUNT(greek_character_accents);
                    while(lo<hi){size_t mid=lo+(hi-lo)/2;if(greek_character_accents[mid].cp<cp)lo=mid+1;else hi=mid;}
                    const struct greek_character_accent *accent=
                        lo<COUNT(greek_character_accents)&&greek_character_accents[lo].cp==cp?
                        greek_character_accents+lo:NULL;
                    if(name||accent) {
                        /* Western base letters still use the Greek native
                           character table; only their accent name is added. */
                        if(!name) {
                            char base=(char)accent->base;
                            failed=say(&b," ")||latin_character(&b,base,spelling,english,context)||say(&b," ");
                        }
                        if(!failed)failed=say(&b," `ts0 ");
                        if(!failed&&name)failed=greek_words(&b,name);
                        if(!failed&&accent)failed=greek_words(&b,accent->names);
                        if(!failed)failed=say(&b,spelling==1?" `ts1 ":" `ts2 ");
                    }
                    else if(mark)failed=say(&b,mark);
                    else if(cp>=0xa0&&cp<=0xff)failed=western_character(&b,cp);
                }
            }
            i+=z;previous=0;
        }
        else if(letter(cp).base){size_t end=i+z;while(text[end]){uint32_t next;size_t k=decode(text+end,&next);if(!k||!letter(next).base)break;end+=k;}size_t after=end;while(text[after]==' '||text[after]=='\t')++after;uint32_t next=0;if(text[after])decode(text+after,&next);failed=greek_word(&b,text+i,end-i,previous||letter(next).base!=0);previous=1;i=end;expanded=1;}
        else if(cp>='0'&&cp<='9'){
            uint64_t number=0;size_t end=i;int overflow=0;while(text[end]>='0'&&text[end]<='9'){unsigned digit=text[end]-'0';if(number>(UINT64_MAX-digit)/10)overflow=1;else number=number*10+digit;++end;}
            if(overflow){for(size_t j=i;j<end&&!failed;++j)failed=cardinal(&b,text[j]-'0');}else failed=cardinal(&b,number);i=end;previous=0;expanded=1;
        } else if((cp>='A'&&cp<='Z')||(cp>='a'&&cp<='z')){
            size_t end=i+1;while((text[end]>='A'&&text[end]<='Z')||(text[end]>='a'&&text[end]<='z')||text[end]=='\'')++end;
            char *word=malloc(end-i+1);if(!word){failed=1;break;}memcpy(word,text+i,end-i);word[end-i]=0;failed=latin(&b,word,english,context);free(word);i=end;previous=0;expanded=1;
        } else if(cp=='\\') {
            /* Internal SPR must not become an escaped literal after a
               caller's backslash. Preserve explicit caller annotations;
               quote literal backslashes when annotations are disabled. */
            size_t end=i+1;while(text[end]=='\\')++end;
            uint32_t next=0;if(text[end])decode(text+end,&next);
            int generated=letter(next).base || (next>='0'&&next<='9') ||
                (next>='A'&&next<='Z') || (next>='a'&&next<='z');
            size_t count=end-i;
            if(!annotations)count*=2;
            else if((generated||!text[end])&&(count&1))++count;
            for(size_t j=0;j<count&&!failed;++j)failed=say(&b,"\\");
            previous=0;i=end;
        } else {const char *mark=punctuation(cp);if(western_symbol(cp))failed=western_character(&b,cp);else if(cp==0x387)failed=western_character(&b,0xb7);else if(mark)failed=say(&b,mark);else if(cp=='`')failed=greek_words(&b,"βαρεία");else if(cp<128){char value=(char)cp;failed=add(&b,&value,1);}else if(western_symbol(cp))failed=western_character(&b,cp);else failed=say(&b," ");if(cp!=' '&&cp!='\t')previous=0;i+=z;}
        /* The spelling rules add separators for generated phrases. Their
           outer padding must not change the caller's punctuation adjacency:
           "(word)" and "( word )" take different native phrase paths. */
        if(expanded&&!failed) {
            size_t first=start;
            while(first<b.length&&b.data[first]==' ')++first;
            if(first>start){memmove(b.data+start,b.data+first,b.length-first);b.length-=first-start;}
            while(b.length>start&&b.data[b.length-1]==' ')--b.length;
            b.data[b.length]=0;
        }
        map[begin]=start;for(size_t j=begin+1;j<=i;++j)map[j]=b.length;
    }
    if(failed||add(&b,"",0)){free(map);free(b.data);return -1;}
    map[length]=b.length;*output=b.data;*offsets=map;return 0;
}
