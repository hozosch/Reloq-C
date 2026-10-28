#include "reloq_german_cross_word_landmarks.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static enum reloq_german_terminal_mark mark_for(const char *s)
{
    size_t length = strlen(s);
    if (length >= 3 && !strcmp(s + length - 3, "..."))
        return RELOQ_DE_TERMINAL_ELLIPSIS;
    switch (s[length-1]) {
    case '?': return RELOQ_DE_TERMINAL_QUESTION;
    case '!': return RELOQ_DE_TERMINAL_EXCLAMATION;
    case ';': return RELOQ_DE_TERMINAL_SEMICOLON;
    case ',': return RELOQ_DE_TERMINAL_COMMA;
    case ':': return RELOQ_DE_TERMINAL_COLON;
    default: return RELOQ_DE_TERMINAL_PERIOD;
    }
}

static int query(const char *s)
{
    struct reloq_german_phone_node phones[256];
    struct reloq_german_phone_sequence_info info;
    if (reloq_german_phone_sequence_build((const unsigned char *)s,
            strlen(s), 0, phones, 256, &info)) return 1;
    struct reloq_german_syllable syllables[128];
    size_t count = 0;
    if (reloq_german_syllable_plan_build(phones, info.phone_count,
            syllables, 128, &count)) return 2;
    struct reloq_german_cross_word_word words[128];
    int64_t total;
    if (reloq_german_cross_word_landmarks_plan(phones, info.phone_count,
            syllables, count, mark_for(s), words, 128, &total)) return 3;
    printf("%zu", count);
    for (size_t i=0;i<count;++i) {
        struct reloq_german_cross_word_word *w=&words[i];
        printf(" %lld %lld %lld %lld %lld %lld %lld %lld %lld",
            (long long)w->onset_origin, (long long)w->burst_start,
            (long long)w->burst_end, (long long)w->vowel_onset,
            (long long)w->vowel_hold, (long long)w->vowel_closure,
            (long long)w->coda_burst_start, (long long)w->boundary_release,
            (long long)w->f0_early_tick);
    }
    printf(" %lld\n",(long long)total);
    return 0;
}
static void selftest(void)
{
    struct reloq_german_phone_node phones[12];
    struct reloq_german_phone_sequence_info info;
    const char *s = "`[.1pA] `2 `[.1tA] `2 `[.1pIp].";
    assert(reloq_german_phone_sequence_build((const unsigned char *)s,
           strlen(s), 0, phones, 12, &info) == 0);
    struct reloq_german_syllable syllables[3];
    size_t count = 0;
    assert(reloq_german_syllable_plan_build(phones, info.phone_count,
           syllables, 3, &count) == 0 && count == 3);
    struct reloq_german_cross_word_word words[3];
    memset(words, 0x73, sizeof words);
    struct reloq_german_cross_word_word keep[3];
    memcpy(keep, words, sizeof keep);
    int64_t total = -137;
    assert(reloq_german_cross_word_landmarks_plan(
           phones, info.phone_count, syllables, count,
           RELOQ_DE_TERMINAL_PERIOD, words, 2, &total) == -1);
    assert(!memcmp(words, keep, sizeof words) && total == -137);
    assert(reloq_german_cross_word_landmarks_plan(
           phones, info.phone_count, syllables, count,
           (enum reloq_german_terminal_mark)100, words, 3, &total) == -1);
    assert(!memcmp(words, keep, sizeof words) && total == -137);
    assert(reloq_german_cross_word_landmarks_plan(
           phones, info.phone_count, syllables, count,
           RELOQ_DE_TERMINAL_PERIOD, words, 3, &total) == 0);
    assert(words[0].vowel_onset == 65 && words[0].vowel_closure == 146 &&
           words[0].boundary_release == 226);
    assert(words[1].vowel_onset == 298 && words[1].vowel_closure == 379 &&
           words[1].f0_early_tick == 186 &&
           words[1].boundary_release == 479);
    assert(words[2].vowel_onset == 529 && words[2].vowel_closure == 617 &&
           words[2].boundary_release == 742 && total == 1202);
    memcpy(keep, words, sizeof keep);
    int64_t kept_total = total;
    syllables[1].stress = 0;
    assert(reloq_german_cross_word_landmarks_plan(
           phones, info.phone_count, syllables, count,
           RELOQ_DE_TERMINAL_PERIOD, words, 3, &total) == 1);
    assert(!memcmp(words, keep, sizeof words) && total == kept_total);
}
int main(int argc,char **argv)
{
    if(argc==2) return query(argv[1]);
    if(argc!=1) return 2;
    selftest();
    puts("German cross-word timing: OK");
    return 0;
}
