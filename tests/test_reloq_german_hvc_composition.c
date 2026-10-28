#include "reloq_german_hv_timing.h"
#include "reloq_german_terminal_source.h"
#include "reloq_german_vowel_nucleus.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(x) do { if (!(x)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); \
    return -1; \
} } while (0)

struct composition {
    struct reloq_german_phone_node phones[4];
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][16];
    struct reloq_target_builder builder;
    int64_t onset, closure, release;
};

static int compose(struct composition *c, const char *annotation)
{
    struct reloq_german_phone_sequence_info info = {0};
    struct reloq_target_buffer storage[RELOQ_TARGET_STREAM_COUNT];
    int64_t relative_closure;
    memset(c, 0, sizeof *c);
    if (reloq_german_phone_sequence_build(
            (const unsigned char *)annotation, strlen(annotation), 0,
            c->phones, 4, &info) || info.phone_count != 3) return -1;
    if (reloq_german_hv_acoustic_anchor(c->phones, 3, &c->onset) ||
        reloq_german_hvc_f1_target_anchor(c->phones, 3, &c->closure))
        return -1;
    for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s)
        storage[s] = (struct reloq_target_buffer){c->points[s], 16, 0};
    if (reloq_target_builder_begin(&c->builder, storage) ||
        reloq_german_vowel_nucleus_append(
            c->phones, 3, 1, c->onset, 1, &c->builder,
            &relative_closure) ||
        c->onset + relative_closure != c->closure ||
        reloq_german_terminal_source_append(
            c->phones, 3, 1, c->closure, &c->builder, &c->release))
        return -1;
    return 0;
}

/* A repeated tick can encode an intentional jump (AV closure; AH source
 * change). Identical consecutive tick/value pairs would instead duplicate
 * the same instruction when independently authored gestures are composed. */
static int check_order(const struct composition *c)
{
    for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s) {
        const struct reloq_target_buffer *stream = &c->builder.stream[s];
        for (size_t i = 1; i < stream->count; ++i) {
            CHECK(stream->points[i - 1].tick <= stream->points[i].tick);
            CHECK(stream->points[i - 1].tick != stream->points[i].tick ||
                  stream->points[i - 1].value != stream->points[i].value);
        }
    }
    const struct reloq_target_buffer *av =
        &c->builder.stream[RELOQ_TARGET_AV];
    const struct reloq_target_buffer *ah =
        &c->builder.stream[RELOQ_TARGET_AH];
    CHECK(av->count == 5 && ah->count == 7);
    CHECK(av->points[1].tick == c->closure &&
          av->points[2].tick == c->closure &&
          av->points[1].value > 0 && av->points[2].value == 0);
    CHECK(av->points[3].tick == c->release &&
          av->points[3].value == 0);
    CHECK(ah->points[1].tick == c->closure &&
          ah->points[2].tick == c->closure &&
          ah->points[1].value == 34 && ah->points[2].value == 0);
    CHECK(ah->points[3].tick == c->release &&
          ah->points[4].tick == c->release &&
          ah->points[3].value == 0 && ah->points[4].value > 0);
    return 0;
}

static void dump(const struct composition *c)
{
    printf("onset=%" PRId64 " closure=%" PRId64 " release=%" PRId64,
           c->onset, c->closure, c->release);
    for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s) {
        const struct reloq_target_buffer *stream = &c->builder.stream[s];
        if (!stream->count) continue;
        printf(" %s=", reloq_target_stream_name((enum reloq_target_stream)s));
        for (size_t i = 0; i < stream->count; ++i)
            printf("%s%" PRId64 ":%" PRId32, i ? "," : "",
                   stream->points[i].tick, stream->points[i].value);
    }
    puts("");
}

int main(int argc, char **argv)
{
    struct composition c;
    if (argc == 2 && strcmp(argv[1], "--self-test") == 0) {
        for (const char *v = "AEIOUaeiou"; *v; ++v)
            for (const char *stop = "ptk"; *stop; ++stop) {
                char annotation[] = "`[.1hAp].";
                annotation[5] = *v;
                annotation[6] = *stop;
                CHECK(compose(&c, annotation) == 0);
                CHECK(check_order(&c) == 0);
            }
        return EXIT_SUCCESS;
    }
    if (argc == 2 && compose(&c, argv[1]) == 0 &&
        check_order(&c) == 0) {
        dump(&c);
        return EXIT_SUCCESS;
    }
    fprintf(stderr, "usage: %s --self-test | SPR\n", argv[0]);
    return EXIT_FAILURE;
}
