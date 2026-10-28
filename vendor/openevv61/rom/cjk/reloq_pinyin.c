/* Native pinyin-prefix recognition from the original Chinese romanizer.
 * The original accepts tones 0..4 and prefix matches in its final table.
 * Retaining those details avoids replacing it with a different linguistic
 * syllable validator. The bounded comparison needs no allocation. */
#include "reloq_pinyin.h"
static const char initials[][3]={"zh","ch","sh","b","p","m","f","d","t","n","l","z","c","s","r","j","q","x","g","k","h","y","w"};
static const char finals[][5]={"iang","iong","uang","ang","eng","ing","iao","ian","ong","uan","uai","uei","uue","ai","ao","an","er","ei","en","ia","ie","in","iu","ou","ua","uo","ui","uu","un","ue","a","e","i","o","u"};
static unsigned lower(unsigned c){return c>='A'&&c<='Z'?c+32:c;}
static int letter(unsigned c){c=lower(c);return c>='a'&&c<='z';}
static int prefix(const unsigned char *text,size_t n,const char *key)
{
    for(size_t i=0;i<n;++i)if(!key[i]||lower(text[i])!=(unsigned char)key[i])return 0;
    return 1;
}
size_t reloq_pinyin_prefix(const unsigned char *text,size_t size)
{
    size_t start=0,n=0,onset=0;
    while(start<size&&(text[start]==' '||text[start]=='\t'||text[start]=='\r'||text[start]=='\n'||text[start]=='\f'||text[start]=='\v'))++start;
    while(start+n<size&&letter(text[start+n]))++n;
    if(!n||start+n>=size||text[start+n]<'0'||text[start+n]>'4')return 0;
    for(size_t i=0;i<sizeof initials/sizeof *initials;++i){
        size_t length=initials[i][1]?2:1;
        if(n>=length&&prefix(text+start,length,initials[i])){onset=length;break;}
    }
    if(n==onset)return n==1&&lower(text[start])=='m'?start+n+1:0;
    for(size_t i=0;i<sizeof finals/sizeof *finals;++i)
        if(prefix(text+start+onset,n-onset,finals[i]))return start+n+1;
    return 0;
}
