#include "reloq_german_lexicon_file.h"

#include "reloq_german_text.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "line %d: %s\n", __LINE__, #condition); \
        exit(1); \
    } \
} while (0)

static void hex(FILE *stream, const unsigned char *input, size_t count)
{
    static const char digits[] = "0123456789ABCDEF";
    for (size_t i = 0; i < count; ++i) {
        CHECK(fputc(digits[input[i] >> 4], stream) != EOF);
        CHECK(fputc(digits[input[i] & 15], stream) != EOF);
    }
}

static void row(FILE *stream, const unsigned char *word, size_t word_length,
                const char *spr)
{
    hex(stream, word, word_length);
    CHECK(fputc('\t', stream) != EOF);
    hex(stream, (const unsigned char *)spr, strlen(spr));
    CHECK(fputc('\n', stream) != EOF);
}

static FILE *new_file(void)
{
    FILE *stream = tmpfile();
    CHECK(stream != NULL);
    CHECK(fputs("RELOQ-DE-SPR-1\n", stream) >= 0);
    return stream;
}

static int read_file(struct reloq_german_lexicon_file *owned, FILE *stream)
{
    CHECK(fflush(stream) == 0);
    rewind(stream);
    int result = reloq_german_lexicon_file_load_stream(owned, stream);
    CHECK(fclose(stream) == 0);
    return result;
}

static void expect_annotation(const struct reloq_lexicon *view,
                              const char *input, const char *expected)
{
    unsigned char output[512];
    size_t length = 0, offset = 99;
    CHECK(reloq_german_text_annotate_checked(
        (const unsigned char *)input, strlen(input), output, sizeof output,
        &length, &offset, view, NULL, NULL) == RELOQ_DE_TEXT_OK);
    CHECK(length == strlen(expected));
    CHECK(strcmp((const char *)output, expected) == 0);
    CHECK(offset == 99);
}

int main(void)
{
    struct reloq_german_lexicon_file owned = {0};
    CHECK(reloq_german_lexicon_file_load_path(
        &owned, "data/german_apple_observed.rlex") == 0);
    CHECK(owned.row_count == 1598);
    expect_annotation(&owned.view, "auch auf aus.",
                      "`[.1'aw'x] `[.1'aw'f] `[.1'aw's].");
    expect_annotation(&owned.view, "au\xC3\x9F" "er.", "`[.1'aw'.0sR].");
    expect_annotation(&owned.view, "konsonantenstau.",
                      "`[.2kOn.0zo.1nAn.0t@n.2St'aw'].");

    /* Loading explicit records connects to the existing annotation API,
     * while unrelated CVC words continue through their established path. */
    FILE *stream = new_file();
    row(stream, (const unsigned char *)"Bett", 4, "`[.1bet]");
    row(stream, (const unsigned char *)"Zug", 3, "`[.1Tuk]");
    CHECK(read_file(&owned, stream) == 0);
    CHECK(owned.row_count == 2);
    expect_annotation(&owned.view, "Bett. Mett.",
                      "`[.1bet]. `[.1mEt].");

    /* Bad replacements must leave the prior valid view usable. */
    stream = new_file();
    row(stream, (const unsigned char *)"Zug", 3, "`[.1Tuk]");
    row(stream, (const unsigned char *)"Bett", 4, "`[.1bet]");
    CHECK(read_file(&owned, stream) == -1);
    expect_annotation(&owned.view, "Bett.", "`[.1bet].");

    stream = new_file();
    row(stream, (const unsigned char *)"Bett", 4, "`[.1bet]");
    row(stream, (const unsigned char *)"Bett", 4, "`[.1bet]");
    CHECK(read_file(&owned, stream) == -1);

    stream = new_file();
    row(stream, (const unsigned char *)"Bett", 4, "`[.1bet");
    CHECK(read_file(&owned, stream) == -1);

    stream = new_file();
    CHECK(fputs("424Z\t605B2E316245745D\n", stream) >= 0);
    CHECK(read_file(&owned, stream) == -1);

    stream = new_file();
    row(stream, (const unsigned char *)"Bett!", 5, "`[.1bet]");
    CHECK(read_file(&owned, stream) == -1);

    stream = new_file();
    row(stream, (const unsigned char *)"Bett", 4, "`[.1bet]");
    CHECK(fputs("00", stream) >= 0); /* Missing final newline. */
    CHECK(read_file(&owned, stream) == -1);
    expect_annotation(&owned.view, "Bett.", "`[.1bet].");

    /* Embedded binary bytes cannot hide the rest of a malformed record. */
    stream = new_file();
    CHECK(fputs("42657474\t605B2E316265745D", stream) >= 0);
    CHECK(fputc(0, stream) == 0);
    CHECK(fputc('\n', stream) == '\n');
    CHECK(read_file(&owned, stream) == -1);
    expect_annotation(&owned.view, "Bett.", "`[.1bet].");

    /* An intentionally empty replacement clears every earlier row. */
    stream = new_file();
    CHECK(read_file(&owned, stream) == 0);
    CHECK(owned.row_count == 0 && owned.rows == NULL && owned.storage == NULL);

    reloq_german_lexicon_file_clear(&owned);
    CHECK(owned.rows == NULL && owned.view.overrides == NULL && owned.storage == NULL);
    puts("German lexicon file and text adapter: PASS");
    return 0;
}
