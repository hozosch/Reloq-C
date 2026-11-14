/* Experimental Persian frontend, GPL-3.0-or-later.
 * The Latin letter/digraph mappings are based on eSpeak NG fa_rules, GPL-3+
 * (Persian rules by Shadyar Khodayari), commit 688cb60368d6e0998f3632af8d1c3c2ad4191fc2.
 * This separate root-project experiment is not included in the MIT provider. */
#include "pinglish_candidate.h"

#include <string.h>

static unsigned char lower(unsigned char c)
{
    return c >= 'A' && c <= 'Z' ? (unsigned char)(c + ('a' - 'A')) : c;
}

char fa_reference_a_to_finnish(char espeak_vowel)
{
    if (espeak_vowel == 'a') return 'A';
    if (espeak_vowel == 'A') return 'a';
    return 0;
}

static int is_salam(const char *word, size_t len)
{
    static const char canonical[] = "salam";
    if (len != sizeof canonical - 1) return 0;
    for (size_t i = 0; i < len; ++i)
        if (lower((unsigned char)word[i]) != (unsigned char)canonical[i]) return 0;
    return 1;
}

static const char *single(unsigned char c, unsigned char next)
{
    static const char *const phones[26] = {
        "a", "b", NULL, "d", "e", "f", "g", "h", "i", "dZ", "k", "l", "m",
        "n", "o", "p", "q1", "R", "s", "t", "u", "v", "w", "ks", "j", "z"
    };
    if (c == 'c') return next == 'e' || next == 'i' || next == 'y' ? "s" : "k";
    return phones[c - 'a'];
}

enum fa_latin_result fa_latin_candidate(enum fa_latin_mode mode,
                                         const char *word, char *out, size_t capacity)
{
    size_t len, used = 0;
    if (out && capacity) out[0] = '\0';
    if (!word || !out || !capacity || (mode != FA_LATIN_ENGLISH && mode != FA_LATIN_PINGLISH))
        return FA_LATIN_INVALID;
    len = strlen(word);
    if (!len) return FA_LATIN_INVALID;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = lower((unsigned char)word[i]);
        if (c < 'a' || c > 'z') return FA_LATIN_NEEDS_CONTEXT;
    }
    if (mode == FA_LATIN_ENGLISH) return FA_LATIN_EN_US;
    /* eSpeak also has letter names, dictionary entries and conditional
       English fallback. Do not pretend to resolve them with letter rules. */
    if (len == 1) return FA_LATIN_NEEDS_CONTEXT;
    /* Pinglish often leaves long /A/ implicit. The greeting's Persian-script
       reference is salAm; preserve the two vowel identities here. This
       intentional exception differs from eSpeak 1.51's fa-Latn salam. */
    if (is_salam(word, len)) {
        static const char phones[] = "salAm";
        if (capacity < sizeof phones) return FA_LATIN_OVERFLOW;
        memcpy(out, phones, sizeof phones);
        return FA_LATIN_CANDIDATE;
    }
    for (size_t i = 0; i < len;) {
        unsigned char c = lower((unsigned char)word[i]);
        unsigned char n = i + 1 < len ? lower((unsigned char)word[i + 1]) : 0;
        unsigned char n2 = i + 2 < len ? lower((unsigned char)word[i + 2]) : 0;
        const char *p = NULL;
        size_t advance = 1;
        if ((c == 'p' || c == 't') && n == 'h') goto needs_context;
        if (c == 'o' && n == 'o' && n2 == 'o') goto needs_context;
        if (c == 'a' && n == 'a') p = "A";
        else if (c == 'c' && n == 'h') p = "tS";
        else if (c == 'e' && n == 'e') p = "i";
        else if (c == 'g' && n == 'h') p = "q1";
        else if (c == 'j' && n == 'h') p = "Z";
        else if (c == 'k' && n == 'h') p = "x";
        else if (c == 'o' && (n == 'u' || n == 'o')) p = "u";
        else if (c == 's' && n == 'h') p = "S";
        if (p) advance = 2;
        else p = single(c, n);
        size_t count = strlen(p);
        if (count >= capacity - used) {
            out[0] = '\0';
            return FA_LATIN_OVERFLOW;
        }
        memcpy(out + used, p, count);
        used += count;
        out[used] = '\0';
        i += advance;
    }
    return FA_LATIN_CANDIDATE;
needs_context:
    out[0] = '\0';
    return FA_LATIN_NEEDS_CONTEXT;
}
