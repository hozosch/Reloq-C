#ifndef RELOQ_PERSIAN_NUMBERS_H
#define RELOQ_PERSIAN_NUMBERS_H

#include <stddef.h>
#include <stdint.h>

/* Linguistic number units, independent of a phoneme or acoustic backend. */
enum reloq_fa_number_kind {
    RELOQ_FA_NUMBER_GROUP = 1,    /* value: cardinal group 0..999 */
    RELOQ_FA_NUMBER_SCALE = 2,    /* value: thousand=0, million=1, milliard=2 */
    RELOQ_FA_NUMBER_DECIMAL = 3,  /* Persian decimal or version separator */
    RELOQ_FA_NUMBER_SIGN = 4      /* value: 0 minus, 1 plus */
};

struct reloq_fa_number_event {
    uint32_t kind;
    uint32_t value;
    uint32_t linked;       /* scale links to a following nonzero group */
    uint32_t weaken;       /* 0 none, 1 weak, 2 secondary */
    uint32_t final_stress; /* put lexical stress on the final syllable */
};

#endif
