#ifndef RELOQ_SAPI_SETTINGS_H
#define RELOQ_SAPI_SETTINGS_H
#ifdef RELOQ_SAPI_TEST_HOST
#include "test_host.h"
#else
#include "sapi_ddk.h"
#endif
#include <stddef.h>
struct sapi_language {int dialect;WORD lcid;const WCHAR *name,*locale,*sample;};
#define SAPI_EQ_FILTER_COUNT 5
struct sapi_eq_filter {DWORD enabled,frequency,gain,width;};
struct sapi_settings {
    DWORD schema,profile,pitch,inflection,head,roughness,breathiness,gender;
    DWORD rate,pauses,tags,abbreviations,prediction,dictionaries,boost;
    WCHAR directory[1024];
    DWORD original_questions,mellow_mode,pitch_hz,pitch_hz_enabled;
    DWORD speed_override,speed_value,pitch_override,volume_override,volume_value;
    DWORD speed_wpm_enabled,speed_wpm;
    /* Encoded gains use 12 as 0 dB; custom values survive preset changes. */
    DWORD eq_enabled,eq_preset,eq_low,eq_mid,eq_high;
    struct sapi_eq_filter eq_filters[SAPI_EQ_FILTER_COUNT];
    DWORD original_acoustic_clock;
};
#ifndef RELOQ_SAPI_TEST_HOST
_Static_assert(sizeof(struct sapi_settings)==2256,"SAPI settings must be compatible across x86/x64");
#endif
static inline void sapi_equalizer_gains(const struct sapi_settings *s,int result[3])
{
    static const int presets[4][3]={{12,12,12},{16,12,11},{11,12,16},{12,11,8}};
    if(!s->eq_enabled){result[0]=result[1]=result[2]=12;return;}
    if(s->eq_preset<4){for(int i=0;i<3;++i)result[i]=presets[s->eq_preset][i];return;}
    result[0]=(int)s->eq_low;result[1]=(int)s->eq_mid;result[2]=(int)s->eq_high;
}
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
struct sapi_dictionary_update {size_t added,total,added_files,total_files;int known,compared;};
HRESULT sapi_dictionary_download_report(struct sapi_dictionary_update *);
int sapi_alternative_dictionary_available(void);
int sapi_default_dictionary_path(WCHAR *,size_t);
#endif
HRESULT sapi_register(BOOL,BOOL);
const WCHAR *sapi_profile_name(unsigned);
int sapi_sample_rate(unsigned);
struct sapi_backend;
struct sapi_backend *sapi_backend_new(int);
struct sapi_backend *sapi_backend_new_prepared(int,const struct sapi_settings *);
void sapi_backend_delete(struct sapi_backend *);
HRESULT sapi_backend_speak(struct sapi_backend *,const struct sapi_settings *,DWORD,const WAVEFORMATEX *,const SPVTEXTFRAG *,ISpTTSEngineSite *);
#endif
