#include "reloq_english_connected.h"
#include <stdint.h>
#include <string.h>

static int vowel(unsigned char c)
{return c && strchr("iIeEAXHxauUocRYWO",c)!=NULL;}

void reloq_english_connected_rules(unsigned char *s,size_t *length,size_t *map)
{
    if(!s || !length)return;
    size_t n=*length,previous_end=SIZE_MAX,previous_before=SIZE_MAX;
    unsigned previous_stress=0,weak=0;
    for(size_t i=0;i<n;) {
        if(s[i]=='`' && i+2<n && s[i+1]=='0') {
            weak=s[i+2]=='0';i+=weak?3:2;continue;
        }
        if(s[i]=='`' && i+1<n && s[i+1]=='[') {
            size_t end=i+2;while(end<n && s[end]!=']')++end;
            if(end==n)return;
            size_t first=i+2,last=SIZE_MAX,before=SIZE_MAX;
            unsigned stress=0,first_stress=0;
            for(size_t j=i+2;j<end;++j) {
                if(s[j]=='.' && j+1<end) {
                    stress=(unsigned)(s[++j]-'0');
                    if(last==SIZE_MAX)first_stress=stress;
                    continue;
                }
                if(last==SIZE_MAX)first=j;
                before=last;last=j;
            }
            if(previous_end!=SIZE_MAX && previous_before!=SIZE_MAX && first<end &&
               (s[previous_end-1]=='t' || s[previous_end-1]=='d') &&
               (vowel(s[previous_before]) || s[previous_before]=='r')) {
                /* word_final_flap calls the weak-function h deletion
                 * before testing the newly adjacent vowel. The reduced
                 * h+reduced-vowel and h+rhotic entries permit that deletion;
                 * a stressed full h-vowel word retains its consonant. */
                if(weak && s[first]=='h' && first+1<end &&
                   (((s[first+1]=='I' || s[first+1]=='X' || s[first+1]=='i') && !first_stress) || (s[first+1]=='R'&&(previous_stress||s[previous_end-1]=='t')))) {
                    memmove(s+first,s+first+1,n-first);
                    if(map)memmove(map+first,map+first+1,(n-first)*sizeof *map);
                    --n;--end;if(last!=SIZE_MAX)--last;
                    if(before!=SIZE_MAX && before>first)--before;
                }
                if(vowel(s[first]))s[previous_end-1]='F';
            }
            previous_end=end;previous_before=before;previous_stress=stress;
            weak=0;i=end+1;continue;
        }
        if(strchr(".,!?;:\n()\034\035\036",s[i])) {
            previous_end=previous_before=SIZE_MAX;previous_stress=weak=0;
        }
        ++i;
    }
    *length=n;
}
