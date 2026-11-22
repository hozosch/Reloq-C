#ifndef RELOQ_PERSIAN_TUNING_H
#define RELOQ_PERSIAN_TUNING_H

#include <stddef.h>
#include <stdint.h>

/* The provider reports native phone edges to the current synthesis request.
 * The clock is in 1/10000 ms, matching the existing syllable trace. */
typedef void (*reloq_fa_phone_sink)(void *,int32_t,int32_t,unsigned);
struct reloq_fa_phone_trace {
    reloq_fa_phone_sink sink;
    void *context;
    int32_t previous_to;
    int64_t origin;
};
struct reloq_fa_phone_trace reloq61_fa_phone_scope(struct reloq_fa_phone_trace);
void reloq61_fa_phone_emit(void *state,int32_t from,int32_t to,unsigned phone);

enum reloq_fa_tuned_phone {
    RELOQ_FA_TUNE_OTHER=0, RELOQ_FA_TUNE_FRONT_A=1, RELOQ_FA_TUNE_BACK_A=2,
    RELOQ_FA_TUNE_E=3, RELOQ_FA_TUNE_O=4, RELOQ_FA_TUNE_R_BEFORE_I=5,
    RELOQ_FA_TUNE_R_BEFORE_BACK_A=6, RELOQ_FA_TUNE_BACK_A_AFTER_R=7,
    RELOQ_FA_TUNE_BEFORE_FINAL_H=8, RELOQ_FA_TUNE_U=9,
    RELOQ_FA_TUNE_R=10
};

/* Slots 11 and 12 are unused by the acoustic frame adapter. They carry
 * request-local tuning metadata through the existing, binary-compatible
 * capture/replay interface, including its mixed-language and streaming paths. */
#define RELOQ_FA_TUNING_MARKER 16400

struct reloq_fa_tuning_band {
    double b0,b1,b2,a1,a2,x1,x2,y1,y2;
    double frequency,width,gain;
    int active;
};
struct reloq_fa_tuning_filter {
    struct reloq_fa_tuning_band bands[5];
    unsigned phone;
    int rate,active;
};

/* Thread-local comparison switch; enabled by default. Restore after use. */
int reloq_fa_tuning_scope(int enabled);
int reloq_fa_tuning_enabled(void);
void reloq_fa_tuning_targets(float controls[62],unsigned phone,double from_ms,double to_ms,
                             double frame_ms,double duration_ms,int rate);
void reloq_fa_tuning_r_to_back_a(float controls[62],double from_ms,double to_ms,
                                double frame_ms,int rate);
void reloq_fa_tuning_prepare(struct reloq_fa_tuning_filter *,const float controls[62],int rate);
double reloq_fa_tuning_sample(struct reloq_fa_tuning_filter *,double sample);

#endif
