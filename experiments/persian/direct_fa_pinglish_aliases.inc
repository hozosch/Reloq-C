/* Common roman spellings whose conventional single a can be the Persian
 * back vowel. Keep this shared by the phrase and word analyzers. */
struct fa_alias {const char *roman,*persian;};
static const struct fa_alias aliases[]={
    {"salam","سلام"},{"salaam","سلام"},{"donya","دنیا"},{"donyaa","دنیا"},
    {"iran","ایران"},{"iraan","ایران"},{"khub","خوب"},{"khoob","خوب"},
    {"nokte","نقطه"},{"nokteh","نقطه"},{"noghte","نقطه"},{"noghteh","نقطه"},
    {"kaghaz","کاغذ"},{"kakhaz","کاغذ"},{"sefr","صفر"},{"yek","یک"},
    {"soal","سوال"},{"soaal","سوال"},{"sual","سوال"},
    {"badbakht","بدبخت"},{"badbacht","بدبخت"},
    {"farsi","فارسی"},{"faarsi","فارسی"},{"ketab","کتاب"},{"ketaab","کتاب"},
    {"eslash","اسلش"},{"ast","است"},{"man","من"},
    {"u","او"},{"oo","او"},{"ou","او"},{"amad","آمد"},{"aamad","آمد"},
    {"va","و"},{"ya","یا"},{"yaa","یا"},{"ke","که"},
    {"ra","را"},{"raa","را"},{"az","از"},{"be","به"},{"ba","با"},
    {"dar","در"},{"ta","تا"},{"taa","تا"},{"joz","جز"}
};

/* Conventional informal spellings recognized by the native word path. */
static const struct fa_alias conventional_words[]={
    {"khane","خانه"},{"khaneh","خانه"},{"farda","فردا"},
    {"khoda","خدا"},{"khab","خواب"},
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
