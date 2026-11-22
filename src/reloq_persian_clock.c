/* Safe bridge from Persian linguistic events to the current native clock.
 * No phone is retimed and no intonation is selected here. */
#include "reloq_persian_clock.h"
#include "reloq_persian_phones.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int vowel(uint32_t phone)
{
    return (phone>=RELOQ_FA_PHONE_AE && phone<=RELOQ_FA_PHONE_U) ||
           phone==RELOQ_FA_PHONE_FRONT_ROUNDED_O || phone==RELOQ_FA_PHONE_FRONT_ROUNDED_U;
}

static int contained(const struct reloq_fa_syllable *phone,const struct reloq_fa_syllable *syllable)
{return syllable->from<=phone->from && phone->to<=syllable->to;}

/* Private slots of the existing scoped backend, kept out of the Persian
 * linguistic plan. CH and J are handled as two acoustic components below;
 * H is a feature on the following segment and has no separate phone clock. */
static uint32_t provider_slot(uint32_t phone)
{
    static const uint8_t slots[]={0,31,34,29,27,33,32,1,4,2,5,3,6,3,3,
                                  7,8,9,10,11,12,18,0,0,0,16,17,21,19,22};
    return phone<sizeof slots?slots[phone]:0;
}

int reloq_fa_align_nucleus_clocks(const struct reloq_fa_plan_event *events,size_t event_count,
    const struct reloq_fa_syllable *syllables,size_t syllable_count,
    const struct reloq_fa_syllable *phones,size_t phone_count,
    const uint32_t *provider_slots,size_t slot_count,
    struct reloq_fa_nucleus_clock *output,size_t capacity,size_t *written)
{
    if(written)*written=0;
    if(!written || (event_count && !events) || (syllable_count && !syllables) ||
       (phone_count && !phones) || (slot_count && !provider_slots) ||
       (!output && capacity) ||
       syllable_count>SIZE_MAX/sizeof(struct reloq_fa_nucleus_clock))return -1;
    if(!event_count || !syllable_count || !phone_count || slot_count!=phone_count)return 1;
    for(size_t i=0;i<syllable_count;++i)
        if(syllables[i].from<0 || syllables[i].to<=syllables[i].from ||
           (i && syllables[i].from<syllables[i-1].to))return 1;
    for(size_t i=0;i<phone_count;++i)
        if(phones[i].from<0 || phones[i].to<=phones[i].from ||
           (i && phones[i].from<phones[i-1].from))return 1;
    struct reloq_fa_nucleus_clock *result=calloc(syllable_count,sizeof *result);
    if(!result)return -3;
    size_t s=0,p=0;uint32_t word=0,previous=UINT32_MAX,stress=0;
    int in_word=0,weak=0,next_weak=0,has_nucleus=0,have_word=0,ended=0,status=0,pending_j=0;
    for(size_t i=0;i<event_count && !status;++i) {
        const struct reloq_fa_plan_event *e=events+i;
        switch(e->kind) {
        case RELOQ_FA_PLAN_WEAK_WORD:
            if(in_word || ended || next_weak){status=1;break;}
            next_weak=1;break;
        case RELOQ_FA_PLAN_WORD_BEGIN:
            if(in_word || ended || word==UINT32_MAX){status=1;break;}
            if(have_word)++word;
            have_word=1;in_word=1;weak=next_weak;next_weak=0;
            previous=UINT32_MAX;has_nucleus=0;break;
        case RELOQ_FA_PLAN_PHONE: {
            if(!in_word || !e->value || e->value>RELOQ_FA_PHONE_Y ||
               e->stress>2 ||
               (e->flags & ~(RELOQ_FA_PHONE_LONG|RELOQ_FA_PLAN_SHORT_NUCLEUS|
                             RELOQ_FA_PLAN_COMPACT_HIATUS)) ||
               ((e->flags & RELOQ_FA_PHONE_LONG) && !vowel(e->value))){status=1;break;}
            if(e->syllable!=previous) {
                if((previous==UINT32_MAX && e->syllable!=0) ||
                   (previous!=UINT32_MAX && (e->syllable!=previous+1 || !has_nucleus)) ||
                   s>=syllable_count){status=1;break;}
                previous=e->syllable;stress=e->stress;has_nucleus=0;
                result[s]=(struct reloq_fa_nucleus_clock){
                    .syllable_from=syllables[s].from,.syllable_to=syllables[s].to,
                    .word=word,.syllable=e->syllable,.stress=e->stress,.weak=(uint32_t)weak};
                ++s;
            } else if(e->stress!=stress){status=1;break;}
            const struct reloq_fa_syllable *owner=syllables+s-1;
            /* H is represented as an onset/transition feature in this
             * backend; it does not own a separate clock. */
            if(e->value==RELOQ_FA_PHONE_H) {
                if(pending_j){status=1;break;}
                break;
            }
            uint32_t first=provider_slot(e->value),second=0;
            if(e->value==RELOQ_FA_PHONE_CH){first=2;second=11;}
            if(e->value==RELOQ_FA_PHONE_J) {
                if(pending_j){first=12;pending_j=0;}
                else if(i+1<event_count && events[i+1].kind==RELOQ_FA_PLAN_PHONE &&
                        events[i+1].value==RELOQ_FA_PHONE_J){first=5;pending_j=1;}
                else {first=5;second=12;}
            } else if(pending_j){status=1;break;}
            if(!first || p>=phone_count || provider_slots[p]!=first ||
               (second && (p+1>=phone_count || provider_slots[p+1]!=second))) {status=1;break;}
            const struct reloq_fa_syllable *actual=phones+p++;
            if(!contained(actual,owner)) {
                /* The old provider can end a syllable inside q or before
                 * its own vowel. Extend only through an identified phone
                 * that does not extend past the next syllable's start. */
                if(s<syllable_count && actual->to<=syllables[s].from &&
                   ((vowel(e->value) && actual->from>=owner->to) ||
                    (e->value==RELOQ_FA_PHONE_UVULAR && actual->from>=owner->from &&
                     actual->to>owner->to))) {
                    if(actual->to>result[s-1].syllable_to)result[s-1].syllable_to=actual->to;
                    result[s-1].syllable_extended=1;
                } else {
                    /* The temporary backend schedules the first half of a
                     * cross-syllable doubled J in the following syllable.
                     * Accept only that exact adjacent pair. */
                    if(e->value!=RELOQ_FA_PHONE_J || i+1>=event_count || s>=syllable_count ||
                       events[i+1].kind!=RELOQ_FA_PLAN_PHONE || events[i+1].value!=RELOQ_FA_PHONE_J ||
                       events[i+1].syllable!=e->syllable+1 ||
                       !contained(actual,syllables+s)) {status=1;break;}
                    result[s-1].bridged_j=1;
                }
            }
            if(second && !contained(phones+p++,owner)){status=1;break;}
            if(vowel(e->value)) {
                if(has_nucleus || (!contained(actual,owner) && !result[s-1].syllable_extended))
                    {status=1;break;}
                has_nucleus=1;
                result[s-1].nucleus=e->value;
                result[s-1].nucleus_from=actual->from;
                result[s-1].nucleus_to=actual->to;
            }
            break;
        }
        case RELOQ_FA_PLAN_WORD_END:
            if(!in_word || previous==UINT32_MAX || !has_nucleus || pending_j){status=1;break;}
            in_word=0;break;
        case RELOQ_FA_PLAN_PUNCTUATION:
            if(in_word || !have_word || next_weak){status=1;break;}
            if(e->value==',' || e->value==0x060c) {
                /* A comma between words has one separately traced pause. */
                if(ended || p>=phone_count || provider_slots[p]!=36){status=1;break;}
                ++p;
            } else if(e->value=='.' || e->value=='?' || e->value=='!' || e->value==0x061f) {
                if(ended){status=1;break;}
                ended=1;
            } else status=1;
            break;
        case RELOQ_FA_PLAN_INPUT_VARIANT:
            if(in_word || e->value>1)status=1;
            break;
        case RELOQ_FA_PLAN_GLOTTAL_ONSET:
            /* The scoped backend marks the next word's onset without a
             * separate traced phone. Its vowel must still align normally. */
            if(in_word || !have_word || ended || i+1>=event_count ||
               events[i+1].kind!=RELOQ_FA_PLAN_WORD_BEGIN)status=1;
            break;
        default:
            /* Mixed-language events and other controls need
             * their own ownership rules before they can be aligned safely. */
            status=1;break;
        }
    }
    /* One final pause follows the last native phone. */
    if(!status && (in_word || next_weak || !have_word || s!=syllable_count ||
                   p>=phone_count || phone_count-p!=1 || provider_slots[p]!=36))status=1;
    if(!status && output && capacity<s)status=-2;
    if(!status) {
        if(output)memcpy(output,result,s*sizeof *output);
        *written=s;
    }
    free(result);
    return status;
}
