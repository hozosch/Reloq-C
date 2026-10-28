#include "reloq_german_syllable_plan.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static size_t parse(const char *annotation,
                    struct reloq_german_phone_node *phones,
                    struct reloq_german_phone_sequence_info *info)
{
    assert(reloq_german_phone_sequence_build(
        (const unsigned char *)annotation, strlen(annotation), 0,
        phones, 32, info) == 0);
    return info->phone_count;
}

static void check_two_syllables(void)
{
    struct reloq_german_phone_node phones[32];
    struct reloq_german_phone_sequence_info info;
    const size_t n = parse(" `[.0pAk.1tIp].", phones, &info);
    assert(n == 6 && info.word_count == 1 && info.syllable_count == 2);
    struct reloq_german_syllable syllables[3];
    size_t count = 77;
    assert(reloq_german_syllable_plan_build(
        phones, n, NULL, 0, &count) == -2 && count == 2);
    count = 77;
    assert(reloq_german_syllable_plan_build(
        phones, n, syllables, 3, &count) == 0 && count == 2);
    assert(syllables[0].first_phone == 0 && syllables[0].phone_count == 3);
    assert(syllables[0].onset_count == 1 && syllables[0].nucleus_count == 1 &&
           syllables[0].coda_count == 1 && syllables[0].nucleus_contiguous);
    assert(syllables[0].stress == 0 && syllables[0].stress_explicit &&
           syllables[0].boundary_after == RELOQ_DE_BOUNDARY_SYLLABLE);
    assert(syllables[1].first_phone == 3 && syllables[1].phone_count == 3 &&
           syllables[1].stress == 1 && syllables[1].stress_explicit &&
           syllables[1].boundary_after == RELOQ_DE_BOUNDARY_UTTERANCE);

    /* The second word may start with a new primary stress; it is not a
     * second syllable of the first word. */
    const size_t m = parse(" `[.1hA.0lo] `2 `[.1vElt].", phones, &info);
    assert(m == 8 && info.word_count == 2 && info.syllable_count == 3);
    assert(reloq_german_syllable_plan_build(
        phones, m, syllables, 3, &count) == 0 && count == 3);
    assert(syllables[0].boundary_after == RELOQ_DE_BOUNDARY_SYLLABLE);
    assert(syllables[1].boundary_after == RELOQ_DE_BOUNDARY_WORD);
    assert(syllables[2].boundary_after == RELOQ_DE_BOUNDARY_UTTERANCE);
    assert(syllables[1].stress == 0 && syllables[2].stress == 1);
}

static void check_three_syllables(void)
{
    struct reloq_german_phone_node phones[32];
    struct reloq_german_phone_sequence_info info;
    const size_t n = parse(" `[.1bEt.2dE.0k@].", phones, &info);
    assert(n == 7 && info.word_count == 1 && info.syllable_count == 3);
    struct reloq_german_syllable syllables[3];
    size_t count = 0;
    assert(reloq_german_syllable_plan_build(
        phones, n, syllables, 3, &count) == 0 && count == 3);
    assert(syllables[0].stress == 1 && syllables[0].coda_count == 1);
    assert(syllables[1].stress == 2 && syllables[1].coda_count == 0);
    assert(syllables[2].stress == 0 && syllables[2].nucleus_count == 1);
    assert(syllables[2].boundary_after == RELOQ_DE_BOUNDARY_UTTERANCE);
}

static void check_transactional(void)
{
    struct reloq_german_phone_node phones[32];
    struct reloq_german_phone_sequence_info info;
    const size_t n = parse(" `[.0pAk.1tIp].", phones, &info);
    struct reloq_german_syllable out[3];
    memset(out, 0xa5, sizeof out);
    struct reloq_german_syllable saved[3];
    memcpy(saved, out, sizeof out);
    size_t count = 77;
    phones[4].stress = 0;
    assert(reloq_german_syllable_plan_build(phones, n, out, 3, &count) == -1);
    assert(count == 77 && !memcmp(saved, out, sizeof out));
    phones[4].stress = 1;
    phones[3].word_index = 3;
    assert(reloq_german_syllable_plan_build(phones, n, out, 3, &count) == -1);
    assert(count == 77 && !memcmp(saved, out, sizeof out));
}

static int print_query(const char *annotation)
{
    const size_t length = strlen(annotation);
    if (length > (size_t)-1 / sizeof(struct reloq_german_phone_node))
        return 1;
    struct reloq_german_phone_node *phones =
        calloc(length + 1, sizeof *phones);
    if (!phones) return 1;
    struct reloq_german_phone_sequence_info info;
    if (reloq_german_phone_sequence_build(
            (const unsigned char *)annotation, length, 0,
            phones, length + 1, &info)) {
        free(phones);
        return 1;
    }
    struct reloq_german_syllable *parts =
        calloc(info.syllable_count + 1, sizeof *parts);
    if (!parts) { free(phones); return 1; }
    size_t count = 0;
    const int result = reloq_german_syllable_plan_build(
        phones, info.phone_count, parts, info.syllable_count + 1, &count);
    if (!result) {
        printf("%zu %zu %zu\n", info.phone_count, info.word_count, count);
        for (size_t i = 0; i < count; ++i) {
            const struct reloq_german_syllable *s = &parts[i];
            printf("%zu %zu %zu %zu %zu %zu %zu %u %u %u %d\n",
                   s->first_phone, s->phone_count, s->word_index,
                   s->syllable_in_word, s->onset_count, s->nucleus_count,
                   s->coda_count, (unsigned)s->nucleus_contiguous,
                   (unsigned)s->stress, (unsigned)s->stress_explicit,
                   (int)s->boundary_after);
        }
    }
    free(parts);
    free(phones);
    return result != 0;
}

int main(int argc, char **argv)
{
    if (argc == 3 && !strcmp(argv[1], "--query"))
        return print_query(argv[2]);
    if (argc != 1) return 2;
    check_two_syllables();
    check_three_syllables();
    check_transactional();
    puts("German structural syllable planner: OK");
    return 0;
}
