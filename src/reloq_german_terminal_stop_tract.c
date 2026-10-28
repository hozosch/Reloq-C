#include "reloq_german_terminal_stop_tract.h"

#include <stdint.h>

/* Compact voice coefficients for the vowel's anticipatory constriction.
 * Columns are the three oral formants at closure before p, t and k. The
 * remainder of the gesture is shared across each place of articulation. */
struct vowel_constriction {
    enum reloq_german_phone phone;
    int32_t f[3][3];
    uint8_t front_release;
    uint8_t front_t_f3;
};

static const struct vowel_constriction vowels[] = {
    {RELOQ_DE_PHONE_R, {{300,1000,2500},{300,1600,2500},{300,1500,2300}},0,0},
    {RELOQ_DE_PHONE_l, {{300,1500,2500},{300,1600,2500},{500,1500,2300}},0,0},
    {RELOQ_DE_PHONE_A, {{500, 900, 2500}, {300, 1600, 2500}, {500, 1750, 2200}}, 0, 0},
    {RELOQ_DE_PHONE_E, {{300, 1000, 2200}, {300, 1800, 2500}, {300, 1900, 2300}}, 1, 0},
    {RELOQ_DE_PHONE_I, {{300, 1000, 2200}, {300, 1800, 2700}, {300, 1900, 2300}}, 1, 1},
    {RELOQ_DE_PHONE_O, {{300, 700, 2500}, {300, 1400, 2500}, {500, 1500, 2300}}, 0, 0},
    {RELOQ_DE_PHONE_U, {{300, 600, 2100}, {300, 1300, 2050}, {400, 1000, 2300}}, 0, 0},
    {RELOQ_DE_PHONE_a, {{500, 900, 2500}, {300, 1600, 2500}, {500, 1750, 2200}}, 0, 0},
    {RELOQ_DE_PHONE_e, {{300, 1500, 2500}, {300, 1900, 2700}, {300, 1900, 2500}}, 1, 1},
    {RELOQ_DE_PHONE_i, {{300, 1500, 2500}, {300, 1900, 2700}, {300, 2200, 2400}}, 1, 1},
    {RELOQ_DE_PHONE_o, {{300, 600, 2100}, {300, 1300, 2050}, {400, 960, 2300}}, 0, 0},
    {RELOQ_DE_PHONE_u, {{300, 600, 2100}, {300, 1300, 2050}, {300, 1000, 2300}}, 0, 0}
};

int reloq_german_terminal_stop_tract_plan(
    const struct reloq_german_phone_node *node,
    struct reloq_german_obstruent_tract *out)
{
    if (!node || !out) return -1;
    if (!node->same_word_left || node->same_word_right) return 0;
    int place=node->phone==RELOQ_DE_PHONE_p?0:
        node->phone==RELOQ_DE_PHONE_t?1:node->phone==RELOQ_DE_PHONE_k?2:-1;
    if (place<0) return 0;
    const struct vowel_constriction *v=NULL;
    for (size_t i=0;i<sizeof vowels/sizeof *vowels;++i)
        if (vowels[i].phone==node->left_phone) {v=vowels+i;break;}
    if (!v) return 0;
    struct reloq_german_obstruent_tract t={
        {v->f[place][0],v->f[place][1],v->f[place][2],
         place==1?3500:3600,place==1?4200:3900},
        {300,place==0?1000:place==1?(v->front_release?1800:1600):
          (v->front_release?1900:1500),
         place==0?2500:place==1?(v->front_t_f3?2700:2500):2300,-1,-1},
        {120,100,150,330,260}};
    *out=t;return 1;
}

static void point(struct reloq_target_event *events, size_t *count,
                  enum reloq_target_stream stream, int64_t offset,
                  int32_t value)
{
    events[(*count)++] = (struct reloq_target_event){stream, offset, value};
}

int reloq_german_terminal_stop_tract_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    size_t vowel_index, int64_t closure_tick,
    struct reloq_target_builder *builder, int64_t *release_tick)
{
    if (!phones || !builder || !release_tick || !phone_count ||
        vowel_index >= phone_count || closure_tick < 0) return -1;
    if ((phone_count != 2 && phone_count != 3) ||
        (vowel_index != 0 && vowel_index != 1) ||
        vowel_index + 2 != phone_count ||
        (vowel_index && phones[0].phone != RELOQ_DE_PHONE_h &&
         phones[0].phone != RELOQ_DE_PHONE_p &&
         phones[0].phone != RELOQ_DE_PHONE_t &&
         phones[0].phone != RELOQ_DE_PHONE_k &&
         phones[0].phone != RELOQ_DE_PHONE_b &&
         phones[0].phone != RELOQ_DE_PHONE_d &&
         phones[0].phone != RELOQ_DE_PHONE_g &&
         phones[0].phone != RELOQ_DE_PHONE_m &&
         phones[0].phone != RELOQ_DE_PHONE_n)) return 1;
    const struct reloq_german_phone_node *vowel = &phones[vowel_index];
    if (vowel->word_index || vowel->syllable_in_word ||
        vowel->stress != 1 || !vowel->stress_explicit) return 1;
    for (size_t i = 0; i < phone_count; ++i)
        if (phones[i].word_index != vowel->word_index ||
            phones[i].syllable_index != vowel->syllable_index ||
            phones[i].stress != 1 || !phones[i].stress_explicit) return 1;

    const struct vowel_constriction *voice = NULL;
    for (size_t i = 0; i < sizeof vowels / sizeof vowels[0]; ++i)
        if (vowels[i].phone == vowel->phone) {
            voice = &vowels[i];
            break;
        }
    if (!voice) return 1;

    enum reloq_german_phone coda = phones[phone_count - 1].phone;
    size_t place;
    int64_t interval, preburst;
    if (coda == RELOQ_DE_PHONE_p) {
        place = 0; interval = 125; preburst = 10;
    } else if (coda == RELOQ_DE_PHONE_t) {
        place = 1; interval = 108; preburst = 8;
    } else if (coda == RELOQ_DE_PHONE_k) {
        place = 2; interval = 122; preburst = 12;
    } else return 1;
    if (closure_tick > INT64_MAX - interval) return -1;

    int32_t final_f2 = place == 0 ? 1000 :
        place == 1 ? (voice->front_release ? 1800 : 1600) :
                     (voice->front_release ? 1900 : 1500);
    int32_t final_f3 = place == 0 ? 2500 :
        place == 1 ? (voice->front_t_f3 ? 2700 : 2500) : 2300;
    const int32_t closure_f[5] = {voice->f[place][0],
                                  voice->f[place][1],
                                  voice->f[place][2],
                                  place == 1 ? 3500 : 3600,
                                  place == 1 ? 4200 : 3900};
    const int32_t final_f[5] = {300, final_f2, final_f3,
                                closure_f[3], closure_f[4]};
    const int32_t widths[5] = {120, 100, 150, 330, 260};
    struct reloq_target_event events[32];
    size_t count = 0;
    for (size_t i = 0; i < 5; ++i) {
        point(events, &count, (enum reloq_target_stream)
              (RELOQ_TARGET_F1 + (int)i), 0, closure_f[i]);
        point(events, &count, (enum reloq_target_stream)
              (RELOQ_TARGET_F1 + (int)i), interval, final_f[i]);
        point(events, &count, (enum reloq_target_stream)
              (RELOQ_TARGET_B1 + (int)i), 0, widths[i]);
        point(events, &count, (enum reloq_target_stream)
              (RELOQ_TARGET_B1 + (int)i), interval, widths[i]);
    }
    for (enum reloq_target_stream s = RELOQ_TARGET_FNP;
         s <= RELOQ_TARGET_FNZ; ++s) {
        point(events, &count, s, 0, 200);
        point(events, &count, s, interval, 200);
    }
    point(events, &count, RELOQ_TARGET_FTP, interval, 250);
    point(events, &count, RELOQ_TARGET_FTZ, interval, 250);
    point(events, &count, RELOQ_TARGET_FL, interval, 0);
    point(events, &count, RELOQ_TARGET_OQ, 0, 56);
    point(events, &count, RELOQ_TARGET_OQ, interval - preburst, 56);
    point(events, &count, RELOQ_TARGET_DI, 0, 0);
    point(events, &count, RELOQ_TARGET_DI, interval - preburst, 0);

    if (reloq_target_builder_append(builder, closure_tick, events, count))
        return -1;
    *release_tick = closure_tick + interval;
    return 0;
}
