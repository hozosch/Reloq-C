#include "reloq_english.h"
#include "reloq_target_builder.h"
#include "reloq_curve_pipeline.h"
#include "reloq_german_voice_defaults.h"
#include <string.h>

struct english_nucleus {unsigned char phone;short duration,f[10],av_begin,av_end;};
#include "reloq_english_nuclei.inc"

struct english_coda {
    unsigned char vowel,phone;
    short contact,body,burst,release,af,ah;
    short noise[6],vowel_f[6],f[10],b[5],closure_voice,tail_voice;
};
#include "reloq_english_coda_targets.inc"
struct english_continuant {struct english_coda coda;short nasal_zero,vowel_b[5];};
#include "reloq_english_continuant_targets.inc"

/* A diphthong owns one source/F0 envelope and two tract gestures. The
 * short terminal glide uses its midpoint target, rather than inserting
 * another vowel or restarting the glottal source at the join. */
struct english_diphthong {
    unsigned char phone;
    short head,contact,tail,release;
    short initial[3],departing[3],arriving[5];
};
static const struct english_diphthong diphthongs[]={
    {'Y',175,75,10,30,{750,1200,2440},{750,1350,2440},{400,1850,2400,3600,3900}},
    {'W',210,50,20,40,{750,1400,2410},{700,1300,2410},{550,900,2200,3300,3600}},
    {'O',175,75,10,30,{480,900,2440},{480,900,2440},{400,1800,2350,3600,3900}}
};

static int point(struct reloq_target_builder *b,enum reloq_target_stream s,int t,int v)
{
    struct reloq_target_event e={s,0,v};
    return reloq_target_builder_append(b,t,&e,1);
}
static int hold(struct reloq_target_builder *b,enum reloq_target_stream s,int a,int z,int v)
{return point(b,s,a,v)||point(b,s,z,v);}

static int render(unsigned char onset,unsigned char phone,unsigned char closing,struct reloq_session *session,
    reloq_session_pcm_sink sink,void *context,size_t *completed)
{
    if(!session || !sink || reloq_session_sample_rate(session)!=11025)
        return RELOQ_ENGLISH_INVALID;
    const struct english_nucleus *v=NULL;
    const struct english_diphthong *d=NULL;
    const struct english_coda *coda=NULL;
    const struct english_continuant *continuant=NULL;
    const int nasal_coda=closing && strchr("mnG",closing)!=NULL;
    const int voiced_coda=closing && strchr("vzZD",closing)!=NULL;
    if(closing) {
        for(size_t i=0;i<sizeof english_codas/sizeof *english_codas;++i)
            if(english_codas[i].vowel==phone && english_codas[i].phone==closing)
                {coda=english_codas+i;break;}
        if(!coda)for(size_t i=0;i<sizeof english_continuants/sizeof *english_continuants;++i)
            if(english_continuants[i].coda.vowel==phone && english_continuants[i].coda.phone==closing)
                {continuant=english_continuants+i;coda=&continuant->coda;break;}
        if(!coda)return RELOQ_ENGLISH_UNCOVERED;
    }
    struct english_nucleus composite={0};
    for(size_t i=0;i<sizeof english_nuclei/sizeof *english_nuclei;++i)
        if(english_nuclei[i].phone==phone){v=english_nuclei+i;break;}
    for(size_t i=0;i<sizeof diphthongs/sizeof *diphthongs;++i)
        if(diphthongs[i].phone==phone){d=diphthongs+i;break;}
    if(d) {
        composite.phone=phone;composite.duration=d->head+d->contact+d->tail;
        composite.av_begin=phone=='O'?56:50;composite.av_end=composite.av_begin-2;
        for(unsigned i=0;i<5;++i) {
            composite.f[2*i]=i<3?d->initial[i]:i==3?3600:3900;
            composite.f[2*i+1]=i<3?d->departing[i]:composite.f[2*i];
        }
        v=&composite;
    }
    const int unvoiced_stop=onset=='p'||onset=='t'||onset=='k';
    const int voiced_stop=onset=='b'||onset=='d'||onset=='g';
    const int stop=unvoiced_stop||voiced_stop;
    const int dental=onset=='T'||onset=='D';
    const int aspirate=onset=='h';
    const int fricative=onset && strchr("fvszSZTD",onset)!=NULL;
    const int voiced_fricative=onset=='v'||onset=='z'||onset=='Z'||onset=='D';
    if(!v || (onset && ((d && !aspirate) ||
       (!stop && !fricative && !aspirate && onset!='m' && onset!='n'))))return RELOQ_ENGLISH_UNCOVERED;
    const struct english_nucleus *isolated=v;
    struct english_nucleus context_vowel;
    int nasal[10]={0},nasal_widths[5]={120,150,105,330,260};
    int body=0,contact=0;
    if(onset) {
        context_vowel=*v;v=&context_vowel;
        const int labial=onset=='m'||onset=='p'||onset=='b'||onset=='f'||onset=='v';body=labial?100:70;
        contact=!labial && (phone=='a'||phone=='c')?50:!labial && phone=='R'?40:30;
        const int initial[5]={labial?(phone=='i'||phone=='u'?250:300):200,
            labial?1000:1500,labial?2200:2550,labial?3300:3600,labial?3600:3900};
        for(unsigned i=0;i<5;++i)nasal[2*i]=nasal[2*i+1]=initial[i];
        if(labial) {
            nasal[5]=2250;
            if(phone=='i'){nasal[3]=1400;nasal[5]=2400;}
            else if(phone=='I'||phone=='e'||phone=='E'||phone=='A')nasal[3]=1250;
            else if(phone=='X')nasal[3]=1200;
            else if(phone=='c'){nasal[3]=920;nasal[5]=2150;}
            switch(phone) {
            case 'E':context_vowel.f[4]=2380;break;
            case 'A':context_vowel.f[2]=1550;break;
            case 'X':context_vowel.f[2]=1400;context_vowel.f[4]=2450;break;
            case 'H':context_vowel.f[2]=1100;break;
            case 'x':context_vowel.f[2]=1200;context_vowel.f[4]=2350;break;
            case 'u':context_vowel.f[2]=1100;break;
            case 'U':context_vowel.f[2]=950;break;
            case 'R':context_vowel.f[2]=1050;context_vowel.f[3]=1200;context_vowel.f[5]=1600;break;
            default:break;
            }
        } else {
            nasal_widths[1]=200;nasal_widths[2]=90;
            if(phone=='i'){nasal[3]=1750;nasal[5]=2750;}
            else if(phone=='I'||phone=='e')nasal[3]=1650;
            else if(phone=='E'||phone=='A')nasal[3]=1600;
            else if(phone=='R')nasal[5]=2450;
            switch(phone) {
            case 'x':context_vowel.f[2]=1450;break;
            case 'a':context_vowel.f[2]=1250;break;
            case 'u':context_vowel.f[2]=1450;context_vowel.f[4]=2540;break;
            case 'o':context_vowel.f[2]=1200;break;
            case 'R':context_vowel.f[2]=1300;context_vowel.f[4]=1750;break;
            default:break;
            }
        }
        if(phone=='x')context_vowel.f[0]=context_vowel.f[1]=500;
        if(fricative) {
            body=onset=='f'||onset=='s'?110:onset=='z'?130:onset=='S'?150:80;
            contact=onset=='f'?30:onset=='s'?(phone=='i'?60:30):40;
            nasal[0]=nasal[1]=onset=='f'?400:300;
            nasal_widths[1]=100;nasal_widths[2]=150;
            if(onset=='v'){nasal_widths[1]=95;nasal_widths[2]=125;}
            if(onset=='s'||onset=='z') {
                nasal[6]=nasal[7]=4000;nasal[8]=nasal[9]=4300;
            } else if(onset=='S'||onset=='Z') {
                nasal[2]=1700;nasal[4]=2400;
                nasal[3]=phone=='i'||phone=='e'?1850:strchr("IEA",phone)?1750:1700;
                nasal[5]=phone=='i'||phone=='e'?2600:strchr("IEA",phone)?2500:2400;
                nasal[6]=nasal[7]=3600;nasal[8]=nasal[9]=4000;
                if(phone=='A'){context_vowel.f[2]=1700;context_vowel.f[4]=2310;}
                if(phone=='X'){context_vowel.f[2]=1700;context_vowel.f[4]=2450;}
                if(phone=='a')context_vowel.f[2]=1200;
            }
            if(dental) {
                /* The dental constriction has a broad bypass source,
                 * its own onset contact, and no German /s/ substitution.
                 * Its vowel adjustment is shared by voiced/unvoiced th. */
                body=onset=='T'?110:15;
                contact=onset=='T'?40:phone=='e'?20:phone=='R'?55:30;
                const int f[]={400,400,1450,phone=='i'?1850:
                    strchr("IeEA",phone)?1550:phone=='X'?1350:1450,
                    2600,2600,3600,3600,onset=='D'?4240:3900,onset=='D'?4240:3900};
                memcpy(nasal,f,sizeof nasal);context_vowel=*isolated;
                if(onset=='D')nasal[2]=nasal[3]=(nasal[2]+nasal[3])/2;
                if(phone=='x')context_vowel.f[0]=context_vowel.f[1]=500;
                if(phone=='u'){context_vowel.f[2]=1450;context_vowel.f[4]=2540;}
            }
        }
        if(stop) {
            body=onset=='p'?10:onset=='t'?15:20;
            contact=onset=='p'?50:70;
            nasal[0]=nasal[1]=300;
            nasal_widths[1]=100;nasal_widths[2]=150;
            if(onset=='k'||onset=='g') {
                context_vowel=*isolated;
                const int back=phone=='u'||phone=='U'||phone=='o'||phone=='c';
                nasal[2]=back?1450:1650;nasal[4]=2300;
                nasal[3]=phone=='i'?2150:phone=='e'?2050:
                    strchr("IEA",phone)?2000:phone=='X'?1850:
                    phone=='u'?1200:phone=='o'?1400:phone=='c'?1350:
                    phone=='R'?1450:1650;
                nasal[5]=phone=='i'?2500:strchr("IeEA",phone)?2600:
                    phone=='X'?2400:phone=='u'?2100:2300;
                nasal[6]=nasal[7]=3600;nasal[8]=nasal[9]=3900;
                if(phone=='e'){context_vowel.f[2]=1900;context_vowel.f[4]=2380;}
                if(phone=='A'){context_vowel.f[2]=1700;context_vowel.f[4]=2310;}
                if(phone=='x')context_vowel.f[0]=context_vowel.f[1]=500;
                if(phone=='u')context_vowel.f[2]=1100;
                if(phone=='o')context_vowel.f[2]=1200;
            } else {
                /* The bilabial/alveolar burst uses the midpoint of its
                 * context tract. The velar source keeps both endpoints. */
                if(unvoiced_stop)for(unsigned i=1;i<3;++i)
                    nasal[2*i]=nasal[2*i+1]=(nasal[2*i]+nasal[2*i+1])/2;
                if(onset=='t'||onset=='d'){nasal[6]=nasal[7]=4000;nasal[8]=nasal[9]=4300;}
                if(voiced_stop)nasal[0]=nasal[1]=200;
            }
            /* The 0.9 post-stop weighting precedes integer durations.
             * E/H have a 212.5-ms underlying isolated target. */
            if(unvoiced_stop)context_vowel.duration=(short)(isolated->duration*9/10+
                (isolated->duration==212));
            if(voiced_stop) {
                body=onset=='b'?95:onset=='d'?80:(phone=='i'||phone=='u')?95:85;
                contact=onset=='b'?(phone=='e'?20:strchr("aoc",phone)?15:30):
                    strchr("iIeEA",phone)?30:onset=='d'&&phone=='o'?35:50;
            }
        }
        if(aspirate) {
            /* English h starts the following vowel's tract before its
             * periodic source. It is not an extra oral fricative gesture. */
            body=80;contact=0;context_vowel=*isolated;
            for(unsigned i=0;i<5;++i)nasal[2*i]=nasal[2*i+1]=v->f[2*i];
            nasal_widths[0]=(phone=='c'||phone=='O')?200:120;
            nasal_widths[1]=100;nasal_widths[2]=phone=='R'?250:150;
        }
    }
    if(coda) {
        /* One nucleus budget, retaining the fractional intrinsic duration
         * until all context factors have been applied. Truncating a CV
         * first and then shortening it would lose a tick in lax vowels. */
        context_vowel=*v;v=&context_vowel;
        double duration=isolated->duration+
            (strchr("IEHU",phone)?.5:0);
        if(strchr("ptkfsST",closing)) {
            if(strchr("IEXHxU",phone))duration=duration*90/100;
            duration=duration*80/100;
        } else duration=duration*(strchr("iu",phone)?115:
            strchr("IEXHxU",phone)?100:105)/100;
        if(voiced_coda)duration=duration*110/100;
        if(unvoiced_stop)duration=duration*90/100;
        context_vowel.duration=(short)duration;
        for(unsigned i=0;i<3;++i) {
            if(!onset)context_vowel.f[2*i]=coda->vowel_f[2*i];
            context_vowel.f[2*i+1]=coda->vowel_f[2*i+1];
        }
    }
    const int voice_start=body+(unvoiced_stop?contact:0);
    const int end=voice_start+v->duration;
    const int tract_end=end-(coda?coda->contact:0);
    const int stop_end=end+(coda?coda->body:0);
    const int stop_burst=stop_end-(coda?coda->burst:0);
    const int quiet=coda?stop_end+coda->release:end+(d?d->release:60),finish=quiet+400;
    const int noise_end=continuant?tract_end:coda?end:finish;
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][24];
    struct reloq_target_buffer storage[RELOQ_TARGET_STREAM_COUNT];
    for(size_t i=0;i<RELOQ_TARGET_STREAM_COUNT;++i)
        storage[i]=(struct reloq_target_buffer){points[i],24,0};
    struct reloq_target_builder b;
    if(reloq_target_builder_begin(&b,storage))return RELOQ_ENGLISH_INVALID;
    for(unsigned i=0;i<5;++i) {
        int a=v->f[2*i],z=v->f[2*i+1];if(z==-1)z=a;
        if(onset && (point(&b,RELOQ_TARGET_F1+i,0,nasal[2*i]) ||
            point(&b,RELOQ_TARGET_F1+i,body,nasal[2*i+1])))return -1;
        if(point(&b,RELOQ_TARGET_F1+i,body+contact,a)||
            (z!=-2 && point(&b,RELOQ_TARGET_F1+i,d?voice_start+d->head:tract_end,z)))return -1;
        if(d && hold(&b,RELOQ_TARGET_F1+i,voice_start+d->head+d->contact,end,d->arriving[i]))return -1;
        if(coda && (point(&b,RELOQ_TARGET_F1+i,end,coda->f[2*i]) ||
            point(&b,RELOQ_TARGET_F1+i,stop_end,coda->f[2*i+1]<0?coda->f[2*i]:coda->f[2*i+1])))return -1;
    }
    const int widths[5]={120,100,150,330,260};
    for(unsigned i=0;i<5;++i) {
        int nucleus_width=widths[i];
        if((phone=='c'||phone=='O') && !i)nucleus_width=200;
        if(phone=='R' && i==2)nucleus_width=250;
        if(continuant)nucleus_width=continuant->vowel_b[i];
        if(onset && hold(&b,RELOQ_TARGET_B1+i,0,body,nasal_widths[i]))return -1;
        if(hold(&b,RELOQ_TARGET_B1+i,body+contact,d?voice_start+d->head:tract_end,nucleus_width))return -1;
        if(d && hold(&b,RELOQ_TARGET_B1+i,voice_start+d->head+d->contact,end,
            phone=='W'&&i==2?500:widths[i]))return -1;
        if(coda && hold(&b,RELOQ_TARGET_B1+i,end,stop_end,coda->b[i]))return -1;
        if(hold(&b,RELOQ_TARGET_B1+i,quiet,finish,widths[i]))return -1;
    }
    for(unsigned i=RELOQ_TARGET_FNP;i<=RELOQ_TARGET_FTZ;++i) {
        if(i==RELOQ_TARGET_FNZ && onset && !stop && !fricative && !aspirate) {
            if(point(&b,i,body/2,350) || hold(&b,i,body+contact,nasal_coda?tract_end:finish,200))return -1;
        } else if(hold(&b,i,0,nasal_coda && i==RELOQ_TARGET_FNZ?tract_end:finish,i<=RELOQ_TARGET_FNZ?200:250))return -1;
        if(nasal_coda && i==RELOQ_TARGET_FNZ &&
           (point(&b,i,end+coda->body/2,continuant->nasal_zero) || hold(&b,i,quiet,finish,200)))return -1;
    }
    if(onset && !stop && !fricative && !aspirate && (hold(&b,RELOQ_TARGET_AV,0,body,onset=='m'?52:49) ||
        hold(&b,RELOQ_TARGET_AH,0,body,0)))return -1;
    if(aspirate && (hold(&b,RELOQ_TARGET_AV,0,body,0) ||
       hold(&b,RELOQ_TARGET_AH,0,body,phone=='A'?39:
            strchr("acYWO",phone)?40:strchr("iIeEH",phone)?42:46)))return -1;
    int burst=0,aspiration=0,pole=0,pole_level=0;
    if(unvoiced_stop) {
        burst=onset=='p'?60:onset=='t'?65:phone=='i'?67:phone=='e'?63:
            strchr("IEA",phone)?57:(phone=='u'||phone=='U')?64:60;
        aspiration=onset=='p'?49:onset=='t'?47:strchr("iIeEA",phone)?42:45;
        pole=onset=='p'?RELOQ_TARGET_AB:onset=='t'?RELOQ_TARGET_A5F:
            (phone=='i'||phone=='e')?RELOQ_TARGET_A3F:RELOQ_TARGET_A2F;
        pole_level=onset=='t'?50:onset=='k'&&(phone=='i'||phone=='e')?60:70;
        if(hold(&b,RELOQ_TARGET_AV,0,voice_start,0) ||
           hold(&b,RELOQ_TARGET_AH,0,body,0) ||
           hold(&b,RELOQ_TARGET_AH,body,voice_start,aspiration))return -1;
    }
    const int closure=onset=='b'?90:70;
    if(voiced_stop) {
        burst=onset=='b'?56:onset=='d'?53:62;
        pole=onset=='b'?RELOQ_TARGET_AB:onset=='d'?RELOQ_TARGET_A5F:RELOQ_TARGET_A2F;
        pole_level=onset=='b'?70:onset=='d'?57:strchr("iIeEA",phone)?65:70;
        if(hold(&b,RELOQ_TARGET_AV,0,closure,20) ||
           hold(&b,RELOQ_TARGET_AV,closure,body,0) ||
           hold(&b,RELOQ_TARGET_AH,0,body,0))return -1;
    }
    if(fricative) {
        if(voiced_fricative) {
            if(point(&b,RELOQ_TARGET_AV,onset=='z'?body-40:body/2,
                     onset=='v'?30:onset=='z'?37:onset=='D'?35:40))return -1;
        } else if(hold(&b,RELOQ_TARGET_AV,0,body,0))return -1;
        if(hold(&b,RELOQ_TARGET_AH,0,body,0))return -1;
    }
    for(unsigned i=RELOQ_TARGET_AF;i<RELOQ_TARGET_STREAM_COUNT;++i) {
        int level=stop?(i==RELOQ_TARGET_AF?burst:(int)i==pole?pole_level:0):0;
        if(fricative) {
            if(i==RELOQ_TARGET_AF)level=onset=='f'?55:onset=='v'?60:
                onset=='s'?60:onset=='z'?65:onset=='S'?58:onset=='T'?60:onset=='D'?52:57;
            else if(onset=='f' && (i==RELOQ_TARGET_A1F || i==RELOQ_TARGET_A2F))level=55;
            else if(i==RELOQ_TARGET_AB && (onset=='f'||onset=='v'))level=onset=='f'?65:60;
            else if(i==RELOQ_TARGET_A5F && (onset=='s'||onset=='z'))level=50;
            else if(i==RELOQ_TARGET_A4F && onset=='z')level=50;
            else if(i==RELOQ_TARGET_A3F && (onset=='S'||onset=='Z'))level=62;
            else if(i==RELOQ_TARGET_A4F && (onset=='S'||onset=='Z'))level=59;
            else if(i==RELOQ_TARGET_AB && dental)level=onset=='T'?60:70;
            if(onset=='s' && i==RELOQ_TARGET_AF) {
                if(point(&b,i,0,60)||point(&b,i,body/2,70)||point(&b,i,body,60))return -1;
            } else if(onset=='T' && i==RELOQ_TARGET_AF) {
                if(hold(&b,i,0,body-20,60)||hold(&b,i,body-20,body,64))return -1;
            } else if(hold(&b,i,0,body,level))return -1;
            if(hold(&b,i,body+contact,noise_end,0))return -1;
        } else if(level) {
            if(hold(&b,i,0,voiced_stop?closure:5,0)||
               hold(&b,i,voiced_stop?closure:5,body,level)||
               hold(&b,i,voiced_stop?body+contact:voice_start,noise_end,0))return -1;
        } else if(hold(&b,i,0,noise_end,0))return -1;
        if(coda) {
            int value=i==RELOQ_TARGET_AF?coda->af:coda->noise[i-RELOQ_TARGET_A1F];
            if(continuant) {
                if(closing=='s' && i==RELOQ_TARGET_AF) {
                    if(point(&b,i,end,value)||point(&b,i,end+coda->body/2,value+10)||
                       point(&b,i,stop_end,value))return -1;
                } else if(closing=='T' && i==RELOQ_TARGET_AF) {
                    if(hold(&b,i,end,stop_end-20,value)||hold(&b,i,stop_end-20,stop_end,value+4))return -1;
                } else if(hold(&b,i,end,stop_end,value))return -1;
            } else if(hold(&b,i,end,stop_burst,0) || hold(&b,i,stop_burst,stop_end,value))return -1;
            if(hold(&b,i,quiet,finish,0))return -1;
        }
    }
    if(voiced_stop && hold(&b,RELOQ_TARGET_TL,0,closure,35))return -1;
    if(hold(&b,RELOQ_TARGET_TL,voiced_stop?closure:0,coda?end:finish,0) ||
       hold(&b,RELOQ_TARGET_FL,0,finish,0) ||
       point(&b,RELOQ_TARGET_AV,voice_start+1,v->av_begin) || point(&b,RELOQ_TARGET_AV,end,v->av_end) ||
       hold(&b,RELOQ_TARGET_AH,voice_start+1,end,34))return -1;
    if(continuant) {
        if(hold(&b,RELOQ_TARGET_TL,end,finish,0) || hold(&b,RELOQ_TARGET_AH,end,stop_end,0) ||
           hold(&b,RELOQ_TARGET_AH,stop_end,quiet,coda->ah))return -1;
        if(nasal_coda) {
            if(point(&b,RELOQ_TARGET_AV,end+coda->body*4/5,coda->closure_voice) ||
               hold(&b,RELOQ_TARGET_AV,stop_end,stop_end+(coda->tail_voice?30:0),coda->tail_voice) ||
               hold(&b,RELOQ_TARGET_AV,stop_end+(coda->tail_voice?30:0),quiet,0))return -1;
        } else if(voiced_coda) {
            if(point(&b,RELOQ_TARGET_AV,end+40,coda->closure_voice) ||
               hold(&b,RELOQ_TARGET_AV,end+40,quiet,0))return -1;
        } else if(hold(&b,RELOQ_TARGET_AV,end,quiet,0))return -1;
    } else if(coda) {
        if(hold(&b,RELOQ_TARGET_TL,end,stop_burst,coda->closure_voice?35:0) ||
           hold(&b,RELOQ_TARGET_TL,stop_burst,finish,0) ||
           hold(&b,RELOQ_TARGET_AV,end,stop_burst,coda->closure_voice) ||
           hold(&b,RELOQ_TARGET_AV,stop_burst,stop_end,0) ||
           hold(&b,RELOQ_TARGET_AV,stop_end,stop_end+(coda->tail_voice?30:0),coda->tail_voice) ||
           hold(&b,RELOQ_TARGET_AV,stop_end+(coda->tail_voice?30:0),quiet,0) ||
           hold(&b,RELOQ_TARGET_AH,end,stop_end,0) ||
           hold(&b,RELOQ_TARGET_AH,stop_end,quiet,coda->ah))return -1;
    } else if(hold(&b,RELOQ_TARGET_AH,end,quiet,35))return -1;
    if(hold(&b,RELOQ_TARGET_AV,quiet,finish,0) ||
       hold(&b,RELOQ_TARGET_AH,quiet,finish,0) ||
       point(&b,RELOQ_TARGET_OQ,0,onset?56:18) || hold(&b,RELOQ_TARGET_OQ,onset?0:21,finish,56) ||
       point(&b,RELOQ_TARGET_DI,0,onset?0:100) || hold(&b,RELOQ_TARGET_DI,onset?0:21,finish,0) ||
       hold(&b,RELOQ_TARGET_F4,quiet,finish,3600) ||
       hold(&b,RELOQ_TARGET_F5,quiet,finish,3900))return -1;
    const int positions[4]={2,35,70,99},pitch[4]={1217,1296,796,743};
    const int rime=(nasal_coda?stop_end:end)-voice_start;
    for(unsigned i=0;i<4;++i)
        if(point(&b,RELOQ_TARGET_F0,onset?(i?voice_start+rime*positions[i]/100:
            aspirate?(voice_start+(d?d->head:v->duration))/10:body/10):
            rime*positions[i]/100,onset && !i?(aspirate?1049:1046):pitch[i]))return -1;
    /* Only the language-independent numeric map/defaults are shared. The
     * vowel identity, tract motion, duration and source targets above are
     * English observations, not German phoneme substitutions. */
    float defaults[62];uint8_t fields[28];struct reloq_session_config config;
    if(reloq_german_voice_defaults(defaults,fields,&config))return -1;
    struct reloq_curve_schedule schedule={.first_tick=0,.end_tick=finish,.frame_ticks=1};
    unsigned char mapped[62];
    if(reloq_target_builder_export(&b,fields,defaults,schedule.tracks,mapped))return -1;
    return reloq_curve_pipeline_render(&schedule,5,session,sink,context,completed);
}

int reloq_english_render_nucleus(unsigned char phone,struct reloq_session *session,
    reloq_session_pcm_sink sink,void *context,size_t *completed)
{return render(0,phone,0,session,sink,context,completed);}

int reloq_english_render_cv(unsigned char onset,unsigned char vowel,
    struct reloq_session *session,reloq_session_pcm_sink sink,void *context,size_t *completed)
{
    if(onset!='m' && onset!='n' && onset!='p' && onset!='t' && onset!='k' &&
       onset!='b' && onset!='d' && onset!='g' && (!onset || !strchr("fvszSZTDh",onset)))return RELOQ_ENGLISH_UNCOVERED;
    return render(onset,vowel,0,session,sink,context,completed);
}

int reloq_english_render_cvc(unsigned char onset,unsigned char vowel,unsigned char coda,
    struct reloq_session *session,reloq_session_pcm_sink sink,void *context,size_t *completed)
{
    if(!coda)return RELOQ_ENGLISH_UNCOVERED;
    return render(onset,vowel,coda,session,sink,context,completed);
}
