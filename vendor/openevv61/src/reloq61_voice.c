#include <stdint.h>

/* Anatomy scales a whole-Hz endpoint by an integer percentage. The lifted
   fixed-point path used an unnecessary *10000 intermediate, overflowing
   signed 32 bits for the upper formants of small heads. Keep one wide
   product and truncate once, as the original floating-point path does. */
int32_t reloq61_size_frequency(int32_t frequency,int32_t percent)
{
    return (int32_t)((int64_t)frequency*percent/100);
}
