#include "reloq_german_voice_defaults.h"

#include <string.h>

#define CHECK(value) do { if (!(value)) return __LINE__; } while (0)

int main(void)
{
    float controls[RELOQ_PARAMETER_TRACKS];
    uint8_t map[RELOQ_TARGET_STREAM_COUNT];
    struct reloq_session_config config;
    memset(controls, 0xa5, sizeof controls);
    memset(map, 0xa5, sizeof map);
    memset(&config, 0xa5, sizeof config);
    CHECK(reloq_german_voice_defaults(controls, map, &config) == 0);
    CHECK(controls[0] == 1 && controls[1] == 1000 &&
          controls[2] == 60 && controls[3] == 50 &&
          controls[21] == 5000 && controls[44] == 80 &&
          controls[51] == 1500 && controls[61] == 0);
    CHECK(map[RELOQ_TARGET_F0] == 1 && map[RELOQ_TARGET_AV] == 2 &&
          map[RELOQ_TARGET_F1] == 9 && map[RELOQ_TARGET_F5] == 19 &&
          map[RELOQ_TARGET_FNP] == 27 && map[RELOQ_TARGET_AB] == 43);
    unsigned seen[RELOQ_PARAMETER_TRACKS] = {0};
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i) {
        CHECK(map[i] > 0 && map[i] < RELOQ_PARAMETER_TRACKS &&
              !seen[map[i]]);
        seen[map[i]] = 1;
    }
    CHECK(config.sample_rate == RELOQ_GERMAN_VOICE_SAMPLE_RATE &&
          config.duration_scale == 1 && config.output_gain == 2.02f &&
          config.initial_seed == 8 && config.source_offset == 4 &&
          config.av_trim == -5 && config.ah_trim == 6 &&
          config.af_trim == 0 && config.max_frame_samples == 64);
    float before_controls[RELOQ_PARAMETER_TRACKS];
    uint8_t before_map[RELOQ_TARGET_STREAM_COUNT];
    struct reloq_session_config before_config;
    memcpy(before_controls, controls, sizeof controls);
    memcpy(before_map, map, sizeof map);
    before_config = config;
    CHECK(reloq_german_voice_defaults(NULL, map, &config) == -1);
    CHECK(reloq_german_voice_defaults(controls, NULL, &config) == -1);
    CHECK(reloq_german_voice_defaults(controls, map, NULL) == -1);
    CHECK(!memcmp(before_controls, controls, sizeof controls) &&
          !memcmp(before_map, map, sizeof map) &&
          !memcmp(&before_config, &config, sizeof config));
    return 0;
}
