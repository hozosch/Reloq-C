#include "reloq_german_generic_nasal_coda.h"

#include "reloq_german_generic_tract.h"

int reloq_german_generic_nasal_coda_plan(
    const struct reloq_german_phone_node *node,
    int coda_after_vowel, int phrase_final, int next_vowel,
    int32_t previous_av, int32_t source_trim,
    struct reloq_german_generic_nasal_coda *out)
{
    if (!node || !out || (coda_after_vowel != 0 && coda_after_vowel != 1) ||
        (phrase_final != 0 && phrase_final != 1) ||
        (next_vowel != 0 && next_vowel != 1) ||
        previous_av < 0 || source_trim < 0 || source_trim > 10)
        return -1;
    const enum reloq_german_phone phone = node->phone;
    if (!coda_after_vowel ||
        (phone != RELOQ_DE_PHONE_m && phone != RELOQ_DE_PHONE_n &&
         phone != RELOQ_DE_PHONE_G)) return 0;
    /* The durations/source levels are a phone-class and boundary rule.  The
     * terminal nasal has an extended low-tract sustain; a nasal before the
     * next vowel is shorter, while an unpunctuated word edge gets the
     * intermediate duration.  Coda nasals before a consonant inside one
     * word keep their existing model until that separate context is known. */
    const int internal_vowel = node->same_word_right &&
        !node->same_syllable_right && next_vowel;
    const int connected_word = !node->same_word_right &&
        node->right_phone != RELOQ_DE_PHONE_NONE && !phrase_final;
    const int internal_h=node->same_word_right && !node->same_syllable_right &&
        node->right_phone==RELOQ_DE_PHONE_h;
    if (!phrase_final && !internal_vowel && !connected_word && !internal_h) return 0;
    struct reloq_german_generic_tract tract;
    if (reloq_german_generic_sonorant_tract(node, &tract)) return -1;
    struct reloq_german_generic_nasal_coda p = {0};
    if (phrase_final) {
        const int schwa=node->left_phone==RELOQ_DE_PHONE_AT;
        const int reduced_r=node->left_phone==RELOQ_DE_PHONE_R;
        p.duration_ticks = phone==RELOQ_DE_PHONE_G?140:
            phone==RELOQ_DE_PHONE_m?(schwa || reduced_r?105:120):
            !node->stress?110:schwa?77:reduced_r?95:120;
        if (phone == RELOQ_DE_PHONE_m) {
            p.start_av = tract.av + source_trim;
            p.end_av = tract.av;
        } else {
            p.start_av = 48 + source_trim;
            p.taper_percent = 80;
            p.taper_av = 42 + source_trim;
            p.end_av = phone == RELOQ_DE_PHONE_G ? 0 : 40 + source_trim;
        }
    } else if (internal_vowel) {
        p.duration_ticks = phone==RELOQ_DE_PHONE_n?30:
            phone==RELOQ_DE_PHONE_m && node->left_phone==RELOQ_DE_PHONE_AT?52:60;
        p.f1_override = phone == RELOQ_DE_PHONE_m ? 300 : 0;
        p.start_av = phone == RELOQ_DE_PHONE_m ? previous_av :
                     48 + source_trim;
        p.end_av = phone == RELOQ_DE_PHONE_m ? 48 + source_trim :
                   p.start_av;
    } else {
        p.duration_ticks = phone == RELOQ_DE_PHONE_G ? 81 : 70;
        if(internal_h) {
            const int schwa=node->left_phone==RELOQ_DE_PHONE_AT;
            p.duration_ticks=phone==RELOQ_DE_PHONE_G?42:
                phone==RELOQ_DE_PHONE_m?(schwa?52:85):schwa?72:node->stress==1?70:40;
        }
        p.f1_override = phone == RELOQ_DE_PHONE_m ? 300 : 0;
        p.start_av = (phone == RELOQ_DE_PHONE_m ? 51 :
                      phone == RELOQ_DE_PHONE_G ? 49 : 48) + source_trim;
        p.end_av = p.start_av;
    }
    if (p.start_av > 70 || p.end_av > 70) return -1;
    *out = p;
    return 1;
}
