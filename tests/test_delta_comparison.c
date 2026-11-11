/* A C library is allowed to return any signed magnitude from memcmp.
 * Reproduce x86-64 bionic's +/-768 and verify both equality and ordering. */
#include "delta.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static _Alignas(delta_vars) unsigned char arena[sizeof(delta_vars) + 16];
unsigned char *evv_arena_base = arena;
size_t evv_arena_size = sizeof arena;
__thread const delta_language *delta_lang_current;
void delta_lang_none(void) { abort(); }

int __wrap_memcmp(const void *a, const void *b, size_t n)
{
    const unsigned char *left=a, *right=b;
    for (size_t i=0; i<n; ++i)
        if (left[i] != right[i])
            return left[i] > right[i] ? 768 : -768;
    return 0;
}

int main(void)
{
    delta_stmt types[1] = {{.length=1}};
    delta_language language = {.stmtbl=types};
    delta_lang_current=&language;
    delta_state state={0};
    state.vars=16;
    unsigned char first=0x60, second=0x5d;
    delta_operand a={.ptr=&first,.kind=0}, b={.ptr=&second,.kind=0};

    vcompare(&state,&a,&a);
    assert(testeq(&state)==0 && testgt(&state)!=0 && testlt(&state)!=0);
    vcompare(&state,&a,&b);
    assert(testeq(&state)!=0 && testgt(&state)==0 && testlt(&state)!=0);
    vcompare(&state,&b,&a);
    assert(testeq(&state)!=0 && testgt(&state)!=0 && testlt(&state)==0);
    puts("Delta comparison: large positive and negative memcmp results passed");
    return 0;
}
