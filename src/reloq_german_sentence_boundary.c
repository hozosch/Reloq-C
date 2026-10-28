#include "reloq_german_sentence_boundary.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int letter(const unsigned char *s, size_t length, size_t *at)
{
    size_t p = *at;
    if (p >= length) return 0;
    if ((s[p] >= 'A' && s[p] <= 'Z') ||
        (s[p] >= 'a' && s[p] <= 'z')) {
        *at = p + 1;
        return 1;
    }
    if (s[p] == 0xc3 && p + 1 < length) {
        switch (s[p + 1]) {
        case 0x84: case 0x96: case 0x9c: /* Ä Ö Ü */
        case 0xa4: case 0xb6: case 0xbc: /* ä ö ü */
        case 0x9f: /* ß */
            *at = p + 2;
            return 1;
        }
    }
    return 0;
}

static int uppercase_initial(const unsigned char *s, size_t length,
                             size_t at)
{
    if (at >= length) return 0;
    if (s[at] >= 'A' && s[at] <= 'Z') return 1;
    return s[at] == 0xc3 && at + 1 < length &&
           (s[at + 1] == 0x84 || s[at + 1] == 0x96 ||
            s[at + 1] == 0x9c); /* Ä Ö Ü */
}

/* One transactional scan first counts, then optionally fills the caller's
 * array after capacity has been established. */
static int scan(const unsigned char *utf8, size_t length,
                struct reloq_german_sentence_span *spans, size_t *count,
                reloq_german_cancel_check cancelled, void *cancel_context)
{
    size_t pos = 0, used = 0, last_poll = 0;
    if (cancelled && cancelled(cancel_context)) return -3;
    while (pos < length) {
        if (pos - last_poll >= 256) {
            last_poll = pos;
            if (cancelled && cancelled(cancel_context)) return -3;
        }
        size_t start = pos;
        if (!letter(utf8, length, &pos)) return 0;
        for (;;) {
            if (pos - last_poll >= 256) {
                last_poll = pos;
                if (cancelled && cancelled(cancel_context)) return -3;
            }
            if (!letter(utf8, length, &pos)) break;
        }
        /* A sentence may contain one internal word boundary. Its actual
         * phones, stress and acoustics are checked later by the connected
         * voice preflight; this scanner only identifies an atomic span. */
        if (pos < length && utf8[pos] == ' ' && pos + 1 < length) {
            size_t next = pos + 1;
            if (letter(utf8, length, &next)) {
                pos = next;
                while (letter(utf8, length, &pos)) {}
            }
        }
        if (pos >= length) return 0;
        const unsigned char mark = utf8[pos];
        const int ellipsis = mark == '.' && length - pos >= 3 &&
                             utf8[pos + 1] == '.' && utf8[pos + 2] == '.';
        if (ellipsis) pos += 3;
        else if (mark == '.' || mark == '?' || mark == '!' ||
                 mark == ':' || mark == ';' || mark == ',') ++pos;
        else return 0;
        if (spans) {
            spans[used].offset = start;
            spans[used].length = pos - start;
        }
        ++used;
        if (pos == length) break;
        if (utf8[pos] != ' ' || pos + 1 == length) return 0;
        /* Comma and ellipsis bind the following word into one prosodic
         * phrase even if it starts with an uppercase letter. Question and
         * exclamation split only before uppercase in the checked reference;
         * period, colon and semicolon split with either case. */
        if (mark == ',' || ellipsis ||
            ((mark == '?' || mark == '!') &&
             !uppercase_initial(utf8, length, pos + 1))) return 0;
        ++pos;
    }
    if (cancelled && cancelled(cancel_context)) return -3;
    if (used < 2) return 0;
    *count = used;
    return 1;
}

int reloq_german_sentence_spans(const unsigned char *utf8, size_t length,
                                struct reloq_german_sentence_span *spans,
                                size_t capacity, size_t *count)
{
    size_t needed = 0;
    if ((!utf8 && length) || !count || (!spans && capacity)) return -1;
    if (!length) return 0;
    if (!scan(utf8, length, NULL, &needed, NULL, NULL)) return 0;
    if (spans && capacity < needed) {
        *count = needed;
        return -2;
    }
    if (spans) {
        size_t filled = 0;
        if (scan(utf8, length, spans, &filled, NULL, NULL) != 1 ||
            filled != needed)
            return -1; /* caller must hold input immutable */
    }
    *count = needed;
    return 1;
}

int reloq_german_sentence_spans_checked(
    const unsigned char *utf8, size_t length,
    struct reloq_german_sentence_span *spans, size_t capacity, size_t *count,
    reloq_german_cancel_check cancelled, void *cancel_context)
{
    if (!cancelled)
        return reloq_german_sentence_spans(utf8, length, spans,
                                           capacity, count);
    if ((!utf8 && length) || !count || (!spans && capacity)) return -1;
    size_t needed = 0;
    int status = scan(utf8, length, NULL, &needed,
                      cancelled, cancel_context);
    if (status != 1) return status;
    if (spans && capacity < needed) {
        *count = needed;
        return -2;
    }
    if (!spans) {
        *count = needed;
        return 1;
    }
    if (needed > SIZE_MAX / sizeof *spans) return -4;
    struct reloq_german_sentence_span *temporary =
        malloc(needed * sizeof *temporary);
    if (!temporary) return -4;
    size_t filled = 0;
    status = scan(utf8, length, temporary, &filled,
                  cancelled, cancel_context);
    if (status == 1 && filled == needed) {
        memcpy(spans, temporary, needed * sizeof *spans);
        *count = needed;
    } else if (status == 1) status = -1; /* input must stay immutable */
    free(temporary);
    return status;
}
