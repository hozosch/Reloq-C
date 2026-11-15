/* Focused portable C smoke test for the private Persian text components. */
#include "direct_fa_lexicon.h"
#include "direct_fa_letters.h"
#include "direct_fa_text.h"
#include <stdio.h>
#include <string.h>

static int check_text(const char *text,const char *expected)
{
    char *out=NULL;size_t *map=NULL,error=0,n=strlen(text);
    int status=reloq_fa_direct_text(text,n,&out,&map,&error);
    if(status || !out || !map || strcmp(out,expected) || map[n]!=strlen(out)) {
        reloq_fa_direct_free(out);reloq_fa_direct_free(map);return 0;
    }
    for(size_t i=1;i<=n;++i)if(map[i]<map[i-1]) {
        reloq_fa_direct_free(out);reloq_fa_direct_free(map);return 0;
    }
    reloq_fa_direct_free(out);reloq_fa_direct_free(map);return 1;
}

static int same_text(const char *first,const char *second)
{
    char *a=NULL,*b=NULL;size_t *ma=NULL,*mb=NULL,ea=0,eb=0;
    int ra=reloq_fa_direct_text(first,strlen(first),&a,&ma,&ea);
    int rb=reloq_fa_direct_text(second,strlen(second),&b,&mb,&eb);
    int same=ra==0 && rb==0 && a && b && !strcmp(a,b);
    reloq_fa_direct_free(a);reloq_fa_direct_free(b);
    reloq_fa_direct_free(ma);reloq_fa_direct_free(mb);return same;
}

int main(void)
{
    const char *alef=reloq_fa_direct_letter(0x0627u);
    const char *qaf=reloq_fa_direct_letter(0x0642u);
    if(!alef || strcmp(alef,"`[.1A.0lef]") ||
       !qaf || strcmp(qaf,"`[.1Raf]") ||
       reloq_fa_direct_letter(0x0041u) ||
       reloq_fa_direct_word_count()<15000 ||
       !check_text("سلام دنیا؟", "`[.0sA.1lam] `[.1do.0nja]?") ||
       !check_text("ﺳﻼﻡ دنیا؟", "`[.0sA.1lam] `[.1do.0nja]?") ||
       !check_text("ايران نقطه.", "`[.1'i:'.0r'a:'n] `[.1noR.0t'e:'].") ||
       !check_text("كاغذ آمد.", "`[.1ka.0RAz] `[.1'a:'.0mAd].") ||
       !check_text("او آمد.", "`[.1'u:'] `[.1'a:'.0mAd].") ||
       !check_text("سلام،دنیا!", "`[.0sA.1lam],`[.1do.0nja]!") ||
       !check_text("سلام؛ دنیا؟", "`[.0sA.1lam]; `[.1do.0nja]?") ||
       !check_text("کتاب‌ها آمد.", "`[.1ke.0tab.0ha] `[.1'a:'.0mAd].") ||
       !check_text("کتاب‌ها؟", "`[.1ke.0tab.1ha]?") ||
       !check_text("۱۲", "`[.0dA.1vaz.0dAhw]") ||
       !check_text("۳٫۱۴", "`[.2se] `[.0noR.2t'e:'] `[.0jek] `[.1Ca.0har]") ||
       !check_text("1001", "`[.2jek] `[.0he.2za.0ro] `[.1jek]") ||
       !check_text("−۳", "`[.1mAn.0fi] `[.1se]") ||
       !check_text("+۳", "`[.1mos.0bAt] `[.1se]") ||
       !check_text("012", "`[.0sefr] `[.0jek] `[.1do]") ||
       !same_text("۱۲٬۳۴۵", "12,345") ||
       !same_text("۱٬۲۳۴٫۵", "1,234.5") ||
       !same_text("١٢٣٤٫٥", "1234.5") ||
       !check_text("او و آمد.", "`[.1'u:'] `[.1vA] `[.1'a:'.0mAd].") ||
       !check_text("من آمد.", "`[.0mAn] `[.1'a:'.0mAd].") ||
       !check_text("او من.", "`[.1'u:'] `[.1mAn].") ||
       !check_text("من بدبخت آمد.", "`[.0mAn] `[.1bAd.0bAGt] `[.1'a:'.0mAd].") ||
       !same_text("اِتِّفاقاتِ", "اِتِّفاقاتِ") ||
       !check_text("علامت‌دار", "`[.0'A:'.1la.0mAt] `[.1dar]") ||
       !check_text("ثبت‌شده", "`[.1sAbt] `[.1So.0d'e:']") ||
       !check_text("آبگوشت،سلام", "`[.1ab.0guSt],`[.0sA.1lam]") ||
       !check_text("سوال", "`[.1so]~`[.0al]") ||
       !check_text("بنابراین", "`[.2bA.0n'a:'.1bAr]~`[.0in]") ||
       !check_text("من، آمد.", "`[.1mAn], `[.1'a:'.0mAd]."))return 1;
    char *out=NULL;size_t *map=NULL,error=0;
    const char *unknown="سلام NVDA";
    if(reloq_fa_direct_text(unknown,strlen(unknown),&out,&map,&error)!=1 ||
       out || map || error!=strlen("سلام "))return 1;
    const char *symbol_only=" ـ ";
    if(reloq_fa_direct_text(symbol_only,strlen(symbol_only),&out,&map,&error)!=1 ||
       out || map || error!=1)return 1;
    const char *numeric_separator="سلام ٫ دنیا";
    if(reloq_fa_direct_text(numeric_separator,strlen(numeric_separator),&out,&map,&error)!=2 ||
       out || map || error!=strlen("سلام "))return 1;
    const char *plural="کتاب‌ها";
    char tiny[4];
    if(reloq_fa_direct_context_word(plural,strlen(plural),1,tiny,sizeof tiny)!=-2 || tiny[0])return 1;
    const char *unknown_plural="ناموجودِمطمئن‌ها";
    if(reloq_fa_direct_text(unknown_plural,strlen(unknown_plural),&out,&map,&error)!=1 || out || map)return 1;
    numeric_separator="سلام ٬ دنیا";
    if(reloq_fa_direct_text(numeric_separator,strlen(numeric_separator),&out,&map,&error)!=2 ||
       out || map || error!=strlen("سلام "))return 1;
    const char *invalid_numbers[]={"3,14","3.1.4","۳٫","۱۲٬۳۴","1234567890123","۱۲-۳"};
    for(size_t i=0;i<sizeof invalid_numbers/sizeof invalid_numbers[0];++i)
        if(reloq_fa_direct_text(invalid_numbers[i],strlen(invalid_numbers[i]),&out,&map,&error)!=2 || out || map)return 1;
    puts("Passed Persian direct C lookup, normalization, source maps and rejection.");
    return 0;
}
