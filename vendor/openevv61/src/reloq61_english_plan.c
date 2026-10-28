/* Word-scoped English phone plans. Capture lexical segments, overlay reduction,
 * then read final vowel color. The graph is never edited by these readers. */
#include "reloq61_english_plan.h"
#include "delta.h"
#include <string.h>
static _Thread_local struct reloq61_english_capture *active_plan;
void *reloq61_english_plan_scope(struct reloq61_english_capture *next)
{void *previous=active_plan;active_plan=next;return previous;}
static int32_t english_position(int32_t p){int32_t at;memcpy(&at,EVV_AT(char*,p)+4,4);return at;}
static void plan_begin(void*d,int32_t a,int32_t b){(void)d;struct reloq61_english_capture*c=active_plan;if(c){c->begin=english_position(a);c->end=english_position(b);c->start=c->count;++c->group;c->active=1;c->captured=0;}}
/* Preserve a letter's identity while English expands neighboring digits.
 * The origin byte has two ghost bits and five otherwise unused letter bits;
 * bit seven remains the temporary reduction-observed flag. */
int reloq61_english_letter_begin(void*state,int32_t a,int32_t b,struct reloq61_english_letter_mark*out)
{
 struct reloq61_english_capture*c=active_plan;if(!c||c->letter_count==c->letter_capacity)return 0;
 delta_state*d=state;int32_t left=english_position(a),end=english_position(b);
 int32_t at=EVV_REF(vmover(d,EVV_AT(void*,left),1));
 if(at==EVV_REF(vmover(d,EVV_AT(void*,end),1)))return 0;
 int32_t n=vgetsc(d,0,0,at,1);if(!n||(*EVV_AT(int32_t*,n)&2))return 0;
 void*p=TFLDS(EVV_AT(void*,n));unsigned id=*(unsigned char*)delta_record_field(&vstmtbl[1],0,p);
 if(id>=(unsigned)vstmtbl[1].fields[0].nvalues)return 0;
 const char*name=((const char*const*)vstmtbl[1].fields[0].values)[id];unsigned letter=0;
 if(name&&name[0]&&!name[1]){unsigned char ch=(unsigned char)name[0];if(ch>='a'&&ch<='z')ch-='a'-'A';if(ch>='A'&&ch<='Z')letter=ch-'A'+1;}
 *out=(struct reloq61_english_letter_mark){left,EVV_AT(int32_t*,n)[1]&~3,(uint8_t)letter};return 1;
}
void reloq61_english_letter_end(const struct reloq61_english_letter_mark*mark)
{struct reloq61_english_capture*c=active_plan;if(c&&c->letter_count<c->letter_capacity)c->letters[c->letter_count++]=*mark;}
static unsigned marked_letter(int32_t left,int32_t right)
{struct reloq61_english_capture*c=active_plan;if(c)for(size_t i=0;i<c->letter_count;++i)if(c->letters[i].left==left||c->letters[i].right==right)return c->letters[i].letter;return 0;}
/* Classify the source root span, including compound roots. Inflectional
 * endings retain their own class and cannot select a spelling class. */
/* Source letters and boundaries are read in place; no input rewrite. */
static unsigned char source_lower(unsigned char c)
{return c>='A'&&c<='Z'?(unsigned char)(c+'a'-'A'):c;}
static unsigned char source_char(delta_state*d,int32_t point,int back,int32_t*edge)
{
 point=back?EVV_REF(vmovel(d,EVV_AT(delta_node*,point),1)):EVV_REF(vmover(d,EVV_AT(void*,point),1));
 int32_t node=vgetsc(d,back,0,point,1);if(!node||(*EVV_AT(int32_t*,node)&2))return 0;
 unsigned id=*(unsigned char*)delta_record_field(&vstmtbl[1],0,TFLDS(EVV_AT(void*,node)));
 if(id>=(unsigned)vstmtbl[1].fields[0].nvalues)return 0;
 const char*name=((const char*const*)vstmtbl[1].fields[0].values)[id];if(!name||!name[0]||name[1])return 0;
 *edge=EVV_AT(const int32_t*,node)[back?0:1]&~3;
 unsigned char c=source_lower((unsigned char)name[0]);return c>='a'&&c<='z'?c:0;
}
static int source_boundary(delta_state*d,int32_t point,unsigned field)
{
 delta_vars*v=EVV_AT(delta_vars*,d->vars);
 point=EVV_REF(vmover(d,EVV_AT(void*,point),(uint8_t)field));
 return (*EVV_AT(const int32_t*,point+(v->fence_base+field)*4)&1)!=0;
}
static unsigned source_phone_flags(delta_state*d,int32_t left,int32_t right,int stop)
{
 int32_t begin=vgetsc(d,1,1,left,1);if(!begin)return 0;
 int32_t edge=0;unsigned char head=source_char(d,begin,0,&edge);
 if(stop)return head=='t'&&edge&&source_char(d,edge,0,&edge)=='t'?RELOQ_SOURCE_HEAD_TT:0;
 unsigned flags=head=='o'?RELOQ_SOURCE_HEAD_O:0;
 int32_t end=vgetsc(d,0,1,right,1);if(!end)return flags;
 if(source_char(d,end,1,&edge)=='a'&&source_boundary(d,end,3)&&!source_boundary(d,edge,7))flags|=RELOQ_SOURCE_AFTER_A;
 return flags;
}
static unsigned source_morph_class(delta_state*d,int32_t left,int32_t*previous,unsigned*value)
{
 struct reloq61_english_capture*c=active_plan;
 if(!c||!c->source_classes||!c->classify_source)return 0;
 int32_t begin=vgetsc(d,1,1,left,3);
 int32_t node=begin?vgetsc(d,0,0,begin,3):0;
 if(!node||(*EVV_AT(int32_t*,node)&2))return 0;
 if(node==*previous)return *value;
 *previous=node;*value=0;
 if(*(unsigned char*)delta_record_field(&vstmtbl[3],0,TFLDS(EVV_AT(void*,node)))!=2)return 0;
 int32_t end=EVV_AT(const int32_t*,node)[1]&~3;
 int32_t word_begin=vgetsc(d,1,1,left,4);
 int32_t word=word_begin?vgetsc(d,0,0,word_begin,4):0;
 if(word&&!(*EVV_AT(int32_t*,word)&2)){
  int32_t stop=EVV_REF(vmover(d,EVV_AT(void*,EVV_AT(const int32_t*,word)[1]&~3),3));
  int32_t scan=word_begin;int found=0;
  for(unsigned guard=0;guard<64;++guard){
   scan=EVV_REF(vmover(d,EVV_AT(void*,scan),3));if(scan==stop)break;
   int32_t morph=vgetsc(d,0,0,scan,3);if(!morph||(*EVV_AT(int32_t*,morph)&2))return 0;
   int32_t right=EVV_AT(const int32_t*,morph)[1]&~3;
   if(*(unsigned char*)delta_record_field(&vstmtbl[3],0,TFLDS(EVV_AT(void*,morph)))==2){if(!found)begin=scan;end=right;found=1;}
   if(!right||right==scan)return 0;scan=right;
  }
  if(scan!=stop)return 0;
 }
 int32_t at=EVV_REF(vmover(d,EVV_AT(void*,begin),1));
 end=EVV_REF(vmover(d,EVV_AT(void*,end),1));
 char text[64];size_t count=0;
 for(unsigned guard=0;guard<256;++guard){
  at=EVV_REF(vmover(d,EVV_AT(void*,at),1));if(at==end)break;
  int32_t n=vgetsc(d,0,0,at,1);if(!n||(*EVV_AT(int32_t*,n)&2)||count+1>=sizeof text)return 0;
  unsigned id=*(unsigned char*)delta_record_field(&vstmtbl[1],0,TFLDS(EVV_AT(void*,n)));
  if(id>=(unsigned)vstmtbl[1].fields[0].nvalues)return 0;
  const char*name=((const char*const*)vstmtbl[1].fields[0].values)[id];if(!name||!name[0]||name[1])return 0;
  text[count++]=name[0];int32_t next=EVV_AT(int32_t*,n)[1]&~3;if(!next||next==at)return 0;at=next;
 }
 if(at!=end)return 0;text[count]=0;*value=c->classify_source(text);
 if(count>=2&&source_lower((unsigned char)text[count-2])=='a'&&source_lower((unsigned char)text[count-1])=='l')*value|=RELOQ_SOURCE_ROOT_AL;
 return *value;
}
int reloq61_english_structured_ready(void)
{
 const struct reloq61_english_capture*c=active_plan;
 if(!c||!c->prefer_segments||c->failed||!c->captured||!c->count)return 0;
 for(size_t i=0;i<c->count;++i){const char*n=c->phones[i].name,*b=c->phones[i].base;
  if((n[1]&&strcmp(n,"a@")&&strcmp(n,"c@")&&strcmp(n,"Xx")&&strcmp(n,"GAP"))||
     (b[1]&&strcmp(b,"a@")&&strcmp(b,"c@")&&strcmp(b,"Xx")&&strcmp(b,"GAP")))return 0;
 }
 return 1;
}

static void capture_graph(void *state,int stage)
{
 struct reloq61_english_capture*c=active_plan;if(!c||c->failed||!c->active||(stage==0&&c->captured))return;
 if(!stage)c->captured=1;
 delta_state*d=state;int32_t at=EVV_REF(vmover(d,EVV_AT(void*,c->begin),2)),end=EVV_REF(vmover(d,EVV_AT(void*,c->end),2));size_t cursor=c->start;unsigned letter=stage?0:marked_letter(c->begin,c->end);
 int32_t previous_morph=0;unsigned morph_class=0;
 for(unsigned guard=0;at!=end&&guard<4096;++guard){
  int32_t n=vgetsc(d,0,0,at,2);if(!n||(*EVV_AT(int32_t*,n)&2)){c->failed=1;return;}
  void*b=TFLDS(EVV_AT(void*,n));unsigned id=*(unsigned char*)delta_record_field(&vstmtbl[2],0,b);if(id>=(unsigned)vstmtbl[2].fields[0].nvalues){c->failed=1;return;}
  const char*name=((const char*const*)vstmtbl[2].fields[0].values)[id];if(!name||strlen(name)>=4){c->failed=1;return;}
  int32_t left=*EVV_AT(int32_t*,n)&~3,right=EVV_AT(int32_t*,n)[1]&~3;struct reloq61_english_segment*p=NULL;
  if(!stage){if(c->count==c->capacity){c->failed=1;return;}if(c->source_classes){unsigned features=0;int is_vowel=strchr("iIeEAxXHRuUoacBYWO",name[0])!=NULL;
   if(is_vowel)features=source_morph_class(d,left,&previous_morph,&morph_class);
   if(is_vowel||name[0]=='t')features|=source_phone_flags(d,left,right,name[0]=='t');
   c->source_classes[c->count]=(uint32_t)features;}p=c->phones+c->count++;memset(p,0,sizeof*p);strcpy(p->name,name);strcpy(p->base,name);p->left=left;p->right=right;p->word=(uint8_t)c->group;p->origin=(uint8_t)(letter<<2);memcpy(&p->stress,delta_record_field(&vstmtbl[2],7,b),2);}
  else{
   for(size_t j=cursor;j<c->count;++j)if(c->phones[j].left==left){p=c->phones+j;cursor=j+1;break;}
   if(p&&((stage==1&&(!strchr("iIeEAxXHRuUoacBYWO",p->base[0])||strchr("iIeEAxXHRuUoacBYWO",name[0])))||(stage==2&&(((p->base[0]=='A'||p->base[0]=='E')&&strchr("iIeEAxXHRuUoacBYWO",name[0]))||(p->base[0]=='u'&&name[0]=='w')||(p->base[0]=='i'&&name[0]=='y'))))){memset(p->name,0,4);strcpy(p->name,name);p->origin=(p->origin&124)|128|*(unsigned char*)delta_record_field(&vstmtbl[2],9,b);}
  }
  if(!right||right==at){c->failed=1;return;}at=right;
 }
 if(at!=end)c->failed=1;
}
static void plan_early(void*d){capture_graph(d,0);}
void reloq61_english_projection(void*d){capture_graph(d,1);struct reloq61_english_capture*c=active_plan;if(!c||c->failed)return;for(size_t i=c->start;i+1<c->count;){if(!(c->phones[i].origin&128)&&strchr("iIeEAxXHRuUoacBYWO",c->phones[i].base[0])&&c->phones[i+1].base[0]=='r'){memmove(c->phones+i,c->phones+i+1,(--c->count-i)*sizeof*c->phones);if(c->source_classes)memmove(c->source_classes+i,c->source_classes+i+1,(c->count-i)*sizeof*c->source_classes);}else ++i;}c->active=0;}
void reloq61_english_surface(void*d){struct reloq61_english_capture*c=active_plan;if(!c||c->failed)return;c->active=1;capture_graph(d,2);c->active=0;}


void reloq61_english_lexical(void*d,int32_t a,int32_t b){plan_begin(d,a,b);plan_early(d);}

void reloq61_english_split_begin(void*d,int32_t a,int32_t b){struct reloq61_english_capture*c=active_plan;if(c&&!c->active){plan_begin(d,a,b);plan_early(d);}}

