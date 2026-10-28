#ifndef RELOQ_UNICODE_H
#define RELOQ_UNICODE_H
#include <stddef.h>
#include <stdint.h>
uint32_t reloq_unicode_compose(uint32_t base,uint32_t combining_mark);
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
#endif
