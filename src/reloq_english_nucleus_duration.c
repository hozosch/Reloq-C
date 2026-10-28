#include "reloq_english_nucleus_duration.h"

#include <limits.h>
#include <math.h>
#include <string.h>
#include "reloq_english_nucleus_rates.inc"

int reloq_english_nucleus_rate_percent(int speed)
{
    if(speed<40||speed>250)return -1;
    return nucleus_rate_percent98[speed-40];
}

/* Authored from the connected find_eng_nucleus -> start_dur ->
 * distribute_nucdur control flow, not fitted word/contact/body trees.
 * See behavior-0.2.95 for complete listings and stage observations. */
static int vocalic(const struct reloq_english_nucleus_phone *p)
{
    return (p->phone && strchr("iIeEAXHxauUocR",p->phone)!=NULL) ||
        (p->syllabic && (p->phone=='m'||p->phone=='n'));
}

/* Internal branches of the vowel/sonorant transition block. These operate
 * on vowel quality and the destination gesture, without word identities. */
static int transition(unsigned char head,unsigned char next)
{
    if(next=='r')return head=='i'||head=='e'?65:head=='u'?50:
        head=='U'||head=='c'?40:30;
    if(next=='l')return head=='i'||head=='e'||head=='E'?60:
        head=='I'||head=='H'?80:head=='A'?55:head=='x'?75:
        strchr("uUoc",head)?50:30;
    if(next=='y')return strchr("iIeEA",head)?30:75;
    if(next=='w')return strchr("IEA",head)?85:head=='a'?50:30;
    return 0;
}

int reloq_english_nucleus_liquid_onset_ticks(unsigned char left,unsigned char head)
{
    if(!head||!strchr("iIeEAXHxauUocR",head))return -1;
    /* get_eng_son_trans_dur's vowel entrance branches, before rate
     * scaling: r distinguishes high front and rounded back targets;
     * l distinguishes the low/open and rounded target groups. */
    if(left=='r')return head=='i'||head=='e'?65:head=='o'||head=='c'?30:45;
    if(left=='l')return strchr("RUoc",head)?20:head=='a'||head=='u'?35:65;
    return -1;
}

int reloq_english_nucleus_exit_ticks(unsigned char member,unsigned char following)
{
    if(!following||!strchr("pbtdkgTDfvszSZCJmnG",following))return -1;
    /* get_eng_son_trans_dur: destination manner/place precedes the
     * otherwise shared l/r/w/y connection. This is independent of the
     * preceding vowel and of the number of nucleus members. */
    if(member=='y')return 15;
    if(member=='l')return 25;
    if(member=='w')return following=='n'?20:
        strchr("tdszSZCJ",following)?40:15;
    if(member=='r')return following=='n'?15:following=='z'?40:
        following=='t'||following=='C'?50:25;
    return -1;
}

int reloq_english_nucleus_hiatus_ticks(unsigned char left,unsigned char right,
    int32_t *outgoing,int32_t *incoming)
{
    if(!outgoing||!incoming)return -1;
    if(!right||!strchr("iIeEAXHxauUocR",right))return 0;
    int total;
    /* get_eng_vow_trans_dur and the sonorant entry block select the full
     * connection before scale_transdur divides ownership at a word break.
     * The odd tick belongs to the incoming part: 65 becomes 32 plus 33. */
    if(left=='r'||left=='l')total=reloq_english_nucleus_liquid_onset_ticks(left,right);
    else if(left=='y')total=50;
    else if(left=='w')total=right=='e'?60:40;
    else if(left&&strchr("iIeEAXHxauUocR",left))
        total=left==right||left=='X'||left=='x'?0:65;
    else return 0;
    *outgoing=total/2;*incoming=total-total/2;return 1;
}

int reloq_english_nucleus_base(const struct reloq_english_nucleus_phone *p,
    size_t count,struct reloq_english_nucleus *out)
{
    if(!p||!out||!count||count>3)return -1;
    if(!vocalic(p))return 0;
    struct reloq_english_nucleus n={.count=count};
    const unsigned char h=p[0].phone;
    if(count==1) {
        if(h=='n')n.base_ticks=!p[0].word_final&&p[0].following=='n'?40:100;
        else if(h=='m')n.base_ticks=100;
        else if(h=='x'||h=='X')n.base_ticks=70;
        else if(h=='I'||h=='U')n.base_ticks=75;
        else if(h=='E'||h=='H')n.base_ticks=85;
        else if(h=='R'&&!p[0].quantity)n.base_ticks=100;
        else if(h=='i'||h=='u')n.base_ticks=100;
        else if(h=='e'||h=='o'||h=='R')n.base_ticks=120;
        else n.base_ticks=130;
    } else if(count==3) {
        if(!p[1].phone||!strchr("lrwy",p[1].phone)||
           !(p[2].phone=='l'||p[2].phone=='r'))return 0;
        n.base_ticks=180;n.closing_ticks[0]=10;n.closing_ticks[1]=40;
    } else if(p[1].phone=='y') {
        n.base_ticks=130;n.closing_ticks[0]=10;
    } else if(p[1].phone=='w') {
        n.base_ticks=140;n.closing_ticks[0]=20;
    } else if(p[1].phone=='l') {
        n.closing_ticks[0]=40;
        if(h=='I'||h=='U')n.base_ticks=125;
        else if(h=='E'||h=='X'||h=='H'||h=='x')n.base_ticks=130;
        else if(h=='e'||h=='c')n.base_ticks=160;
        else if(h=='A')n.base_ticks=165;
        else n.base_ticks=150;
    } else if(p[1].phone=='r') {
        n.closing_ticks[0]=30;
        n.base_ticks=h=='e'?160:(h=='u'||h=='U')?130:140;
    } else return 0;
    if(count>1)n.transition_ticks[0]=transition(h,p[1].phone);
    if(count>1&&(count>2||p[0].closed)&&h=='x'&&p[1].phone=='l')n.transition_ticks[0]=30;
    if(count>2) {
        const unsigned char middle=p[1].phone;
        n.transition_ticks[1]=middle=='y'?50:middle=='r'?45:
            middle=='l'?30:p[2].phone=='r'?30:h=='a'?50:15;
    }
    *out=n;return 1;
}

int reloq_english_nucleus_final_percent(
    const struct reloq_english_nucleus_phone *p,size_t count)
{
    struct reloq_english_nucleus n;
    if(count<2||reloq_english_nucleus_base(p,count,&n)!=1)return 0;
    /* pf_word_final_syll's primary-stressed terminal branch: complete
     * nucleus first, then its head quality, then the lengthening factor. */
    if(count==3)return 170;
    if(p[1].phone=='r')return strchr("XHU",p[0].phone)?170:180;
    if(p[1].phone=='l')return p[0].phone=='a'||p[0].phone=='c'?200:190;
    return strchr("IEXHxU",p[0].phone)?250:200;
}

int reloq_english_nucleus_terminal_budget(
    const struct reloq_english_nucleus_phone *p,size_t count,
    unsigned char coda,double *ticks)
{
    if(!p||!ticks)return -1;
    struct reloq_english_nucleus n;
    int found=reloq_english_nucleus_base(p,count,&n);
    if(found!=1)return found;
    if(!strchr("iIeEAXHxauUocR",p[0].phone))return 0;
    if(coda && !strchr("pbtdkgTDfvszSZCJmnG",coda))return 0;
    const int lax=strchr("IEXHxU",p[0].phone)!=NULL;
    int tenths;
    /* pf_word_final_syll's unvoiced-coda path precedes its liquid/glide
     * tests. A closing sonorant thus does not retain the open-syllable
     * percentage in front of a voiceless closure or fricative. */
    if(coda && strchr("ptkTfsSC",coda))tenths=count==1 && lax?18:16;
    else if(coda=='J' && count==2 && p[1].phone=='l')tenths=17;
    else if(count>1 && (p[count-1].phone=='l'||p[count-1].phone=='r'))
        tenths=reloq_english_nucleus_final_percent(p,count)/10;
    else if(!coda)tenths=lax?25:20;
    else if(lax)tenths=25;
    else tenths=count==1 && strchr("iu",p[0].phone)?23:21;
    double value=n.base_ticks*(double)tenths/10.0;
    /* The subsequent dur_adjustments branch lengthens voiced fricative
     * connections independently of the phrase-final percentage. */
    if(coda && strchr("DvzZ",coda))value=value*11.0/10.0;
    *ticks=value;return 1;
}

int reloq_english_nucleus_medial_budget(
    const struct reloq_english_nucleus_phone *p,size_t count,
    unsigned char coda,unsigned char next_word_onset,double *ticks)
{
    if(!p||!ticks)return -1;
    struct reloq_english_nucleus n;
    int found=reloq_english_nucleus_base(p,count,&n);
    if(found!=1)return found;
    if(!strchr("iIeEAXHxauUocR",p[0].phone) ||
       (coda&&!strchr("pbtdkgTDfvszSZCJmnG",coda)))return 0;
    int tenths=10;
    /* phrase_medial_dur -> pre_voice_lengthening ->
     * word_final_lengthening: voicing is tested before nucleus quality;
     * a high long monophthong and A+nasal have their own voiced branches. */
    if(!coda)tenths=count>1?14:p[0].phone=='c'?12:
        strchr("ieouR",p[0].phone)?14:10;
    else if(!strchr("ptkTfsSC",coda))tenths=
        count==1&&strchr("iu",p[0].phone)&&!strchr("DvzZ",coda)?17:
        count==1&&p[0].phone=='A'&&strchr("mnG",coda)?14:15;
    double value=n.base_ticks*(double)tenths/10.0;
    if(!coda&&next_word_onset&&strchr("bdgDvzZJmnGlrwy",next_word_onset)) {
        if(count==1&&strchr("IEXHxU",p[0].phone))value=n.base_ticks*12.0/10.0;
        else value=value*11.0/10.0;
    }
    if(coda&&strchr("DvzZ",coda))value=value*11.0/10.0;
    *ticks=value;return 1;
}

int reloq_english_nucleus_find(const struct reloq_english_nucleus_phone *p,
    size_t count,struct reloq_english_nucleus *out)
{
    if((!p&&count)||!out)return -1;
    size_t first=0;
    while(first<count&&!vocalic(p+first))++first;
    if(first==count)return 0;
    size_t end=first+1;
    if(end<count&&p[end].phone&&strchr("lrwy",p[end].phone))++end;
    if(end<count&&(p[end].phone=='l'||p[end].phone=='r'))++end;
    struct reloq_english_nucleus_phone members[3];
    memcpy(members,p+first,(end-first)*sizeof *members);
    members[0].closed=end<count;
    if(end<count)members[0].following=p[end].phone;
    members[0].word_final=members[0].word_final&&!members[0].closed;
    struct reloq_english_nucleus n;
    int result=reloq_english_nucleus_base(members,end-first,&n);
    if(result!=1)return result;
    n.first=first;*out=n;return 1;
}

int reloq_english_nucleus_distribute(
    const struct reloq_english_nucleus_interval *in,size_t count,
    double budget,int32_t minimum,int32_t *fixed,int32_t *free_ticks)
{
    if((count&&(!in||!fixed))||!free_ticks||count>INT16_MAX||
       !isfinite(budget)||budget<0||budget>INT16_MAX||minimum<0||minimum>INT16_MAX)return -1;
    int32_t total=0;
    for(size_t i=0;i<count;++i) {
        if(in[i].ticks<0||in[i].ticks>INT16_MAX||in[i].minimum<0||
           in[i].minimum>INT16_MAX||total>INT16_MAX-in[i].ticks)return -1;
        total+=in[i].ticks;
    }
    double first=1,second=1;
    int32_t free_part;
    if(budget<total) {
        const double percent=budget*100.0/total;
        const int32_t target=(int32_t)(total*percent/100.0);
        first=(double)target/total;
        int32_t rounded=0;
        for(size_t i=0;i<count;++i)rounded+=(int32_t)(in[i].ticks*first+.5);
        int32_t remainder=rounded-minimum;
        if(remainder<0)remainder=0;
        second=rounded?(double)remainder/rounded:0;
        free_part=minimum;
    } else {
        free_part=(int32_t)(budget-total);
        if(free_part<minimum)free_part=minimum;
    }
    for(size_t i=0;i<count;++i) {
        int32_t t=(int32_t)(in[i].ticks*first+.5);
        t=(int32_t)(t*second+.5);
        fixed[i]=t<in[i].minimum?in[i].minimum:t;
    }
    *free_ticks=free_part;return 0;
}
