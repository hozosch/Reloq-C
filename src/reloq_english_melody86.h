/* Native realization of observed US intonation commands. All model inputs
 * are phonetic or phrase-structural facts; no text or sentence identities. */
#include "reloq_english_melody86.inc"
#include "reloq_english_intonation.h"
struct pitch_syllable86 {size_t first,last,nucleus,last_nucleus;};
static int measured_phrase_pitch86(struct events *events,struct unit *u,size_t begin,size_t end,
    const struct reloq_german_generic_settings *settings)
{
    size_t count=1;for(size_t i=begin+1;i<=end;++i)count+=u[i].syllable!=u[i-1].syllable;
    /* Keep the already calibrated isolated and short reference path. */
    if(count<3)return 1;
    struct pitch_syllable86 *s=calloc(count,sizeof *s);
    if(!s)return -1;
    size_t n=0;
    for(size_t i=begin;i<=end;++i) {
        if(i==begin||u[i].syllable!=u[i-1].syllable) {
            s[n++]=(struct pitch_syllable86){i,i,SIZE_MAX,SIZE_MAX};
        }
        struct pitch_syllable86 *p=s+n-1;p->last=i;
        if(vowel(u[i].phone)){if(p->nucleus==SIZE_MAX)p->nucleus=i;p->last_nucleus=i;}
    }
    size_t saved=events->n,nuclear=count-1,total_accents=0,words=1;
    for(size_t i=begin+1;i<=end;++i)words+=u[i].word!=u[i-1].word;
    for(size_t k=0;k<count;++k) {
        if(s[k].nucleus==SIZE_MAX){free(s);return 1;}
        const struct unit *v=u+s[k].nucleus;
        if(v->stress==1&&!v->weak&&v->accent>0){nuclear=k;++total_accents;}
    }
    size_t ordinal=0;int64_t previous=-1;
    /* Product policy: every question rises, including wh and alternative
     * questions. Ordinary text and explicit SPR follow the same policy. */
    const int rising=u[end].question;
    size_t nuclear_events[2]={SIZE_MAX,SIZE_MAX};
    int64_t nuclear_left[2]={0,0},nuclear_right[2]={0,0};
    int nuclear_first_percent=10;
    struct reloq_english_nuclear_context nuclear_context={0};
    for(size_t k=0;k<count;++k) {
        size_t a=s[k].first,b=s[k].last,v=s[k].nucleus,z=s[k].last_nucleus;
        size_t pv=k?s[k-1].nucleus:SIZE_MAX,nv=k+1<count?s[k+1].nucleus:SIZE_MAX;
        size_t wf=a,wl=b;while(wf>begin&&u[wf-1].word==u[a].word)--wf;
        while(wl<end&&u[wl+1].word==u[b].word)++wl;
        int accented=u[v].stress==1&&!u[v].weak&&u[v].accent>0;
        int back=99,forward=99;
        for(size_t j=k;j-->0;)if(u[s[j].nucleus].stress==1&&!u[s[j].nucleus].weak&&u[s[j].nucleus].accent>0){back=(int)(k-j);break;}
        for(size_t j=k+1;j<count;++j)if(u[s[j].nucleus].stress==1&&!u[s[j].nucleus].weak&&u[s[j].nucleus].accent>0){forward=(int)(j-k);break;}
        size_t word=0;for(size_t j=begin+1;j<=a;++j)word+=u[j].word!=u[j-1].word;
        int f[44]={number(u[a].phone),number(u[v].phone),number(u[z].phone),number(u[b].phone),
            a>begin?number(u[a-1].phone):-1,b<end?number(u[b+1].phone):-1,
            nv!=SIZE_MAX?number(u[nv].phone):-1,pv!=SIZE_MAX?number(u[pv].phone):-1,
            u[v].stress,u[v].weak,u[v].accent+1,accented,(int)k,(int)(count-k-1),(int)count,
            (int)k-(int)nuclear,(int)ordinal,(int)total_accents,back,forward,
            a==wf,b==wl,(int)(u[v].syllable-u[wf].syllable),(int)(u[wl].syllable-u[wf].syllable+1),
            (int)word,(int)(words-word-1),u[v].question,u[v].continuation,u[v].aspirated,
            (int)(v-a),(int)(b-a),(int)(z-a),pv!=SIZE_MAX?u[pv].stress:-1,nv!=SIZE_MAX?u[nv].stress:-1,
            pv!=SIZE_MAX?u[pv].weak:-1,nv!=SIZE_MAX?u[nv].weak:-1,
            a>begin&&u[a].word!=u[a-1].word,b<end&&u[b].word!=u[b+1].word};
        for(unsigned j=0;j<6;++j)f[38+j]=a+j<=b?number(u[a+j].phone):-1;
        unsigned ix=0;
        for(unsigned depth=0;depth<28;++depth) {
            const struct english_pitch_node *node=english_pitch_nodes+ix;
            if(node->feature==255)break;
            int yes=node->equality?f[node->feature]==node->threshold:f[node->feature]<=node->threshold;
            ix=yes?node->yes:node->no;
        }
        const struct english_pitch_sequence *sequence=english_pitch_sequences+english_pitch_nodes[ix].label;
        struct reloq_english_tone_point nuclear_points[2];int native_nuclear=0;
        if(k==nuclear && !u[end].continuation && sequence->count>=2) {
            const struct english_pitch_command *c=english_pitch_commands+sequence->first;
            if(c[0].value==50 && (c[1].value==25 || c[1].value==100 || c[1].value==120)) {
                const int low=c[1].value==25;
                const struct reloq_english_nuclear_context context={
                    low?RELOQ_EN_TONE_LOW:RELOQ_EN_TONE_HIGH,
                    u[v].word==u[begin].word,u[v].word==u[end].word,
                    u[b].word==u[end].word && b==end,total_accents<=1};
                native_nuclear=!reloq_english_nuclear_tone(&context,nuclear_points);
                nuclear_context=context;
            }
        }
#ifdef RELOQ_ENGLISH_PHONE_TRACE
        if(getenv("RELOQ_ENGLISH_MELODY_TRACE")) {
            fputs("melody_features",stderr);for(unsigned j=0;j<44;++j)fprintf(stderr," %d",f[j]);fputc('\n',stderr);
        }
#endif
        for(unsigned j=0;j<sequence->count;++j) {
            const struct english_pitch_command *command=english_pitch_commands+sequence->first+j;
            /* The phrase ending is now generated from the last actual
             * nucleus. A selected command sequence cannot borrow a tail
             * range from a preceding word or an unrelated consonant. */
            if(k+1==count && sequence->count>=2 && j+2>=sequence->count &&
               english_pitch_commands[sequence->first+sequence->count-1].percent==99)continue;
            int64_t left=(int64_t)a+command->start_offset,right=(int64_t)a+command->end_offset;
            if(left<(int64_t)begin||right<(int64_t)begin||left>(int64_t)end||right>(int64_t)end)goto fallback;
            const struct unit *l=u+left,*r=u+right;
            int64_t lt[]={l->start,l->contact,l->end,l->release,l->head,(l->start+l->contact)/2,l->head+(strchr("JC",l->phone)!=NULL)};
            int64_t rt[]={r->start,r->contact,r->end,r->release,r->head,(r->start+r->contact)/2,r->head+(strchr("JC",r->phone)!=NULL)};
            int64_t start=lt[command->start_clock],finish=rt[command->end_clock];
            if(finish<start)goto fallback;
            int percent=native_nuclear && j<2?nuclear_points[j].percent:command->percent;
            if(native_nuclear && !j && command->percent==2)percent=2;
            int level=native_nuclear && j<2?nuclear_points[j].level:command->value;
            int64_t tick;
            if(reloq_english_tone_tick(start,finish,percent,&tick))goto fallback;
            if(tick<=previous)continue;
            int inflection=settings->native_inflection_plus_one?settings->native_inflection_plus_one-1:30;
            double pitch=reloq_voice_pitch_f0_scale(u[v].pitch);int value;
            if(reloq_english_tone_value(1189.15*pitch,inflection,count,
                (double)k+command->decline_offset,level,&value))goto fallback;
            if(native_nuclear && j<2) {
                nuclear_events[j]=events->n;
                nuclear_left[j]=start;nuclear_right[j]=finish;
                if(!j)nuclear_first_percent=percent;
            }
            if(add(events,RELOQ_TARGET_F0,tick,value)){free(s);return -1;}
            previous=tick;
        }
        ordinal+=accented;
    }
    if(nuclear_events[0]!=SIZE_MAX && nuclear_events[1]!=SIZE_MAX) {
        struct reloq_english_tone_point p[2];nuclear_context.tone=rising?RELOQ_EN_TONE_LOW:RELOQ_EN_TONE_HIGH;
        if(reloq_english_nuclear_tone(&nuclear_context,p))goto fallback;
        p[0].percent=nuclear_first_percent;
        const struct unit *v=u+s[nuclear].nucleus;
        int inflection=settings->native_inflection_plus_one?settings->native_inflection_plus_one-1:30;
        for(unsigned j=0;j<2;++j) {
            int64_t tick;int value;
            if(reloq_english_tone_tick(nuclear_left[j],nuclear_right[j],p[j].percent,&tick) ||
               reloq_english_tone_value(1189.15*reloq_voice_pitch_f0_scale(v->pitch),
                    inflection,count,(double)nuclear,p[j].level,&value))goto fallback;
            events->p[nuclear_events[j]].tick=tick;events->p[nuclear_events[j]].value=value;
        }
    }
    {
        size_t v=s[count-1].nucleus,last=s[count-1].last_nucleus;
        while(last<end && u[last+1].syllable==u[v].syllable &&
              strchr("lrwy",u[last+1].phone))++last;
        const int64_t first=u[v].start,finish=u[last].pause?u[last].end:u[last].release;
        struct reloq_english_tone_point tail[2];
        const int weak=u[v].weak;
        if(reloq_english_final_tone(rising,u[end].continuation,
            u[s[nuclear].nucleus].word==u[end].word,weak,tail))goto fallback;
        int inflection=settings->native_inflection_plus_one?settings->native_inflection_plus_one-1:30;
        double pitch=reloq_voice_pitch_f0_scale(u[v].pitch);
        for(unsigned j=0;j<2;++j) {
            int64_t tick;int value;
            if(reloq_english_tone_tick(first,finish,tail[j].percent,&tick) ||
               reloq_english_tone_value(1189.15*pitch,inflection,count,
                   (double)count-1,tail[j].level,&value))goto fallback;
            if(tick<=previous)continue;
            if(add(events,RELOQ_TARGET_F0,tick,value)){free(s);return -1;}
            previous=tick;
        }
    }
    free(s);return events->n>saved?0:1;
fallback:
    events->n=saved;free(s);return 1;
}
