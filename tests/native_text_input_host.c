/* Component boundary for host tests; allocation belongs to each invocation. */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "eci_rom.h"
#include "reloq_greek.h"
extern EvvRom *reloq_chinese_romanizer(const char *);
void *cpp_new(uint32_t size){return malloc(size);}
void cpp_delete(void *p){free(p);}
int romanize(const char *text,size_t length,size_t split,char *output,size_t capacity)
{
    EvvRom *r=reloq_chinese_romanizer("");if(!r||split>length)return -1;
    int result=-1;char *converted=NULL;
    if(r->ops->addText(r,text,(int32_t)split,0)>0&&
       r->ops->addText(r,text+split,(int32_t)(length-split),0)>0&&
       r->ops->processSentence(r,&converted,1)==2&&converted) {
        size_t n=strlen(converted);
        if(n<capacity){memcpy(output,converted,n+1);result=(int)n;}
    }
    r->ops->release(r);return result;
}
struct english_words {char *text;size_t length,capacity;};
static int english(void *context,const char *word,char *phonemes,size_t capacity)
{
    struct english_words *words=context;size_t n=strlen(word);
    if(words->length+n+2>words->capacity||capacity<15)return -1;
    memcpy(words->text+words->length,word,n);words->length+=n;
    words->text[words->length++]='\t';words->text[words->length]=0;
    strcpy(phonemes,"`[.1hE.0loU]");return 0;
}
int greek_english_words(const char *text,char *output,size_t capacity)
{
    if(!capacity)return -1;output[0]=0;
    struct english_words words={output,0,capacity};char *converted=NULL;size_t *offsets=NULL;
    int result=reloq_greek_text(text,0,english,&words,&converted,&offsets);
    free(converted);free(offsets);return result<0?-1:(int)words.length;
}
