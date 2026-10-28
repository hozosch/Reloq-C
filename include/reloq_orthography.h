#ifndef RELOQ_ORTHOGRAPHY_H
#define RELOQ_ORTHOGRAPHY_H
#include <stddef.h>

/* A word after case folding, before morphological or phonetic analysis.
 * Spelling uses CP1252 here, as does the existing German lexical frontend.
 * The native record/alphabet representation belongs to its adapter only. */
struct reloq_orthographic_letter { unsigned char spelling, vowel; };
enum reloq_orthographic_edit {
    RELOQ_ORTHOGRAPHY_KEEP,
    RELOQ_ORTHOGRAPHY_REPLACE,
    RELOQ_ORTHOGRAPHY_DELETE,
    RELOQ_ORTHOGRAPHY_CONSONANT
};
struct reloq_orthographic_change { unsigned char action, spelling; };
/* Plan each original letter once. Endings are tested within this word,
 * never across a word boundary. Returns the number of changes. */
size_t reloq_german_orthography(const struct reloq_orthographic_letter *,
    size_t,struct reloq_orthographic_change *);
#endif
