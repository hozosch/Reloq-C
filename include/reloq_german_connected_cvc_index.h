#ifndef RELOQ_GERMAN_CONNECTED_CVC_INDEX_H
#define RELOQ_GERMAN_CONNECTED_CVC_INDEX_H

#include <stddef.h>

/* Exact public SPR callback agreement is certified only for measured
 * word/carrier/position pairs in the finite German CVC matrix. The carriers
 * are Pack, Hut and App; additionally 19 /k/→/t/ and 10 /k/→/p/ holdout
 * pairs were captured individually. Their orthography is matched byte for
 * byte. The
 * result is 1 if the observed whole callback equals two independently
 * observed `.1` SPR groups separated by a single `2 numeric control,
 * 0 for every other pair, or -1 for invalid arguments. This is phonetic
 * coverage, not a guarantee of acoustic fidelity. */
int reloq_german_connected_cvc_reference_pair(
    const unsigned char *left, size_t left_length,
    const unsigned char *right, size_t right_length);

#endif
