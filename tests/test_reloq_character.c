#include "reloq_character.h"
#include "reloq_english_voice.h"
#include "reloq_german_best_effort_text.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int cancel(void *unused){(void)unused;return 1;}
static unsigned char *english(const char *s)
{
    unsigned char *out=NULL;size_t n=0,spelled=0;
    assert(!reloq_english_text_annotation((const unsigned char *)s,strlen(s),&out,&n,&spelled));
    assert(out&&strlen((char *)out)==n);return out;
}
static void equivalent(const char *a,const char *b)
{
    unsigned char *x=english(a),*y=english(b);
    assert(!strcmp((char *)x,(char *)y));free(x);free(y);
}
int main(void)
{
    struct reloq_character_reading r;
    assert(reloq_character_read(0xe8,1,&r)==1&&r.original_spr&&r.count==2);
    assert(!strncmp(r.groups[1],".1grav]",7));
    assert(reloq_character_read(0xe8,0,&r)==1&&r.original_spr&&r.count==2);
    assert(!strncmp(r.groups[1],".1gra.0vIs]",10));
    assert(reloq_character_read(0x113,1,&r)==1&&!r.original_spr&&r.count==2);
    assert(!strncmp(r.groups[0],".1i]",4));
    assert(reloq_character_read(0x1d8,0,&r)==1&&r.count==2);
    /* U with trema and acute extends the native umlaut reading. */
    assert(!strncmp(r.groups[0],".1y]",4));
    assert(reloq_character_read(0x416,0,&r)==0);
    unsigned char *s=english("è");assert(strstr((char *)s,".1grav]")&&!strstr((char *)s,".1grev]"));free(s);
    equivalent("è","e\xcc\x80");equivalent("ē","e\xcc\x84");
    equivalent("ǘ","u\xcc\x88\xcc\x81");equivalent("café","cafe\xcc\x81");
    /* A word retains its word analysis; an isolated letter retains accents. */
    equivalent("café","cafe");
    unsigned char bytes[8]={0x55};uint32_t ids[8]={0x5555};size_t n=77;
    assert(!reloq_character_encode_text((const unsigned char *)"èē",4,bytes,ids,8,&n,NULL,NULL));
    assert(n==2&&bytes[0]==0xe8&&bytes[1]=='e'&&ids[0]==0xe8&&ids[1]==0x113);
    bytes[0]=0x55;ids[0]=0x5555;
    assert(reloq_character_encode_text((const unsigned char *)"ē",2,bytes,ids,1,&n,NULL,NULL)==-2);
    assert(bytes[0]==0x55&&ids[0]==0x5555&&n==1);
    assert(reloq_character_encode_text((const unsigned char *)"ē",2,bytes,ids,8,&n,cancel,NULL)==1);
    assert(bytes[0]==0x55&&ids[0]==0x5555);
    puts("Shared original/extended character path, native accent pronunciation, NFC/NFD, word context and transactional codec passed");
    return 0;
}
