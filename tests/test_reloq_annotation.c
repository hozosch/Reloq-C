#include "reloq_annotation.h"
#include <stdint.h>
#include <string.h>

#define CHECK(condition) do { if (!(condition)) return __LINE__; } while (0)

int main(void)
{
    const unsigned char annotated[] =
        "`[.1vA.0sR] `p0 `[.1vA.0sR] `p100 ";
    const unsigned char escaped[] = "a\\`p9 b`p3 c";
    const unsigned char oversized[] = "`p4294967296 ";
    const unsigned char ambiguous[] = "`p4x ";
    struct reloq_annotation_cursor at = {0};
    struct reloq_annotation_event e = {0};
    CHECK(reloq_annotation_next(annotated, sizeof annotated - 1,
                                &at, 200, &e) == 1 &&
          e.kind == RELOQ_ANNOTATION_TEXT &&
          e.offset == 0 && e.length == 12 &&
          at.in_pronunciation == 0);
    CHECK(reloq_annotation_next(annotated, sizeof annotated - 1,
                                &at, 200, &e) == 1 &&
          e.kind == RELOQ_ANNOTATION_EXPLICIT_PAUSE &&
          e.requested_ms == 0 && e.length == 3);
    CHECK(reloq_annotation_next(annotated, sizeof annotated - 1,
                                &at, 200, &e) == 1 &&
          e.kind == RELOQ_ANNOTATION_TEXT);
    CHECK(reloq_annotation_next(annotated, sizeof annotated - 1,
                                &at, 200, &e) == 1 &&
          e.kind == RELOQ_ANNOTATION_EXPLICIT_PAUSE &&
          e.requested_ms == 100 && e.length == 5);
    CHECK(reloq_annotation_next(annotated, sizeof annotated - 1,
                                &at, 200, &e) == 1 && e.length == 1);
    CHECK(reloq_annotation_next(annotated, sizeof annotated - 1,
                                &at, 200, &e) == 0);

    at = (struct reloq_annotation_cursor){0};
    CHECK(reloq_annotation_next(escaped, sizeof escaped - 1,
                                &at, 10, &e) == 1 &&
          e.kind == RELOQ_ANNOTATION_TEXT && e.length == 7);
    CHECK(reloq_annotation_next(escaped, sizeof escaped - 1,
                                &at, 10, &e) == 1 &&
          e.kind == RELOQ_ANNOTATION_EXPLICIT_PAUSE &&
          e.requested_ms == 3);

    at = (struct reloq_annotation_cursor){0};
    CHECK(reloq_annotation_next(oversized, sizeof oversized - 1,
                                &at, UINT32_MAX, &e) == -1 && at.offset == 0);
    CHECK(reloq_annotation_next(ambiguous, sizeof ambiguous - 1,
                                &at, 10, &e) == -1 && at.offset == 0);
    CHECK(reloq_annotation_next((const unsigned char *)"`p11 ", 5,
                                &at, 10, &e) == -1 && at.offset == 0);
    at = (struct reloq_annotation_cursor){0};
    const unsigned char inside[] = "`[a`p7] `p3 ";
    CHECK(reloq_annotation_next(inside, sizeof inside - 1,
                                &at, 10, &e) == 1 &&
          e.kind == RELOQ_ANNOTATION_TEXT && e.length == 8 &&
          at.in_pronunciation == 0);
    CHECK(reloq_annotation_next(inside, sizeof inside - 1,
                                &at, 10, &e) == 1 &&
          e.kind == RELOQ_ANNOTATION_EXPLICIT_PAUSE &&
          e.requested_ms == 3);
    CHECK(reloq_annotation_next(NULL, 0, &at, 10, &e) == -1);
    return 0;
}
