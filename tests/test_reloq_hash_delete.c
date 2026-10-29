/* Hash ownership at destruction, including large tables and borrowed data. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "evv_arena.h"

extern void *hashNew(int32_t, int32_t, int32_t);
extern int32_t hashDelete(void *, int32_t, int32_t);
extern int32_t hashInsertString(void *, char *, void *);
extern void *hashLookupString(void *, const char *);
extern int32_t hashInsertInt(void *, int32_t, void *);
extern void *hashLookupInt(void *, int32_t);

static int live(const void *p)
{
    uint32_t origin;
    return evv_arena_whence_of(p, &origin);
}

static void strings(size_t count, int owns, int keys, int values)
{
    char *names[300], *data[300], *sentinels[300];
    void *table = hashNew(211, owns, 1);
    assert(table);
    for (size_t i = 0; i < count; ++i) {
        names[i] = malloc(24);
        data[i] = i % 3 ? malloc(24) : NULL;
        sentinels[i] = malloc(17);
        assert(names[i] && (data[i] || !(i % 3)) && sentinels[i]);
        snprintf(names[i], 24, "entry-%u", (unsigned)i);
        if (data[i]) snprintf(data[i], 24, "value-%u", (unsigned)i);
        memset(sentinels[i], 0x5a, 17);
        assert(hashInsertString(table, names[i], data[i]));
    }
    for (size_t i = 0; i < count; ++i)
        assert(hashLookupString(table, names[i]) == data[i]);
    assert(!hashDelete(table, keys, values));
    for (size_t i = 0; i < count; ++i) {
        assert(live(names[i]) == !(owns && keys));
        if (!(owns && keys)) {
            char wanted[24];
            snprintf(wanted, sizeof wanted, "entry-%u", (unsigned)i);
            assert(!strcmp(names[i], wanted));
            free(names[i]);
        }
        if (data[i]) {
            assert(live(data[i]) == !values);
            if (!values) free(data[i]);
        }
        assert(live(sentinels[i]));
        for (size_t j = 0; j < 17; ++j) assert(sentinels[i][j] == 0x5a);
        free(sentinels[i]);
    }
}

static void numbers(int values)
{
    void *data[300], *table = hashNew(211, 0, 1);
    assert(table);
    for (int i = 0; i < 300; ++i) {
        data[i] = malloc(4);
        assert(data[i]);
        memcpy(data[i], &i, 4);
        assert(hashInsertInt(table, i, data[i]));
        assert(hashLookupInt(table, i) == data[i]);
    }
    assert(!hashDelete(table, 1, values)); /* Integer keys are borrowed. */
    for (int i = 0; i < 300; ++i) {
        assert(live(data[i]) == !values);
        if (!values) {assert(!memcmp(data[i], &i, 4));free(data[i]);}
    }
}

int main(void)
{
    for (int owns = 0; owns < 2; ++owns)
        for (int keys = 0; keys < 2; ++keys)
            for (int values = 0; values < 2; ++values) {
                strings(4, owns, keys, values);
                strings(300, owns, keys, values);
            }
    numbers(0);numbers(1);
    assert(!hashDelete(hashNew(0, 1, 0), 1, 1));
    assert(!hashDelete(NULL, 1, 1));
    puts("Hash destruction: string/integer keys, ownership and untouched adjacent data passed");
}
