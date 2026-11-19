/* Request-local Persian U-to-A transition, moved from the speech adapter.
 * Candidate word boundaries are identified by the caller's phone stream;
 * timing guards, tract interpolation and optional pitch are owned here. */
#include "reloq_persian_frames.h"
#include <stdint.h>
#include <stdlib.h>

struct planned_glide {
    size_t first,end,peak,pitch_end;
    float start[24],difference[24],pitch_start,pitch_difference;
    int pitch;
};
static const unsigned tract[24]={9,10,13,14,15,16,17,18,19,20,21,22,23,24,
                                  25,26,27,28,29,30,31,32,33,34};
static size_t lower(const struct reloq_fa_frame *rows,size_t count,double tick)
{
    size_t lo=0,hi=count;
    while(lo<hi){size_t mid=lo+(hi-lo)/2;if((double)rows[mid].tick<tick)lo=mid+1;else hi=mid;}
    return lo;
}
static size_t upper(const struct reloq_fa_frame *rows,size_t count,double tick)
{
    size_t lo=0,hi=count;
    while(lo<hi){size_t mid=lo+(hi-lo)/2;if((double)rows[mid].tick<=tick)lo=mid+1;else hi=mid;}
    return lo;
}
static double ease(double x)
{
    if(x<0)x=0;
    else if(x>1)x=1;
    return x*x*(3.0-2.0*x);
}
int reloq_fa_apply_glides(struct reloq_fa_frame *rows,size_t count,
    const struct reloq_fa_glide_candidate *candidates,size_t candidate_count,
    int pitch,size_t *applied)
{
    if(applied)*applied=0;
    if(!applied || !rows || !count || count>120000 || (candidate_count&&!candidates) ||
       candidate_count>4096 || (pitch!=0 && pitch!=1) || rows[0].tick<0 || rows[count-1].tick>=120000)return -1;
    for(size_t i=1;i<count;++i)if(rows[i].tick<rows[i-1].tick)return -1;
    for(size_t i=0;i<candidate_count;++i) {
        const struct reloq_fa_glide_candidate *c=candidates+i;
        if(!c->word || c->left<0 || c->left>=c->boundary || c->boundary>=c->right ||
           c->right>rows[count-1].tick+1 || (i && c->word<=candidates[i-1].word))return -1;
    }
    if(!candidate_count)return 0;
    unsigned char *occupied=calloc(count,1);
    struct planned_glide *plan=calloc(candidate_count,sizeof *plan);
    if(!occupied || !plan){free(occupied);free(plan);return -2;}
    size_t planned=0;
    for(size_t i=0;i<candidate_count;++i) {
        const struct reloq_fa_glide_candidate *c=candidates+i;
        double t0=(double)c->boundary,early=(double)c->left+(t0-(double)c->left)/2.0;
        if(early<t0-25.0)early=t0-25.0;
        double end_limit=(double)c->right<t0+70.0?(double)c->right:t0+70.0;
        size_t boundary=lower(rows,count,t0),first=lower(rows,count,early),limit=lower(rows,count,end_limit);
        if(boundary<first || boundary-first<3 || limit<boundary || limit-boundary<8)continue;
        for(size_t j=first;j<limit;++j)
            if(rows[j].controls[2]<=0 || rows[j].controls[8]!=0 || rows[j].controls[6]!=0){limit=j;break;}
        if(limit<boundary || limit-boundary<8)continue;
        size_t end=boundary;int rising=0;
        for(size_t j=boundary+1;j<limit;++j) {
            float delta=(float)((double)rows[j].controls[9]-(double)rows[j-1].controls[9]);
            if(delta>0.25f){rising=1;end=j;}
            else if(rising || (double)rows[j].tick-t0>5.0)break;
        }
        if(end-boundary<4 || (double)rows[end].tick-t0>65.0 ||
           (double)rows[end].controls[9]-(double)rows[boundary].controls[9]<100.0 ||
           (double)rows[end].controls[13]-(double)rows[boundary].controls[13]<100.0 ||
           rows[end].tick<=rows[first].tick)continue;
        int overlap=0;
        for(size_t j=first;j<=end;++j)if(occupied[j]){overlap=1;break;}
        if(overlap)continue;
        for(size_t j=first;j<=end;++j)occupied[j]=1;
        struct planned_glide *g=plan+planned++;
        g->first=first;g->end=end;
        for(size_t j=0;j<24;++j) {
            g->start[j]=rows[first].controls[tract[j]];
            g->difference[j]=(float)((double)rows[end].controls[tract[j]]-(double)g->start[j]);
        }
        size_t peak_limit=upper(rows,count,t0+15.0);
        if(peak_limit>limit)peak_limit=limit;
        size_t peak=boundary;
        for(size_t j=boundary+1;j<peak_limit;++j)
            if(rows[j].controls[1]>rows[peak].controls[1])peak=j;
        double after=((double)rows[end].tick-t0)/2.0;
        if(after<0)after=0;else if(after>15)after=15;
        double until=t0+45.0;
        if(until>(double)rows[end].tick+after)until=(double)rows[end].tick+after;
        size_t pitch_end=lower(rows,count,until);
        if(pitch_end>=limit)pitch_end=limit-1;
        float drop=(float)((double)rows[peak].controls[1]-(double)rows[pitch_end].controls[1]);
        if(pitch_end>=peak && pitch_end-peak>=3 && rows[pitch_end].tick>rows[peak].tick && drop>=50.0f) {
            g->pitch=1;g->peak=peak;g->pitch_end=pitch_end;g->pitch_start=rows[peak].controls[1];
            g->pitch_difference=(float)((double)rows[pitch_end].controls[1]-(double)g->pitch_start);
        }
    }
    for(size_t i=0;i<planned;++i) {
        struct planned_glide *g=plan+i;
        double span=(double)rows[g->end].tick-(double)rows[g->first].tick;
        for(size_t j=g->first;j<=g->end;++j) {
            double progress=ease(((double)rows[j].tick-(double)rows[g->first].tick)/span);
            for(size_t k=0;k<24;++k)rows[j].controls[tract[k]]=(float)((double)g->start[k]+progress*(double)g->difference[k]);
        }
        if(pitch && g->pitch) {
            span=(double)rows[g->pitch_end].tick-(double)rows[g->peak].tick;
            for(size_t j=g->peak;j<=g->pitch_end;++j) {
                double progress=ease(((double)rows[j].tick-(double)rows[g->peak].tick)/span);
                rows[j].controls[1]=(float)((double)g->pitch_start+progress*(double)g->pitch_difference);
            }
        }
    }
    free(occupied);free(plan);*applied=planned;return 0;
}
