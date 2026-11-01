#ifndef RELOQ_PARAMETER_MARKS_H
#define RELOQ_PARAMETER_MARKS_H
#include <stddef.h>

/* Queue metadata in the caller's current source coordinate system. Only
 * speed (0..250) and volume (0..100) are accepted. Public input never
 * supplies these records. */
struct reloq_parameter_mark { size_t offset; int kind,value; };

/* Serialize trusted controls into an already admitted native rule stream.
 * This is below Unicode/text admission: it cannot enable caller annotations
 * or split AddText pieces into separate utterances. map[i] gives the old
 * stream position of output byte i, including the terminal byte. */
int reloq_parameter_marks_insert(const unsigned char *,size_t,
    const struct reloq_parameter_mark *,size_t,unsigned char **,size_t *,size_t **);
#endif
