/* Alternative spellings consistent with the explicit a/aa distinction.
 * Keep these shared by the phrase and word analyzers. */
struct fa_alias {const char *roman,*persian;};
static const struct fa_alias aliases[]={
    {"salaam","سلام"},{"donyaa","دنیا"},
    {"iraan","ایران"},{"khub","خوب"},{"khoob","خوب"},
    {"nokte","نقطه"},{"nokteh","نقطه"},{"noghte","نقطه"},{"noghteh","نقطه"},
    {"sefr","صفر"},{"yek","یک"},
    {"soaal","سوال"},
    {"badbakht","بدبخت"},{"badbacht","بدبخت"},
    {"faarsi","فارسی"},{"ketaab","کتاب"},
    {"eslash","اسلش"},{"ast","است"},{"man","من"},
    {"u","او"},{"oo","او"},{"ou","او"},{"aamad","آمد"},
    {"va","و"},{"yaa","یا"},{"ke","که"},
    {"raa","را"},{"az","از"},{"be","به"},{"baa","با"},
    {"dar","در"},{"taa","تا"},{"joz","جز"}
};

static const char *lookup_fa_alias(const struct fa_alias *table,size_t count,
                                   const char *word,size_t n)
{
    for(size_t i=0;i<count;++i) {
        const char *p=table[i].roman;
        if(strlen(p)!=n)continue;
        size_t j=0;
        for(;j<n;++j) {
            unsigned char c=(unsigned char)word[j];
            if(c>='A' && c<='Z')c=(unsigned char)(c+('a'-'A'));
            if(c!=(unsigned char)p[j])break;
        }
        if(j==n)return table[i].persian;
    }
    return NULL;
}

const char *reloq_fa_pinglish_common_alias(const char *word,size_t n)
{
    return lookup_fa_alias(aliases,sizeof aliases/sizeof aliases[0],word,n);
}
