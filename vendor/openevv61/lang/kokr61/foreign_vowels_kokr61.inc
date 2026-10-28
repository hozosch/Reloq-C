/* Spelling classes used by the foreign-vowel rules. */
enum foreign_vowel_class {
 FV_LONG_I_PAIR=1u<<0,
 FV_REDUCED_HIGH=1u<<1,
 FV_SCHWA_OPEN=1u<<2,
 FV_SCHWA_ROUND=1u<<3,
 FV_SCHWA_HIGH=1u<<4,
 FV_UH_OPEN=1u<<5,
 FV_UH_ROUND=1u<<6,
 FV_AE_OPEN=1u<<7,
 FV_AA_ROUND=1u<<8,
 FV_AA_CENTRAL=1u<<9,
 FV_AW_CENTRAL=1u<<10,
};
struct foreign_vowel_word {const char *word;unsigned short flags;};
static const struct foreign_vowel_word foreign_vowel_words[]={
 {"acknowledgeindustry",FV_REDUCED_HIGH},
 {"anybody",FV_UH_OPEN},
 {"before",FV_SCHWA_HIGH},
 {"chaos",FV_AA_ROUND},
 {"coffee",FV_AW_CENTRAL},
 {"com",FV_AA_CENTRAL},
 {"commission",FV_SCHWA_ROUND},
 {"committee",FV_SCHWA_ROUND},
 {"compare",FV_SCHWA_ROUND},
 {"conference",FV_AA_CENTRAL},
 {"dos",FV_AA_ROUND},
 {"from",FV_UH_OPEN},
 {"gatt",FV_AE_OPEN},
 {"kal",FV_AE_OPEN},
 {"kansas",FV_SCHWA_OPEN},
 {"kotra",FV_AA_ROUND},
 {"los",FV_AA_ROUND},
 {"nasa",FV_AE_OPEN},
 {"nobody",FV_UH_OPEN},
 {"of",FV_UH_ROUND},
 {"on",FV_AA_ROUND},
 {"polar",FV_AA_ROUND},
 {"policy",FV_REDUCED_HIGH},
 {"politics",FV_REDUCED_HIGH},
 {"real",FV_LONG_I_PAIR},
 {"really",FV_LONG_I_PAIR},
 {"responsibility",FV_REDUCED_HIGH},
 {"rocket",FV_AA_ROUND},
 {"rom",FV_AA_ROUND},
 {"solitude",FV_AA_ROUND},
 {"somebody",FV_UH_OPEN},
 {"success",FV_REDUCED_HIGH},
 {"support",FV_REDUCED_HIGH},
 {"top",FV_AA_ROUND},
};
static unsigned foreign_vowel_class(const char *word)
{
 char lower[64];size_t n=strlen(word);if(n>=sizeof lower)return 0;
 for(size_t i=0;i<=n;++i){unsigned char c=(unsigned char)word[i];lower[i]=(char)(c>='A'&&c<='Z'?c+'a'-'A':c);}
 size_t lo=0,hi=sizeof foreign_vowel_words/sizeof*foreign_vowel_words;
 while(lo<hi){size_t mid=lo+(hi-lo)/2;int c=strcmp(lower,foreign_vowel_words[mid].word);if(!c)return foreign_vowel_words[mid].flags;if(c<0)hi=mid;else lo=mid+1;}
 return 0;
}
