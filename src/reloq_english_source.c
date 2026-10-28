#include "reloq_english_source.h"

#include <limits.h>
#include <string.h>

/* Connected find_acoustic_nuc / do_son_amps / do_non_son_amps source
 * ownership. The amplitude/tract models remain separate. Full original
 * control flow and transparent stage captures: behavior-0.2.96. */
static int in(unsigned char c,const char *set)
{
    return c && strchr(set,c)!=NULL;
}
static int vocalic(unsigned char c)
{
    return in(c,"iIeEAXHxauUocRYWONM");
}
static int same_syllable(const struct reloq_english_source_phone *p,size_t right)
{
    return right && !p[right-1].pause && !p[right].word_start &&
        !p[right].syllable_start;
}
static int preceding_s_stop(const struct reloq_english_source_phone *p,size_t i)
{
    return i>1 && p[i-2].phone=='s' && !p[i-2].pause && !p[i-1].pause &&
        !p[i-1].word_start;
}

int reloq_english_source_boundary_contact(unsigned char stop,int boundary)
{
    return boundary && in(stop,"ptk") ? stop=='t'?20:30 : -1;
}

int reloq_english_stop_closure_av(unsigned char phone,unsigned char previous,
    unsigned char following)
{
    if(!in(phone,"bdg") || previous=='h' || following=='h')return -1;
    if(in(previous,"ptk?TDfvszSZCJ") || in(following,"TDfvszSZ") ||
       (phone=='d' && previous=='l'))return 0;
    return in(previous,"mnGNM")?35:20;
}

int reloq_english_source_window(int64_t contact,int64_t before,
    int64_t ticks,int64_t *first,int64_t *end)
{
    if(!first||!end||contact<0||before<0||before>contact||ticks<1)return -1;
    const int64_t begin=contact-before;
    if(begin>INT64_MAX-ticks)return -1;
    *first=begin+1;*end=begin+ticks;
    return 0;
}

int reloq_english_source_plan(const struct reloq_english_source_phone *p,size_t n,
    struct reloq_english_source_span *out)
{
    if(n && (!p||!out))return -1;
    for(size_t i=0;i<n;++i) {
        if(!p[i].phone || p[i].start<0 || p[i].contact<p[i].start ||
           p[i].end<p[i].contact || p[i].release<p[i].end ||
           p[i].contact==INT64_MAX || (vocalic(p[i].phone)&&p[i].end==p[i].contact) ||
           (i&&p[i].start<p[i-1].end))return -1;
    }
    for(size_t i=0;i<n;++i) {
        const int fresh=!i||p[i-1].pause;
        const unsigned char left=fresh?0:p[i-1].phone;
        const int stop=in(left,"ptk");
        const int unasp=stop && (p[i].word_start || preceding_s_stop(p,i));
        struct reloq_english_source_span s={.owner=i,.last=i,.first=p[i].contact,
            .end=p[i].end,.unaspirated_stop=(unsigned char)unasp};
        if(vocalic(p[i].phone)) {
            s.nucleus=1;
            s.early=(unsigned char)(!p[i].aspirated &&
                (!left || (!vocalic(left)&&!in(left,"lrwy")&&(!stop||unasp))));
            if(i+1<n && same_syllable(p,i+1) && in(p[i+1].phone,"lrwy")) {
                s.last=i+1;
                if(i+2<n && same_syllable(p,i+2) && in(p[i+2].phone,"lr"))s.last=i+2;
            }
            s.end=p[s.last].pause||p[s.last].before_h?p[s.last].end:p[s.last].release;
            const int64_t begin=s.early?p[i].start:p[i].contact;
            /* Validation above makes this scalar window infallible. */
            (void)reloq_english_source_window(p[i].contact,p[i].contact-begin,
                s.end-begin,&s.first,&s.end);
            out[i]=s;
            for(size_t j=i+1;j<=s.last;++j)out[j]=s;
            i=s.last;
            continue;
        }
        if(in(p[i].phone,"lrwymnG")) {
            /* Strong p/t/k onsets retain the separately planned devoicing.
             * Other liquids/glides and consonantal nasals are voiced. */
            s.voiced_sonorant=(unsigned char)(!p[i].aspirated &&
                (in(p[i].phone,"mnG") || !stop || unasp ||
                 p[i].word_start || p[i].stress!=1));
            if(in(p[i].phone,"lrwy") && !fresh && !vocalic(left) &&
               !in(left,"lrwy") && (!stop||unasp))s.first=p[i].start;
        }
        out[i]=s;
    }
    return 0;
}

int reloq_english_source_needs_plateau(unsigned char phone,
    const struct reloq_english_source_span *s,int positive,int zero,
    int next_silent,int previous_sonorant)
{
    if(!s || !s->voiced_sonorant || (positive&&!zero))return 0;
    /* A preceding nucleus can intentionally interpolate into a voiced
     * nasal/closure. Preserve that existing handoff if no zero interrupts it. */
    if(in(phone,"mnG") && !zero && !next_silent && previous_sonorant)return 0;
    return 1;
}
