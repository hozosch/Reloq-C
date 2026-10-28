/* Typed lexical observations shared by foreign acoustic-bank dispatch. */
#include "reloq61_english_bank.h"
#include "delta.h"
#include <stdlib.h>
#include <string.h>
static _Thread_local struct reloq61_english_bank *current_bank;
void *reloq61_english_bank_scope(struct reloq61_english_bank*p){void*previous=current_bank;current_bank=p;return previous;}
int reloq61_english_bank_active(void){return current_bank!=NULL;}
void reloq61_english_bank_free(struct reloq61_english_bank*p){if(p){free(p->spans);free(p->atoms);free(p->syllables);free(p->pitch);free(p);}}
static int32_t edge(int32_t p){int32_t v;memcpy(&v,EVV_AT(char*,p)+4,4);return v;}
static int32_t right(delta_state*d,int32_t p,unsigned field){return EVV_REF(vmover(d,EVV_AT(void*,p),(uint8_t)field));}
static int32_t record(delta_state*d,int32_t p,unsigned field){int32_t n=vgetsc(d,0,0,right(d,p,field),(uint8_t)field);return n&&!(*EVV_AT(int32_t*,n)&2)?n:0;}
static int32_t owner(delta_state*d,int32_t p,unsigned field){return record(d,vgetsc(d,1,1,p,(uint8_t)field),field);}
void reloq61_english_bank_words(void*state,int32_t a,int32_t b){
 struct reloq61_english_bank*p=current_bank;if(!p||p->rendering)return;delta_state*d=state;int32_t at=edge(a),end=right(d,edge(b),4);
 for(unsigned guard=0;right(d,at,4)!=end&&guard<255;++guard){int32_t n=record(d,at,4);if(!n)return;const unsigned char*body=TFLDS(EVV_AT(void*,n));int16_t name;memcpy(&name,body,2);
  if(name>0&&name<=255&&!p->early_present[name-1]){memcpy(p->early[name-1].fields,body,16);p->early_present[name-1]=1;}
  int32_t next=EVV_AT(int32_t*,n)[1]&~3;if(!next||next==at)return;at=next;
 }
}
void reloq61_english_bank_phones(void*state,int32_t a,int32_t b){
 struct reloq61_english_bank*p=current_bank;if(!p||p->failed||p->rendering)return;delta_state*d=state;int32_t at=edge(a),end=right(d,edge(b),2),last_word=p->last_word,last_syllable=p->last_syllable;unsigned word=p->word_cursor,syllable=p->syllable_cursor;
 for(unsigned guard=0;right(d,at,2)!=end&&guard<4096;++guard){
  int32_t n=record(d,at,2);if(!n){p->failed=1;return;}const unsigned char*body=TFLDS(EVV_AT(void*,n));unsigned id=body[8];const delta_stmt*t=vstmtbl+2;
  if(id>=(unsigned)t->fields[0].nvalues){p->failed=1;return;}const char*name=((const char*const*)t->fields[0].values)[id];
  if(strcmp(name,"GAP")&&strcmp(name,"#")){
   int32_t from=*EVV_AT(int32_t*,n)&~3,wn=owner(d,from,4),sn=owner(d,from,7);
   if(!wn||!sn||p->phone_count==255||strlen(name)>3){p->failed=1;return;}
   if(wn!=last_word){if(word==255){p->failed=1;return;}last_word=wn;const unsigned char*w=TFLDS(EVV_AT(void*,wn));struct reloq61_bank_word*copy=p->words+word++;memcpy(copy->fields,w,16);memcpy(p->pitch_word_stress+word-1,w+8,2);int16_t number;memcpy(&number,w,2);if(number>0&&number<=255&&p->early_present[number-1])memcpy(copy->fields+8,p->early[number-1].fields+8,2);}
   if(sn!=last_syllable){last_syllable=sn;++syllable;}
   struct reloq61_bank_phone*out=p->phones+p->phone_count++;memset(out,0,sizeof*out);strcpy(out->name,name);memcpy(&out->stress,body+6,2);memcpy(&out->duration,body+12,2);memcpy(out->traits,body,6);out->transition=body[9];out->ghost=body[10];out->after=body[11];out->word=(uint16_t)word;out->syllable=(uint16_t)syllable;memcpy(&out->syllable_stress,TFLDS(EVV_AT(void*,sn)),2);
  }
  int32_t next=EVV_AT(int32_t*,n)[1]&~3;if(!next||next==at){p->failed=1;return;}at=next;
 }
 p->word_count=word;p->last_word=last_word;p->last_syllable=last_syllable;p->word_cursor=word;p->syllable_cursor=syllable;
}

/* A foreign word inside a continuing Mandarin phrase is not an isolated
 * English phrase. Feed that boundary into the existing duration rules. */
void reloq61_english_bank_duration_context(void *state)
{
 const struct reloq61_english_bank*p=current_bank;
 if(p&&p->rendering&&p->nonfinal){
  int16_t continuing=0;const unsigned fields[]={198,330,334};
  for(unsigned i=0;i<sizeof fields/sizeof*fields;++i)memcpy((char*)state+DG_BASE+fields[i],&continuing,2);
 }
}

/* Observe the donor's completed acoustic timing, after phone transitions have
 * been inserted. Symbolic phone boundaries cannot supply these durations. */
static void *bank_grow(void *buffer,size_t *capacity,size_t count,size_t width,size_t limit)
{
 if(count<*capacity)return buffer;
 size_t n=*capacity?*capacity*2:16;if(n>limit)n=limit;
 if(n<=count)return NULL;void *out=realloc(buffer,n*width);
 if(out)*capacity=n;return out;
}
void reloq61_english_bank_timing(void *state)
{
 struct reloq61_english_bank *p=current_bank;if(!p||p->failed)return;
 delta_state *d=state;int32_t left,end,from,to;
 memcpy(&left,(char*)d+DG_BASE+248,4);memcpy(&end,(char*)d+DG_BASE+256,4);
 memcpy(&from,(char*)d+DG_BASE+1168,4);memcpy(&to,(char*)d+DG_BASE+1176,4);
 int32_t sn=owner(d,left,7),wn=owner(d,left,4);int16_t stress=0,word=0;
 if(!sn||!wn||from<0||to<from){p->failed=1;return;}
 memcpy(&stress,TFLDS(EVV_AT(void*,sn)),2);memcpy(&word,TFLDS(EVV_AT(void*,wn)),2);
 void *more=bank_grow(p->syllables,&p->syllable_capacity,p->syllable_count,sizeof*p->syllables,255);
 if(!more){p->failed=1;return;}p->syllables=more;
 struct reloq61_bank_syllable *sy=p->syllables+p->syllable_count++;
 *sy=(struct reloq61_bank_syllable){from/10000,to/10000,p->atom_count,0,stress,(uint16_t)word};
 int clock=sy->from;int32_t at=left;
 for(unsigned guard=0;right(d,at,8)!=right(d,end,8)&&guard<4096;++guard){
  int32_t msnode=record(d,at,8),ph=owner(d,at,2);int16_t ms;
  if(!msnode||!ph){p->failed=1;return;}memcpy(&ms,TFLDS(EVV_AT(void*,msnode)),2);
  if(ms<0){p->failed=1;return;}
  more=bank_grow(p->atoms,&p->atom_capacity,p->atom_count,sizeof*p->atoms,4096);
  if(!more){p->failed=1;return;}p->atoms=more;
  struct reloq61_bank_atom *out=p->atoms+p->atom_count++;memset(out,0,sizeof*out);
  const unsigned char *body=TFLDS(EVV_AT(void*,ph));unsigned id=body[8];
  const char *name=((const char*const*)vstmtbl[2].fields[0].values)[id];
  if(strlen(name)>3){p->failed=1;return;}strcpy(out->phone.name,name);
  memcpy(out->phone.traits,body,6);memcpy(&out->phone.stress,body+6,2);
  out->phone.transition=body[9];out->phone.ghost=body[10];out->phone.after=body[11];
  memcpy(&out->phone.duration,body+12,2);out->from=clock;out->to=clock+ms;
  out->group=ph;clock+=ms;int32_t next=EVV_AT(int32_t*,msnode)[1]&~3;
  if(!next||next==at){p->failed=1;return;}at=next;
 }
 sy->end=p->atom_count;
 if(clock!=sy->to||word<=0||word>255||sy->begin==sy->end)p->failed=1;
}
