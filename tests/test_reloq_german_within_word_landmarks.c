#include "reloq_german_within_word_landmarks.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static int query(const char *annotation,
                 struct reloq_german_within_word_landmarks *out)
{
    struct reloq_german_phone_node phones[8];
    struct reloq_german_phone_sequence_info info;
    const size_t length = strlen(annotation);
    if (reloq_german_phone_sequence_build(
            (const unsigned char *)annotation, length, 0,
            phones, 8, &info)) return -1;
    struct reloq_german_syllable syllables[2];
    size_t count = 0;
    if (reloq_german_syllable_plan_build(
            phones, info.phone_count, syllables, 2, &count)) return -1;
    return reloq_german_within_word_landmarks_plan(
        phones, info.phone_count, syllables, count, out);
}

static void unit(void)
{
    struct reloq_german_within_word_landmarks m;
    assert(query("`[.0pAp.1tIp].", &m) == 0);
    assert(m.first_burst_end == 15 && m.first_vowel_onset == 34);
    assert(m.first_vowel_hold == 71 && m.first_closure == 101);
    assert(m.first_coda_burst_start == 191 &&
           m.first_coda_release == 201);
    assert(m.second_onset_origin == 202 &&
           m.second_burst_start == 254 && m.second_burst_end == 272);
    assert(m.second_vowel_onset == 344 &&
           m.second_vowel_hold == 407 && m.second_closure == 432);
    assert(m.second_coda_release == 557 && m.planned_ticks == 1017);

    assert(query("`[.0kUk.1tIp].", &m) == 0);
    assert(m.first_burst_end == 17 && m.first_vowel_onset == 67);
    assert(m.first_vowel_hold == 81 && m.first_closure == 121);
    assert(m.first_coda_burst_start == 173 &&
           m.first_coda_release == 181);
    assert(m.second_onset_origin == 201 &&
           m.second_burst_start == 273 && m.second_burst_end == 291);
    assert(m.second_vowel_onset == 334 &&
           m.second_vowel_hold == 397 && m.second_closure == 422);
    assert(m.second_coda_release == 547 && m.planned_ticks == 1007);

    assert(query("`[.0pAp.1tAt].", &m) == 0);
    assert(m.first_coda_release == 201 &&
           m.second_burst_start == 254 && m.second_burst_end == 266);
    assert(m.second_vowel_onset == 338 &&
           m.second_vowel_hold == 385 && m.second_closure == 425);
    assert(m.second_coda_release == 533 && m.planned_ticks == 993);

    struct reloq_german_within_word_landmarks keep = m;
    assert(query("`[.1pAp.0tIp].", &m) == 1);
    assert(!memcmp(&m, &keep, sizeof m));
    assert(query("`[.0pAp] `[.1tIp].", &m) == 1);
    assert(!memcmp(&m, &keep, sizeof m));
    assert(query("`[.0pAt.1tIp].", &m) == 1);
    assert(!memcmp(&m, &keep, sizeof m));
    assert(query("`[.0pAp.1tYp].", &m) == 1);
    assert(!memcmp(&m, &keep, sizeof m));
    assert(reloq_german_within_word_landmarks_plan(NULL, 0, NULL, 0,
                                                    &m) == -1);
    assert(!memcmp(&m, &keep, sizeof m));
}

int main(int argc, char **argv)
{
    if (argc == 3 && !strcmp(argv[1], "--query")) {
        struct reloq_german_within_word_landmarks m;
        if (query(argv[2], &m)) return 1;
        printf("%lld %lld %lld %lld %lld %lld %lld %lld %lld "
               "%lld %lld %lld %lld %lld\n",
               (long long)m.first_burst_end,
               (long long)m.first_vowel_onset,
               (long long)m.first_vowel_hold,
               (long long)m.first_closure,
               (long long)m.first_coda_burst_start,
               (long long)m.first_coda_release,
               (long long)m.second_onset_origin,
               (long long)m.second_burst_start,
               (long long)m.second_burst_end,
               (long long)m.second_vowel_onset,
               (long long)m.second_vowel_hold,
               (long long)m.second_closure,
               (long long)m.second_coda_release,
               (long long)m.planned_ticks);
        return 0;
    }
    if (argc != 1) return 2;
    unit();
    puts("German within-word two-syllable landmarks: OK");
    return 0;
}
