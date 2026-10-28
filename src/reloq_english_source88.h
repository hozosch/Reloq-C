/* Original-observed source envelopes; phonetic decisions, no word keys.
 * A vowel may own a target in its following liquid. Defer those targets
 * until both phone clocks are known, retaining the earlier owner's order. */
#include "reloq_english_source88.inc"
static int source_point88(struct events *events,const struct unit *u,
    const struct unit *next,const struct english_source_point88 *p)
{
    const int64_t clocks[]={u->start,u->contact,u->end,u->release,u->head};
    int64_t t;
    if(p->anchor<5)t=clocks[p->anchor];
    else if(p->anchor<8) {
        unsigned a=p->anchor-5;
        t=clocks[a]+(int64_t)llround((double)(clocks[a+1]-clocks[a])*p->fraction/1000);
    } else if(p->anchor>=12) {
        t=(p->anchor==12?u->start:u->contact)+1;
    } else {
        if(!next)return -1;
        if(p->anchor==8)t=next->end;
        else if(p->anchor==9)t=next->release;
        else if(p->anchor==10)t=next->contact;
        else t=u->release+(int64_t)llround((double)(next->release-u->release)*p->fraction/1000);
    }
    if(u->before_h && t>u->end)t=u->end;
    return add(events,p->stream,t,p->value);
}
static int measured_source88(struct events *events,struct unit *u,const int f[35])
{
    if(!(f[21]>1 || f[6] || f[16]+f[17]>=3))return 0;
    unsigned index=english_source_roots88[f[0]];
    for(unsigned depth=0;depth<30;++depth) {
        const struct english_source_node88 *node=english_source_nodes88+index;
        if(node->feature==255)break;
        int yes=node->equality?f[node->feature]==node->threshold:f[node->feature]<=node->threshold;
        index=yes?node->yes:node->no;
    }
    unsigned profile=english_source_nodes88[index].label;
    if(!profile)return 0;
    unsigned first=english_source_profiles88[profile].first,count=english_source_profiles88[profile].count;
    /* An unavailable next phone cannot own an anticipated target. */
    for(unsigned j=0;j<count;++j)
        if(english_source_points88[first+j].anchor>=8 &&
           english_source_points88[first+j].anchor<12 && f[2]<0)return 0;
    /* A measured envelope is reusable only with the same kind of source
     * boundary. The statistical selector must not import aspiration into a
     * voiced sonorant or defer a nucleus through an unrelated closure.
     * Retain the connected pair gesture when these structural facts differ. */
    int next=f[2]>=0?english_inventory[f[2]]:0;
    int previous=f[1]>=0?english_inventory[f[1]]:0;
    if(strchr("lrwy",u->phone) && next && vowel(next) && previous &&
       !vowel(previous) && !strchr("lrwy",previous)) {
        int has_av=0;
        for(unsigned j=0;j<count;++j)
            if(english_source_points88[first+j].stream==RELOQ_TARGET_AV)has_av=1;
        if(!has_av)return 0;
    }
    /* Rare consonant clusters must not borrow the devoiced /w/ of a
     * stressed p/t/k onset. In the original, s/z/sh and voiced closures
     * retain the following liquid/glide's voicing (532 cluster probes).
     * Keep the connected gestures when the selected envelope contradicts
     * this source boundary, including an unstressed medial p/t/k+w. */
    if(strchr("lrwy",u->phone) && next && vowel(next) && previous &&
       (!strchr("ptk",previous) || f[10] || !u->stress))
        for(unsigned j=0;j<count;++j) {
            const struct english_source_point88 *p=english_source_points88+first+j;
            if(p->stream==RELOQ_TARGET_AV && !p->value &&
               (p->anchor<=2 || p->anchor==5 || p->anchor==6))return 0;
        }
    if(f[11] && strchr("ptk",u->phone) && next && strchr("lrwy",next))return 0;
    if(vowel(u->phone))for(unsigned j=0;j<count;++j) {
        const struct english_source_point88 *p=english_source_points88+first+j;
        if(p->stream!=RELOQ_TARGET_AV && p->stream!=RELOQ_TARGET_AH)continue;
        if(p->anchor>=8 && p->anchor<=11 && next && !vowel(next) &&
            !strchr("lrwy",next))return 0;
        if(p->stream==RELOQ_TARGET_AV && !p->value && p->anchor<=1 &&
            previous && strchr("bdgFvDzZJmnGlrwy",previous) && !u->aspirated)return 0;
    }
    u->source_profile=profile;
    size_t keep=u->event_begin;
    for(size_t i=keep;i<events->n;++i)
        if(events->p[i].stream<RELOQ_TARGET_TL) {
            events->p[keep]=events->p[i];events->p[keep].serial=keep;++keep;
        }
    events->n=keep;
    for(unsigned j=0;j<count;++j) {
        const struct english_source_point88 *p=english_source_points88+first+j;
        if(p->anchor>=8 && p->anchor<12)continue;
        /* Start and contact coincide in isolated onsets. Such a profile
         * cannot move a closure's zero back into a preceding vowel's
         * voiced transition when the actual clocks differ. */
        struct english_source_point88 point=*p;
        if(point.anchor==0 && previous && vowel(previous) &&
           strchr("bpdtkgCJszSZfvTD",u->phone))point.anchor=1;
        if(source_point88(events,u,NULL,&point))return -1;
    }
    return 0;
}
static int deferred_source88(struct events *events,const struct unit *u,const struct unit *next)
{
    if(!u->source_profile)return 0;
    unsigned first=english_source_profiles88[u->source_profile].first;
    unsigned count=english_source_profiles88[u->source_profile].count;
    for(unsigned j=0;j<count;++j) {
        const struct english_source_point88 *p=english_source_points88+first+j;
        if(p->anchor<8 || p->anchor>=12)continue;
        if(source_point88(events,u,next,p))return -1;
        events->p[events->n-1].serial=u->event_begin;
    }
    return 0;
}
