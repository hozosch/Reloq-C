/* Original-observed, phone-owned oral targets selected by phonetic context. */
#include "reloq_english_acoustics87.inc"
static int measured_acoustics87(struct events *events,const struct unit *u,const int f[35])
{
    /* Non-nuclear phones keep the original pair-owned oral targets. The
     * nucleus prototype selector cannot substitute a whole tract there. */
    if(!reloq_english_tract_nuclear(u->phone))return 0;
    if(!(f[21]>1 || f[6]))return 0;
    unsigned index=english_acoustic_roots87[f[0]];
    for(unsigned depth=0;depth<30;++depth) {
        const struct english_acoustic_node87 *node=english_acoustic_nodes87+index;
        if(node->feature==255)break;
        const int yes=node->equality?f[node->feature]==node->threshold:f[node->feature]<=node->threshold;
        index=yes?node->yes:node->no;
    }
    unsigned profile=english_acoustic_nodes87[index].label;
    if(!profile)return 0;
    unsigned first=english_acoustic_profiles87[profile].first,count=english_acoustic_profiles87[profile].count;
    size_t keep=u->event_begin;
    for(size_t i=keep;i<events->n;++i)
        if(events->p[i].stream<RELOQ_TARGET_F1 || events->p[i].stream>RELOQ_TARGET_FNZ)
            {events->p[keep]=events->p[i];events->p[keep].serial=keep;++keep;}
    events->n=keep;
    for(unsigned j=0;j<count;++j) {
        const struct english_acoustic_point87 *p=english_acoustic_points87+first+j;
        const int64_t clocks[]={u->start,u->contact,u->end,u->release,u->head};
        int64_t t;
        if(p->anchor<5)t=clocks[p->anchor];
        else {
            unsigned a=p->anchor-5;
            t=clocks[a]+(int64_t)llround((double)(clocks[a+1]-clocks[a])*p->fraction/1000);
        }
        if(add(events,p->stream,t,p->value))return -1;
    }
    return 0;
}
