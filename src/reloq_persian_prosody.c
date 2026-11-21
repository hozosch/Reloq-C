/* Native word and syllable alignment for the approved US-derived Persian
 * contour.  This moves contour fitting into the shared core; collecting the
 * Persian and donor utterances is still the caller's responsibility. */
#include "reloq_persian_frame_internal.h"
#include <stdint.h>
#include <stdlib.h>

/* Round nonnegative 1/20000-ms values exactly as Python's round: ties even. */
static int64_t fa_round_boundary(int64_t n)
{
    int64_t q=n/20000,r=n%20000;
    return q+(r>10000 || (r==10000 && (q&1)));
}

/* Resolve all syllables against word marks, including one possible local
 * provider-clock reset at the start of a subsequent sentence. */
static int fa_syllable_groups(const struct reloq_fa_syllable *s,size_t ns,
    const int64_t *bounds,const uint32_t *counts,size_t words,int64_t *edges)
{
    size_t total=0;
    if(!s || !ns || !counts)return 0;
    for(size_t i=0;i<words;++i) {
        if(!counts[i] || counts[i]>ns-total)return 0;
        total+=counts[i];
    }
    if(total!=ns)return 0;
    size_t pos=0,out=0;int64_t offset=0,previous=-1;
    for(size_t i=0;i<words;++i) {
        int64_t last_end=0;
        for(size_t j=0;j<counts[i];++j) {
            int64_t a=s[pos+j].from,b=s[pos+j].to;
            if(a<0 || a>=b)return 0;
            if(previous>=0 && a<previous) {
                if(j || i+1>=words)return 0;
                offset=bounds[i+1]*10000-(int64_t)s[pos+counts[i]-1].to;
                if(offset<0)return 0;
            }
            previous=b;a+=offset;b+=offset;
            if(a+b<bounds[i]*20000 || a+b>=bounds[i+1]*20000)return 0;
            if(j)edges[out++]=fa_round_boundary(last_end+a);
            last_end=b;
        }
        pos+=counts[i];
    }
    return 1;
}

/* Fall back to a single word if the complete sentence's clocks cannot be
 * verified. A provider clock reset ends the first reliable prefix. */
static size_t fa_syllable_word(const struct reloq_fa_syllable *s,size_t ns,
    int64_t from,int64_t to,uint32_t expected,int64_t *edges)
{
    if(!s || expected<2)return 0;
    size_t found=0;int64_t last_end=0,previous=-1;
    for(size_t i=0;i<ns;++i) {
        int64_t a=s[i].from,b=s[i].to;
        if(a<0 || a>=b) return 0;
        if(previous>=0 && a<previous)break;
        previous=b;
        if(a+b<from*20000 || a+b>=to*20000)continue;
        if(found>=expected)return 0;
        if(found)edges[found-1]=fa_round_boundary(last_end+a);
        last_end=b;++found;
    }
    return found==expected?found-1:0;
}

/* Returns 0 for a fitted contour, 1 when word alignment is unavailable,
 * and -1 for invalid input or allocation. Boundaries are in milliseconds;
 * syllable edges use the provider's 1/10000-ms clock. No speech or indexes
 * are emitted here, and no state is kept beyond the current request. */
int reloq_fa_transfer_us_words(struct reloq_fa_frame *target,size_t nt,
    const struct reloq_fa_frame *donor,size_t nd,const int64_t *tb,const int64_t *db,
    size_t words,const struct reloq_fa_syllable *ts,size_t nts,
    const struct reloq_fa_syllable *ds,size_t nds,const uint32_t *counts)
{
    if(!target || !nt || !donor || !nd || !tb || !db || !words || words>4096 ||
       target[nt-1].tick<0 || donor[nd-1].tick<0 ||
       target[nt-1].tick>=120000 || donor[nd-1].tick>=120000)return -1;
    for(size_t i=0;i<=words;++i)
        if(tb[i]<0 || db[i]<0 || tb[i]>target[nt-1].tick+1 || db[i]>donor[nd-1].tick+1 ||
           (i && (tb[i]<=tb[i-1] || db[i]<=db[i-1])))return 1;
    size_t length=(size_t)donor[nd-1].tick+1;
    float *curve=malloc(length*sizeof *curve);
    int64_t *first=malloc(words*sizeof *first),*last=malloc(words*sizeof *last);
    size_t edge_count=0;
    if(counts)for(size_t i=0;i<words;++i) {
        if(!counts[i] || counts[i]>256 || edge_count>120000-(counts[i]-1)) {
            free(curve);free(first);free(last);return -1;
        }
        edge_count+=counts[i]-1;
    }
    size_t edge_capacity=edge_count?edge_count:1;
    int64_t *te=malloc(edge_capacity*sizeof *te),*de=malloc(edge_capacity*sizeof *de);
    if(!curve || !first || !last || !te || !de) {
        free(curve);free(first);free(last);free(te);free(de);return -1;
    }
    int status=-1;
    if(reloq_fa_dense_curve(donor,nd,1,curve,length))goto done;
    size_t cursor=0;
    for(size_t i=0;i<words;++i) {
        while(cursor<nd && donor[cursor].tick<db[i])++cursor;
        size_t k=cursor,n=0;first[i]=last[i]=-1;
        while(k<nd && donor[k].tick<db[i+1]) {
            if(donor[k].controls[2]>0 && donor[k].controls[1]>0) {
                if(!n)first[i]=donor[k].tick;
                last[i]=donor[k].tick;++n;
            }
            ++k;
        }
        if(n<2 || first[i]>=last[i]){status=1;goto done;}
        cursor=k;
    }
    int tg=counts && fa_syllable_groups(ts,nts,tb,counts,words,te);
    int dg=counts && fa_syllable_groups(ds,nds,db,counts,words,de);
    size_t edge=0;
    for(size_t i=0;i<words;++i) {
        size_t count=counts?counts[i]-1:0;
        size_t a=tg?count:fa_syllable_word(ts,nts,tb[i],tb[i+1],counts?counts[i]:0,te+edge);
        size_t b=dg?count:fa_syllable_word(ds,nds,db[i],db[i+1],counts?counts[i]:0,de+edge);
        int result=1;
        if(a && a==b) {
            result=reloq_fa_fit_pitch_anchors(target,nt,curve,length,tb[i],tb[i+1],
                                               first[i],last[i],te+edge,de+edge,a);
            if(result<0)goto done;
        }
        if(result) {
            result=reloq_fa_fit_pitch(target,nt,curve,length,tb[i],tb[i+1],first[i],last[i]);
            if(result)goto done;
        }
        edge+=count;
    }
    status=0;
done:
    free(curve);free(first);free(last);free(te);free(de);return status;
}

/* Captured callback positions are samples. Convert them to the frame clock
 * without floating-point rounding differences at half-millisecond ties. */
static int fa_sample_milliseconds(uint64_t sample,unsigned rate,int64_t *milliseconds)
{
    if(!rate || sample>UINT64_MAX/1000u)return -1;
    uint64_t scaled=sample*1000u,whole=scaled/rate,remainder=scaled%rate;
    if(remainder*2u>rate || (remainder*2u==rate && (whole&1u)))++whole;
    if(whole>INT64_MAX)return -1;
    *milliseconds=(int64_t)whole;return 0;
}

int reloq_fa_transfer_us_samples(struct reloq_fa_frame *target,size_t nt,
    const struct reloq_fa_frame *donor,size_t nd,const uint64_t *target_samples,
    const uint64_t *donor_samples,size_t words,unsigned rate,
    const struct reloq_fa_syllable *ts,size_t nts,const struct reloq_fa_syllable *ds,
    size_t nds,const uint32_t *counts)
{
    if(!target || !nt || !donor || !nd || !target_samples || !donor_samples ||
       !words || words>4096 || !rate || target[nt-1].tick<0 || donor[nd-1].tick<0 ||
       target[nt-1].tick>=120000 || donor[nd-1].tick>=120000)return -1;
    size_t bounds=words+1;
    int64_t *storage=malloc(bounds*2*sizeof *storage);
    if(!storage)return -1;
    int64_t *tb=storage,*db=storage+bounds;int status=-1;
    for(size_t i=0;i<bounds;++i)
        if(fa_sample_milliseconds(target_samples[i],rate,tb+i) ||
           fa_sample_milliseconds(donor_samples[i],rate,db+i))goto done;
    if(tb[words]>target[nt-1].tick+1)tb[words]=target[nt-1].tick+1;
    if(db[words]>donor[nd-1].tick+1)db[words]=donor[nd-1].tick+1;
    status=reloq_fa_transfer_us_words(target,nt,donor,nd,tb,db,words,ts,nts,ds,nds,counts);
done:
    free(storage);return status;
}
