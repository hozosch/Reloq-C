#include "reloq_acoustic_curve.h"

#include <limits.h>
#include <math.h>
#include <string.h>

#define CHECK(condition) do { if (!(condition)) return __LINE__; } while (0)

int main(void)
{
    static const struct reloq_acoustic_point points[] = {
        {0, 100}, {4, 60}, {4, 20}, {8, 40}
    };
    const struct reloq_acoustic_curve curve = {7.0f, points, 4};
    struct reloq_acoustic_cursor cursor = {0};
    float value = NAN;
    CHECK(reloq_acoustic_begin(&curve, &cursor) == 0);
    static const struct {int64_t tick; float value;} answers[] = {
        {-3, 100}, {-1, 100}, {0, 100}, {1, 90}, {2, 80},
        {3, 70}, {4, 60}, {5, 25}, {6, 30}, {7, 35}, {8, 40},
        {19, 40}
    };
    for (size_t i = 0; i < sizeof answers / sizeof answers[0]; ++i) {
        CHECK(reloq_acoustic_value(&cursor, answers[i].tick, &value) == 0);
        CHECK(value == answers[i].value);
    }
    struct reloq_acoustic_cursor before = cursor;
    float before_value = value;
    CHECK(reloq_acoustic_value(&cursor, 18, &value) == -1 &&
          !memcmp(&before, &cursor, sizeof cursor) && value == before_value);

    static const struct reloq_acoustic_point first_jump[] = {
        {0, 100}, {0, 200}, {0, 300}, {10, 400}
    };
    const struct reloq_acoustic_curve jump = {5, first_jump, 4};
    CHECK(reloq_acoustic_begin(&jump, &cursor) == 0);
    CHECK(reloq_acoustic_value(&cursor, -1, &value) == 0 && value == 100);
    CHECK(reloq_acoustic_value(&cursor, 0, &value) == 0 && value == 100);
    CHECK(reloq_acoustic_value(&cursor, 1, &value) == 0 && value == 310);

    /* Actual pre-Klatt stream targets for the second `Hallo Welt.` array
     * run. The two targets at 162 are a real AV discontinuity; F1 takes a
     * linear 40-tick transition. Expected values are Apple frame callbacks,
     * including the equality frame at the discontinuity. */
    static const struct reloq_acoustic_point apple_av[] = {
        {81, 51}, {162, 49}, {162, 46}, {212, 46}
    };
    static const struct reloq_acoustic_point apple_f1[] = {
        {122, 780}, {162, 400}, {212, 400}
    };
    const struct reloq_acoustic_curve av = {60, apple_av, 4};
    const struct reloq_acoustic_curve f1 = {500, apple_f1, 3};
    const struct {int64_t tick; float av, f1;} apple[] = {
        {160, 49, 419}, {161, 49, 409}, {162, 49, 400},
        {163, 46, 400}, {164, 46, 400}
    };
    struct reloq_acoustic_cursor f1_cursor = {0};
    CHECK(reloq_acoustic_begin(&av, &cursor) == 0);
    CHECK(reloq_acoustic_begin(&f1, &f1_cursor) == 0);
    for (size_t i = 0; i < sizeof apple / sizeof apple[0]; ++i) {
        CHECK(reloq_acoustic_value(&cursor, apple[i].tick, &value) == 0 &&
              value == apple[i].av);
        CHECK(reloq_acoustic_value(&f1_cursor, apple[i].tick, &value) == 0 &&
              value == apple[i].f1);
    }

    static const struct reloq_acoustic_point crossing[] = {
        {0, -9}, {4, 0}, {9, 11}
    };
    const struct reloq_acoustic_curve zero = {0, crossing, 3};
    CHECK(reloq_acoustic_begin(&zero, &cursor) == 0);
    CHECK(reloq_acoustic_value(&cursor, 2, &value) == 0 && value == -4);
    CHECK(reloq_acoustic_value(&cursor, 5, &value) == 0 && value == 2);

    const struct reloq_acoustic_curve empty = {0.75f, NULL, 0};
    CHECK(reloq_acoustic_begin(&empty, &cursor) == 0);
    CHECK(reloq_acoustic_value(&cursor, INT64_MIN, &value) == 0 &&
          value == 0.75f);
    CHECK(reloq_acoustic_value(&cursor, INT64_MAX, &value) == 0 &&
          value == 0.75f);

    static const struct reloq_acoustic_point reverse[] = {{1, 2}, {0, 3}};
    static const struct reloq_acoustic_point span[] = {
        {INT64_MIN, 0}, {INT64_MAX, 1}
    };
    static const struct reloq_acoustic_point delta[] = {
        {0, INT32_MIN}, {1, INT32_MAX}
    };
    before = cursor;
    const struct reloq_acoustic_curve bad[] = {
        {0, reverse, 2}, {0, span, 2}, {0, delta, 2},
        {NAN, NULL, 0}, {0, NULL, 1}
    };
    for (size_t i = 0; i < sizeof bad / sizeof bad[0]; ++i)
        CHECK(reloq_acoustic_begin(&bad[i], &cursor) == -1 &&
              !memcmp(&before, &cursor, sizeof cursor));
    CHECK(reloq_acoustic_begin(NULL, &cursor) == -1);
    CHECK(reloq_acoustic_begin(&empty, NULL) == -1);
    CHECK(reloq_acoustic_value(NULL, 0, &value) == -1);
    CHECK(reloq_acoustic_value(&cursor, 0, NULL) == -1);
    return 0;
}
