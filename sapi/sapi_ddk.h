/* C ABI declarations for the public SAPI 5 TTS DDI.
 * https://learn.microsoft.com/previous-versions/windows/desktop/ee431802(v=vs.85)
 * The implementation uses C COM vtables and the Windows SDK's public sapi.h. */
#ifndef RELOQ_SAPI_DDK_H
#define RELOQ_SAPI_DDK_H
#ifndef COBJMACROS
#define COBJMACROS
#endif
#include <windows.h>
#include <sapi.h>
#include <mmreg.h>
typedef enum {SPVST_SENTENCE=1} SPVSKIPTYPE;
enum {SPVES_CONTINUE=0,SPVES_ABORT=1,SPVES_SKIP=2,SPVES_RATE=4,SPVES_VOLUME=8};
typedef struct SPVTEXTFRAG {
    struct SPVTEXTFRAG *pNext;
    SPVSTATE State;
    LPCWSTR pTextStart;
    ULONG ulTextLen,ulTextSrcOffset;
} SPVTEXTFRAG;
typedef struct ISpTTSEngine ISpTTSEngine;
typedef struct ISpTTSEngineSite ISpTTSEngineSite;
typedef struct ISpTokenUI ISpTokenUI;
typedef struct ISpTTSEngineSiteVtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(ISpTTSEngineSite *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *AddRef)(ISpTTSEngineSite *);
    ULONG (STDMETHODCALLTYPE *Release)(ISpTTSEngineSite *);
    HRESULT (STDMETHODCALLTYPE *AddEvents)(ISpTTSEngineSite *,const SPEVENT *,ULONG);
    HRESULT (STDMETHODCALLTYPE *GetEventInterest)(ISpTTSEngineSite *,ULONGLONG *);
    DWORD (STDMETHODCALLTYPE *GetActions)(ISpTTSEngineSite *);
    HRESULT (STDMETHODCALLTYPE *Write)(ISpTTSEngineSite *,const void *,ULONG,ULONG *);
    HRESULT (STDMETHODCALLTYPE *GetRate)(ISpTTSEngineSite *,LONG *);
    HRESULT (STDMETHODCALLTYPE *GetVolume)(ISpTTSEngineSite *,USHORT *);
    HRESULT (STDMETHODCALLTYPE *GetSkipInfo)(ISpTTSEngineSite *,SPVSKIPTYPE *,LONG *);
    HRESULT (STDMETHODCALLTYPE *CompleteSkip)(ISpTTSEngineSite *,LONG);
} ISpTTSEngineSiteVtbl;
struct ISpTTSEngineSite {const ISpTTSEngineSiteVtbl *lpVtbl;};
typedef struct ISpTTSEngineVtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(ISpTTSEngine *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *AddRef)(ISpTTSEngine *);
    ULONG (STDMETHODCALLTYPE *Release)(ISpTTSEngine *);
    HRESULT (STDMETHODCALLTYPE *Speak)(ISpTTSEngine *,DWORD,REFGUID,const WAVEFORMATEX *,const SPVTEXTFRAG *,ISpTTSEngineSite *);
    HRESULT (STDMETHODCALLTYPE *GetOutputFormat)(ISpTTSEngine *,const GUID *,const WAVEFORMATEX *,GUID *,WAVEFORMATEX **);
} ISpTTSEngineVtbl;
struct ISpTTSEngine {const ISpTTSEngineVtbl *lpVtbl;};
typedef struct ISpTokenUIVtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(ISpTokenUI *,REFIID,void **);
    ULONG (STDMETHODCALLTYPE *AddRef)(ISpTokenUI *);
    ULONG (STDMETHODCALLTYPE *Release)(ISpTokenUI *);
    HRESULT (STDMETHODCALLTYPE *IsUISupported)(ISpTokenUI *,LPCWSTR,void *,ULONG,IUnknown *,BOOL *);
    HRESULT (STDMETHODCALLTYPE *DisplayUI)(ISpTokenUI *,HWND,LPCWSTR,LPCWSTR,void *,ULONG,ISpObjectToken *,IUnknown *);
} ISpTokenUIVtbl;
struct ISpTokenUI {const ISpTokenUIVtbl *lpVtbl;};
extern const GUID IID_ISpTTSEngine,IID_ISpTTSEngineSite,IID_ISpTokenUI;
extern const GUID CLSID_ReLoqEngine,CLSID_ReLoqSettings;
#define RELOQ_ENGINE_CLSID L"{2B1AD1A7-E915-43FD-8BF1-05DA4156B5F4}"
#define RELOQ_SETTINGS_CLSID L"{47EBD361-973C-4ED5-A04E-EA80FA3C5E83}"
#endif
