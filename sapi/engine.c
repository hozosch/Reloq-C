#include "settings.h"
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

HMODULE sapi_module;
volatile LONG sapi_objects,sapi_locks;
struct engine {ISpTTSEngine speak;ISpObjectWithToken token_iface;LONG refs;ISpObjectToken *token;int dialect;struct sapi_backend *backend;};
static struct engine *from_token(ISpObjectWithToken *p)
{return CONTAINING_RECORD(p,struct engine,token_iface);}
static HRESULT STDMETHODCALLTYPE engine_qi(ISpTTSEngine *p,REFIID iid,void **out)
{
    if(!out)return E_POINTER;*out=NULL;struct engine *e=(struct engine *)p;
    if(IsEqualIID(iid,&IID_IUnknown)||IsEqualIID(iid,&IID_ISpTTSEngine))*out=&e->speak;
    else if(IsEqualIID(iid,&IID_ISpObjectWithToken))*out=&e->token_iface;
    else return E_NOINTERFACE;
    InterlockedIncrement(&e->refs);return S_OK;
}
static ULONG STDMETHODCALLTYPE engine_add(ISpTTSEngine *p)
{return (ULONG)InterlockedIncrement(&((struct engine *)p)->refs);}
static ULONG STDMETHODCALLTYPE engine_release(ISpTTSEngine *p)
{
    struct engine *e=(struct engine *)p;LONG n=InterlockedDecrement(&e->refs);
    if(!n){sapi_backend_delete(e->backend);if(e->token)ISpObjectToken_Release(e->token);free(e);InterlockedDecrement(&sapi_objects);}return (ULONG)n;
}
static HRESULT STDMETHODCALLTYPE token_qi(ISpObjectWithToken *p,REFIID iid,void **out)
{return engine_qi(&from_token(p)->speak,iid,out);}
static ULONG STDMETHODCALLTYPE token_add(ISpObjectWithToken *p){return engine_add(&from_token(p)->speak);}
static ULONG STDMETHODCALLTYPE token_release(ISpObjectWithToken *p){return engine_release(&from_token(p)->speak);}
static HRESULT STDMETHODCALLTYPE token_set(ISpObjectWithToken *p,ISpObjectToken *token)
{
    struct engine *e=from_token(p);if(!token)return E_POINTER;
    if(e->token)return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
    DWORD dialect=0;HRESULT hr=ISpObjectToken_GetDWORD(token,L"ReLoqDialect",&dialect);
    if(FAILED(hr)||sapi_language_index((int)dialect)<0)return E_INVALIDARG;
    ISpObjectToken_AddRef(token);e->token=token;e->dialect=(int)dialect;return S_OK;
}
static HRESULT STDMETHODCALLTYPE token_get(ISpObjectWithToken *p,ISpObjectToken **out)
{
    if(!out)return E_POINTER;*out=from_token(p)->token;
    if(!*out)return CO_E_NOTINITIALIZED;ISpObjectToken_AddRef(*out);return S_OK;
}
static ISpObjectWithTokenVtbl token_vtable={token_qi,token_add,token_release,token_set,token_get};
static int rate_supported(DWORD rate)
{return rate==8000||rate==11025||rate==16000||rate==22050||rate==32000||rate==44100||rate==48000;}
static HRESULT STDMETHODCALLTYPE format(ISpTTSEngine *p,const GUID *target,const WAVEFORMATEX *input,GUID *id,WAVEFORMATEX **out)
{
    if(!id||!out)return E_POINTER;*out=NULL;
    struct engine *e=(struct engine *)p;struct sapi_settings settings;
    if(FAILED(sapi_settings_read(e->dialect,&settings)))sapi_settings_default(&settings);
    static const DWORD rates[]={8000,11025,22050,16000,32000,44100,48000};
    DWORD rate=rates[settings.rate];
    if(target&&IsEqualGUID(target,&SPDFID_WaveFormatEx)&&input&&rate_supported(input->nSamplesPerSec))rate=input->nSamplesPerSec;
    WAVEFORMATEX *f=CoTaskMemAlloc(sizeof *f);if(!f)return E_OUTOFMEMORY;
    *f=(WAVEFORMATEX){WAVE_FORMAT_PCM,1,rate,rate*2,2,16,0};
    *id=SPDFID_WaveFormatEx;*out=f;return S_OK;
}
static HRESULT STDMETHODCALLTYPE speak(ISpTTSEngine *p,DWORD flags,REFGUID id,const WAVEFORMATEX *f,
    const SPVTEXTFRAG *fragments,ISpTTSEngineSite *site)
{
    struct engine *e=(struct engine *)p;
    if(!site||!f||!id)return E_POINTER;
    if(!e->token)return CO_E_NOTINITIALIZED;
    if(!IsEqualGUID(id,&SPDFID_WaveFormatEx)||f->wFormatTag!=WAVE_FORMAT_PCM||f->nChannels!=1||
       f->wBitsPerSample!=16||f->nBlockAlign!=2||f->nAvgBytesPerSec!=2*f->nSamplesPerSec||!rate_supported(f->nSamplesPerSec))
        return E_INVALIDARG;
    struct sapi_settings settings;HRESULT hr=sapi_settings_read(e->dialect,&settings);if(FAILED(hr))return hr;
    if(!e->backend){e->backend=sapi_backend_new(e->dialect);if(!e->backend)return E_OUTOFMEMORY;}
    return sapi_backend_speak(e->backend,&settings,flags,f,fragments,site);
}
static const ISpTTSEngineVtbl engine_vtable={engine_qi,engine_add,engine_release,speak,format};
struct token_ui {ISpTokenUI iface;LONG refs;};
static HRESULT STDMETHODCALLTYPE ui_qi(ISpTokenUI *p,REFIID iid,void **out)
{
    if(!out)return E_POINTER;*out=NULL;
    if(!IsEqualIID(iid,&IID_IUnknown)&&!IsEqualIID(iid,&IID_ISpTokenUI))return E_NOINTERFACE;
    *out=p;InterlockedIncrement(&((struct token_ui *)p)->refs);return S_OK;
}
static ULONG STDMETHODCALLTYPE ui_add(ISpTokenUI *p){return InterlockedIncrement(&((struct token_ui *)p)->refs);}
static ULONG STDMETHODCALLTYPE ui_release(ISpTokenUI *p)
{LONG n=InterlockedDecrement(&((struct token_ui *)p)->refs);if(!n){free(p);InterlockedDecrement(&sapi_objects);}return n;}
static HRESULT STDMETHODCALLTYPE ui_supported(ISpTokenUI *p,LPCWSTR type,void *extra,ULONG bytes,IUnknown *object,BOOL *supported)
{
    (void)p;(void)extra;(void)bytes;(void)object;if(!supported)return E_POINTER;
    *supported=type&&!wcscmp(type,L"EngineProperties");return S_OK;
}
static HRESULT STDMETHODCALLTYPE ui_display(ISpTokenUI *p,HWND parent,LPCWSTR title,LPCWSTR type,
    void *extra,ULONG bytes,ISpObjectToken *token,IUnknown *object)
{
    (void)p;(void)title;(void)extra;(void)bytes;(void)object;
    if(!type||wcscmp(type,L"EngineProperties"))return E_INVALIDARG;
    DWORD dialect=0x10000;if(token)ISpObjectToken_GetDWORD(token,L"ReLoqDialect",&dialect);
    return sapi_settings_dialog(parent,(int)dialect);
}
static const ISpTokenUIVtbl ui_vtable={ui_qi,ui_add,ui_release,ui_supported,ui_display};
struct factory {IClassFactory iface;LONG refs;int ui;};
static HRESULT STDMETHODCALLTYPE factory_qi(IClassFactory *p,REFIID iid,void **out)
{
    if(!out)return E_POINTER;*out=NULL;
    if(!IsEqualIID(iid,&IID_IUnknown)&&!IsEqualIID(iid,&IID_IClassFactory))return E_NOINTERFACE;
    *out=p;InterlockedIncrement(&((struct factory *)p)->refs);return S_OK;
}
static ULONG STDMETHODCALLTYPE factory_add(IClassFactory *p){return InterlockedIncrement(&((struct factory *)p)->refs);}
static ULONG STDMETHODCALLTYPE factory_release(IClassFactory *p)
{LONG n=InterlockedDecrement(&((struct factory *)p)->refs);if(!n){free(p);InterlockedDecrement(&sapi_objects);}return n;}
static HRESULT STDMETHODCALLTYPE factory_create(IClassFactory *p,IUnknown *outer,REFIID iid,void **out)
{
    if(!out)return E_POINTER;*out=NULL;if(outer)return CLASS_E_NOAGGREGATION;
    if(((struct factory *)p)->ui){struct token_ui *u=calloc(1,sizeof *u);if(!u)return E_OUTOFMEMORY;
        u->iface.lpVtbl=&ui_vtable;u->refs=1;InterlockedIncrement(&sapi_objects);
        HRESULT hr=ui_qi(&u->iface,iid,out);ui_release(&u->iface);return hr;
    }
    struct engine *e=calloc(1,sizeof *e);if(!e)return E_OUTOFMEMORY;
    e->speak.lpVtbl=&engine_vtable;e->token_iface.lpVtbl=&token_vtable;e->refs=1;e->dialect=0x10000;
    InterlockedIncrement(&sapi_objects);HRESULT hr=engine_qi(&e->speak,iid,out);engine_release(&e->speak);return hr;
}
static HRESULT STDMETHODCALLTYPE factory_lock(IClassFactory *p,BOOL lock)
{(void)p;if(lock)InterlockedIncrement(&sapi_locks);else InterlockedDecrement(&sapi_locks);return S_OK;}
static IClassFactoryVtbl factory_vtable={factory_qi,factory_add,factory_release,factory_create,factory_lock};
__declspec(dllexport) HRESULT WINAPI DllGetClassObject(REFCLSID clsid,REFIID iid,void **out)
{
    if(!out)return E_POINTER;*out=NULL;int ui=IsEqualCLSID(clsid,&CLSID_ReLoqSettings);
    if(!ui&&!IsEqualCLSID(clsid,&CLSID_ReLoqEngine))return CLASS_E_CLASSNOTAVAILABLE;
    struct factory *f=calloc(1,sizeof *f);if(!f)return E_OUTOFMEMORY;
    f->iface.lpVtbl=&factory_vtable;f->refs=1;f->ui=ui;InterlockedIncrement(&sapi_objects);
    HRESULT hr=factory_qi(&f->iface,iid,out);factory_release(&f->iface);return hr;
}
__declspec(dllexport) HRESULT WINAPI DllCanUnloadNow(void)
{return !sapi_objects&&!sapi_locks?S_OK:S_FALSE;}
__declspec(dllexport) HRESULT WINAPI DllRegisterServer(void){return sapi_register(TRUE,TRUE);}
__declspec(dllexport) HRESULT WINAPI DllUnregisterServer(void){return sapi_register(FALSE,TRUE);}
__declspec(dllexport) HRESULT WINAPI ReLoqSettings(HWND parent,int dialect){return sapi_settings_dialog(parent,dialect);}
BOOL WINAPI DllMain(HINSTANCE instance,DWORD reason,void *reserved)
{(void)reserved;if(reason==DLL_PROCESS_ATTACH){sapi_module=instance;DisableThreadLibraryCalls(instance);}return TRUE;}
