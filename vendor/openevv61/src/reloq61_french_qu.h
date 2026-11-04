/* When French qu is followed by a terminal il, its u is silent: i must
 * supply the syllable's vowel and l must remain a consonant.  The native
 * input spine already carries normalized letters and word boundaries. */
static int reloq61_french_next_letter(delta_state *d,int32_t *at,int back,unsigned *type)
{
    if(!*at)return -1;
    int32_t node=vgetsc(d,back,0,*at,1);
    for(unsigned n=0;node&&(*EVV_AT(const int32_t *,node)&2)&&n<64;++n) {
        int32_t next=vgetsc(d,back,0,node,1);
        if(next==node)return -1;
        node=next;
    }
    if(!node||(*EVV_AT(const int32_t *,node)&2))return -1;
    void *record=TFLDS(EVV_AT(void *,node));
    unsigned name=*(const uint8_t *)delta_record_field(&vstmtbl[1],0,record);
    *type=*(const uint8_t *)delta_record_field(&vstmtbl[1],3,record);
    int32_t next=*EVV_AT(const int32_t *,node+(back?0:4))&~3;
    if(next==*at)return -1;
    *at=next;return (int)name;
}
static int reloq61_french_terminal_quil(delta_state *d,int32_t begin,int at_l)
{
    /* Input symbol numbers are shared by French (France and Canada). */
    enum { I=3,U=5,L=19,Q=23,S=25,LETTER=1 };
    int32_t left=begin,right=begin;unsigned type=0;
    if(at_l && reloq61_french_next_letter(d,&left,1,&type)!=I)return 0;
    if(reloq61_french_next_letter(d,&left,1,&type)!=U ||
       reloq61_french_next_letter(d,&left,1,&type)!=Q)return 0;
    if(!at_l && reloq61_french_next_letter(d,&right,0,&type)!=I)return 0;
    if(reloq61_french_next_letter(d,&right,0,&type)!=L)return 0;
    int following=reloq61_french_next_letter(d,&right,0,&type);
    if(following==S)following=reloq61_french_next_letter(d,&right,0,&type);
    return following<0 || type!=LETTER;
}
