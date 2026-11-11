#include "nvda_dictionary.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

enum { MAX_DICTIONARY_BYTES = 8 * 1024 * 1024 };

static int valid_utf8(const unsigned char *s, size_t n)
{
    size_t i = 0;
    while (i < n) {
        unsigned c = s[i++], extra, code, minimum;
        if (!c) return 0;
        if (c < 0x80) continue;
        if (c >= 0xc2 && c <= 0xdf) { extra = 1; code = c & 0x1f; minimum = 0x80; }
        else if (c >= 0xe0 && c <= 0xef) { extra = 2; code = c & 0x0f; minimum = 0x800; }
        else if (c >= 0xf0 && c <= 0xf4) { extra = 3; code = c & 0x07; minimum = 0x10000; }
        else return 0;
        if (extra > n - i) return 0;
        for (unsigned j = 0; j < extra; ++j) {
            unsigned next = s[i++];
            if ((next & 0xc0) != 0x80) return 0;
            code = (code << 6) | (next & 0x3f);
        }
        if (code < minimum || code > 0x10ffff || (code >= 0xd800 && code <= 0xdfff)) return 0;
    }
    return 1;
}

static int blank(const unsigned char *p, size_t n)
{
    for (size_t i = 0; i < n; ++i)
        if (p[i] != ' ' && p[i] != '\t' && p[i] != '\v' && p[i] != '\f') return 0;
    return 1;
}

/* NVDA escapes a literal # as backslash-hash in both text columns. */
static size_t unescape(char *out, const unsigned char *in, size_t length)
{
    size_t written = 0;
    for (size_t i = 0; i < length; ++i) {
        if (in[i] == '\\' && i + 1 < length && in[i + 1] == '#') ++i;
        out[written++] = (char)in[i];
    }
    out[written] = 0;
    return written;
}

int reloq_nvda_parse(const unsigned char *data, size_t length,
                     reloq_nvda_entry_fn callback, void *context, size_t *count)
{
    if (count) *count = 0;
    /* Do not deliver any entries from a malformed or mixed-format file. */
    if (callback) {
        int checked = reloq_nvda_parse(data, length, NULL, NULL, NULL);
        if (checked != RELOQ_NVDA_VALID) return checked;
    }
    if ((!data && length) || length > MAX_DICTIONARY_BYTES || !valid_utf8(data, length))
        return RELOQ_NVDA_INVALID;
    size_t offset = length >= 3 && !memcmp(data, "\xef\xbb\xbf", 3) ? 3 : 0;
    size_t entries = 0;
    while (offset < length) {
        size_t end = offset;
        while (end < length && data[end] != '\r' && data[end] != '\n') ++end;
        size_t line_length = end - offset;
        if (line_length && data[offset] != '#' && !blank(data + offset, line_length)) {
            size_t tabs[3], tab_count = 0;
            for (size_t i = offset; i < end; ++i) {
                if (data[i] == '\t') {
                    if (tab_count < 3) tabs[tab_count] = i;
                    ++tab_count;
                }
            }
            if (tab_count != 3 || tabs[1] + 2 != tabs[2] ||
                (data[tabs[1] + 1] != '0' && data[tabs[1] + 1] != '1') ||
                tabs[2] + 2 != end || data[tabs[2] + 1] < '0' || data[tabs[2] + 1] > '6')
                return RELOQ_NVDA_INVALID;
            if (callback) {
                size_t pattern_length = tabs[0] - offset;
                size_t replacement_length = tabs[1] - tabs[0] - 1;
                if (pattern_length > SIZE_MAX - replacement_length - 2) return RELOQ_NVDA_MEMORY;
                char *strings = malloc(pattern_length + replacement_length + 2);
                if (!strings) return RELOQ_NVDA_MEMORY;
                char *replacement = strings + pattern_length + 1;
                struct reloq_nvda_entry entry = {
                    strings, replacement,
                    unescape(strings, data + offset, pattern_length),
                    unescape(replacement, data + tabs[0] + 1, replacement_length),
                    (unsigned)(data[tabs[1] + 1] - '0'),
                    (unsigned)(data[tabs[2] + 1] - '0')
                };
                int accepted = callback(context, &entry);
                free(strings);
                if (!accepted) return RELOQ_NVDA_CALLBACK;
            }
            ++entries;
        }
        offset = end;
        if (offset < length && data[offset] == '\r') ++offset;
        if (offset < length && data[offset] == '\n') ++offset;
    }
    if (count) *count = entries;
    return RELOQ_NVDA_VALID;
}
