#ifndef RELOQ_SAPI_SETTINGS_H
#define RELOQ_SAPI_SETTINGS_H
#ifdef RELOQ_SAPI_TEST_HOST
#include "test_host.h"
#else
#include "sapi_ddk.h"
#endif
#include <stddef.h>
struct sapi_language {int dialect;WORD lcid;const WCHAR *name,*locale,*sample;};
struct sapi_settings {
    DWORD schema,profile,pitch,inflection,head,roughness,breathiness,gender;
    DWORD rate,pauses,tags,abbreviations,prediction,dictionaries,boost;
    WCHAR directory[1024];
};
#ifndef RELOQ_SAPI_TEST_HOST
_Static_assert(sizeof(struct sapi_settings)==2108,"SAPI settings must be compatible across x86/x64");
#endif
extern HMODULE sapi_module;
extern volatile LONG sapi_objects,sapi_locks;
const struct sapi_language *sapi_language_at(size_t);
size_t sapi_language_count(void);
int sapi_language_index(int);
void sapi_settings_default(struct sapi_settings *);
void sapi_settings_profile(struct sapi_settings *,unsigned);
HRESULT sapi_settings_read(int,struct sapi_settings *);
HRESULT sapi_settings_write(int,const struct sapi_settings *);
HRESULT sapi_settings_shared(BOOL *);
HRESULT sapi_settings_set_shared(int,BOOL);
HRESULT sapi_settings_dialog(HWND,int);
int sapi_module_path(WCHAR *,size_t,const WCHAR *);
#ifndef RELOQ_SAPI_TEST_HOST
DWORD sapi_dictionary_source(void);
HRESULT sapi_dictionary_choose(DWORD);
HRESULT sapi_dictionary_download(void);
int sapi_default_dictionary_path(WCHAR *,size_t);
#endif
HRESULT sapi_register(BOOL,BOOL);
const WCHAR *sapi_profile_name(unsigned);
int sapi_sample_rate(unsigned);
struct sapi_backend;
struct sapi_backend *sapi_backend_new(int);
void sapi_backend_delete(struct sapi_backend *);
HRESULT sapi_backend_speak(struct sapi_backend *,const struct sapi_settings *,DWORD,const WAVEFORMATEX *,const SPVTEXTFRAG *,ISpTTSEngineSite *);
#endif
