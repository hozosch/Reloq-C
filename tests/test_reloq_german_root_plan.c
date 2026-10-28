#include "reloq_german_root_plan.h"
#include "reloq_german_lexicon.h"
#include <assert.h>
#include <string.h>

/* Complete public phoneme readings, chosen to separate root stress,
 * lexical ie classes and the following weak ending. */
int main(void)
{
    static const struct {const char *word,*spr;} cases[]={
        {"Familie","`[.0fA.1mi.0lj@]"},
        {"Linie","`[.1li.0nj@]"},
        {"Linien","`[.1li.0nj@n]"},
        {"Studie","`[.1Stu.0dj@]"},
        {"Studien","`[.1Stu.0dj@n]"},
        {"Kopie","`[.0ko.1pi]"},
        {"Kopien","`[.0ko.1pi.0@n]"},
        {"Serien","`[.1ze.0ri.0@n]"},
        {"Flexibilit\344t","`[.2flEk.0si.0bi.0li.1t'E:'t]"},
        {"Funktionalit\344t","`[.2fUGk.0Tjo.0nA.0li.1t'E:'t]"},
        {"Nationalismus","`[.2nA.0Tjo.0nA.1lIs.0mUs]"},
        {"Koordination","`[.0ko.2Or.0di.0nA.1Tjon]"}
    };
    for(size_t i=0;i<sizeof cases/sizeof *cases;++i) {
        const unsigned char *w=(const unsigned char *)cases[i].word;
        size_t n=0,got=0;unsigned char out[1024];memset(out,0xa5,sizeof out);
        assert(reloq_german_root_plan_word(NULL,w,strlen(cases[i].word),NULL,0,&n)==-2);
        assert(n==strlen(cases[i].spr));
        assert(reloq_german_root_plan_word(NULL,w,strlen(cases[i].word),out,n,&got)==-2);
        assert(out[0]==0xa5&&got==n);
        assert(reloq_german_root_plan_word(NULL,w,strlen(cases[i].word),out,n+1,&got)==1);
        assert(got==n&&!strcmp((const char *)out,cases[i].spr));
        assert(out[n+1]==0xa5);
        assert(reloq_german_lexicon_validate_spr(out,n,NULL)==1);
    }
    static const char *const uncovered[]={"antike","begeister","geriee","monnaieer","Tempo"};
    for(size_t i=0;i<sizeof uncovered/sizeof *uncovered;++i) {
        size_t n=123;unsigned char out[16]="unchanged";
        assert(reloq_german_root_plan_word(NULL,(const unsigned char *)uncovered[i],strlen(uncovered[i]),out,sizeof out,&n)==0);
        assert(n==123&&!strcmp((const char *)out,"unchanged"));
    }
    return 0;
}
