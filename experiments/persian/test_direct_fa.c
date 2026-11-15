/* Focused portable C smoke test for the private Persian text components. */
#include "direct_fa_lexicon.h"
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

int main(void)
{
    if(reloq_fa_direct_word_count()<15000 ||
       !check_text("سلام دنیا؟", "`[.0sA.1lam] `[.1do.0nja]?") ||
       !check_text("ﺳﻼﻡ دنیا؟", "`[.0sA.1lam] `[.1do.0nja]?") ||
       !check_text("ايران نقطه.", "`[.1'i:'.0r'a:'n] `[.1noR.0t'e:'].") ||
       !check_text("كاغذ آمد.", "`[.1ka.0RAz] `[.1'a:'.0mAd].") ||
       !check_text("او آمد.", "`[.1'u:'] `[.1'a:'.0mAd].") ||
       !check_text("سلام،دنیا!", "`[.0sA.1lam],`[.1do.0nja]!") ||
       !check_text("سلام؛ دنیا؟", "`[.0sA.1lam]; `[.1do.0nja]?") ||
       !check_text("کتاب‌ها آمد.", "`[.1ke.0tab.0ha] `[.1'a:'.0mAd].") ||
       !check_text("کتاب‌ها؟", "`[.1ke.0tab.1ha]?") ||
       !check_text("او و آمد.", "`[.1'u:'] `[.1vA] `[.1'a:'.0mAd].") ||
       !check_text("من آمد.", "`[.0mAn] `[.1'a:'.0mAd].") ||
       !check_text("او من.", "`[.1'u:'] `[.1mAn].") ||
       !check_text("من، آمد.", "`[.1mAn], `[.1'a:'.0mAd]."))return 1;
    char *out=NULL;size_t *map=NULL,error=0;
    const char *unknown="سلام NVDA";
    if(reloq_fa_direct_text(unknown,strlen(unknown),&out,&map,&error)!=1 ||
       out || map || error!=strlen("سلام "))return 1;
    const char *numeric_separator="سلام ٫ دنیا";
    if(reloq_fa_direct_text(numeric_separator,strlen(numeric_separator),&out,&map,&error)!=1 ||
       out || map || error!=strlen("سلام "))return 1;
    const char *plural="کتاب‌ها";
    char tiny[4];
    if(reloq_fa_direct_context_word(plural,strlen(plural),1,tiny,sizeof tiny)!=-2 || tiny[0])return 1;
    const char *unknown_plural="ناموجودِمطمئن‌ها";
    if(reloq_fa_direct_text(unknown_plural,strlen(unknown_plural),&out,&map,&error)!=1 || out || map)return 1;
    numeric_separator="سلام ٬ دنیا";
    if(reloq_fa_direct_text(numeric_separator,strlen(numeric_separator),&out,&map,&error)!=1 ||
       out || map || error!=strlen("سلام "))return 1;
    puts("Passed Persian direct C lookup, normalization, source maps and rejection.");
    return 0;
}
