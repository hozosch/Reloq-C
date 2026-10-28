#include "reloq_german_voice_model.h"
#include "reloq_german_obstruent_tract.h"
#include <math.h>

typedef enum reloq_german_phone Phone;
#define P(x) RELOQ_DE_PHONE_##x
static int nasal(Phone p) {return p==P(m)||p==P(n)||p==P(G);}
static int vowel(Phone p)
{
    return (p>=P(a)&&p<=P(AT))||p==P(R)||p>=P(Q_E_COLON);
}
static int back(Phone p)
{
    return p==P(o)||p==P(O)||p==P(u)||p==P(U)||p==P(R)||p==P(Q_o_TILDE);
}
static int front(Phone p)
{
    return p==P(e)||p==P(E)||p==P(i)||p==P(I)||p==P(y)||p==P(Y)||
        p==P(Q_oe)||p==P(Q_OE)||p==P(Q_E_COLON)||p==P(Q_E_TILDE);
}
unsigned reloq_german_voice_model_formants(const struct reloq_german_voice_model *m)
{
    if(m && m->sample_rate>=44100)return 8;
    if(m && m->sample_rate>=32000)return 7;
    if(m && m->sample_rate>=16000)return 6;
    unsigned base=m && m->head_size<=(m->gender?40:30)?4:5;
    return m && m->sample_rate==8000?base-1:base;
}

/* Observed source controls, model defaults and anatomy filters. The filters
 * act on each endpoint, not on interpolated audio frames. This distinction
 * matters where a formant crosses one of the female model's range edges.
 * Evidence: reference-apple/behavior-0.2.56. */
static int frequency(unsigned index,int value)
{
    switch(index) {
    case 0:return (int)(value*(value<=350?1.07:value<=500?1.10:value<=650?1.18:1.25));
    case 1:return (int)(value*(value<=1100?1.07:value<=1300?1.15:value<=1700?1.17:1.21));
    case 2:return (int)(value*1.19);
    case 3:return value>3650?4200:(int)(value*(value<=3000?1.2:1.15));
    default:return value>=4000?4600:(int)(value*(value<=2700?1.2:1.15));
    }
}
static int vowel_breath(const struct reloq_german_voice_model *m,
    const struct reloq_german_phone_node *n,
    const struct reloq_german_phone_node *right,Phone p)
{
    int ah=50*m->breathiness/100;
    if(m->sample_rate==8000)ah=ah*8/10;
    if(m->breathiness==100 || !n || !vowel(p))return ah;
    /* A diphthong keeps its first nucleus's source through the glide.
     * Its release belongs to that glide, not to a final simple vowel. */
    if(p==P(Q_aj)||p==P(Q_aw))p=P(A);
    else if(p==P(Q_oj))p=P(O);
    if(back(p)) {if(ah>0)ah+=3;}
    else if(p==P(a)||p==P(A)||front(p)) {
        if(p==P(a)||p==P(A))ah=ah>=5?ah-8:0;
        else ah=ah>=9?ah-5:0;
    }
    if(!n->stress)ah+=3;
    if(nasal(n->right_phone) && (n->same_syllable_right ||
       (right && ((n->stress==1 && !right->stress && n->same_word_right) ||
                  (!n->stress && right->stress==1))))) {
        if(ah>=4)ah-=3;
    }
    if(n->right_phone==P(NONE) && n->phone!=P(Q_aj) &&
       n->phone!=P(Q_aw) && n->phone!=P(Q_oj) && ah>=4)ah-=4;
    return ah;
}
int32_t reloq_german_voice_model_target(const struct reloq_german_voice_model *m,
    const struct reloq_german_phone_node *n,
    const struct reloq_german_phone_node *right_node,
    enum reloq_target_stream stream,int32_t value)
{
    if(!m || (!m->gender && m->head_size==50 && !m->roughness && !m->breathiness && m->sample_rate!=8000))return value;
    const int female=m->gender,b=m->breathiness;
    unsigned measured=m->sample_rate>=16000?(unsigned)(m->head_size<=(female?40:30)?4:5):reloq_german_voice_model_formants(m);
    Phone p=n?n->phone:P(NONE);
    if(p==P(r)&&n->same_syllable_left&&vowel(n->left_phone))p=P(R);
    const int small=female && m->head_size<30;
    const int reduced_noise=m->sample_rate==8000 && measured==3 && !small;
    int tilt=(female?36:35)*b/100;
    if(female && m->sample_rate==8000)tilt=tilt*8/10;
    tilt+=small?18:0;
    /* The original 8-kHz female word tilt is reduced first. Its
     * consonant excitation adjustment restores 125 percent afterwards,
     * including the child offset and integer truncation. */
    const int excitation_tilt=female && m->sample_rate==8000?tilt*5/4:tilt;
    /* A reduced tract moves sibilant excitation onto an active pole.
     * The small female source uses pole three; the four-formant adult
     * model uses pole four. These are filter levels, not source gains. */
    if(p==P(s)||p==P(z)) {
        if(small && stream==RELOQ_TARGET_A3F)return p==P(s)?46:50;
        /* Intentional repair of the original three-pole 8-kHz dropout:
         * use the active top pole, with the four-pole source levels. */
        if(reduced_noise && stream==RELOQ_TARGET_A3F)return p==P(s)?48:50;
        if(!small && measured==4 &&
           stream==RELOQ_TARGET_A4F && p==P(s))return 48;
    }
    if(small && p==P(S) && stream==RELOQ_TARGET_A2F && value==56)return 59;
    if(p==P(k) && value>0 && (stream==RELOQ_TARGET_A2F || stream==RELOQ_TARGET_A3F)) {
        struct reloq_german_obstruent_tract tract;
        if(reloq_german_obstruent_tract_plan(n,n->left_phone==P(NONE),0,&tract)==1) {
            unsigned index=stream==RELOQ_TARGET_A2F?1:2;
            int before=tract.departing[index];
            int after=reloq_german_voice_model_target(m,n,right_node,
                RELOQ_TARGET_F1+index,before);
            /* get_amp_scale observes the anatomy-filtered endpoint. */
            value-=(after-before)/200;
        }
        return value>0?value:0;
    }
    if(small && p==P(k) && stream==RELOQ_TARGET_AF && value>0)value=value>4?value-4:0;
    if(stream>=RELOQ_TARGET_F1 && stream<=RELOQ_TARGET_F5) {
        unsigned index=(unsigned)(stream-RELOQ_TARGET_F1);
        if(reduced_noise && index==2 &&
           (p==P(s)||p==P(z)||p==P(t)||p==P(d)))
            value=p==P(s)?3400:3500;
        if(small && index==2 && (p==P(s)||p==P(z)))value=p==P(s)?3400:2450;
        /* These consonants supply explicit high formants. Other phones
         * inherit the model's word defaults, including the quiet tail. */
        if(female && index==3 && value==3600 && p!=P(S)&&p!=P(Z))value=4200;
        if(female && index==4 && value==3900 && p!=P(l))value=4600;
        if(m->sample_rate==8000 && (p==P(t)||p==P(d))) {
            if(index==3)value=3500;
            if(index==4)value=3700;
        }
        if(measured==4 && (p==P(s)||p==P(z))) {
            if(index==3)value=m->sample_rate==8000?3400:4700;
            if(index==4)value=3900;
        }
        if(m->sample_rate==8000 && (p==P(s)||p==P(z))) {
            if(index==3)value=female?3800:p==P(z)?3500:3400;
            if(index==4)value=female?4600:3900;
        }
        value=value*((250-m->head_size)/2)/100;
        if(female)value=frequency(index,value);
        /* eightkHz_filter limits only the two upper pole endpoints.
         * It does not compress the vowel space or the interpolated curve. */
        if(m->sample_rate==8000) {
            if(index==2 && value>3600)value=3600;
            if(index==3 && value>3900)value=3900;
        }
        return value;
    }
    if(stream>=RELOQ_TARGET_B1 && stream<=RELOQ_TARGET_B5) {
        unsigned index=(unsigned)(stream-RELOQ_TARGET_B1);
        if(female) {
            const int base[3][5]={{300,200,150,300,500},{200,100,350,500,500},{350,100,350,1000,1000}};
            const int *v=base[m->head_size<30?1:b==100?2:0];
            if(index==0) {
                if(value==108)value=v[0]*9/10;
                else if(value==120)value=small && (vowel(p) ||
                    (p==P(h) && n && vowel(n->right_phone)) ||
                    /* Closing sonorants keep the vowel's first-pole damping.
                     * Independent onsets still use the child base. */
                    ((p==P(l)||p==P(j)||p==P(w)) && n &&
                     n->same_syllable_left && vowel(n->left_phone)))?300:v[0];
                else if(value>=220 && value<=320)value=v[0]+value-70;
            } else if(index==1) {
                if(nasal(p))value=value*v[1]/100;
                else if(value==100 || value==200)value+=v[1]-100;
            } else if(index==2) {
                if(v[2]==150 && (value==80 || value==120))value*=2;
                else if(value==105)value=(int)(v[2]*0.7);
                else if(value==150 || value==300)value+=v[2]-150;
            } else if(index==3)value+=v[3]-330;
            else value+=v[4]-260;
        }
        /* At 11 kHz the small-head filter damps the unused upper pole. */
        if(index==4 && !female && m->head_size<=40 )value*=8;
        if(index==3 && female && m->head_size<=40)value=(int)(value*1.6);
        if(index==3 && female && value>3000)value=3000;
        if(m->sample_rate==8000) {
            if(index==3)value=female?1000:(value*3/2>3000?3000:value*3/2);
            if(index==2 && female) {
                int factor=m->head_size<30?8:m->head_size<=40?4:2;
                int ceiling=m->head_size<30?3000:m->head_size<=40?1000:500;
                value*=factor;if(value>ceiling)value=ceiling;
            }
        }
        return value;
    }
    if(stream==RELOQ_TARGET_FNP)return female&&nasal(p)&&value==200?250:value;
    if(stream==RELOQ_TARGET_FNZ)return female&&(value==350||value==400)?value+100:value;
    if(stream==RELOQ_TARGET_DI)return value?value:4*m->roughness/10;
    if(stream==RELOQ_TARGET_OQ) {
        int oq=female?(m->head_size<=40?85:100):56+43*b/100;
        if(female && m->sample_rate==8000)oq-=15;
        return value==18?oq/3:value==56?oq:value;
    }
    if(stream==RELOQ_TARGET_AV) {
        if(b==100)return 0;
        if(m->sample_rate==8000 && measured==3 && value==20 &&
           (p==P(b)||p==P(d)||p==P(g)||p==P(v)||p==P(z)||p==P(Z)))return 35;
        /* ger_ph_j lowers its own voiced source for the child tract.
         * This is independent of the continuously controlled breath mix. */
        if(small && p==P(j) && value>0 &&
           !(n->same_syllable_left && vowel(n->left_phone)))value-=9;
        return value;
    }
    if(stream==RELOQ_TARGET_TL) {
        if(value && (p!=P(z) || (!b && !small)))return value;
        if(!n || p==P(NONE))return value;
        if(!vowel(p) && !nasal(p) && p!=P(l) && p!=P(r) && p!=P(v) &&
           p!=P(z) && p!=P(Z) && p!=P(j) && p!=P(w) && p!=P(h))return value;
        if(!female && nasal(p) && n->right_phone!=P(NONE))return value;
        int result=(p==P(v)||p==P(z)||p==P(Z))?excitation_tilt:tilt;
        if(p==P(r) && n->same_syllable_left && !vowel(n->left_phone))
            result=excitation_tilt;
        if(female && p==P(h) && n)p=n->right_phone;
        if(female) {
            if(p==P(m))result=result>=5?result-5:0;
            else if(b<100 && vowel(p)) {
                if(back(p)||p==P(Q_oj)) {if(result>=2)--result;}
                else if(p!=P(AT))++result;
                if(!n->stress && result>0)++result;
            }
        }
        return result;
    }
    if(stream==RELOQ_TARGET_AH && value) {
        if(value==35 && !female && b && n && n->right_phone==P(NONE) &&
           (vowel(p)||p==P(l)||p==P(r)||p==P(j)||p==P(w)))
            return 30+(34+20*b/100)/10;
        if(value==34) {
            int ah=female?vowel_breath(m,n,right_node,p):34+20*b/100;
            return ah;
        }
        /* do_release uses the current female vowel source, and the base
         * breath source for a continuing glide. Conversion truncates after
         * adding 30; even a negative AH control remains meaningful. */
        if(value==35 && female && b && n && n->right_phone==P(NONE) &&
           (vowel(p)||p==P(l)||p==P(r)||p==P(j)||p==P(w))) {
            int ah=p==P(Q_aj)||p==P(Q_aw)||p==P(Q_oj)?
                (m->sample_rate==8000?(50*b/100)*8/10:50*b/100):
                vowel_breath(m,n,right_node,p);
            return (int)(30+ah/10.0);
        }
        if(tilt && n) {
            /* Oral aspiration depends on the following vowel's place;
             * the voiced vowel source is treated separately above. */
            Phone right=n->right_phone;
            if(p==P(h)) {
                /* The original h source applies a quarter of the tilt;
                 * it inherits the following vowel's tract, not its AH law. */
                value=(int)(value-tilt*.25);
            } else if(p==P(p)||p==P(t)||p==P(k)||p==P(b)||p==P(d)||p==P(g)||p==P(n)||
                      (p==P(m) && n->right_phone==P(NONE))) {
                int coefficient=front(right)?6:back(right)?3:5;
                if(right==P(u)||right==P(U))coefficient=0;
                value=(int)(value-excitation_tilt*coefficient/10.0);
            } else if((p==P(f)||p==P(s)||p==P(S)||p==P(X)) &&
                      (right==P(r)||right==P(l))) {
                /* A fricative's liquid-release aspiration uses half of
                 * excitation tilt. It is not the r constriction source. */
                value=(int)(value-excitation_tilt*.5);
            } else if(p==P(r)) {
                /* The rhotic source uses word tilt, including at 8 kHz;
                 * the 125-percent consonant-noise restoration does not
                 * apply to this aspiration. */
                if(value!=35)value-=tilt/4;
            } else if(value==27||value==52)value=(int)(value-excitation_tilt/5.0);
        }
        return value<0?0:value;
    }
    /* Rhotic frication, like aspiration, uses a quarter of word tilt. */
    if(stream==RELOQ_TARGET_AF && p==P(r) && value>0)
        return value-tilt/4;
    /* The dental closure inside /ts/ and /tS/ keeps its burst source.
     * Breathiness shapes the following fricative independently; applying
     * the ordinary stop reduction here attenuates this burst twice. */
    if(stream==RELOQ_TARGET_AF && p==P(t) && n &&
       (n->affricate_role==3 || n->affricate_role==5))return value;
    if(stream==RELOQ_TARGET_AF && value>=65 &&
       (b || (small && p!=P(t)&&p!=P(d))) && p!=P(z)&&p!=P(v)&&p!=P(Z))
        return (int)(value-excitation_tilt*(value>=71?3:2)/10.0);
    return value;
}

/* The wideband voiced extension retains its added oral poles. The core's
 * separate AF profile adds OpenEVV spectral shaping to the 16-kHz bank;
 * these voiced poles do not also become parallel frication channels.
 * The public SO language interface remains limited to 8/11.025 kHz. */
void reloq_german_voice_model_frame(void *context,float c[62])
{
    const struct reloq_german_voice_model *m=context;
    unsigned count=reloq_german_voice_model_formants(m);
    if(m->sample_rate>=16000) {
        const float base[3]={5000,6300,7500};
        const float width[3]={1000,1250,1500};
        float scale=((250-m->head_size)/2)/100.0f*(m->gender?4600.0f/3900.0f:1);
        /* The added poles describe a broad upper tract envelope. Keeping
         * their old 500-Hz widths while shifting their centers created
         * narrow high-Q peaks, especially for smaller/female anatomy. */
        for(unsigned i=5;i<count;++i) {
            c[11+2*i]=base[i-5]*scale;
            c[12+2*i]=width[i-5]*scale;
        }
        for(unsigned i=5;i<count;++i)c[35+i]=0;
    }

    /* Only the requested extension needs a Nyquist safety margin.
     * Legacy 8 kHz limits are applied to endpoints by eightkHz_filter. */
    if(m->sample_rate<16000)return;
    for(unsigned i=0;i<count;++i) {
        unsigned f=i?11+2*i:9,b=f+1;
        float limit=m->sample_rate*.5f-fmaxf(100,c[b]*.5f);
        if(c[f]>limit)c[f]=limit;
    }
}
