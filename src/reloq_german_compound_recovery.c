#include "reloq_german_compound_recovery.h"

#include "reloq_german_g2p.h"
#include "reloq_german_lexicon.h"
#include "reloq_german_spr.h"

#include <stdint.h>
#include <string.h>

#define COMPONENT_CAPACITY 512u
#define COMPOUND_CAPACITY 1024u

static int letter(unsigned char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           c == 0xc4 || c == 0xd6 || c == 0xdc || c == 0xe4 ||
           c == 0xf6 || c == 0xfc || c == 0xdf;
}

static unsigned char lower(unsigned char c)
{
    if (c >= 'A' && c <= 'Z') return (unsigned char)(c + ('a' - 'A'));
    if (c == 0xc4) return 0xe4;
    if (c == 0xd6) return 0xf6;
    if (c == 0xdc) return 0xfc;
    return c;
}

static int vowel(unsigned char c)
{
    c = lower(c);
    return c == 'a' || c == 'e' || c == 'i' || c == 'o' ||
           c == 'u' || c == 'y' || c == 0xe4 || c == 0xf6 ||
           c == 0xfc;
}

/* Reject artificial fragments which the permissive best-effort transducer
 * could otherwise read as isolated words: Tonq + ualität, Tonqu + alität,
 * Bil + dqualität, Sti + mmqualität, and internal multi-letter graphemes.
 * This is an orthographic *onset* grammar, not a list of words. */
static int plausible_head_onset(const unsigned char *text, size_t length)
{
    if (!length) return 0;
    unsigned char first = lower(text[0]);
    if (vowel(first)) return 1;
    if (first == 'q') return length > 2 && lower(text[1]) == 'u' &&
                             vowel(text[2]);
    if (length == 1) return 0;
    unsigned char second = lower(text[1]);
    if (vowel(second)) return first != 'c' || second == 'h';
    if (first == second) return 0;
    if ((first == 's' && (second == 'p' || second == 't' ||
                         second == 'c' || second == 'k' || second == 'm' ||
                         second == 'n' || second == 'l' || second == 'w')) ||
        (first == 'c' && second == 'h') ||
        (first == 'p' && (second == 'f' || second == 'h' ||
                         second == 'l' || second == 'r')) ||
        (first == 't' && (second == 'h' || second == 'r' ||
                         second == 's')) ||
        (first == 'z' && second == 'w') ||
        (first == 'b' && (second == 'l' || second == 'r')) ||
        (first == 'd' && second == 'r') ||
        (first == 'g' && (second == 'l' || second == 'r' ||
                         second == 'n')) ||
        (first == 'k' && (second == 'l' || second == 'r' ||
                         second == 'n')) ||
        (first == 'f' && (second == 'l' || second == 'r')) ||
        (first == 'v' && second == 'r'))
        return length >= 3 && (vowel(text[2]) ||
            (first == 's' && second == 'c' && length >= 4 &&
             lower(text[2]) == 'h') ||
            (first == 's' && (second == 'p' || second == 't') &&
             (lower(text[2]) == 'r' || lower(text[2]) == 'l')));
    return 0;
}

static int valid_boundary(const unsigned char *word, size_t length,
                          size_t split)
{
    if (split < 3 || length - split < 5 || vowel(word[split - 1]) ||
        lower(word[split - 1]) == 'q' ||
        (lower(word[split - 1]) == 'c' &&
         lower(word[split - 2]) != 'k') ||
        !plausible_head_onset(word + split, length - split)) return 0;
    /* A doubled consonant belongs to the same spelling component unless
     * both copies are on the first side, e.g. Stimm + qualität. */
    if (lower(word[split - 1]) == lower(word[split])) return 0;
    /* An artificial split in an orthographic digraph cannot stand for a
     * morpheme boundary. Real compounds across these rare boundaries need
     * an explicit lexical pronunciation. */
    if ((lower(word[split - 1]) == 'c' && lower(word[split]) == 'h') ||
        (lower(word[split - 1]) == 'c' && lower(word[split]) == 'k') ||
        (lower(word[split - 1]) == 'n' && lower(word[split]) == 'g'))
        return 0;
    return 1;
}

static int component(const struct reloq_lexicon *lexicon,
                     const unsigned char *word, size_t length,
                     unsigned char *spr, size_t *spr_length)
{
    int result = reloq_german_lexicon_word(
        lexicon, word, length, spr, COMPONENT_CAPACITY, spr_length);
    if (result == 0)
        result = reloq_german_g2p_word_best_effort(
            word, length, spr, COMPONENT_CAPACITY, spr_length, NULL);
    return result == 1 &&
           reloq_german_lexicon_validate_spr(spr, *spr_length, NULL) == 1;
}

static int shape(const unsigned char *spr, size_t length,
                 int head, size_t *primary_offset)
{
    if (length < 6 || spr[0] != '`' || spr[1] != '[' ||
        spr[length - 1] != ']') return 0;
    struct reloq_german_spr_cursor cursor = {0};
    struct reloq_german_spr_event event;
    size_t syllables = 0, primary = 0, secondary = 0;
    int state;
    while ((state = reloq_german_spr_next(
                spr, length, &cursor, &event)) == 1) {
        if (event.kind == RELOQ_DE_SPR_SYLLABLE) ++syllables;
        else if (event.kind == RELOQ_DE_SPR_STRESS) {
            if (event.stress == 1) {
                ++primary;
                *primary_offset = event.offset;
            } else if (event.stress == 2) ++secondary;
        }
    }
    return state == 0 && primary == 1 && secondary == 0 &&
           (head ? (syllables >= 2 && syllables <= 3) :
                   syllables == 1);
}

/* Productive -en+s linking is admitted only after a separately covered,
 * weak-schwa /@n/ ending and before a consonant-initial compound head.
 * This joins Leben+s+Qualität without treating arbitrary ...s prefixes as
 * phonetic roots or changing the standalone word Leben. */
static int weak_en_link(const unsigned char *word, size_t root_length,
                        const unsigned char *spr, size_t spr_length)
{
    if (root_length < 5 || spr_length < 9 ||
        lower(word[root_length - 2]) != 'e' ||
        lower(word[root_length - 1]) != 'n' ||
        spr[spr_length - 3] != '@' || spr[spr_length - 2] != 'n')
        return 0;
    struct reloq_german_spr_cursor cursor = {0};
    struct reloq_german_spr_event event;
    size_t syllables = 0, primary = 0, secondary = 0;
    int state;
    while ((state = reloq_german_spr_next(
                spr, spr_length, &cursor, &event)) == 1) {
        if (event.kind == RELOQ_DE_SPR_SYLLABLE) ++syllables;
        else if (event.kind == RELOQ_DE_SPR_STRESS) {
            if (event.stress == 1 && event.syllable == 0) ++primary;
            else if (event.stress == 2) ++secondary;
        }
    }
    return state == 0 && syllables == 2 && primary == 1 && secondary == 0;
}

int reloq_german_compound_recover(
    const struct reloq_lexicon *lexicon,
    const unsigned char *word, size_t word_length,
    unsigned char *output, size_t capacity, size_t *byte_length)
{
    if (!word || !byte_length || (!output && capacity) || !word_length)
        return -1;
    if (word_length < 8 || word_length >= 128) return 0;
    for (size_t i = 0; i < word_length; ++i)
        if (!letter(word[i]) || (i && word[i] >= 'A' && word[i] <= 'Z'))
            return -1;
    /* Do not reinterpret a complete word that the established language
     * frontend already handles. In particular an ordinary long word must
     * never acquire a spurious compound stress merely because two of its
     * substrings can each be given an approximate pronunciation. */
    size_t existing_length = 0;
    int existing = reloq_german_lexicon_word(
        lexicon, word, word_length, NULL, 0, &existing_length);
    if (existing == 0)
        existing = reloq_german_g2p_word_best_effort(
            word, word_length, NULL, 0, &existing_length, NULL);
    if (existing != 0) return 0;
    unsigned char selected[COMPOUND_CAPACITY];
    size_t selected_length = 0;
    int candidates = 0;
    for (size_t split = 3; split + 5 <= word_length; ++split) {
        if (!valid_boundary(word, word_length, split)) continue;
        unsigned char left[COMPONENT_CAPACITY], right[COMPONENT_CAPACITY];
        size_t left_length = 0, right_length = 0;
        size_t first_primary = 0, head_primary = 0;
        int linking = lower(word[split - 1]) == 's' &&
                      split > 5 && !vowel(word[split]);
        size_t root_length = linking ? split - 1 : split;
        if (!component(lexicon, word, root_length, left, &left_length) ||
            !component(lexicon, word + split, word_length - split,
                       right, &right_length) ||
            !(linking ? weak_en_link(word, root_length, left, left_length) :
                        shape(left, left_length, 0, &first_primary)) ||
            !shape(right, right_length, 1, &head_primary)) continue;
        if (linking) {
            if (left_length + 1 >= sizeof left) continue;
            left[left_length - 1] = 's';
            left[left_length++] = ']';
            left[left_length] = 0;
        }
        if (++candidates > 1) return 0;
        size_t length = left_length + right_length - 3;
        if (length >= sizeof selected) return 0;
        memcpy(selected, left, left_length - 1);
        memcpy(selected + left_length - 1, right + 2, right_length - 2);
        selected[left_length - 1 + head_primary - 2] = '2';
        if (!reloq_german_lexicon_validate_spr(selected, length, NULL))
            return 0;
        selected_length = length;
    }
    if (candidates != 1) return 0;
    if (!output || capacity <= selected_length) {
        *byte_length = selected_length;
        return -2;
    }
    memcpy(output, selected, selected_length);
    output[selected_length] = 0;
    *byte_length = selected_length;
    return 1;
}
