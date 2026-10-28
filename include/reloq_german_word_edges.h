#ifndef RELOQ_GERMAN_WORD_EDGES_H
#define RELOQ_GERMAN_WORD_EDGES_H

#include <stdbool.h>
#include <stddef.h>

/* Spelling runs contain consonants only, in reading order. The caller
 * supplies the letter categories and respects the word/stem boundary.
 * Six stored letters suffice: longer root codas are always rejected;
 * the two productive onset prefixes need only their first two letters. */
struct reloq_german_consonants {
    unsigned char spelling[6];
    size_t length;
};

enum reloq_german_ending {
    RELOQ_GERMAN_WORD_END,
    RELOQ_GERMAN_DENTAL_END,
    RELOQ_GERMAN_SONORANT_END
};

bool reloq_german_onset(const struct reloq_german_consonants *run);
bool reloq_german_root_coda(const struct reloq_german_consonants *run);

/* A removed ending can expose a different stem boundary. Consequently
 * shortened is its own run, rather than an assumed substring of root. */
bool reloq_german_extended_coda(const struct reloq_german_consonants *root,
    const struct reloq_german_consonants *shortened,
    unsigned char last, unsigned char previous, bool has_previous,
    enum reloq_german_ending ending);

#endif
