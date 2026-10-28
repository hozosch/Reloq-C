#ifndef RELOQ_GERMAN_LIQUID_TRACT_H
#define RELOQ_GERMAN_LIQUID_TRACT_H
#include "reloq_german_obstruent_tract.h"

/* Independently observed r/l gestures. The shared sparse endpoint carrier
 * is also used by obstruents; no liquid-specific emission machinery is needed.
 * The user's retained r after long oe deliberately returns zero. */
int reloq_german_liquid_tract_plan(const struct reloq_german_phone_node *node,
    int phrase_initial, int phrase_final, struct reloq_german_obstruent_tract *out);
int reloq_german_j_av(const struct reloq_german_phone_node *node);
#endif
