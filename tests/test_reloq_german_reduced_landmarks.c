#include "reloq_german_reduced_landmarks.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static int parse(const char *annotation,
                 struct reloq_german_reduced_landmarks *landmarks)
{
    struct reloq_german_phone_node phones[8];
    struct reloq_german_phone_sequence_info info;
    if (reloq_german_phone_sequence_build(
            (const unsigned char *)annotation, strlen(annotation), 0,
            phones, 8, &info)) return -1;
    struct reloq_german_syllable syllables[2];
    size_t count = 0;
    if (reloq_german_syllable_plan_build(
            phones, info.phone_count, syllables, 2, &count)) return -1;
    return reloq_german_reduced_landmarks_plan(
        phones, info.phone_count, syllables, count, landmarks);
}

static void tests(void)
{
    struct reloq_german_reduced_landmarks t;
    assert(parse("`[.1pAp.0t@].", &t) == 0);
    assert(t.first_burst_end == 15 && t.first_vowel_onset == 65 &&
           t.first_vowel_hold == 109 && t.first_closure == 139);
    assert(t.first_coda_burst_start == 229 &&
           t.first_coda_release == 239);
    assert(t.second_onset_origin == 240 &&
           t.second_burst_start == 292 && t.second_burst_end == 304);
    assert(t.second_vowel_onset == 332 &&
           t.second_schwa_target == 332 && t.second_vowel_hold == 413);
    assert(t.second_nasal_onset == 0 &&
           t.second_closure == 413 && t.planned_ticks == 873);

    assert(parse("`[.1pAk.0@n].", &t) == 0);
    assert(t.first_coda_release == 209 &&
           t.second_vowel_onset == 209 && t.second_schwa_target == 257);
    assert(t.second_vowel_hold == 260 &&
           t.second_nasal_onset == 275 &&
           t.second_nasal_mid == 363 && t.second_closure == 385);
    assert(t.planned_ticks == 815);

    assert(parse("`[.1pA.0p@].", &t) == 0);
    assert(t.first_closure == 139 && t.first_vowel_hold == 109);
    assert(t.first_coda_burst_start == 0 &&
           t.second_onset_origin == 139 &&
           t.second_burst_start == 229 && t.second_burst_end == 239);
    assert(t.first_coda_release == 239 &&
           t.second_vowel_onset == 258 && t.second_closure == 339);
    assert(t.planned_ticks == 799);

    assert(parse("`[.1pa.0p@].", &t) == 0);
    assert(t.first_closure == 216 && t.first_coda_release == 301 &&
           t.second_vowel_onset == 320 && t.second_closure == 401);
    assert(parse("`[.1tA.0p@n].", &t) == 0);
    assert(t.first_closure == 163 && t.first_coda_release == 263 &&
           t.second_vowel_onset == 282 && t.second_nasal_onset == 341 &&
           t.second_closure == 451 && t.planned_ticks == 881);

    struct reloq_german_reduced_landmarks keep = t;
    assert(parse("`[.0tA.1p@n].", &t) == 1);
    assert(!memcmp(&t, &keep, sizeof t));
    assert(parse("`[.1tA] `[.0p@n].", &t) == 1);
    assert(!memcmp(&t, &keep, sizeof t));
    assert(parse("`[.1tA.0pYn].", &t) == 1);
    assert(!memcmp(&t, &keep, sizeof t));
    assert(reloq_german_reduced_landmarks_plan(NULL, 0, NULL, 0, &t) == -1);
    assert(!memcmp(&t, &keep, sizeof t));
}

int main(int argc, char **argv)
{
    if (argc == 3 && !strcmp(argv[1], "--query")) {
        struct reloq_german_reduced_landmarks t;
        if (parse(argv[2], &t)) return 1;
        printf("%lld %lld %lld %lld %lld %lld %lld %lld %lld "
               "%lld %lld %lld %lld %lld %lld %lld %lld %lld\n",
               (long long)t.first_burst_end,
               (long long)t.first_vowel_onset,
               (long long)t.first_vowel_hold,
               (long long)t.first_closure,
               (long long)t.first_coda_burst_start,
               (long long)t.first_coda_release,
               (long long)t.second_onset_origin,
               (long long)t.second_burst_start,
               (long long)t.second_burst_end,
               (long long)t.second_vowel_onset,
               (long long)t.second_schwa_target,
               (long long)t.second_vowel_hold,
               (long long)t.second_noise_fade,
               (long long)t.second_nasal_onset,
               (long long)t.second_nasal_mid,
               (long long)t.second_closure,
               (long long)t.second_coda_release,
               (long long)t.planned_ticks);
        return 0;
    }
    if (argc != 1) return 2;
    tests();
    puts("German reduced-syllable timing: OK");
    return 0;
}
