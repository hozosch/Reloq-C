#ifndef RELOQ_INLINE_TAGS_H
#define RELOQ_INLINE_TAGS_H
#include <stddef.h>

/* Independent, bounded recognizer. Unknown/malformed tags remain text.
 * VP is a private annotation carrier for a validated public p command;
 * it avoids confusing text-analysis passes with an acoustic pause token. */
enum reloq_inline_kind { RELOQ_TAG_NONE, RELOQ_TAG_SPR, RELOQ_TAG_SPELL,
    RELOQ_TAG_PROMINENCE, RELOQ_TAG_PITCH, RELOQ_TAG_SPEED,
    RELOQ_TAG_VOLUME, RELOQ_TAG_INFLECTION, RELOQ_TAG_PAUSE,
    RELOQ_TAG_GENDER, RELOQ_TAG_HEAD_SIZE, RELOQ_TAG_ROUGHNESS,
    RELOQ_TAG_BREATHINESS, RELOQ_TAG_PRESET, RELOQ_TAG_LANGUAGE };
struct reloq_inline_tag { enum reloq_inline_kind kind; size_t length; int value; };
struct reloq_inline_word {
    int pitch,speed,volume,inflection,pause,explicit_pause;
    size_t pitch_offset,annotation_offset;
    int gender,head_size,roughness,breathiness;
};
static inline struct reloq_inline_tag reloq_inline_tag_read(
    const unsigned char *s,size_t n,int internal)
{
    struct reloq_inline_tag t={RELOQ_TAG_NONE,0,0};
    if(n<2 || s[0]!='`')return t;
    if(s[1]=='[') {
        for(size_t i=2;i<n;++i) {
            if(s[i]=='`' || s[i]=='[' || !s[i])return t;
            if(s[i]==']') {t.kind=RELOQ_TAG_SPR;t.length=i+1;return t;}
        }
        return t;
    }
    size_t at=1;unsigned limit=100;
    if(s[1]>='0' && s[1]<='9') {t.kind=RELOQ_TAG_PROMINENCE;limit=9;}
    else if(s[1]=='l') {t.kind=RELOQ_TAG_LANGUAGE;at=2;limit=32767;}
    else if(n>2 && s[1]=='v' && s[2]>='1' && s[2]<='9')
        {t.kind=RELOQ_TAG_PRESET;at=2;limit=9;}
    else if(n>3 && s[1]=='t' && s[2]=='s') {t.kind=RELOQ_TAG_SPELL;at=3;limit=2;}
    else if(s[1]=='p') {t.kind=RELOQ_TAG_PAUSE;at=2;limit=60000;}
    else if(n>3 && s[1]=='v') {
        at=3;
        switch(s[2]) {
        case 'b':t.kind=RELOQ_TAG_PITCH;break;
        case 's':t.kind=RELOQ_TAG_SPEED;limit=250;break;
        case 'v':t.kind=RELOQ_TAG_VOLUME;break;
        case 'f':t.kind=RELOQ_TAG_INFLECTION;break;
        case 'g':t.kind=RELOQ_TAG_GENDER;limit=1;break;
        case 'h':t.kind=RELOQ_TAG_HEAD_SIZE;break;
        case 'r':t.kind=RELOQ_TAG_ROUGHNESS;break;
        case 'y':t.kind=RELOQ_TAG_BREATHINESS;break;
        case 'p':if(internal){t.kind=RELOQ_TAG_PAUSE;limit=60000;}break;
        default:break;
        }
    }
    if(!t.kind || at>=n || s[at]<'0' || s[at]>'9')return (struct reloq_inline_tag){0};
    unsigned value=0;size_t start=at;
    while(at<n && s[at]>='0' && s[at]<='9') {
        unsigned d=s[at++]-'0';
        if(value>limit/10 || value*10+d>limit)return (struct reloq_inline_tag){0};
        value=value*10+d;
    }
    if(t.kind==RELOQ_TAG_PROMINENCE && at-start>1 && !(at-start==2 && !value))
        return (struct reloq_inline_tag){0};
    if(t.kind==RELOQ_TAG_SPEED && value<40)return (struct reloq_inline_tag){0};
    if(t.kind==RELOQ_TAG_LANGUAGE) {
        unsigned dialect=0;
        if(at+1<n && s[at]=='.' && s[at+1]>='0' && s[at+1]<='9') {
            ++at;
            do {
                unsigned d=s[at++]-'0';
                if(dialect>6553 || dialect*10+d>65535)return (struct reloq_inline_tag){0};
                dialect=dialect*10+d;
            } while(at<n && s[at]>='0' && s[at]<='9');
        }
        /* The UTF-8 entry point already decoded the input. Accept the
           optional ECI encoding suffix without reinterpreting its bytes. */
        if(at+1<n && s[at]=='.' && s[at+1]>='0' && s[at+1]<='9') {
            unsigned encoding=0;++at;
            do {
                unsigned d=s[at++]-'0';
                if(encoding>6553 || encoding*10+d>65535)return (struct reloq_inline_tag){0};
                encoding=encoding*10+d;
            } while(at<n && s[at]>='0' && s[at]<='9');
        }
        value=(value<<16)|dialect;
    }
    t.length=at;t.value=(int)value;return t;
}
/* Observed ECI voice presets, in the public parameter scale. The ninth
 * preset is the module's neutral editable voice. These are parameters,
 * not acoustic samples or retained synthesis results. */
static inline void reloq_inline_preset(struct reloq_inline_word *word,int voice)
{
    static const unsigned char presets[9][8]={
        {0,50,65,30,0,0,50,92}, {1,50,81,30,0,50,50,100},
        {1,22,93,35,0,0,50,90}, {0,86,56,47,0,0,50,93},
        {0,50,69,34,0,0,70,92}, {1,56,89,35,0,40,70,95},
        {1,45,68,30,3,40,50,90}, {0,30,61,44,18,20,50,90},
        {0,50,69,34,0,0,50,92}};
    if(voice<1 || voice>9)return;
    const unsigned char *v=presets[voice-1];
    word->gender=v[0];word->head_size=v[1];word->pitch=v[2];
    word->inflection=v[3];word->roughness=v[4];word->breathiness=v[5];
    word->speed=v[6];word->volume=v[7];
}
#endif
