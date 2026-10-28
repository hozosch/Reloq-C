#include "reloq_german_boundary_profile.h"

#include "reloq_german_connected_kp.h"
#include "reloq_german_connected_pt.h"

#include <stddef.h>

static const struct reloq_german_boundary_profile measured[] = {
    {
        RELOQ_DE_PHONE_k, RELOQ_DE_PHONE_t, 20, RELOQ_TARGET_A5F,
        reloq_german_connected_kt_onset_landmarks,
        reloq_german_connected_kt_coda_append,
        reloq_german_connected_kt_onset_append
    },
    {
        RELOQ_DE_PHONE_k, RELOQ_DE_PHONE_p, 1, RELOQ_TARGET_AB,
        reloq_german_connected_kp_onset_landmarks,
        reloq_german_connected_kp_coda_append,
        reloq_german_connected_kp_onset_append
    },
    {
        RELOQ_DE_PHONE_p, RELOQ_DE_PHONE_t, 1, RELOQ_TARGET_A5F,
        reloq_german_connected_pt_onset_landmarks,
        reloq_german_connected_pt_coda_append,
        reloq_german_connected_pt_onset_append
    }
};

const struct reloq_german_boundary_profile *
reloq_german_boundary_profile_find(enum reloq_german_phone first_coda,
                                   enum reloq_german_phone second_initial)
{
    for (size_t i = 0; i < sizeof measured / sizeof measured[0]; ++i)
        if (measured[i].first_coda == first_coda &&
            measured[i].second_initial == second_initial)
            return &measured[i];
    return NULL;
}
