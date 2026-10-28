#include "reloq_german_lexicon_file.h"

#include "reloq_german_lexicon.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILE_BYTES (16u * 1024u * 1024u)
#define MAX_ROWS 100000u
#define MAX_KEY 127u
#define MAX_SPR 4096u
#define MAX_LINE_BYTES 16383u

void reloq_german_lexicon_file_clear(struct reloq_german_lexicon_file *owned)
{
    if (!owned) return;
    free(owned->rows);
    free(owned->storage);
    memset(owned, 0, sizeof *owned);
}

static int hex_digit(unsigned char c)
{
    if (c >= '0' && c <= '9') return (int)(c - '0');
    if (c >= 'A' && c <= 'F') return (int)(c - 'A') + 10;
    return -1;
}

static int decode(const unsigned char *start, size_t hex_length,
                   size_t limit, unsigned char *decoded)
{
    if (!hex_length || (hex_length & 1u) || hex_length / 2u > limit)
        return -1;
    size_t length = hex_length / 2u;
    for (size_t i = 0; i < length; ++i) {
        int high = hex_digit(start[2u * i]);
        int low = hex_digit(start[2u * i + 1u]);
        if (high < 0 || low < 0) return -1;
        decoded[i] = (unsigned char)(16 * high + low);
    }
    decoded[length] = 0;
    return 0;
}

static int word_letter(unsigned char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           c == 0xc4 || c == 0xd6 || c == 0xdc || c == 0xe4 ||
           c == 0xf6 || c == 0xfc || c == 0xdf;
}

static int parse_row(const unsigned char *line, size_t length,
                     struct reloq_lexicon_override *row,
                     unsigned char *storage, size_t *used)
{
    const unsigned char *tab = memchr(line, '\t', length);
    if (!tab) return -1;
    size_t first = (size_t)(tab - line);
    size_t second = length - first - 1u;
    if (memchr(tab + 1, '\t', second)) return -1;
    unsigned char *key = storage + *used;
    if (decode(line, first, MAX_KEY, key)) return -1;
    unsigned char *spr = key + first / 2u + 1u;
    if (decode(tab + 1, second, MAX_SPR, spr)) return -1;
    for (size_t i = 0; i < first / 2u; ++i)
        if (!word_letter(key[i])) return -1;
    /* Full phone/stress validation happens once when binding the view. */
    if (memchr(spr, 0, second / 2u)) return -1;
    row->key = key;
    row->key_length = first / 2u;
    row->spr = spr;
    row->spr_length = second / 2u;
    *used += first / 2u + second / 2u + 2u;
    return 0;
}

/* Sequential streams are supported. Large dictionaries require two owned
 * blocks after loading, rather than two heap allocations per word. The
 * temporary hex input is released before the new view becomes visible. */
static unsigned char *read_bytes(FILE *stream, size_t *length)
{
    size_t used = 0, capacity = 65536u;
    unsigned char *bytes = malloc(capacity);
    if (!bytes) return NULL;
    for (;;) {
        if (used == capacity) {
            if (capacity == MAX_FILE_BYTES) {
                if (fgetc(stream) != EOF || ferror(stream)) goto error;
                break;
            }
            size_t next = capacity * 2u;
            if (next > MAX_FILE_BYTES) next = MAX_FILE_BYTES;
            unsigned char *grown = realloc(bytes, next);
            if (!grown) goto error;
            bytes = grown;
            capacity = next;
        }
        size_t count = fread(bytes + used, 1, capacity - used, stream);
        used += count;
        if (!count) {
            if (ferror(stream) || !feof(stream)) goto error;
            break;
        }
    }
    *length = used;
    return bytes;
error:
    free(bytes);
    return NULL;
}

int reloq_german_lexicon_file_load_stream(struct reloq_german_lexicon_file *owned,
                                          FILE *stream)
{
    if (!owned || !stream) return -1;
    static const unsigned char header[] = "RELOQ-DE-SPR-1\n";
    size_t bytes = 0;
    unsigned char *input = read_bytes(stream, &bytes);
    if (!input) return -1;
    struct reloq_lexicon_override *rows = NULL;
    unsigned char *storage = NULL;
    size_t start = sizeof header - 1u, count = 0, used = 0;
    if (bytes < start || memcmp(input, header, start) ||
        input[bytes - 1u] != '\n') goto error;
    for (size_t i = start; i < bytes; ++i)
        count += input[i] == '\n';
    if (count > MAX_ROWS) goto error;
    if (count) {
        rows = calloc(count, sizeof *rows);
        storage = malloc(bytes / 2u + 2u * count);
        if (!rows || !storage) goto error;
    }
    for (size_t i = 0; i < count; ++i) {
        const unsigned char *end = memchr(input + start, '\n', bytes - start);
        if (!end) goto error;
        size_t length = (size_t)(end - (input + start));
        if (length < 4u || length > MAX_LINE_BYTES ||
            parse_row(input + start, length, &rows[i], storage, &used))
            goto error;
        start += length + 1u;
    }
    struct reloq_lexicon view = {0};
    if (reloq_lexicon_bind(&view, NULL, 0, rows, count,
                           reloq_german_lexicon_validate_spr, NULL))
        goto error;
    free(input);
    reloq_german_lexicon_file_clear(owned);
    owned->rows = rows;
    owned->row_count = count;
    owned->view = view;
    owned->storage = storage;
    return 0;
error:
    free(input);
    free(rows);
    free(storage);
    return -1;
}

int reloq_german_lexicon_file_load_path(struct reloq_german_lexicon_file *owned,
                                        const char *path)
{
    if (!owned || !path) return -1;
    FILE *stream = fopen(path, "rb");
    if (!stream) return -1;
    struct reloq_german_lexicon_file loaded = {0};
    int result = reloq_german_lexicon_file_load_stream(&loaded, stream);
    int closed = fclose(stream);
    if (result || closed) {
        reloq_german_lexicon_file_clear(&loaded);
        return -1;
    }
    reloq_german_lexicon_file_clear(owned);
    *owned = loaded;
    return 0;
}
