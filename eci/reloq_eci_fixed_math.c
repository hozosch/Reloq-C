/* The private I/O converter uses shared fixed-point helpers. PE retains
 * their numeric table reference even when the logarithm helper is discarded.
 * Supply its twenty Q14 powers directly, without the old renderer's much
 * larger waveform/filter tables or its allocation/initialization path. */
#include <stdint.h>
const int16_t klatt_fxl2[20] = {
    16384, 16962, 17560, 18179, 18820, 19484, 20171, 20882, 21619, 22381,
    23170, 23988, 24834, 25709, 26616, 27554, 28526, 29532, 30574, 31652
};
