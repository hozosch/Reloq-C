#include "reloq_inline_tags.h"
#include "reloq_german_generic_melody.h"
#include <limits.h>
#include <math.h>

/* Independent realization of the observed pitch commands. Accent selection,
 * consonant onset, voiced rime and postnuclear tail have separate clocks.
 * See behavior-0.2.40: commands were observed before acoustic realization. */
struct melody {
    struct reloq_target_builder *builder;
    double base, window, decline, pitch, time;
    int flat;
    int64_t previous;
    int64_t restart_tick;
    const struct reloq_inline_word *controls;
    const struct reloq_german_generic_melody_syllable *syllables;
    size_t count;
};
static int valid_mark(enum reloq_german_terminal_mark mark)
{
    return mark>=RELOQ_DE_TERMINAL_IMPLICIT &&
           mark<=RELOQ_DE_TERMINAL_CONTROL;
}
static int64_t fraction(int64_t a,int64_t b,int percent)
{
    const int64_t d=b-a;
    return a+d/100*percent+(d%100)*percent/100;
}
static int point(struct melody *m,int64_t tick,int percent,int syllable)
{
    /* The native stream rejects a target at or before an existing target.
     * Moving rejected commands forward would invent another pitch gesture. */
    if (tick<0 || tick<=m->previous) return 0;
    if(m->controls && syllable>=0 && (size_t)syllable<m->count) {
        int f=m->controls[m->syllables[syllable].word_index].inflection;
        m->window=1189.15*m->pitch*f/100.0;m->base=1189.15*m->pitch-m->window*0.9;
        m->decline=log(trunc(m->base)/fmax(10.0,trunc(m->base)-3.0*m->count))/m->count;
        m->flat=f==0;
    }
    double value=m->flat?1189.15*m->pitch:
        m->base*exp(-m->decline*syllable)+m->window*percent/100.0;
    if (value<trunc(594.575*m->pitch)) value=trunc(594.575*m->pitch);
    value=trunc(value);
    const double at=tick*m->time;
    if (!isfinite(value) || value<0 || value>INT32_MAX ||
        !isfinite(at) || at<0 || at>(double)INT64_MAX-1) return -1;
    const struct reloq_target_event e={RELOQ_TARGET_F0,0,(int32_t)lround(value)};
    if (m->previous<0 && m->restart_tick>=0 && m->restart_tick<tick &&
        reloq_target_builder_append(m->builder,
            (int64_t)llround(m->restart_tick*m->time),&e,1)) return -1;
    if (reloq_target_builder_append(m->builder,(int64_t)llround(at),&e,1)) return -1;
    m->previous=tick;
    return 0;
}
static int weak(const struct reloq_german_generic_melody_syllable *s)
{
    int control=s->prosody_control&~RELOQ_DE_MELODY_TAIL;
    return control==RELOQ_DE_WORD_CONTROL_0 || control==RELOQ_DE_WORD_CONTROL_00;
}
static int64_t onset_end(const struct reloq_german_generic_melody_syllable *s)
{
    if(s->onset_begin_tick<s->start_tick && s->onset_end_tick>s->onset_begin_tick)
        return s->onset_end_tick;
    if (s->first_phone==RELOQ_DE_PHONE_h)
        return s->vowel_core_end_tick>s->nucleus_tick?
            s->vowel_core_end_tick:s->closure_tick;
    if (s->nucleus_tick==s->start_tick) return s->closure_tick;
    return s->onset_end_tick>s->start_tick?s->onset_end_tick:s->nucleus_tick;
}
static int onset(struct melody *m,
    const struct reloq_german_generic_melody_syllable *s,
    int time,int pitch,int syllable)
{
    return point(m,fraction(s->onset_begin_tick,onset_end(s),time),pitch,
        syllable-(s->first_phone==RELOQ_DE_PHONE_h || s->onset_begin_tick<s->start_tick));
}
static int rime(struct melody *m,
    const struct reloq_german_generic_melody_syllable *s,
    int time,int pitch,int syllable)
{
    return point(m,fraction(s->nucleus_tick,s->closure_tick,time),pitch,syllable);
}
static int emit_phrase(
    const struct reloq_german_generic_melody_syllable *s,
    size_t first,size_t after,enum reloq_german_terminal_mark mark,
    int inflection,double pitch_scale,double time_scale,int restart,int continuing_comma,int following_tail,
    const struct reloq_inline_word *controls,
    struct reloq_target_builder *builder)
{
    mark=reloq_german_terminal_contour_mark(mark);
    const int question=mark==RELOQ_DE_TERMINAL_QUESTION;
    const size_t last=after-1;
    const double window=1189.15*pitch_scale*inflection/100.0;
    const double base=1189.15*pitch_scale-window*0.9;
    const double fall=3.0*(after-first);
    const double final_base=fmax(10.0,trunc(base)-fall);
    struct melody m={builder,base,window,
        log(trunc(base)/final_base)/(after-first),
        pitch_scale,time_scale,inflection==0,-1,restart?s[first].start_tick:-1,
        controls,s+first,after-first};
    if(s[first].prosody_control&RELOQ_DE_MELODY_TAIL) {
        /* A final reporting/thought clause continues below the main
         * nucleus. Its lexical stress stays intact, without a new high
         * phrase accent. Both targets share the ordinary inflection law. */
        if(onset(&m,s+first,5,35,0) ||
           rime(&m,s+last,99,-35,(int)(last-first)))return -1;
        return 0;
    }
    size_t nucleus=after;
    /* Original rising questions retain their selected nucleus. In ordinary
     * text this is the final focus; explicit earlier focus still governs
     * the following words instead of inventing a later accent. */
    for (size_t i=first;i<after;++i)
        if (s[i].stress==1 && (s[i].prosody_control==RELOQ_DE_WORD_CONTROL_2 ||
            s[i].prosody_control==RELOQ_DE_WORD_CONTROL_3 ||
            s[i].prosody_control==RELOQ_DE_WORD_CONTROL_4))
            nucleus=i;
    if (nucleus==after)
        for (size_t i=first;i<after;++i)
            if (s[i].stress==1 && !weak(s+i)) nucleus=i;
    if(nucleus==after && question)nucleus=last;
    if (nucleus==after && continuing_comma &&
        mark!=RELOQ_DE_TERMINAL_QUESTION && mark!=RELOQ_DE_TERMINAL_EXCLAMATION) {
        /* Native 0/00 supplied no accent for this continuation. Do not
           manufacture a nucleus on its first weak word (e.g. zum Beispiel).
           Keep the established low continuation and ordinary inflection law. */
        if(onset(&m,s+first,5,35,0))return -1;
        /* A deaccented noun still has a lexical stress anchor. Its small
           fall precedes the low boundary; this is not a new nucleus. */
        size_t lexical=after;
        for(size_t i=first;i<after;++i)if(s[i].stress==1)lexical=i;
        if(lexical<after&&rime(&m,s+lexical,55,-20,(int)(lexical-first)))return -1;
        if(rime(&m,s+last,99,-35,(int)(last-first)))return -1;
        return 0;
    }
    if (nucleus==after) nucleus=first;
    const int exclaim=mark==RELOQ_DE_TERMINAL_EXCLAMATION;
    const int comma=mark==RELOQ_DE_TERMINAL_COMMA || mark==RELOQ_DE_TERMINAL_SEMICOLON;
    const int suspended=mark==RELOQ_DE_TERMINAL_ELLIPSIS;
    const int first_word=s[nucleus].word_index==s[first].word_index;
    const int last_word=s[nucleus].word_index==s[last].word_index;
    const size_t tail=last-nucleus;
    /* A secondary-stressed prehead after a continuing comma receives its
     * own start target. Initial utterances and unaccented syllables do not.
     * Native unmarked lexical accents remain ordinary prehead accents in a
     * question. The explicit 0/00 controls still suppress them. */
    if (((continuing_comma && s[first].stress==2) || weak(s+first)) &&
        onset(&m,s+first,5,70,0)) return -1;
    size_t previous_accent=after;
    size_t final_prehead=after;
    if(question)for(size_t i=first;i<nucleus;++i)
        if(s[i].stress==1 && !weak(s+i))final_prehead=i;
    int last_accent_return=0;
    for (size_t i=first;i<nucleus;++i) {
        if (s[i].stress!=1 || weak(s+i)) continue;
        const int adjacent=previous_accent<after && previous_accent+1==i;
        const int opening=i==first && s[i].onset_begin_tick==s[i].start_tick;
        const int inherited=s[i].onset_begin_tick<s[i].start_tick;
        /* Adjacent accents share the preceding onset range. A vowel
         * range is used near its end; a consonantal coda keeps the
         * midpoint. Using the midpoint for a vowel can put this target
         * before the previous peak, causing the scheduler to reject it. */
        const int t=opening?5:adjacent?(inherited&&s[i].inherited_vowel?99:50):20;
        const int p=opening?80:adjacent?75:50;
        if (onset(&m,s+i,t,p,(int)(i-first)) ||
            rime(&m,s+i,75,100,(int)(i-first))) return -1;
        /* The falling return can lie in the coda of this syllable or
         * the first eligible phone of its weak successor. Nasal and h
         * heads are skipped by the range builder, independently of stress. */
        const int coda=s[i].coda_body_end_tick>s[i].coda_begin_tick &&
            i+1<nucleus && (s[i+1].stress!=1 || weak(s+i+1));
        const int successor=i+1<nucleus && s[i+1].word_index==s[i].word_index &&
            s[i+1].stress!=1 && (!exclaim || i+2<nucleus);
        last_accent_return=0;
        /* The original rising contour omits the falling return after its
         * final prehead accent. Earlier returns remain: suppressing all
         * unmarked accents, or adding this last fall, changes its onset. */
        if ((coda || successor) && !(question && i==final_prehead)) {
            const struct reloq_german_generic_melody_syllable *q=s+i+(coda?0:1);
            const int64_t a=coda?q->coda_begin_tick:q->return_begin_tick;
            const int64_t b=coda?q->coda_body_end_tick:q->return_end_tick;
            if (b>a) {
                if (point(&m,fraction(a,b,80),50,(int)(i+(coda?0:1)-first))) return -1;
                last_accent_return=1;
            }
        }
        previous_accent=i;
    }
    const struct reloq_german_generic_melody_syllable *n=s+nucleus;
    const int vowel_initial=n->nucleus_tick==n->start_tick;
    int peak=first_word?(last_word?35:60):last_word?(tail?50:15):40;
    int height=120;
    if(comma && following_tail)height=132;
    if (question) {peak=first_word?(last_word?60:15):20;height=25;}
    else if (exclaim) {peak=tail?50:15;height=130;}
    else if (suspended) {peak=tail>1?90:tail?50:10;height=(vowel_initial || n->first_phone==RELOQ_DE_PHONE_h) && !tail?96:120;}
    /* Heavy focus uses the native nucleus window; its surrounding lexical
     * accents were already selected by the complete sentence analysis. */
    if(n->prosody_control==RELOQ_DE_WORD_CONTROL_3)height=height*120/100;
    else if(n->prosody_control==RELOQ_DE_WORD_CONTROL_4)height=height*110/100;
    const int inherited=n->onset_begin_tick<n->start_tick;
    if (onset(&m,n,inherited?(exclaim || n->preceding_stress?10:50):vowel_initial?2:10,
        inherited?(exclaim?(n->preceding_stress?40:15):50):vowel_initial?((question||exclaim)?50:98):(exclaim?(first_word || (previous_accent<after && !last_accent_return)?40:15):50),
        (int)(nucleus-first)) ||
        rime(&m,n,peak,height,(int)(nucleus-first))) return -1;
    if (tail>1 && !suspended) {
        int time=question?7:exclaim?90:50;
        size_t word_first=nucleus;
        while (word_first>first && s[word_first-1].word_index==n->word_index) --word_first;
        int level=question?70:comma?25:after-word_first>3?35:10;
        if (rime(&m,s+nucleus+1,time,level,(int)(nucleus+1-first))) return -1;
    }
    int cadence=70,level=exclaim?-10:comma?20:-20,finish=comma?0:-35;
    size_t cadence_syllable=last;
    if (question) {cadence=last_word?(tail?70:90):s[last].coda_body_end_tick?55:85;level=100;finish=150;}
    else if (suspended) {
        cadence=tail>1?95:tail?5:70;level=25;finish=35;
        if (tail>1) cadence_syllable=nucleus+1;
    } else if (!last_word) {
        cadence=s[last].coda_body_end_tick?55:85;
        if (comma) level=25;
    }
    if (rime(&m,s+cadence_syllable,cadence,level,(int)(cadence_syllable-first)) ||
        rime(&m,s+last,99,finish,(int)(last-first))) return -1;
    return 0;
}

int reloq_german_generic_melody_append_controls(
    const struct reloq_german_generic_melody_syllable *syllables,
    size_t syllable_count,
    const enum reloq_german_terminal_mark *marks, size_t mark_count,
    int inflection, double pitch_scale, double time_scale,
    const struct reloq_inline_word *controls,
    struct reloq_target_builder *builder)
{
    if (!syllables || !syllable_count || !builder || inflection < 0 ||
        inflection > 100 || !isfinite(pitch_scale) || pitch_scale <= 0 ||
        !isfinite(time_scale) || time_scale <= 0 ||
        builder->stream[RELOQ_TARGET_F0].count ||
        (marks && !mark_count)) return -1;
    for (size_t i = 0; i < syllable_count; ++i) {
        const struct reloq_german_generic_melody_syllable *s = syllables + i;
        if (s->start_tick < 0 || s->nucleus_tick < s->start_tick ||
            s->closure_tick <= s->nucleus_tick ||
            s->end_tick < s->closure_tick || s->stress > 2 ||
            (s->prosody_control&~RELOQ_DE_MELODY_TAIL) > RELOQ_DE_WORD_CONTROL_4 ||
            (marks && s->word_index >= mark_count) ||
            (i && (s->start_tick < syllables[i - 1].end_tick ||
                   s->word_index < syllables[i - 1].word_index))) return -1;
    }
    if (marks)
        for (size_t w = 0; w < mark_count; ++w)
            if (!valid_mark(marks[w])) return -1;
    size_t first = 0;
    for (size_t i = 0; i < syllable_count; ++i) {
        const int end_word = i + 1 == syllable_count ||
            syllables[i + 1].word_index != syllables[i].word_index;
        enum reloq_german_terminal_mark mark = marks ?
            marks[syllables[i].word_index] : RELOQ_DE_TERMINAL_IMPLICIT;
        if (end_word && (i + 1 == syllable_count ||
                         (mark != RELOQ_DE_TERMINAL_IMPLICIT && mark!=RELOQ_DE_TERMINAL_CONTROL))) {
            int following_tail=i+1<syllable_count &&
                (syllables[i+1].prosody_control&RELOQ_DE_MELODY_TAIL);
            if(!following_tail&&mark==RELOQ_DE_TERMINAL_COMMA&&i+1<syllable_count) {
                /* The native frontend marks a wholly deaccented final
                   addition with 0/00, including a written-out abbreviation. */
                following_tail=1;
                for(size_t j=i+1;j<syllable_count;++j) {
                    if(!weak(syllables+j)){following_tail=0;break;}
                    if(marks&&marks[syllables[j].word_index]!=RELOQ_DE_TERMINAL_IMPLICIT&&
                       marks[syllables[j].word_index]!=RELOQ_DE_TERMINAL_CONTROL&&
                       marks[syllables[j].word_index]!=RELOQ_DE_TERMINAL_PERIOD) {
                        following_tail=0;break;
                    }
                }
            }
            if (emit_phrase(syllables, first, i + 1, mark, inflection,
                            pitch_scale, time_scale,first && marks &&
                            !reloq_german_terminal_continues(marks[syllables[first-1].word_index]),
                            first && marks && marks[syllables[first-1].word_index]==RELOQ_DE_TERMINAL_COMMA,
                            following_tail,
                            controls,builder)) return -1;
            /* Each completed original phrase has its own sparse F0 stream.
             * Sustain its last target before restarting the following one:
             * a cross-phrase ramp would intrude into the audible release. */
            if (i+1<syllable_count && !reloq_german_terminal_continues(mark)) {
                const struct reloq_target_buffer *f0=builder->stream+RELOQ_TARGET_F0;
                const double scaled=syllables[i+1].start_tick*time_scale;
                if (!isfinite(scaled) || scaled>(double)INT64_MAX-1) return -1;
                const int64_t end=(int64_t)llround(scaled)-1;
                if (f0->count && end>f0->points[f0->count-1].tick) {
                    const struct reloq_target_event hold={RELOQ_TARGET_F0,0,
                        f0->points[f0->count-1].value};
                    if (reloq_target_builder_append(builder,end,&hold,1)) return -1;
                }
            }
            first = i + 1;
        }
    }
    return 0;
}

int reloq_german_generic_melody_append(
    const struct reloq_german_generic_melody_syllable *syllables,
    size_t syllable_count,
    const enum reloq_german_terminal_mark *marks, size_t mark_count,
    int inflection, double pitch_scale, double time_scale,
    struct reloq_target_builder *builder)
{
    return reloq_german_generic_melody_append_controls(syllables,syllable_count,
        marks,mark_count,inflection,pitch_scale,time_scale,NULL,builder);
}
