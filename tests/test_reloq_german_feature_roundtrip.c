#include "reloq_german_word_features.h"
#include "reloq_german_generic_voice.h"
#include "reloq_german_voice_defaults.h"
#include "reloq_german_phone_sequence.h"
#include "reloq_german_nucleus_duration.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct pcm {int16_t *data;size_t count;};
static int collect(void *context,const int16_t *data,size_t count)
{
    struct pcm *p=context;
    void *next=realloc(p->data,(p->count+count)*sizeof *p->data);
    if(!next)return -1;
    p->data=next;memcpy(p->data+p->count,data,count*sizeof *data);
    p->count+=count;return 0;
}
static void roundtrip(const char *spr,const struct reloq_german_word_features *known,
    size_t count,int plain_equal)
{
    unsigned char *rich=NULL,*again=NULL;size_t rn=0,an=0;
    assert(!reloq_german_word_features_annotate((const unsigned char*)spr,strlen(spr),
        known,count,&rich,&rn));
    assert(!reloq_german_word_features_annotate(rich,rn,NULL,0,&again,&an));
    assert(rn==an&&!memcmp(rich,again,rn));
    struct pcm samples[3]={{0},{0},{0}};
    for(unsigned i=0;i<(plain_equal?3u:2u);++i) {
        float defaults[RELOQ_PARAMETER_TRACKS];uint8_t fields[RELOQ_TARGET_STREAM_COUNT];
        struct reloq_session_config config;
        assert(!reloq_german_voice_defaults(defaults,fields,&config));
        struct reloq_session *session=reloq_session_create(&config);assert(session);
        struct reloq_german_utterance_info info={0};
        int rc=i?reloq_german_generic_render_annotation(i==1?rich:(const unsigned char*)spr,
            i==1?rn:strlen(spr),session,collect,samples+i,&info):
            reloq_german_generic_render_text_plan((const unsigned char*)spr,strlen(spr),
                NULL,known,count,NULL,0,NULL,0,session,collect,NULL,samples+i,&info);
        if(rc || !samples[i].count)fprintf(stderr,"roundtrip mode %u status %d: %s\n",i,rc,spr);
        assert(!rc&&samples[i].count);
        reloq_session_destroy(session);
        if(i)assert(samples[i].count==samples[0].count &&
            !memcmp(samples[i].data,samples[0].data,samples[0].count*sizeof(int16_t)));
    }
    for(unsigned i=0;i<3;++i)free(samples[i].data);
    free(rich);free(again);
}
static int cancel(void *context){(void)context;return 1;}
int main(void)
{
    const char *invalid[]={
        "`[!0,0,0,0;.1a]", "`[!65,0,0,0;.1a]", "`[!1,2,0,0;.1a]",
        "`[!1,0,2,0;.1a]", "`[!1,0,0,2;.1a]", "`[!2,1,0,0;.1a]",
        "`[!1,1,0,0;.1a.0b@]", "`[!1,1,0,0;!1,1,0,0;.1a]",
        "`[.1a!1,1,0,0;]", "`[!1,,0,0;.1a]",
        "`[!1,10000000000000000,0,0;.1a]", "`[!1,1,0,0.1a]"
    };
    for(size_t i=0;i<sizeof invalid/sizeof *invalid;++i) {
        struct reloq_german_word_features f={.word_index=77},before=f;
        assert(reloq_german_word_features_from_spr((const unsigned char*)invalid[i],
            strlen(invalid[i]),&f)==-1 && !memcmp(&f,&before,sizeof f));
        unsigned char *out=NULL;size_t n=99;
        assert(reloq_german_word_features_annotate((const unsigned char*)invalid[i],
            strlen(invalid[i]),NULL,0,&out,&n)==-1 && !out && n==99);
    }
    const char *explicit_spr="`[!2,1,2,0;.1ze.2i]";
    struct reloq_german_word_features f={0};
    assert(reloq_german_word_features_from_spr((const unsigned char*)explicit_spr,
        strlen(explicit_spr),&f)==2);
    assert(f.syllable_count==2&&f.glottal_syllables==1&&f.full_quantity_syllables==2);
    struct reloq_german_word_features *out=NULL;size_t count=99;
    assert(reloq_german_word_features_resolve((const unsigned char*)explicit_spr,
        strlen(explicit_spr),NULL,0,&out,&count,cancel,NULL)==1&&!out&&count==99);

    const struct reloq_german_word_features hiatus={.syllable_count=3,.glottal_syllables=1};
    roundtrip("`[.0te.1a.0tR].",&hiatus,1,0);
    roundtrip("`[.2e.0lO.1kvEn.0T@].",NULL,0,1);
    roundtrip("`[.1kO.0mA]",NULL,0,1);
    roundtrip("`[.1kO.0mA] `[.1tEkst].",NULL,0,1);
    roundtrip("`[.1'aj'n.0Unt.2Ax.0TIX].",NULL,0,1);
    roundtrip("`[.1ze.2i.0g@l], `[.1tEkst]?",NULL,0,1);
    roundtrip(explicit_spr,NULL,0,1);

    /* Text-mode pretonic quantity is shared with authored SPR. The
     * original SPR-only posttonic shortening bug is intentionally absent. */
    struct reloq_german_phone_node nodes[16];
    struct reloq_german_phone_sequence_info info;
    const char *s="`[.2e.1lA.2o.0n@]";
    assert(!reloq_german_phone_sequence_build((const unsigned char*)s,strlen(s),0,nodes,16,&info));
    assert(nodes[0].primary_ahead&&reloq_german_nucleus_shortened(nodes));
    assert(nodes[3].phone==RELOQ_DE_PHONE_o&&!nodes[3].primary_ahead &&
        !reloq_german_nucleus_shortened(nodes+3));
    /* Original text observations distinguish a long closed primary
     * syllable from a short one and from a single-consonant coda. */
    const char *quantity_cases[]={
        "`[.2e.0li.1gons]", "`[.2e.0li.1gon]",
        "`[.2e.0li.1gOns]", "`[.2e.0li.1gons.2tAk]",
        "`[.2e.0li.1gon.2stAk]", "`[.0e.0li.1gons]"
    };
    const int shortened[]={0,1,1,0,1,1};
    for(size_t i=0;i<sizeof quantity_cases/sizeof *quantity_cases;++i) {
        s=quantity_cases[i];
        assert(!reloq_german_phone_sequence_build((const unsigned char*)s,strlen(s),0,nodes,16,&info));
        assert(reloq_german_nucleus_shortened(nodes)==shortened[i]);
        assert(reloq_german_nucleus_shortened(nodes+2));
    }
    puts("phonetic features: invalid input, idempotence, byte-exact PCM round trips, shared quantity passed");
    return 0;
}
