#include "reloq61.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
static int sink(void *context,const int32_t p[62],int32_t volume)
{ (void)volume;if(context)fwrite(p,sizeof *p,62,context);return 0; }
static int spans(void *context,int32_t from,int32_t to,
    const struct reloq61_curve curves[62],int32_t volume)
{
    for(int32_t at=from;at<to;++at) {
        int32_t frame[62];frame[0]=1;
        for(unsigned k=1;k<62;++k) {
            const struct reloq61_curve *c=curves+k;
            float value=(float)c->val_l;
            if(c->at_r!=c->at_l && c->val_r!=c->val_l) {
                float offset=(float)(at-c->at_l);
                offset*=(float)(c->val_r-c->val_l);
                offset/=(float)(c->at_r-c->at_l);value+=offset;
            }
            frame[k]=(int32_t)value;
        }
        if(sink(context,frame,volume))return 1;
    }
    return 0;
}
int main(int argc,char **argv)
{
    if(argc<4)return 2;
    struct reloq61_request q={.language=atoi(argv[1]),.annotations=1,
        .phonemes_only=!strcmp(argv[2],"ph"),.voice={0,50,65,30,0,0,50,100},
        .text=argv[3],.frame=sink};
    if(!strcmp(argv[2],"spans") || getenv("RELOQ61_PROBE_SPANS"))q.span=spans;
    const char *sample_rate=getenv("RELOQ61_SAMPLE_RATE");
    if(sample_rate)q.sample_rate=atoi(sample_rate);
    const char *voice=getenv("RELOQ61_VOICE");
    if(voice && sscanf(voice,"%d,%d,%d,%d,%d,%d,%d,%d",q.voice,q.voice+1,q.voice+2,q.voice+3,q.voice+4,q.voice+5,q.voice+6,q.voice+7)!=8)return 3;
    char *ph=calloc(1,1048576);q.phonemes=ph;q.phonemes_capacity=1048576;
    if(argc>4)q.context=fopen(argv[4],"wb");
    struct reloq61_result r;int status=reloq61_generate(&q,&r);
    if(q.context)fclose(q.context);
    printf("status %d unsupported %d error %d frames %zu ticks %lld phonemes %zu\n%s\n",
        status,r.unsupported,r.error,r.frames,(long long)r.ticks,r.phonemes_length,ph);
    free(ph);return status<0?1:0;
}
