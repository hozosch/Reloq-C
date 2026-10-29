#include "delta.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "reloq61_english_bank.h"
#include "reloq61_latin.h"
#include "delta_rules_c.h"
#include <stdio.h>
#include <stdlib.h>
static int32_t argument_point(int32_t a){int32_t p;memcpy(&p,EVV_AT(char*,a)+4,4);return p;}
static int32_t right_point(delta_state*d,int32_t p,unsigned field){return EVV_REF(vmover(d,EVV_AT(void*,p),(uint8_t)field));}
static int32_t projected_record(delta_state*d,int32_t p,unsigned field){int32_t n=vgetsc(d,0,0,right_point(d,p,field),(uint8_t)field);return n&&!(*EVV_AT(int32_t*,n)&2)?n:0;}


static delta_fielddesc word_fields[11];static uint16_t word_offsets[11];
static void (*word_put[11])(void*,const void*);static int word_ready;
static const char *const bank_names[]={"native","english"};
static const char *word_value_names[11][64];
static int word_value(int field,const char*name)
{
 delta_fielddesc*f=word_fields+field;
 for(int i=0;i<f->nvalues;++i)if(!strcmp(word_value_names[field][i],name))return i;
 if(f->nvalues>=64)return -1;
 word_value_names[field][f->nvalues]=name;return f->nvalues++;
}
static void put_word_bank(void*p,const void*v){memcpy((char*)p+9,v,1);}
int install_word_fields(void)
{
 delta_stmt*t=&vstmtbl[4];if(word_ready)return 0;if(t->nfields!=10||t->length!=16)return -1;
 memcpy(word_fields,t->fields,10*sizeof*word_fields);memcpy(word_offsets,t->field_offset,10*sizeof*word_offsets);memcpy(word_put,t->put,10*sizeof*word_put);
 const delta_stmt*en=delta_lang_by_id(0x10000)->stmtbl+4;
 for(int f=1;f<=2;++f){
  memcpy(word_value_names[f],t->fields[f].values,t->fields[f].nvalues*sizeof(char*));
  word_fields[f].values=word_value_names[f];
  const char*const*values=en->fields[f].values;
  for(int i=0;i<en->fields[f].nvalues;++i)if(word_value(f,values[i])<0)return -1;
 }
 word_fields[10]=(delta_fielddesc){.name="forgn",.format="%d",.values=bank_names,.nvalues=2,.kind=-1};word_offsets[10]=9;word_put[10]=put_word_bank;
 t->fields=word_fields;t->field_offset=word_offsets;t->put=word_put;t->nfields=11;word_ready=1;return 0;
}
static char input_character(delta_state*d,int32_t at,int32_t*next)
{
 int32_t n=projected_record(d,at,1);if(!n)return 0;void*p=TFLDS(EVV_AT(void*,n));unsigned id=*(unsigned char*)delta_record_field(&vstmtbl[1],0,p);
 if(id>=(unsigned)vstmtbl[1].fields[0].nvalues)return 0;const char*s=((const char*const*)vstmtbl[1].fields[0].values)[id];*next=EVV_AT(int32_t*,n)[1]&~3;return s&&s[0]&&!s[1]?s[0]:0;
}
int reloq_zhcn61_bank_property(void*state,int32_t a)
{
 delta_state*d=state;int32_t at=argument_point(a),next;char g=input_character(d,at,&next);if(g!='g')return -1;at=next;char kind=input_character(d,at,&next);at=next;if(input_character(d,at,&next)!='_')return -1;
 return kind=='9'?1:((kind>='0'&&kind<='9')||(kind>='a'&&kind<='z'))?0:-1;
}
int reloq_zhcn61_bank_pending(void*state){return *((unsigned char*)state+DG_BASE+2284+9)!=0;}
void reloq_zhcn61_bank_set_property(void*state,int value){*((unsigned char*)state+DG_BASE+2284+9)=(unsigned char)value;}
static int bank_letter(unsigned char c){return reloq61_latin_letter_byte(c);}
int32_t reloq_zhcn61_bank_letters(void*state,int32_t a,int32_t b)
{
 if(!reloq_zhcn61_bank_pending(state))return -1;delta_state*d=state;int32_t at=argument_point(a),last=at;unsigned count=0,letters=0;char prev=0;
 for(;count<255;++count){int32_t next;char c=input_character(d,last,&next);int alpha=bank_letter((unsigned char)c);int valid=alpha||(c>='0'&&c<='9')||(count&&(c=='\''||c=='-'));
  if(c=='-'){int32_t after;char following=input_character(d,next,&after);valid=count&&bank_letter((unsigned char)following);}
  /* Keep internal initials together; a final period remains punctuation. */
  if(c=='.'&&count){int32_t after;char q=input_character(d,next,&after);valid=(prev>='0'&&prev<='9'&&q>='0'&&q<='9')||(bank_letter((unsigned char)prev)&&bank_letter((unsigned char)q));}
  if(!valid)break;letters+=alpha;prev=c;last=next;
 }
 if(!count||!letters)return -1;memcpy(EVV_AT(char*,b)+4,&last,4);memcpy((char*)state+DG_BASE+1386,(char*)state+DG_BASE+1406,2);return 0;
}


static delta_fielddesc phone_fields[15];static uint16_t phone_offsets[15];
static void(*phone_put[15])(void*,const void*);static uint8_t phone_variants[1280];
static const char*phone_names[128];static const char*feature_names[15][64];
static int phone_count,phone_ready;
static void put_phone_ghost(void*p,const void*v){memcpy((char*)p+11,v,1);}
static int feature_id(int field,const char*name)
{
 delta_fielddesc*f=phone_fields+field;for(int i=0;i<f->nvalues;++i)if(!strcmp(feature_names[field][i],name))return i;
 if(f->nvalues==64)return -1;feature_names[field][f->nvalues]=name;return f->nvalues++;
}
int install_phone_fields(void)
{
 if(phone_ready)return phone_count;delta_stmt*t=&vstmtbl[2];const delta_stmt*en=delta_lang_by_id(0x10000)->stmtbl+2;
 if(t->nfields!=14||t->length!=18||t->stride!=10||t->fields[0].nvalues!=47)return -1;
 memcpy(phone_fields,t->fields,14*sizeof*phone_fields);memcpy(phone_offsets,t->field_offset,14*sizeof*phone_offsets);memcpy(phone_put,t->put,14*sizeof*phone_put);memcpy(phone_variants,t->variants,470);
 for(int f=1;f<14;++f)if(t->fields[f].values){if(t->fields[f].nvalues>64)return -1;memcpy(feature_names[f],t->fields[f].values,t->fields[f].nvalues*sizeof(char*));phone_fields[f].values=feature_names[f];}
 phone_count=47;memcpy(phone_names,t->fields[0].values,47*sizeof(char*));
 const char*const*names=en->fields[0].values;
 for(int p=0;p<en->fields[0].nvalues;++p){int id=0;for(;id<phone_count;++id)if(!strcmp(phone_names[id],names[p]))break;if(id<phone_count)continue;
  if(phone_count==128)return -1;id=phone_count++;phone_names[id]=names[p];unsigned char*row=phone_variants+id*10;memcpy(row,t->deflt,10);
  for(int f=1;f<=6;++f){const char*const*values=en->fields[f].values;unsigned value=en->variants[p*en->stride+en->field_offset[f]];int mapped=feature_id(f,values[value]);if(mapped<0)return -1;row[t->field_offset[f]]=(uint8_t)mapped;}
 }
 phone_fields[0].values=phone_names;phone_fields[0].nvalues=(int16_t)phone_count;
 phone_fields[14]=en->fields[9];phone_offsets[14]=11;phone_put[14]=put_phone_ghost;
 t->fields=phone_fields;t->field_offset=phone_offsets;t->put=phone_put;t->variants=phone_variants;t->nfields=15;phone_ready=1;return phone_count;
}
static int source_text(delta_state*d,int32_t first,int32_t last,char*out,size_t cap)
{
 int32_t end=right_point(d,last,1),at=first;size_t n=0;
 while(right_point(d,at,1)!=end){int32_t next;char c=input_character(d,at,&next);if(!c||n+1==cap)return -1;out[n++]=c;at=next;}
 out[n]=0;return (int)n;
}
int word_text(void*state,int32_t a,int32_t b,char*out,size_t cap)
{
 delta_state*d=state;int32_t first=argument_point(a),word=projected_record(d,first,4);
 if(!word||!*((unsigned char*)TFLDS(EVV_AT(void*,word))+9))return 0;
 return source_text(d,first,argument_point(b),out,cap);
}

int reloq_zhcn61_bank_init(void){return install_word_fields()||install_phone_fields()!=97?94:0;}
int reloq_zhcn61_bank_word(void*state,int32_t a){delta_state*d=state;int32_t point=vgetsc(d,1,1,argument_point(a),4);int32_t n=projected_record(d,point,4);return n&&*((unsigned char*)TFLDS(EVV_AT(void*,n))+9)!=0;}
int reloq_zhcn61_bank_character(int character){const char*const*names=vstmtbl[1].fields[0].values;for(int i=0;i<vstmtbl[1].fields[0].nvalues;++i)if(names[i]&&names[i][0]==character&&!names[i][1])return i;return -1;}
int reloq_zhcn61_bank_dot(void){return reloq_zhcn61_bank_character('.');}
static int32_t record_end(int32_t p){return EVV_AT(int32_t*,p)[1]&~3;}
/* The native word builder has already applied bank annotations. Preserve
 * punctuation, line breaks and voice/rate commands between English words. */
static int continuing_source(delta_state*d,int32_t left,int32_t right)
{
 int32_t end=right_point(d,right,1),at=left;
 for(unsigned guard=0;right_point(d,at,1)!=end&&guard<4096;++guard){
  int32_t next;char c=input_character(d,at,&next);
  if(c==' '||c=='\t'){at=next;continue;}
  if(c!='`')return 0;
  const char*tag="`g9_";
  for(unsigned i=0;tag[i];++i){if(right_point(d,at,1)==end||input_character(d,at,&next)!=tag[i])return 0;at=next;}
 }
 return right_point(d,at,1)==end;
}
static struct reloq61_english_bank *build_foreign_run(delta_state*d,int32_t left,int32_t*end)
{
 char text[256],token[256];int32_t ends[128];size_t lengths[128],count=1;
 int length=source_text(d,left,*end,text,sizeof text);if(length<=0)return NULL;
 lengths[0]=(size_t)length;ends[0]=*end;
 int32_t phrase=vgetsc(d,1,1,left,5),last=*end;
 while(count<128){
  int32_t word=projected_record(d,last,4);if(!word)break;
  const unsigned char*w=TFLDS(EVV_AT(void*,word));
  int32_t wl=*EVV_AT(int32_t*,word)&~3,wr=record_end(word);
  if(!w[9]||wr==last||vgetsc(d,1,1,wl,5)!=phrase||!continuing_source(d,last,wl)||reloq61_english_bank_for_word(d,word))break;
  int n=source_text(d,wl,wr,token,sizeof token);
  if(n<=0||(size_t)length+1+(size_t)n>=sizeof text)break;
  text[length++]=' ';memcpy(text+length,token,(size_t)n+1);length+=n;
  last=wr;ends[count]=last;lengths[count++]=(size_t)length;
 }
 /* Fit the bounded phone plan before changing the host graph. The plan is
  * built once and consumed by this sentence, with no retained text results. */
 for(;;){
  text[lengths[count-1]]=0;
  struct reloq61_english_bank*p=reloq61_english_bank_build(d,text);
  if(p){*end=ends[count-1];return p;}
  if(count==1)return NULL;count=(count+1)/2;
 }
}
static int project_word(delta_state*d,int32_t left,int32_t end,struct reloq61_english_bank*bank)
{
 if(!bank->phone_count||bank->phone_count>255||!bank->word_count||bank->word_count>255)return 94;
 int32_t host=projected_record(d,left,4);if(!host)return 94;int16_t host_name;memcpy(&host_name,TFLDS(EVV_AT(void*,host)),2);
 unsigned replaced=0;int32_t cursor=left,limit=right_point(d,end,4);
 for(;right_point(d,cursor,4)!=limit&&replaced<255;++replaced){int32_t word=projected_record(d,cursor,4);if(!word)return 94;int32_t next=record_end(word);if(!next||next==cursor)return 94;cursor=next;}
 if(!replaced||right_point(d,cursor,4)!=limit)return 94;
 const delta_stmt*en=delta_lang_by_id(0x10000)->stmtbl+2;unsigned char ids[255];
 for(size_t i=0;i<bank->phone_count;++i){unsigned id=0;for(;id<(unsigned)phone_count;++id)if(!strcmp(phone_names[id],bank->phones[i].name))break;if(id==(unsigned)phone_count)return 94;ids[i]=(unsigned char)id;}
 delta_token first={0,left},last={0,end};lpta_rpta_loadp(d,&first,&last);vdel_2pt(d,7,left,end);
 static const unsigned char morph=1;lpta_rpta_loadp(d,&first,&last);if(insert_2pt_s(d,3,1,&morph,0))return 94;
 lpta_rpta_loadp(d,&first,&last);if(insert_2pt_s(d,2,(uint8_t)bank->phone_count,ids,0))return 94;
 int32_t points[256];points[0]=left;int32_t at=left;
 for(size_t i=0;i<bank->phone_count;++i){int32_t n=projected_record(d,at,2);if(!n)return 94;unsigned char*p=TFLDS(EVV_AT(void*,n));const struct reloq61_bank_phone*source=bank->phones+i;
  for(int f=1;f<=6;++f){const char*const*names=en->fields[f].values;int id=feature_id(f,names[source->traits[f-1]]);if(id<0)return 94;*(uint8_t*)delta_record_field(vstmtbl+2,f,p)=(uint8_t)id;}
  memcpy(p+12,&source->stress,2);p[9]=source->transition;p[11]=source->ghost;p[14]=source->after;memcpy(p+16,&source->duration,2);at=record_end(n);points[i+1]=at;
 }
 for(size_t i=0;i<bank->phone_count;){size_t j=i+1;while(j<bank->phone_count&&bank->phones[j].syllable==bank->phones[i].syllable)++j;uint8_t stress=(uint8_t)bank->phones[i].syllable_stress;delta_token a={0,points[i]},b={0,points[j]};lpta_rpta_loadp(d,&a,&b);if(insert_2pt_s(d,7,1,&stress,0))return 94;int32_t syl=projected_record(d,points[i],7);if(!syl)return 94;memcpy(TFLDS(EVV_AT(void*,syl)),&bank->phones[i].syllable_stress,2);i=j;}
 vdel_2pt(d,4,left,end);unsigned converted=0;const delta_stmt*enw=delta_lang_by_id(0x10000)->stmtbl+4;
 for(size_t i=0;i<bank->phone_count;){size_t j=i+1;unsigned id=bank->phones[i].word;if(!id||id!=converted+1||id>bank->word_count)return 94;while(j<bank->phone_count&&bank->phones[j].word==id)++j;
  const unsigned char*w=bank->words[id-1].fields;const unsigned char zero=0;delta_token a={0,points[i]},b={0,points[j]};lpta_rpta_loadp(d,&a,&b);if(insert_2pt_s(d,4,1,&zero,0))return 94;int32_t n=projected_record(d,points[i],4);if(!n)return 94;unsigned char*p=TFLDS(EVV_AT(void*,n));
  const char*const*cats=enw->fields[1].values,*const*subcats=enw->fields[2].values;int cat=word_value(1,cats[w[2]]),sub=word_value(2,subcats[w[3]]);if(cat<0||sub<0)return 94;
  int16_t name=(int16_t)(host_name+converted);memcpy(p,&name,2);p[2]=(uint8_t)cat;p[3]=(uint8_t)sub;memcpy(p+4,w+8,2);p[6]=w[10];p[7]=w[11];p[8]=w[12];p[9]=1;bank->word_points[converted]=*EVV_AT(int32_t*,n)&~3;memcpy(p+12,w+14,2);
  int16_t syllables=1;for(size_t k=i+1;k<j;++k)syllables+=bank->phones[k].syllable!=bank->phones[k-1].syllable;memcpy(p+14,&syllables,2);++converted;i=j;
 }
 if(converted!=bank->word_count)return 94;
 if(converted!=replaced){int32_t phrase_end;memcpy(&phrase_end,(char*)d+DG_BASE+544,4);at=end;int32_t limit=right_point(d,phrase_end,4);
  for(unsigned guard=0;guard<4096&&right_point(d,at,4)!=limit;++guard){int32_t n=projected_record(d,at,4);if(!n)return 94;unsigned char*p=TFLDS(EVV_AT(void*,n));int16_t name;memcpy(&name,p,2);if(name>0){name+=(int16_t)((int)converted-(int)replaced);memcpy(p,&name,2);}int32_t next=record_end(n);if(!next||next==at)return 94;at=next;}
 }
 return 0;
}
struct bank_edit {delta_rule_block scope;int32_t status;delta_loc loc[2];};
int reloq_zhcn61_bank_assign(void*state,int32_t a,int32_t b)
{
 delta_state*host_state=state;int32_t existing_word=projected_record(host_state,argument_point(a),4);
 if(existing_word){const unsigned char*w=TFLDS(EVV_AT(void*,existing_word));if(w[9]&&reloq61_english_bank_for_word(state,existing_word))return 0;}
 char text[256];int length=word_text(state,a,b,text,sizeof text);if(length<=0)return -1;
 int32_t end=argument_point(b);
 struct reloq61_english_bank*bank=build_foreign_run(state,argument_point(a),&end);if(!bank)return 94;unsigned index=reloq61_english_bank_attach(state,bank);if(!index){reloq61_english_bank_free(bank);return 94;}
 delta_state*d=state;struct bank_edit*f=evv_frame_push(sizeof*f);if(!f)return 94;memset(f,0,sizeof*f);f->status=94;volatile int entered=0;
 if(EVV_LAND_SAVE((intptr_t)f->scope.landing))goto done;if(ventproc(d,&f->scope.rec,f->scope.fence[0],f->scope.fence[1],f->scope.fence[2],f->scope.landing))goto done;entered=1;fence(d,0,NULL);
 bank->host_left=argument_point(a);bank->host_right=end;
 f->status=project_word(d,bank->host_left,bank->host_right,bank);
 done:if(entered&&vretproc(d,f->status?94:1))f->status=94;int result=f->status;evv_land_forget((uintptr_t)f,(uintptr_t)f+sizeof*f);evv_frame_pop(f);return result;
}

static int32_t owner_record(delta_state*d,int32_t at,unsigned field){return projected_record(d,vgetsc(d,1,1,at,(uint8_t)field),field);}
int reloq_zhcn61_bank_restore(void*state,int32_t a,int32_t b)
{
 delta_state*d=state;int32_t at=argument_point(a),end=right_point(d,argument_point(b),4);
 for(unsigned guard=0;right_point(d,at,4)!=end&&guard<4096;++guard){int32_t n=projected_record(d,at,4);if(!n)break;unsigned char*w=TFLDS(EVV_AT(void*,n));int32_t next=record_end(n);
  if(w[9]){const struct reloq61_english_bank*bank=reloq61_english_bank_for_word(state,n);if(bank){int32_t point=bank->host_left;for(size_t i=0;i<bank->phone_count;++i){int32_t phone=projected_record(d,point,2),syllable=owner_record(d,point,7);if(!phone||!syllable)return 94;memcpy((char*)TFLDS(EVV_AT(void*,phone))+12,&bank->phones[i].stress,2);memcpy(TFLDS(EVV_AT(void*,syllable)),&bank->phones[i].syllable_stress,2);point=record_end(phone);}}}
  if(!next||next==at)break;at=next;
 }
 return 0;
}

int reloq_zhcn61_bank_print(void*state,int32_t a,int32_t b)
{
 if(!reloq_zhcn61_bank_word(state,a))return -1;delta_state*d=state;int32_t at=argument_point(a),n=projected_record(d,at,2);if(!n)return 94;
 int32_t from=*EVV_AT(int32_t*,n)&~3,syllable=owner_record(d,from,7);if(!syllable)return 94;int32_t end=record_end(syllable),limit=right_point(d,end,2);
 if((*EVV_AT(int32_t*,syllable)&~3)==from){int16_t stress;memcpy(&stress,TFLDS(EVV_AT(void*,syllable)),2);char digit[12];snprintf(digit,sizeof digit,"%d",stress);print_lit(d,8,digit);}
 for(unsigned guard=0;right_point(d,at,2)!=limit&&guard<255;++guard){n=projected_record(d,at,2);if(!n)return 94;const unsigned char*p=TFLDS(EVV_AT(void*,n));unsigned name=p[10];if(name>=(unsigned)phone_count)return 94;print_lit(d,8,phone_names[name]);int32_t next=record_end(n);if(!next||next==at)return 94;at=next;}
 memcpy(EVV_AT(char*,b)+4,&end,4);return 0;
}
extern int32_t reloq_zhcn61_init_word_vars(void*,int32_t);
extern int32_t reloq_zhcn61_set_klattparms(void*,int32_t);
extern int32_t reloq_zhcn61_init_syll_vars(void*,int32_t);
extern int32_t reloq_zhcn61_set_syll_offset_vals(void*,int32_t);
extern int32_t reloq_zhcn61_insert_pause_durs(void*,int32_t);
extern int32_t reloq_zhcn61_text_index(void*,int32_t,int32_t);
extern int32_t reloq_zhcn61_handle_pause(void*,int32_t,int32_t);
extern int32_t reloq_zhcn61_set_phone_offset_vals(void*,int32_t);
#include "foreign_pitch_zhcn61.h"
/* Give the English duration rules the punctuation that actually follows
 * their run, instead of ending every donor request with an implicit period. */
static void foreign_boundary(delta_state*d,struct reloq61_english_bank*p)
{
 if(p->trim_pause)return;
 int32_t at=p->host_right;
 for(unsigned guard=0;guard<16;++guard){
  int32_t next;char c=input_character(d,at,&next);
  if(c==' '||c=='\t'){at=next;continue;}
  if(c&&strchr(",.;:!?",c)){size_t n=strlen(p->text);
   if(n+1<sizeof p->text){p->text[n]=c;p->text[n+1]=0;}
  }
  break;
 }
}
int reloq_zhcn61_bank_speech(void*state,int32_t machine)
{
 delta_state*d=state;int32_t left,end;memcpy(&left,(char*)d+DG_BASE+160,4);memcpy(&end,(char*)d+DG_BASE+168,4);int32_t word=projected_record(d,left,4);if(!word)return -1;const unsigned char*w=TFLDS(EVV_AT(void*,word));if(pitch_prepare_phrase(d,left))return 94;if(!w[9])return -1;struct reloq61_english_bank*bank=reloq61_english_bank_for_word(state,word);if(!bank)return 94;
 /* A lexical token may expand to several English word records. Its bank is
  * emitted once; the outer iterator still visits each original word. */
 if(bank->emitted)return 0;int32_t saved_end=end;end=bank->host_right;
 struct bank_edit*f=evv_frame_push(sizeof*f);if(!f)return 94;memcpy((char*)d+DG_BASE+168,&end,4);memset(f,0,sizeof*f);f->status=94;volatile int entered=0;
 if(EVV_LAND_SAVE((intptr_t)f->scope.landing))goto done;if(ventproc(d,&f->scope.rec,f->scope.fence[0],f->scope.fence[1],f->scope.fence[2],f->scope.landing))goto done;entered=1;fence(d,0,NULL);
 if(reloq_zhcn61_init_word_vars(state,machine))goto done;
 if(reloq_zhcn61_set_klattparms(state,machine))goto done;
 f->loc[0].value=left;if(reloq_zhcn61_text_index(state,machine,EVV_REF(f->loc)))goto done;
 if(!bank->emitted){int32_t phrase=owner_record(d,bank->host_left,5);bank->trim_pause=phrase&&right_point(d,record_end(phrase),4)!=right_point(d,bank->host_right,4);foreign_boundary(d,bank);if(reloq61_english_bank_render(state,bank,bank->trim_pause))goto done;int32_t duration=bank->spans[bank->span_count-1].to;if(bank->trim_pause&&bank->pause_to>bank->pause_from)duration=(int32_t)bank->pause_from;
  if(duration<=0||duration>32767)goto done;uint8_t ms[2]={(uint8_t)(duration>>8),(uint8_t)duration};
  f->loc[0].value=bank->host_left;f->loc[1].value=bank->host_right;lpta_rpta_loadp(d,(const delta_token*)f->loc,(const delta_token*)(f->loc+1));if(insert_2pt_i(d,8,2,ms,0))goto done;
  memcpy((char*)d+DG_BASE+248,&bank->host_left,4);memcpy((char*)d+DG_BASE+256,&bank->host_right,4);
  if(reloq_zhcn61_set_syll_offset_vals(state,machine))goto done;int32_t from,to;memcpy(&from,(char*)d+DG_BASE+1080,4);memcpy(&to,(char*)d+DG_BASE+1088,4);
  if(bank_host_pitch(d,machine,bank,from/10000,to/10000))goto done;if(reloq61_english_bank_emit(state,bank,from/10000,to/10000))goto done;bank->emitted=1;
  /* Commit the same phone clock and anchor as the native acoustic path.
   * Following Mandarin targets must begin after the bank just emitted. */
  memcpy((char*)d+DG_BASE+712,&bank->host_left,4);memcpy((char*)d+DG_BASE+720,&bank->host_right,4);
  if(reloq_zhcn61_set_phone_offset_vals(state,machine))goto done;
  memcpy((char*)d+DG_BASE+3080,&bank->host_right,4);
  free(bank->spans);bank->spans=NULL;bank->span_count=bank->span_capacity=0;}
 /* Pitch planning has already advanced every donor syllable exactly once. */
 if(bank->pitch){/* No phrase-final pause is a normal no-match, just as in the native
  * syllable caller. It must not abandon this bank's finalization. */
 (void)reloq_zhcn61_insert_pause_durs(state,machine);free(bank->pitch);bank->pitch=NULL;bank->pitch_count=0;free(bank->atoms);bank->atoms=NULL;bank->atom_count=bank->atom_capacity=0;free(bank->syllables);bank->syllables=NULL;bank->syllable_count=bank->syllable_capacity=0;}
 f->loc[0].value=end;lpta_loadp(d,(const delta_token*)f->loc);if(!setscan_r(d,2)&&!advanc(d))savescptr(d,14,f->loc);
 lpta_loadpn(d,(const delta_token*)f->loc);lpta_ctxtl(d,0);lpta_storep(d,f->loc+1);
 if(reloq_zhcn61_text_index(state,machine,EVV_REF(f->loc+1)))goto done;
 (void)reloq_zhcn61_handle_pause(state,machine,EVV_REF(f->loc));f->status=0;
 done:if(entered&&vretproc(d,f->status?94:1))f->status=94;int result=f->status;memcpy((char*)d+DG_BASE+168,&saved_end,4);evv_land_forget((uintptr_t)f,(uintptr_t)f+sizeof*f);evv_frame_pop(f);return result;
}
