#include "reloq61.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct timeline {
    int64_t ticks;
    int64_t start;
};

static int span(void *context,int32_t from,int32_t to,
    const struct reloq61_curve curves[62],int32_t volume)
{
    struct timeline *t=context;(void)volume;
    printf("%4d..%-4d av=%d..%-3d f0=%d..%-4d window=%lld\n",
        from,to,curves[2].val_l,curves[2].val_r,curves[1].val_l,curves[1].val_r,
        (long long)(t->ticks-t->start));
    t->ticks+=to-from;
    return 0;
}

static void render(const char *text)
{
    struct timeline t={.start=0};
    struct reloq61_request q={.language=0x60000,.annotations=1,.text=text,
        .voice={0,50,65,30,0,0,75,100},.context=&t,.span=span,.output_rate=11025};
    struct reloq61_result r={0};
    printf("TEXT: %s\n",text);
    int status=reloq61_generate(&q,&r);
    printf("STATUS=%d ERR=%d UNSUPPORTED=%d TICKS=%lld\n\n",status,r.error,r.unsupported,
        (long long)r.ticks);
}

int main(void)
{
    render("正在接收ai响应");
    render("正在接收 AI 响应");
    render("正在接收响应");
    return 0;
}
