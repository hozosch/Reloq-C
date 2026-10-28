#ifndef RELOQ_GERMAN_LEXICON_FILE_H
#define RELOQ_GERMAN_LEXICON_FILE_H

#include <stddef.h>
#include <stdio.h>

#include "reloq_lexicon.h"

/* Owned lexicon view loaded from the portable RELOQ-DE-SPR-1 format.
 * Zero-initialize before first use. The view can be passed directly to
 * reloq_german_text_annotate_checked; keep it alive throughout annotation.
 * Loading is transactional: on error an earlier valid lexicon stays active.
 * This is word-to-SPR data only; it does not create acoustic trajectories.
 */
struct reloq_german_lexicon_file {
    struct reloq_lexicon view;
    struct reloq_lexicon_override *rows;
    size_t row_count;
    /* Keys and SPR share one owned byte pool. Individual row pointers are
     * views into it, never separately allocated or freed. */
    unsigned char *storage;
};

/* 0 on success; -1 invalid file, I/O or allocation failure. All input rows
 * must be unique and strictly sorted by their CP1252 word byte sequence.
 * Files are limited to 16 MiB, 100,000 entries, 127-byte German words and
 * 4,096-byte complete German SPR groups. The FILE stays caller-owned. */
int reloq_german_lexicon_file_load_stream(struct reloq_german_lexicon_file *owned,
                                          FILE *stream);
int reloq_german_lexicon_file_load_path(struct reloq_german_lexicon_file *owned,
                                        const char *path);
void reloq_german_lexicon_file_clear(struct reloq_german_lexicon_file *owned);

#endif
