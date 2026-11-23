#ifndef RELOQ_UNICODE_H
#define RELOQ_UNICODE_H
#include <stddef.h>
#include <stdint.h>
uint32_t reloq_unicode_compose(uint32_t base,uint32_t combining_mark);
/* Read the standard English Unicode character name as lowercase ASCII.
 * Return 1 for a name, 0 for an unnamed/invalid scalar and -1 for an
 * insufficient buffer. The destination remains unchanged on failure. */
int reloq_unicode_name_english(uint32_t codepoint, char *output, size_t capacity);
/* Short English Greek alphabet name, or 0 when the scalar is not in the
 * shared Greek-letter inventory. Output is unchanged on failure. */
int reloq_unicode_greek_name_english(uint32_t codepoint, char *output, size_t capacity);
/* Whether a scalar can participate in a Unicode/CLDR emoji presentation. */
int reloq_unicode_is_emoji(uint32_t codepoint);
/* A scalar rendered as emoji without requiring an emoji variation selector. */
int reloq_unicode_is_default_emoji(uint32_t codepoint);
/* Complete-utterance admission. Modes are one byte per input byte (0 text,
 * 1 character). Map has length+1 source boundary entries. Output reserves
 * two trailing bytes for the ECI implicit terminal mark and NUL. Language
 * policy: 0 German, 1 US English, 2 native western-language character rules. */
int reloq_unicode_admit(const unsigned char *text,size_t length,
    const unsigned char *modes,int english,int tags,unsigned char **output,
    size_t *written,size_t **map);
int reloq_unicode_admit_language(const unsigned char *text,size_t length,
    const unsigned char *modes,int language,int tags,unsigned char **output,
    size_t *written,size_t **map);
/* Preserve dictionary-owned v/V in normal text. Bits 0 and 1 protect v/V;
 * character-mode names remain independent of lexical overrides. */
int reloq_unicode_admit_language_masked(const unsigned char *text,size_t length,
    const unsigned char *modes,int language,int tags,unsigned protected_v,
    unsigned char **output,size_t *written,size_t **map);
#endif
