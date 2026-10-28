/* Keep Mandarin pitch planning separate from the embedded English range.
 * Acoustic time and tract/energy targets remain owned by the English bank. */
extern int32_t reloq_zhcn61_find_nucleus(void*,int32_t);
extern int32_t reloq_zhcn61_find_acoustic_nuc(void*,int32_t);
extern int32_t reloq_zhcn61_init_mcvoice_vars(void*,int32_t,int32_t);
extern int32_t reloq_zhcn61_generate_tone(void*,int32_t,int32_t,int32_t);

struct bank_pitch_grid {int32_t *time,*point;size_t count;};
static int pitch_time_compare(const void *a,const void *b)
{
 int32_t x=*(const int32_t*)a,y=*(const int32_t*)b;return (x>y)-(x<y);
}
static int32_t pitch_grid_point(const struct bank_pitch_grid *g,int32_t time)
{
 size_t a=0,b=g->count;while(a<b){size_t m=a+(b-a)/2;if(g->time[m]<time)a=m+1;else b=m;}
 return a<g->count&&g->time[a]==time?g->point[a]:0;
}
static int pitch_acoustic_grid(delta_state *d,struct reloq61_english_bank *p,int32_t limit,struct bank_pitch_grid *g)
{
 size_t capacity=p->atom_count+p->syllable_count+2;
 g->time=malloc(2*capacity*sizeof*g->time);if(!g->time)return 94;
 g->point=g->time+capacity;g->count=0;g->time[g->count++]=0;
 for(size_t i=0;i<p->syllable_count;++i)g->time[g->count++]=p->syllables[i].from;
 for(size_t i=0;i<p->atom_count;++i)g->time[g->count++]=p->atoms[i].to;
 g->time[g->count++]=limit;qsort(g->time,g->count,sizeof*g->time,pitch_time_compare);
 size_t used=1;for(size_t i=1;i<g->count;++i)if(g->time[i]!=g->time[used-1])g->time[used++]=g->time[i];g->count=used;
 int32_t at=p->host_left;g->point[0]=at;
 for(size_t i=1;i<g->count;++i){
  if(g->time[i]<0||g->time[i]>limit)return 94;
  if(g->time[i]==limit)g->point[i]=p->host_right;
  else{at=vsplit_time(d,8,at,g->time[i]-g->time[i-1]);if(!at)return 94;g->point[i]=at;}
 }
 vdel_2pt(d,2,p->host_left,p->host_right);vdel_2pt(d,7,p->host_left,p->host_right);
 const delta_stmt *en=delta_lang_by_id(0x10000)->stmtbl+2;
 for(size_t s=0;s<p->syllable_count;++s){
  const struct reloq61_bank_syllable *sy=p->syllables+s;
  int32_t left=pitch_grid_point(g,sy->from),right=pitch_grid_point(g,sy->to);
  if(!left||!right||left==right)return 94;
  delta_token a={0,left},b={0,right};uint8_t zero=0;lpta_rpta_loadp(d,&a,&b);
  if(insert_2pt_s(d,7,1,&zero,0))return 94;
  int32_t n=projected_record(d,left,7);if(!n)return 94;
  /* English stress selects the native foreign-syllable tone class. */
  unsigned char *sb=TFLDS(EVV_AT(void*,n));int16_t stress=p->pitch_word_stress[sy->word-1]<=0?0:sy->stress,tone=stress>0?1:0;
  memcpy(sb,&stress,2);memcpy(sb+2,&tone,2);
  for(size_t i=sy->begin;i<sy->end;){
   size_t j=i+1;while(j<sy->end&&p->atoms[j].group==p->atoms[i].group)++j;
   const struct reloq61_bank_atom *r=p->atoms+i;
   int32_t pl=pitch_grid_point(g,r->from),pr=pitch_grid_point(g,p->atoms[j-1].to);
   if(!pl||!pr)return 94;if(pl==pr){i=j;continue;}
   unsigned id=0;while(id<(unsigned)phone_count&&strcmp(phone_names[id],r->phone.name))++id;
   if(id==(unsigned)phone_count)return 94;
   delta_token pa={0,pl},pb={0,pr};uint8_t name=(uint8_t)id;lpta_rpta_loadp(d,&pa,&pb);
   if(insert_2pt_s(d,2,1,&name,0))return 94;
   int32_t pn=projected_record(d,pl,2);if(!pn)return 94;unsigned char *body=TFLDS(EVV_AT(void*,pn));
   for(unsigned f=1;f<=6;++f){
    const char *trait=((const char*const*)en->fields[f].values)[r->phone.traits[f-1]];
    int v=feature_id(f,trait);if(v<0)return 94;*(uint8_t*)delta_record_field(vstmtbl+2,f,body)=(uint8_t)v;
   }
   memcpy(body+12,&r->phone.stress,2);body[9]=r->phone.transition;body[11]=r->phone.ghost;
   body[14]=r->phone.after;memcpy(body+16,&r->phone.duration,2);i=j;
  }
 }
 /* Donor transitions may lie outside its lexical syllable intervals. Their
  * durations remain timed gaps, while tract and energy targets stay intact. */
 for(size_t i=0;i+1<g->count;++i){
  int covered=0;for(size_t r=0;r<p->atom_count;++r)if(p->atoms[r].from<=g->time[i]&&p->atoms[r].to>=g->time[i+1]){covered=1;break;}
  if(!covered){delta_token a={0,g->point[i]},b={0,g->point[i+1]};uint8_t gap=0;lpta_rpta_loadp(d,&a,&b);if(insert_2pt_s(d,2,1,&gap,0))return 94;}
 }
 return 0;
}
static int pitch_phrase_syllables(delta_state *d,int32_t left)
{
 int32_t phrase=owner_record(d,left,5);if(!phrase)return 0;
 int32_t scan=*EVV_AT(int32_t*,phrase)&~3,end=record_end(phrase);int count=0;
 for(unsigned guard=0;guard<4096&&right_point(d,scan,7)!=right_point(d,end,7);++guard){
  int32_t rec=projected_record(d,scan,7);if(!rec)break;++count;
  int32_t next=record_end(rec);if(!next||next==scan)return 0;scan=next;
 }
 int16_t total=(int16_t)count;memcpy((char*)TFLDS(EVV_AT(void*,phrase))+10,&total,2);return count;
}
static int pitch_prepare_phrase(delta_state*d,int32_t left)
{
 int32_t phrase=owner_record(d,left,5);if(!phrase)return 0;
 int32_t scan=*EVV_AT(int32_t*,phrase)&~3,end=right_point(d,record_end(phrase),4);
 if(right_point(d,scan,4)!=right_point(d,left,4))return 0;
 for(unsigned guard=0;guard<4096&&right_point(d,scan,4)!=end;++guard){
  int32_t word=projected_record(d,scan,4);if(!word)break;
  if(((unsigned char*)TFLDS(EVV_AT(void*,word)))[9])return pitch_phrase_syllables(d,left)>0?0:94;
  int32_t next=record_end(word);if(!next||next==scan)break;scan=next;
 }
 return 0;
}
/* Donor transitions can occupy timed gaps between lexical syllables.
 * Carry the preceding target through that gap before the next nucleus. */
static int pitch_keep_transitions(struct reloq61_english_bank*p)
{
 struct reloq61_bank_pitch holds[255];size_t used=0,at=0;
 for(size_t s=1;s<p->syllable_count;++s){
  int32_t time=p->syllables[s].from,previous=p->syllables[s-1].to;
  if(time<=previous)continue;
  while(at<p->pitch_count&&p->pitch[at].at<time)++at;
  if(at&&at<p->pitch_count&&p->pitch[at].at>time&&p->pitch[at-1].at==previous)
   holds[used++]=(struct reloq61_bank_pitch){time,p->pitch[at-1].value};
 }
 if(!used)return 0;
 if(p->pitch_count>4096-used)return 94;
 size_t total=p->pitch_count+used;
 void*more=realloc(p->pitch,total*sizeof*p->pitch);if(!more)return 94;p->pitch=more;
 size_t a=p->pitch_count,b=used,k=total;
 while(b){
  if(a&&p->pitch[a-1].at>holds[b-1].at)p->pitch[--k]=p->pitch[--a];
  else p->pitch[--k]=holds[--b];
 }
 p->pitch_count=total;return 0;
}
static int bank_host_pitch(delta_state *d,int32_t machine,struct reloq61_english_bank *p,int32_t from,int32_t to)
{
 if(!p->syllable_count||!p->atom_count||reloq61_english_bank_prepare(d,p,from))return 94;
 struct bank_pitch_grid grid={0};int result=94;
 int32_t saved_phrase[2];memcpy(saved_phrase,(char*)d+DG_BASE+648,4);memcpy(saved_phrase+1,(char*)d+DG_BASE+656,4);
 int phrase_start=right_point(d,saved_phrase[0],4)==right_point(d,p->host_left,4);
 int32_t saved_words[2];memcpy(saved_words,(char*)d+DG_BASE+160,4);
 memcpy(saved_words+1,(char*)d+DG_BASE+168,4);
 int16_t saved_foreign;memcpy(&saved_foreign,(char*)d+DG_BASE+4060,2);
 if(pitch_acoustic_grid(d,p,to-from,&grid))goto done;
 int count=pitch_phrase_syllables(d,p->host_left);if(count<=0||count>32767)goto done;
 int16_t total=(int16_t)count,foreign=1;memcpy((char*)d+DG_BASE+4036,&total,2);
 memcpy((char*)d+DG_BASE+4060,&foreign,2);
 memcpy((char*)d+DG_BASE+3056,&p->host_left,4);int32_t absolute=from*10000;memcpy((char*)d+DG_BASE+3064,&absolute,4);
 unsigned current_word=0;
 for(size_t s=0;s<p->syllable_count;++s){
  const struct reloq61_bank_syllable *sy=p->syllables+s;
  if(!sy->word||sy->word>p->word_count)goto done;
  if(current_word!=sy->word){
   current_word=sy->word;int32_t word=projected_record(d,p->word_points[current_word-1],4);if(!word)goto done;
   int32_t wl=*EVV_AT(int32_t*,word)&~3,wr=record_end(word);
   memcpy((char*)d+DG_BASE+160,&wl,4);memcpy((char*)d+DG_BASE+168,&wr,4);
   memcpy((char*)d+DG_BASE+648,saved_phrase,4);memcpy((char*)d+DG_BASE+656,saved_phrase+1,4);
   unsigned char *word_fields=TFLDS(EVV_AT(void*,word));int16_t saved_stress;
   memcpy(&saved_stress,word_fields+4,2);
   int16_t native_stress=p->pitch_word_stress[current_word-1];
   if(native_stress<=0)memcpy(word_fields+4,&native_stress,2);
   if(reloq_zhcn61_init_word_vars(d,machine)){memcpy(word_fields+4,&saved_stress,2);goto done;}
   memcpy(word_fields+4,&saved_stress,2);
   int16_t neutral=0;int32_t unset=0;
   memcpy((char*)d+DG_BASE+3148,&neutral,2);memcpy((char*)d+DG_BASE+3536,&unset,4);
   if(reloq_zhcn61_init_mcvoice_vars(d,machine,EVV_REF((char*)d+DG_BASE+4020)))goto done;
   if(phrase_start){int32_t first=pitch_grid_point(&grid,p->syllables[0].from);memcpy((char*)d+DG_BASE+648,&first,4);}
   if(!p->trim_pause){int32_t last=pitch_grid_point(&grid,p->syllables[p->syllable_count-1].to);memcpy((char*)d+DG_BASE+656,&last,4);}
   size_t last=s;while(last+1<p->syllable_count&&p->syllables[last+1].word==current_word)++last;
   int32_t sl=pitch_grid_point(&grid,sy->from),sr=pitch_grid_point(&grid,p->syllables[last].to);
   memcpy((char*)d+DG_BASE+176,&sl,4);memcpy((char*)d+DG_BASE+184,&sr,4);
  }
  int32_t left=pitch_grid_point(&grid,sy->from),end=pitch_grid_point(&grid,sy->to);
  memcpy((char*)d+DG_BASE+248,&left,4);memcpy((char*)d+DG_BASE+256,&end,4);
  if(reloq_zhcn61_init_syll_vars(d,machine)||reloq_zhcn61_find_nucleus(d,machine)||reloq_zhcn61_find_acoustic_nuc(d,machine)||reloq_zhcn61_set_syll_offset_vals(d,machine))goto done;
  if(reloq_zhcn61_generate_tone(d,machine,EVV_REF((char*)d+DG_BASE+4020),EVV_REF((char*)d+DG_BASE+796)))goto done;
 }
 if(reloq61_english_bank_pitch(d,p,from,to)||pitch_keep_transitions(p))goto done;result=0;
 done:free(grid.time);memcpy((char*)d+DG_BASE+648,saved_phrase,4);memcpy((char*)d+DG_BASE+656,saved_phrase+1,4);memcpy((char*)d+DG_BASE+160,saved_words,4);memcpy((char*)d+DG_BASE+168,saved_words+1,4);
 memcpy((char*)d+DG_BASE+4060,&saved_foreign,2);return result;
}
