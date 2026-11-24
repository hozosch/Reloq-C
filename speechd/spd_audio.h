#ifndef RELOQ_SPD_AUDIO_H
#define RELOQ_SPD_AUDIO_H

/* Speech Dispatcher installs spd_module_main.h for out-of-tree modules, but
 * that header includes spd_audio.h, which Speech Dispatcher 0.11 and 0.12 do
 * not install. The module only needs the public audio types below. */
#include <speech-dispatcher/spd_audio_plugin.h>

#endif
