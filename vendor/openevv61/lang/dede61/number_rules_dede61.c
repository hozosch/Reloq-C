/* Number policies shared by native token normalization and sentence input. */
#include "delta.h"
#include <stddef.h>

static int input_next(delta_state *d,int32_t *at)
{
    static const unsigned char alphabet[]={
#include "reloq61_german_alphabet.inc"
    };
    if(!*at)return -1;
    int32_t node=vgetsc(d,0,0,*at,1);
    for(unsigned steps=0;node&&(*EVV_AT(int32_t *,node)&2)&&steps<16;++steps)
        node=vgetsc(d,0,0,node,1);
    if(!node)return -1;
    unsigned name=*(const unsigned char *)delta_record_field(&vstmtbl[1],0,TFLDS(EVV_AT(void *,node)));
    *at=EVV_AT(const int32_t *,node)[1]&~3;
    return name<sizeof alphabet?alphabet[name]:-1;
}
static int letter(int c)
{return (c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>=0xc0&&c<=0xff&&c!=0xd7&&c!=0xf7);}

/* Ordinary comma fractions name each digit. Euro/cent quantities keep their
 * amount reading. Inspect the existing input projection without replacing,
 * inserting or rescanning a text string, and without changing its cursor. */
int reloq_dede61_decimal_digits(void *state,int32_t after)
{
    delta_state *d=state;int c=-1;
    for(unsigned spaces=0;spaces<32;++spaces) {
        c=input_next(d,&after);
        if(c!=0&&c!=' '&&c!='\t'&&c!=0xa0)break;
    }
    if(c==0x80)return 0;
    if(c!='e'&&c!='E')return 1;
    c=input_next(d,&after);if(c!='u'&&c!='U')return 1;
    c=input_next(d,&after);if(c!='r'&&c!='R')return 1;
    c=input_next(d,&after);
    if(c=='o'||c=='O')c=input_next(d,&after);
    return letter(c);
}
