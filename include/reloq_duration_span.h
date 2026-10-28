#ifndef RELOQ_DURATION_SPAN_H
#define RELOQ_DURATION_SPAN_H

#include <stddef.h>
#include <stdint.h>

/* Typed intermediate for the original duration-stage semantics. The caller
 * supplies ordinary stream items in physical order; synchronization markers
 * are topology and contribute neither a value nor an item. There is no
 * reference-module memory layout or runtime dependency in this API. */
enum reloq_duration_storage {
    RELOQ_DURATION_NONNUMERIC = 0,
    RELOQ_DURATION_I16 = 1,
    RELOQ_DURATION_I64 = 2
};

struct reloq_duration_item {
    int64_t value;
    enum reloq_duration_storage storage;
};

#define RELOQ_DURATION_UNDEFINED INT64_C(-2147483647)

/* Sum for timeDuration: 64-bit fields are reduced to signed 32 bits before
 * being accumulated. Query below deliberately retains their full width. */
int64_t reloq_duration_time_sum(const struct reloq_duration_item *items,
                               size_t count);

/* Query the interval between the argument positions. For different anchors,
 * items describe the physically ordered interval, and reversed says the
 * first argument is its right end. A partial offset is used only when its
 * flag is set. Equal anchors return end_offset - begin_offset regardless
 * of flags, as the original duration primitive does. A marker-only empty
 * zero interval is undefined; a measured zero item is a defined zero. */
int64_t reloq_duration_span_query(const struct reloq_duration_item *items,
    size_t count, int same_anchor, int reversed,
    int64_t begin_offset, int begin_partial,
    int64_t end_offset, int end_partial);

/* Candidate interval redistribution arithmetic, separated from stream writes.
 * Locally tested; full reference-operation substitution is still pending.
 * The denominator is a duration query result. Short fields receive a signed
 * half-unit bias and narrow after truncation; wide fields receive no bias.
 * Rounding each field independently does not guarantee an exact new total. */
int reloq_duration_redistribute(struct reloq_duration_item *items, size_t count,
                              int64_t old_total, int64_t requested_total);

/* Candidate scaler: full reference-operation substitution is still pending.
 * The transition scaler narrows to a short before applying its minimum of
 * one tick. This is distinct from the wider interval redistribution. */
int16_t reloq_duration_scale_transition(int16_t ticks, int16_t percent);

#endif
