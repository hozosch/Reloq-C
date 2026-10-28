#include "reloq_german_hv_timing.h"

#include <stddef.h>

#define CHECK(condition) do { if (!(condition)) return __LINE__; } while (0)

int main(void)
{
    const enum reloq_german_phone vowels[] = {
        RELOQ_DE_PHONE_A, RELOQ_DE_PHONE_E, RELOQ_DE_PHONE_I,
        RELOQ_DE_PHONE_O, RELOQ_DE_PHONE_U, RELOQ_DE_PHONE_a,
        RELOQ_DE_PHONE_e, RELOQ_DE_PHONE_i, RELOQ_DE_PHONE_o,
        RELOQ_DE_PHONE_u
    };
    const enum reloq_german_phone codas[] = {
        RELOQ_DE_PHONE_p, RELOQ_DE_PHONE_t, RELOQ_DE_PHONE_k
    };
    const int32_t expected_relative[2][10] = {
        {97, 97, 91, 91, 78, 163, 169, 156, 169, 156},
        {97, 105, 98, 98, 84, 168, 182, 168, 182, 168}
    };
    const int32_t expected_lead[10][3] = {
        {30, 40, 40}, {25, 25, 25}, {25, 25, 25},
        {30, 50, 40}, {30, 40, 40}, {30, 50, 20},
        {25, 25, 25}, {25, 25, 25}, {30, 40, 40},
        {30, 40, 40}
    };
    struct reloq_german_phone_node phones[3] = {{0}};
    int64_t tick = -9;
    phones[0].phone = RELOQ_DE_PHONE_h;
    for (int stress = 0; stress <= 2; ++stress) {
        for (size_t v = 0; v < sizeof vowels / sizeof vowels[0]; ++v) {
            phones[1].phone = vowels[v];
            for (size_t size = 2; size <= 3; ++size) {
                for (size_t c = 0; c < sizeof codas / sizeof codas[0]; ++c) {
                    phones[2].phone = codas[c];
                    for (size_t i = 0; i < size; ++i) {
                        phones[i].stress = (uint8_t)stress;
                        phones[i].stress_explicit = 1;
                    }
                    CHECK(reloq_german_hv_acoustic_anchor(
                        phones, size, &tick) == 0);
                    CHECK(tick == (stress == 1 ? 80 : 75));
                    int64_t coda_tick = -9;
                    if (size == 2) {
                        CHECK(reloq_german_hvc_f1_target_anchor(
                            phones, size, &coda_tick) == 1);
                        CHECK(coda_tick == -9);
                        CHECK(reloq_german_hvc_first_send_end(
                            phones, size, &coda_tick) == 1);
                        CHECK(coda_tick == -9);
                    } else {
                        CHECK(reloq_german_hvc_f1_target_anchor(
                            phones, size, &coda_tick) == 0);
                        CHECK(coda_tick == tick +
                            expected_relative[stress > 0][v]);
                        struct reloq_german_phone_node vc[2] = {
                            phones[1], phones[2]
                        };
                        coda_tick = -9;
                        CHECK(reloq_german_vc_f1_target_anchor(
                            vc, 2, &coda_tick) == 0);
                        CHECK(coda_tick == expected_relative[stress > 0][v]);
                        int64_t vc_first_send = -9;
                        CHECK(reloq_german_vc_first_send_end(
                            vc, 2, &vc_first_send) == 0);
                        CHECK(vc_first_send == coda_tick -
                              expected_lead[v][c]);
                        int64_t first_send = -9;
                        CHECK(reloq_german_hvc_first_send_end(
                            phones, size, &first_send) == 0);
                        CHECK(first_send == tick +
                            expected_relative[stress > 0][v] -
                            expected_lead[v][c]);
                        CHECK(first_send == tick + vc_first_send);
                    }
                }
            }
        }
    }
    phones[0].stress = phones[1].stress = 1;
    phones[0].stress_explicit = phones[1].stress_explicit = 1;
    phones[1].phone = RELOQ_DE_PHONE_A;
    tick = 123;
    phones[0].phone = RELOQ_DE_PHONE_p;
    CHECK(reloq_german_hv_acoustic_anchor(phones, 2, &tick) == 1);
    phones[0].phone = RELOQ_DE_PHONE_h;
    phones[1].syllable_index = 1;
    CHECK(reloq_german_hv_acoustic_anchor(phones, 2, &tick) == 1);
    phones[1].syllable_index = 0;
    phones[1].stress_explicit = 0;
    CHECK(reloq_german_hv_acoustic_anchor(phones, 2, &tick) == 1);
    CHECK(tick == 123);
    phones[1].stress_explicit = 1;
    phones[0].stress = 3;
    CHECK(reloq_german_hv_acoustic_anchor(phones, 2, &tick) == -1);
    phones[0].stress = 1;
    struct reloq_german_phone_node longer[4] = {
        phones[0], phones[1], phones[2], phones[1]
    };
    longer[2].stress = 1;
    longer[2].stress_explicit = 1;
    longer[3].syllable_index = 1;
    CHECK(reloq_german_hvc_first_send_end(longer, 4, &tick) == 1);
    CHECK(reloq_german_hv_acoustic_anchor(NULL, 2, &tick) == -1);
    CHECK(reloq_german_hv_acoustic_anchor(phones, 2, NULL) == -1);
    CHECK(tick == 123);
    return 0;
}
