/* Native English token/phrase analysis. Development evidence in
 * behavior-0.2.92: punctuation, misc_chars, identify_eng_year and the
 * public phoneme/acoustic API. No runtime reference or external rewriting.
 * 0x1c/1d/1e carry medial question/exclamation/ellipsis phrase classes. */
struct normalized92 {struct text_buffer text;size_t *map,capacity;};
static int normalized_add92(struct normalized92 *b,const void *s,size_t n,
    size_t begin,size_t end)
{
    size_t before=b->text.n;
    if(append(&b->text,s,n))return -2;
    if(b->text.cap>b->capacity) {
        if(b->text.cap>SIZE_MAX/sizeof *b->map)return -2;
        void *p=realloc(b->map,b->text.cap*sizeof *b->map);
        if(!p)return -2;
        b->map=p;b->capacity=b->text.cap;
    }
    for(size_t i=before;i<b->text.n;++i)b->map[i]=begin;
    b->map[b->text.n]=end;return 0;
}
static int digit92(unsigned char c){return c>='0'&&c<='9';}
static int space92(unsigned char c){return c==' '||(c>=9&&c<=13);}
static int text_word92(struct text_buffer *b,const char *s)
{return append(b,s,strlen(s))||append(b," ",1);}
static int cardinal92(struct text_buffer *b,uint64_t value)
{
    static const char *const small[]={"zero","one","two","three","four","five",
        "six","seven","eight","nine","ten","eleven","twelve","thirteen",
        "fourteen","fifteen","sixteen","seventeen","eighteen","nineteen"};
    static const char *const tens[]={"","","twenty","thirty","forty","fifty",
        "sixty","seventy","eighty","ninety"};
    static const struct {uint64_t n;const char *s;} scales[]={
        {UINT64_C(1000000000000),"trillion"},{UINT64_C(1000000000),"billion"},
        {1000000,"million"},{1000,"thousand"},{100,"hundred"}};
    for(size_t i=0;i<sizeof scales/sizeof *scales;++i)if(value>=scales[i].n) {
        if(cardinal92(b,value/scales[i].n)||text_word92(b,scales[i].s))return -2;
        value%=scales[i].n;if(!value)return 0;
    }
    if(value<20)return text_word92(b,small[value]);
    if(text_word92(b,tens[value/10]))return -2;
    return value%10?text_word92(b,small[value%10]):0;
}
static int digits92(struct text_buffer *b,const unsigned char *s,size_t n,int oh)
{
    for(size_t i=0;i<n;++i)if(s[i]=='0'&&oh) {
        if(text_word92(b,"o"))return -2;
    } else if(cardinal92(b,s[i]-'0'))return -2;
    return 0;
}
static int ordinal92(struct text_buffer *b,uint64_t value)
{
    if(cardinal92(b,value))return -2;
    if(b->n&&b->s[b->n-1]==' ')--b->n;
    size_t start=b->n;while(start&&b->s[start-1]!=' ')--start;
    b->s[b->n]=0;const char *last=(char *)b->s+start,*replacement=NULL;
    static const char *const pairs[][2]={{"one","first"},{"two","second"},
        {"three","third"},{"five","fifth"},{"eight","eighth"},
        {"nine","ninth"},{"twelve","twelfth"}};
    for(size_t i=0;i<sizeof pairs/sizeof *pairs;++i)
        if(!strcmp(last,pairs[i][0]))replacement=pairs[i][1];
    if(replacement){b->n=start;return text_word92(b,replacement);}
    if(b->n&&b->s[b->n-1]=='y'){--b->n;return text_word92(b,"ieth");}
    return text_word92(b,"th");
}
static int year92(struct text_buffer *b,uint64_t v)
{
    if(v%1000<10)return cardinal92(b,v);
    if(cardinal92(b,v/100))return -2;
    unsigned rest=(unsigned)(v%100);
    if(!rest)return text_word92(b,"hundred");
    if(rest<10&&text_word92(b,"o"))return -2;
    return cardinal92(b,rest);
}
/* Consume only structurally complete grouped, decimal, money, time, date
 * and ordinal forms. A failed candidate leaves every character available
 * to ordinary token analysis; malformed input cannot swallow an utterance. */
static int number92(const unsigned char *s,size_t n,size_t at,
    size_t *end,struct text_buffer *b)
{
    size_t i=at;int currency=0;
    if(s[i]=='$'){currency=1;++i;}
    else if(i+1<n&&s[i]==0xc2&&s[i+1]==0xa3){currency=2;i+=2;}
    else if(i+2<n&&s[i]==0xe2&&s[i+1]==0x82&&s[i+2]==0xac){currency=3;i+=3;}
    if(i>=n||(!digit92(s[i])&&!(s[i]=='.'&&i+1<n&&digit92(s[i+1]))))return 0;
    size_t start=i;uint64_t whole=0;size_t count=0;
    while(i<n&&digit92(s[i])) {
        if(++count>12)return 0;
        whole=whole*10+s[i++]-'0';
    }
    int grouped=0;
    if(count&&count<=3)while(i+3<n&&s[i]==','&&digit92(s[i+1])&&
        digit92(s[i+2])&&digit92(s[i+3])&&(i+4==n||!digit92(s[i+4]))) {
        if(count+3>12)return 0;
        grouped=1;
        for(size_t j=1;j<=3;++j)whole=whole*10+s[i+j]-'0';
        count+=3;i+=4;
    }
    size_t decimal=0,decimals=0;
    if(i+1<n&&s[i]=='.'&&digit92(s[i+1])) {
        decimal=++i;while(i<n&&digit92(s[i]))++i;decimals=i-decimal;
    }
    if(!currency&&!grouped&&!decimals&&count>=1&&count<=2&&i+2<n&&
       s[i]==':'&&digit92(s[i+1])&&digit92(s[i+2])&&
       (i+3==n||!wordchar(s[i+3]))) {
        unsigned minutes=(s[i+1]-'0')*10+s[i+2]-'0';
        if(cardinal92(b,whole))return -2;
        if(!minutes){if(text_word92(b,"o'clock"))return -2;}
        else {
            if(minutes<10&&text_word92(b,"o"))return -2;
            if(cardinal92(b,minutes))return -2;
        }
        *end=i+3;return 1;
    }
    if(!currency&&!grouped&&!decimals&&count>=1&&count<=2&&
       whole>=1&&whole<=12&&i<n&&s[i]=='/') {
        size_t day_start=i+1,k=day_start;unsigned day=0;
        while(k<n&&digit92(s[k])&&k-day_start<2)day=day*10+s[k++]-'0';
        if(k>day_start&&day>=1&&day<=31&&k<n&&s[k]=='/') {
            size_t ys=++k;uint64_t yr=0;
            while(k<n&&digit92(s[k])&&k-ys<4)yr=yr*10+s[k++]-'0';
            if((k-ys==2||k-ys==4)&&(k==n||!wordchar(s[k]))) {
                if(cardinal92(b,whole)||cardinal92(b,day)||
                   (k-ys==4?year92(b,yr):cardinal92(b,yr)))return -2;
                *end=k;return 1;
            }
        }
    }
    if(!currency&&!grouped&&!decimals&&count&&i+1<n) {
        unsigned tail=(unsigned)(whole%100);
        const char *suffix=tail>=11&&tail<=13?"th":whole%10==1?"st":
            whole%10==2?"nd":whole%10==3?"rd":"th";
        if(!memcmp(s+i,suffix,2)&&(i+2==n||!wordchar(s[i+2]))) {
            if(ordinal92(b,whole))return -2;
            *end=i+2;return 1;
        }
    }
    if(currency) {
        const char *unit=currency==1?(whole==1?"dollar":"dollars"):
            currency==2?(whole==1?"pound":"pounds"):(whole==1?"euro":"euros");
        unsigned cents=decimals==2?(s[decimal]-'0')*10+s[decimal+1]-'0':0;
        if(whole||!decimals||!cents||decimals!=2) {
            if(cardinal92(b,whole))return -2;
            if(decimals&&decimals!=2&&
               (text_word92(b,"point")||digits92(b,s+decimal,decimals,1)))return -2;
            if(text_word92(b,unit))return -2;
        }
        if(decimals==2&&cents) {
            if(whole&&text_word92(b,"and"))return -2;
            if(cardinal92(b,cents)||text_word92(b,currency==2?
               (cents==1?"penny":"pence"):(cents==1?"cent":"cents")))return -2;
        }
    } else if(grouped||decimals) {
        if(count&&cardinal92(b,whole))return -2;
        if(decimals&&(text_word92(b,"point")||digits92(b,s+decimal,decimals,1)))return -2;
        /* Further periods in software versions are decimal separators,
         * not successive full stops. Keep all following digit groups. */
        while(i+1<n&&s[i]=='.'&&digit92(s[i+1])) {
            size_t ds=++i;while(i<n&&digit92(s[i]))++i;
            if(text_word92(b,"point")||digits92(b,s+ds,i-ds,1))return -2;
        }
    } else if(count==4&&s[start]!='0'&&(i==n||!letter(s[i]))) {
        const char *known=NULL,*v=NULL,*c=NULL;
        if(lookup(s+start,count,&known,&v,&c))return 0;
        if(year92(b,whole))return -2;
    } else return 0;
    *end=i;return 1;
}
static int latin_fold98(uint32_t cp,char fold[3])
{
    unsigned char base;
    if((cp>='A'&&cp<='Z')||(cp>='a'&&cp<='z'))base=(unsigned char)cp;
    else if(reloq_character_latin_base(cp,&base)!=1) {
        switch(cp) {
        case 0xdf:memcpy(fold,"ss",3);return 2;
        case 0xc6:case 0xe6:memcpy(fold,cp==0xc6?"AE":"ae",3);return 2;
        case 0x152:case 0x153:memcpy(fold,cp==0x152?"OE":"oe",3);return 2;
        case 0xd8:base='O';break;case 0xf8:base='o';break;
        case 0xd0:base='D';break;case 0xf0:base='d';break;
        case 0xde:case 0xfe:memcpy(fold,cp==0xde?"TH":"th",3);return 2;
        default:return 0;
        }
    }
    fold[0]=(char)base;fold[1]=0;return 1;
}
static int latin_next98(const unsigned char *s,size_t n,size_t at)
{
    if(at>=n)return 0;
    uint32_t cp;char fold[3];
    return reloq_utf8_next(s,n,&at,&cp)==1 && latin_fold98(cp,fold)>0;
}
static int normalize92(const unsigned char *s,size_t n,int tags,struct normalized92 *out)
{
    for(size_t i=0;i<n;) {
        size_t end=i+1;struct text_buffer phrase={0};int status=0;
        uint32_t cp;size_t next=i;
        if(reloq_utf8_next(s,n,&next,&cp)!=1)return -1;
        while(next<n) {
            size_t following=next;uint32_t mark;
            if(reloq_utf8_next(s,n,&following,&mark)!=1)return -1;
            uint32_t composed=reloq_unicode_compose(cp,mark);
            if(!composed)break;
            cp=composed;next=following;
        }
        if(s[i]>=128||next>i+1) {
            char fold[3];int folded=latin_fold98(cp,fold);
            struct reloq_character_reading reading;
            int named=reloq_character_read(cp,1,&reading);
            if(named<0)return -1;
            if(!folded && named!=1) {
                int numeric=number92(s,n,i,&end,&phrase);
                if(numeric<0)status=-2;
                else if(numeric)status=normalized_add92(out,phrase.s,phrase.n,i,end);
                else {end=next;status=normalized_add92(out,s+i,end-i,i,end);}
                free(phrase.s);if(status)return status;i=end;continue;
            }
            end=next;
            int in_word=(out->text.n&&letter(out->text.s[out->text.n-1]))||latin_next98(s,n,next);
            if(in_word && folded)status=normalized_add92(out,fold,(size_t)folded,i,end);
            else if(named==1) {
                if(reading.original_spr)status=append(&phrase,reading.original_spr,strlen(reading.original_spr));
                else for(size_t k=0;k<reading.count&&!status;++k) {
                    const char *finish=strchr(reading.groups[k],']');
                    if(!finish){status=-1;break;}
                    const char *prefix=k?" `2 `[":"`[";
                    status=append(&phrase,prefix,strlen(prefix))||
                        append(&phrase,reading.groups[k],(size_t)(finish-reading.groups[k]))||append(&phrase,"]",1);
                }
                if(!status)status=append(&phrase," ",1);
                if(!status)status=normalized_add92(out,phrase.s,phrase.n,i,end);
            } else status=normalized_add92(out,fold,(size_t)folded,i,end);
        } else if(s[i]=='`') {
            struct reloq_inline_tag t=reloq_inline_tag_read(s+i,n-i,0);
            if(t.length&&(tags||t.kind==RELOQ_TAG_SPR||t.kind==RELOQ_TAG_PROMINENCE))end=i+t.length;
            status=normalized_add92(out,s+i,end-i,i,end);
        } else if(space92(s[i])) {
            while(end<n&&space92(s[end]))++end;
            status=normalized_add92(out," ",1,i,end);
        } else if((digit92(s[i])||s[i]=='$'||s[i]=='.'||s[i]>=128)&&
            (i==0||!wordchar(s[i-1]))) {
            int r=number92(s,n,i,&end,&phrase);
            if(r<0)status=-2;
            else if(r)status=normalized_add92(out,phrase.s,phrase.n,i,end);
            else goto ordinary;
        } else {
ordinary:;
            const char *name=NULL;unsigned char c=s[i],output=c;
            size_t next=i+1;while(next<n&&space92(s[next]))++next;
            size_t left=i;while(left&&space92(s[left-1]))--left;
            int numeric_left=left&&digit92(s[left-1]);
            switch(c) {
            case '/':name="slash";break;case '\\':name="backslash";break;
            case '_':name="underscore";break;case '*':name="asterisk";break;
            case '+':name="plus";break;case '=':name="equals";break;
            case '&':name="and";break;case '@':name="at";break;
            case '#':name=next<n&&digit92(s[next])?"number":"pound";break;
            case '%':name="percent";break;case '^':name="caret";break;
            case '~':name="tilda";break;case '|':name="bar";break;
            case '$':name="dollar";break;
            case '<':if(i+1==n||space92(s[i+1]))name="less than";else output=' ';break;
            case '>':name="greater than";break;
            case '\'':if((i==0||!wordchar(s[i-1]))&&(i+1==n||!letter(s[i+1])))name="apostrophe";break;
            case '"':case '[':case ']':case '{':case '}':output=' ';break;
            case ')':if(i+1<n&&letter(s[i+1]))name="right parenthesis";break;
            case '-':
                while(end<n&&s[end]=='-')++end;
                if(next<n&&digit92(s[next]))name=numeric_left?"dash":"minus";
                else output=end>i+1||(i&&space92(s[i-1]))||(i+1<n&&space92(s[i+1]))?'(':' ';
                break;
            case '.':
                while(end<n&&s[end]=='.')++end;
                if(end-i>=2)output=036;
                else if(i+1<n&&wordchar(s[i+1]))name="dot";
                break;
            case ':':if(i+1<n&&wordchar(s[i+1]))name="colon";break;
            case '!':case '?': {
                while(end<n&&(s[end]=='!'||s[end]=='?'))++end;
                if(end<n&&wordchar(s[end])) {end=i+1;name=c=='?'?"question mark":"exclamation point";}
                else {
                    int question=memchr(s+i,'?',end-i)!=NULL;
                    size_t following=end;while(following<n&&space92(s[following]))++following;
                    int medial=following<n&&s[following]>='a'&&s[following]<='z';
                    output=medial?(question?034:035):(question?'?':'!');
                }
                break;
            }
            default:if(space92(c)||c<32)output=' ';break;
            }
            if(name) {
                if(append(&phrase," ",1)||text_word92(&phrase,name))status=-2;
                else status=normalized_add92(out,phrase.s,phrase.n,i,end);
            } else if(end==i+1&&output==c)status=normalized_add92(out,s+i,1,i,end);
            else status=normalized_add92(out,&output,1,i,end);
        }
        free(phrase.s);if(status)return -2;i=end;
    }
    return normalized_add92(out,"",0,n,n);
}
