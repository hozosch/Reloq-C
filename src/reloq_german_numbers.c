#include "reloq_german_numbers.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>

static const char *const names[100]={
#include "reloq_german_number_names.inc"
};
#include "reloq_german_date_names.inc"
static int digit(unsigned char c) {return c>='0' && c<='9';}
static int number_space(unsigned char c) {return c==' ' || c==0xa0;}
static int ascii_letter(unsigned char c)
{return (c>='a' && c<='z') || (c>='A' && c<='Z');}

static int group(unsigned n,const char *suffix,int attributive,
    reloq_german_number_emit emit,void *ctx)
{
    char body[96];
    const char *base=n==1 && (suffix || attributive)?".1'aj'n":names[n];
    size_t len=strlen(base),tail=suffix?strlen(suffix):0;
    if (len+tail>=sizeof body) return -1;
    memcpy(body,base,len);
    if (suffix) memcpy(body+len,suffix,tail);
    body[len+tail]=0;
    /* The ein-und-tens component loses secondary stress before tausend. */
    if (n>=21 && n%10==1 && suffix && !strcmp(suffix,".2t'aw'.0z@nt")) {
        char *stress=strstr(body,".2");
        if (stress) stress[1]='0';
    }
    if (suffix) {
        char *stress=strstr(body,".2Unt");
        if(!stress)stress=strstr(body,".2rUnt");
        if (stress) stress[1]='0';
    }
    return emit(ctx,body,0);
}

/* Productive spelling analysis for the observed unit-und-decade compounds.
 * This reuses cardinal syllables and stress instead of sending their letters
 * through generic G2P. Larger/inflected forms remain in lexical analysis. */
int reloq_german_number_word(const unsigned char *word,size_t length,
    reloq_german_number_emit emit,void *ctx)
{
    static const char *const units[]={"ein","zwei","drei","vier","f\374nf",
        "sechs","sieben","acht","neun"};
    static const char *const tens[]={"zwanzig","drei\337ig","vierzig","f\374nfzig",
        "sechzig","siebzig","achtzig","neunzig"};
    if (!word || !emit) return -1;
    char folded[32];
    if (!length || length>=sizeof folded) return 1;
    for (size_t i=0;i<length;++i) {
        unsigned c=word[i];
        if ((c>='A' && c<='Z') || c==0xc4 || c==0xd6 || c==0xdc) c+=32;
        folded[i]=(char)c;
    }
    folded[length]=0;
    for (unsigned i=0;i<9;++i) {
        size_t n=strlen(units[i]);
        if (length<n+3 || memcmp(folded,units[i],n) || memcmp(folded+n,"und",3)) continue;
        const char *tail=folded+n+3;
        if (!*tail) {
            if(i==3)return emit(ctx,".1fi.2rUnt",0);
            char body[64];
            const char *unit=i?names[i+1]:".1'aj'n";
            if (snprintf(body,sizeof body,"%s.2Unt",unit)>=(int)sizeof body) return -1;
            return emit(ctx,body,0);
        }
        for (unsigned j=0;j<8;++j)
            if (!strcmp(tail,tens[j])) return group((j+2)*10+i+1,NULL,0,emit,ctx);
    }
    return 1;
}

static int small(unsigned n,int thousand,int attributive,
    reloq_german_number_emit emit,void *ctx)
{
    int status;
    if (n>=100) {
        status=group(n/100,".2hUn.0dRt",0,emit,ctx);
        if (status) return status;
        n%=100;
        if (!n) return thousand?emit(ctx,".1t'aw'.0z@nt",0):0;
    }
    if (attributive==2 && n==1) return emit(ctx,".0'aj'n",1);
    return group(n,thousand?".2t'aw'.0z@nt":NULL,attributive,emit,ctx);
}

static int cardinal(uint64_t n,int year,int attributive,
    reloq_german_number_emit emit,void *ctx)
{
    static const uint64_t scales[]={UINT64_C(1000000000),UINT64_C(1000000)};
    static const char *const singular[]={".0mI.1ljAr.0d@",".0mI.1ljon"};
    static const char *const plural[]={".0mI.1ljAr.0d@n",".0mI.1ljo.0n@n"};
    int status;
    for (size_t i=0;i<2;++i) if (n>=scales[i]) {
        unsigned q=(unsigned)(n/scales[i]);n%=scales[i];
        status=q==1?emit(ctx,".1'aj'.0n@",1):small(q,0,2,emit,ctx);
        if (status) return status;
        status=emit(ctx,q==1?singular[i]:plural[i],0);
        if (status || !n) return status;
    }
    if (year && n>=1100 && n<=1999) {
        status=group((unsigned)(n/100),".2hUn.0dRt",0,emit,ctx);
        if (status || !(n%=100)) return status;
    } else if (n>=1000) {
        status=small((unsigned)(n/1000),1,0,emit,ctx);
        if (status || !(n%=1000)) return status;
    }
    return small((unsigned)n,0,attributive,emit,ctx);
}

static int digits(const unsigned char *text,size_t n,
    reloq_german_number_emit emit,void *ctx)
{
    for (size_t i=0;i<n;++i) if (digit(text[i])) {
        int result=emit(ctx,names[text[i]-'0'],0);
        if (result) return result;
    }
    return 0;
}

static int emit_day(unsigned day,int suffix,reloq_german_number_emit emit,void *ctx)
{
    const char *base=days[day-1];
    if (suffix==0) return emit(ctx,base,0);
    char body[96];
    size_t n=strlen(base);
    if (n<2 || base[n-1]!='R' || n+2>=sizeof body) return emit(ctx,base,0);
    memcpy(body,base,n-1);
    body[n-1]='@';
    body[n]=suffix==1?'n':0;
    body[n+1]=0;
    return emit(ctx,body,0);
}

/* Parse the complete clock before emitting anything. Colon forms accept
 * one or two digits per field; a dot remains a clock only with "Uhr".
 * This explicit extension also covers seconds, rather than recursively
 * interpreting a tail such as 05:03 as a second clock. */
static int clock_read(const unsigned char *text,size_t length,size_t hour_end,
    unsigned hour,size_t *consumed,reloq_german_number_emit emit,void *ctx,
    int *matched)
{
    *matched=0;
    if (hour_end>2 || hour>24 || hour_end>=length ||
        (text[hour_end]!=':' && text[hour_end]!='.')) return 0;
    const unsigned char separator=text[hour_end];
    unsigned values[2]={0,0};size_t at=hour_end+1,fields=0;
    for (;;) {
        size_t start=at;
        while (at<length && digit(text[at]) && at-start<2)
            values[fields]=values[fields]*10+(text[at++]-'0');
        if (at==start || (at<length && digit(text[at])) || values[fields]>59)
            return 0;
        ++fields;
        if (fields==2 || separator!=':' || at==length || text[at]!=':') break;
        ++at;
    }
    if (at<length && (text[at]==':' || text[at]=='/' ||
        (text[at]=='.' && at+1<length && digit(text[at+1])))) return 0;
    size_t unit=at;
    while (unit<length && (number_space(text[unit]) || text[unit]=='\t')) ++unit;
    const int has_unit=unit+3<=length &&
        (text[unit]=='U' || text[unit]=='u') &&
        (text[unit+1]=='H' || text[unit+1]=='h') &&
        (text[unit+2]=='R' || text[unit+2]=='r') &&
        (unit+3==length || (!ascii_letter(text[unit+3]) && !digit(text[unit+3])));
    if ((separator=='.' && (!has_unit || at-hour_end!=3)) ||
        (!has_unit && at<length && ascii_letter(text[at]))) return 0;
    *matched=1;
    int result=cardinal(hour,0,1,emit,ctx);
    /* User clock policy: Uhr is a weak unit before a nonzero minute
     * amount. Minute units are never spoken, even with seconds present. */
    if (!result) result=emit(ctx,".1ur",values[0]!=0);
    if (!result && values[0]) result=cardinal(values[0],0,0,emit,ctx);
    if (fields==2) {
        if (!result) result=emit(ctx,".1Unt",1);
        if (!result) result=values[1]==1?emit(ctx,".1'aj'.0n@",1):
            cardinal(values[1],0,0,emit,ctx);
        if (!result) result=emit(ctx,values[1]==1?
            ".0ze.1kUn.0d@":".0ze.1kUn.0d@n",0);
    }
    if (!result) *consumed=has_unit?unit+3:at;
    return result;
}

int reloq_german_number_read_ctx(const unsigned char *text,size_t length,
    size_t *consumed,reloq_german_number_emit emit,void *ctx,
    const struct reloq_german_number_context *context)
{
    const int day_suffix=context?context->day_suffix:0;
    if (!text || !length || !digit(text[0]) || !consumed || !emit) return -1;
    size_t end=0,count=0;
    uint64_t value=0;
    while (end<length && digit(text[end])) {
        if (count<12) value=value*10+(text[end]-'0');
        ++count;++end;
    }
    if (count<=2) {
        int matched=0;
        int result=clock_read(text,length,end,(unsigned)value,consumed,emit,ctx,&matched);
        if (matched || result) return result;
    }
    /* Dates have their own word formation, before cardinal punctuation.
     * This mirrors the observed numeric format, including day 31 in any
     * month; it is speech normalization, not calendar validation. */
    if (count<=2 && value>=1 && value<=31 && end<length && text[end]=='.') {
        size_t mstart=end+1,mend=mstart;
        unsigned month=0;
        while (mend<length && digit(text[mend]) && mend-mstart<2)
            month=month*10+(text[mend++]-'0');
        if (month>=1 && month<=12 && mend<length && text[mend]=='.') {
            size_t ystart=mend+1,yend=ystart;unsigned year_value=0;
            while (yend<length && digit(text[yend]) && yend-ystart<5)
                year_value=year_value*10+(text[yend++]-'0');
            size_t year_count=yend-ystart;
            if (year_count==0 || year_count==2 || year_count==4) {
                int result=emit_day((unsigned)value,day_suffix,emit,ctx);
                if (!result) result=emit(ctx,months[month-1],0);
                if (!result && year_count) result=cardinal(year_value,year_count==4,0,emit,ctx);
                if (result) return result;
                *consumed=year_count?yend:mend;return 0;
            }
        }
    }
    if (count==1 && value>=1 && end+1<length && text[end]=='/' &&
        text[end+1]>='2' && text[end+1]<='9' &&
        (end+2==length || !digit(text[end+2]))) {
        static const char *const denominators[]={"", "",
            ".1hAl.0b@", ".2drI.0t@l", ".2fIr.0t@l", ".2fYnf.0t@l",
            ".2zEk.0st@l", ".2zip.0t@l", ".2Ax.0t@l", ".2n'oj'n.0t@l"};
        unsigned denominator=text[end+1]-'0';
        const char *joined=value==3 && denominator==9?".1dr'aj'n.2'oj'n.0t@l":
            value==7 && denominator==8?".1zi.0b@.2nAx.0t@l":
            value==9 && denominator==8?".1n'oj'.2nAx.0t@l":NULL;
        int result=joined?emit(ctx,joined,0):denominator==2?emit(ctx,value==1?".0'aj'n.1hAlp":denominators[2],0):
            group((unsigned)value,denominators[denominator],0,emit,ctx);
        if (result) return result;
        *consumed=end+2;return 0;
    }
    int grouped=0;
    if (count<=3 && text[0]!='0') {
        for (;;) {
            size_t start=end;
            if (start<length && text[start]=='.') ++start;
            else while (start<length && number_space(text[start])) ++start;
            if (start==end || length-start<3 || !digit(text[start]) ||
                !digit(text[start+1]) || !digit(text[start+2]) ||
                (start+3<length && digit(text[start+3]))) break;
            for (size_t j=0;j<3;++j) {
                if (count<12) value=value*10+(text[start+j]-'0');
                ++count;
            }
            end=start+3;grouped=1;
        }
    }
    int sequential=count>12 || (count>1 && text[0]=='0');
    size_t fraction=end,tail=end;
    unsigned cents=0;
    if (end+1<length && (text[end]==',' || text[end]=='.' || text[end]==':') &&
        digit(text[end+1])) {
        fraction=end+1;tail=fraction;
        while (tail<length && digit(text[tail])) {
            if (tail-fraction<2) cents=cents*10+(text[tail]-'0');
            ++tail;
        }
    }
    size_t unit=tail;
    while (unit<length && (number_space(text[unit]) || text[unit]=='\t')) ++unit;
    size_t unit_length=unit<length && text[unit]==0x80?1:0;
    if (unit+3<=length && (text[unit]=='E' || text[unit]=='e') &&
        (text[unit+1]=='U' || text[unit+1]=='u') &&
        (text[unit+2]=='R' || text[unit+2]=='r')) {
        unit_length=3;
        if (unit+3<length && (text[unit+3]=='o' || text[unit+3]=='O')) unit_length=4;
        if (unit+unit_length<length &&
            ((text[unit+unit_length]>='a' && text[unit+unit_length]<='z') ||
             (text[unit+unit_length]>='A' && text[unit+unit_length]<='Z')))
            unit_length=0;
    }
    int euro=unit_length && !sequential &&
        (tail==end || (text[end]==',' && tail-fraction==2));
    int status;
    if (euro && value==0 && cents && tail>end) {
        status=cardinal(cents,0,1,emit,ctx);
        if (!status) status=emit(ctx,".1TEnt",0);
        if (status) return status;
        *consumed=unit+unit_length;return 0;
    }
    status=sequential?digits(text,end,emit,ctx):
        cardinal(value,!grouped && count==4,euro,emit,ctx);
    if (status) return status;
    if (euro) {
        status=emit(ctx,".1'oj'.0ro",0);
        if (!status && tail>end && cents) status=cardinal(cents,0,euro,emit,ctx);
        if (status) return status;
        *consumed=unit+unit_length;return 0;
    }
    /* Ordinary comma numbers preserve the requested digit-by-digit reading.
     * Monetary amounts were handled above as euro and cent quantities.
     * Other numeric separators retain their observed two-digit grouping. */
    while (tail>end) {
        status=emit(ctx,text[end]==','?".1kO.0mA":
            text[end]=='.'?".1pUGkt":".1dO.0p@l.2pUGkt",0);
        if (status) return status;
        status=text[end]!=',' && tail-fraction<=2 && !(tail-fraction>1 && text[fraction]=='0')?
            cardinal(cents,0,0,emit,ctx):digits(text+fraction,tail-fraction,emit,ctx);
        if (status) return status;
        end=tail;
        /* Multiple periods delimit numeric components, as in version
         * strings. A second comma ends this numeric token. */
        if (end+1>=length || text[end]!='.' || !digit(text[end+1])) break;
        fraction=end+1;tail=fraction;cents=0;
        while (tail<length && digit(text[tail])) {
            if (tail-fraction<2) cents=cents*10+(text[tail]-'0');
            ++tail;
        }
    }
    *consumed=end;return 0;
}

int reloq_german_number_read(const unsigned char *text,size_t length,
    size_t *consumed,reloq_german_number_emit emit,void *ctx)
{
    return reloq_german_number_read_ctx(text,length,consumed,emit,ctx,NULL);
}

/* Ordinal numerals: the cardinal word groups with the final member turned
 * into its ordinal. 1..19 are lexical (with the regular -t/-st stem for 13..19),
 * everything larger takes -st. A dative ending adds a final n. */
struct group_buffer {
    char text[24][96];
    size_t count;
};
static int collect_group(void *context,const char *body,int weak)
{
    struct group_buffer *g=context;
    (void)weak;
    size_t n=strlen(body);
    if (g->count>=24 || n>=sizeof g->text[0]) return -1;
    memcpy(g->text[g->count],body,n+1);
    ++g->count;
    return 0;
}

int reloq_german_ordinal_emit(uint64_t value,int dative,
    reloq_german_number_emit emit,void *ctx)
{
    static const char *const small[20]={
        ".1nUl.0st@",".1er.0st@",".1Tv'aj'.0t@",".1drI.0t@",".1fir.0t@",
        ".1fYnf.0t@",".1zEk.0st@",".1zip.0t@",".1Ax.0t@",".1n'oj'n.0t@",
        ".1Ten.0t@",".1Elf.0t@",".1Tv'OE'lf.0t@"};
    struct group_buffer g={{{0}},0};
    int status=cardinal(value,0,0,collect_group,&g);
    if (status || !g.count) return status?status:-1;
    const unsigned low=(unsigned)(value%100);
    char *last=g.text[g.count-1];
    char replaced[96];
    if (value==0 || (low>=1 && low<=19)) {
        const unsigned k=value==0?0:low;
        if (k<=12) snprintf(replaced,sizeof replaced,"%s",small[k]);
        else snprintf(replaced,sizeof replaced,"%s.0t@",names[k]);
    } else snprintf(replaced,sizeof replaced,"%s.0st@",last);
    snprintf(last,sizeof g.text[0],"%s",replaced);
    if (dative) {
        size_t n=strlen(last);
        if (n+2>=sizeof g.text[0]) return -1;
        last[n]='n';last[n+1]=0;
    }
    for (size_t i=0;i<g.count;++i) {
        status=emit(ctx,g.text[i],0);
        if (status) return status;
    }
    return 0;
}
