#ifndef RELOQ_SAPI_SPEECH_DICTIONARY_H
#define RELOQ_SAPI_SPEECH_DICTIONARY_H
#include "settings.h"
#include "../common/nvda_speech.h"

/* Load matching, already validated files from an immutable snapshot. */
int sapi_speech_dictionary_load(const WCHAR *directory,int dialect,unsigned profile,
                                struct reloq_nvda_speech **out);
#endif
