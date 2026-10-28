#include "reloq_duration_span.h"

#include <limits.h>
#include <math.h>

static int64_t signed64(uint64_t bits)
{
    return bits <= INT64_MAX ? (int64_t)bits :
        -INT64_C(1) - (int64_t)(UINT64_MAX - bits);
}

static int64_t signed16(uint64_t bits)
{
    uint16_t low = (uint16_t)bits;
    return low <= INT16_MAX ? (int64_t)low : (int64_t)low - INT64_C(65536);
}

static int64_t signed32(uint64_t bits)
{
    uint32_t low = (uint32_t)bits;
    return low <= INT32_MAX ? (int64_t)low : (int64_t)low - INT64_C(4294967296);
}

static int64_t item_value(const struct reloq_duration_item *item, int narrow)
{
    if (item->storage == RELOQ_DURATION_I16)
        return signed16((uint64_t)item->value);
    if (item->storage == RELOQ_DURATION_I64)
        return narrow ? signed32((uint64_t)item->value) : item->value;
    return 0;
}

int64_t reloq_duration_time_sum(const struct reloq_duration_item *items,
                               size_t count)
{
    uint64_t sum = 0;
    if (!items && count) return RELOQ_DURATION_UNDEFINED;
    for (size_t i = 0; i < count; ++i)
        sum += (uint64_t)item_value(items + i, 1);
    return signed64(sum);
}

int64_t reloq_duration_span_query(const struct reloq_duration_item *items,
    size_t count, int same_anchor, int reversed,
    int64_t begin_offset, int begin_partial,
    int64_t end_offset, int end_partial)
{
    if (same_anchor)
        return signed64((uint64_t)end_offset - (uint64_t)begin_offset);
    if (!items && count) return RELOQ_DURATION_UNDEFINED;
    uint64_t sum = 0;
    for (size_t i = 0; i < count; ++i)
        sum += (uint64_t)item_value(items + i, 0);
    if (reversed) {
        if (begin_partial) sum += (uint64_t)begin_offset;
        if (end_partial) sum -= (uint64_t)end_offset;
        sum = UINT64_C(0) - sum;
    } else {
        if (begin_partial) sum -= (uint64_t)begin_offset;
        if (end_partial) sum += (uint64_t)end_offset;
    }
    if (!sum && !count) return RELOQ_DURATION_UNDEFINED;
    return signed64(sum);
}

/* cvttsd2si returns INT64_MIN for a nonfinite/out-of-range operand on the
 * inspected architecture. Preserve that defined outcome without a C cast
 * outside its representable range. No multiply-add contraction is allowed. */
static int64_t truncate64(double value)
{
    if (!isfinite(value) || value >= 9223372036854775808.0 ||
        value < -9223372036854775808.0)
        return INT64_MIN;
    return (int64_t)value;
}

int reloq_duration_redistribute(struct reloq_duration_item *items, size_t count,
                              int64_t old_total, int64_t requested_total)
{
    if (!items && count) return -1;
    for (size_t i = 0; i < count; ++i)
        if (items[i].storage != RELOQ_DURATION_I16 &&
            items[i].storage != RELOQ_DURATION_I64)
            return -1;
    const double factor = old_total ?
        (double)requested_total / (double)old_total : 0.0;
    const double bias = factor >= 0.0 ? 0.5 : -0.5;
    for (size_t i = 0; i < count; ++i) {
        double scaled = (double)item_value(items + i, 0) * factor;
        if (items[i].storage == RELOQ_DURATION_I16) scaled += bias;
        int64_t value = truncate64(scaled);
        items[i].value = items[i].storage == RELOQ_DURATION_I16 ?
            signed16((uint64_t)value) : value;
    }
    return 0;
}

int16_t reloq_duration_scale_transition(int16_t ticks, int16_t percent)
{
    const int32_t product = (int32_t)ticks * (int32_t)percent;
    const int32_t scaled = product / 100;
    int16_t narrowed = (int16_t)signed16((uint64_t)(int64_t)scaled);
    return narrowed > 0 ? narrowed : 1;
}
