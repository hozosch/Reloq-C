/* Test-only POSIX implementation of the small Win32 synchronization boundary.
 * Production always uses Windows headers and the Windows COM calling ABI. */
#ifndef RELOQ_SAPI_TEST_HOST_H
#define RELOQ_SAPI_TEST_HOST_H
#include <stdint.h>
#include <stddef.h>
#include <wchar.h>
#include <limits.h>
typedef uint32_t DWORD,ULONG;typedef int32_t LONG,HRESULT;typedef uint16_t WORD,USHORT;
typedef wchar_t WCHAR;typedef const WCHAR *LPCWSTR;typedef int BOOL;typedef void *PVOID,*HMODULE,*HWND;
typedef uintptr_t WPARAM;typedef intptr_t LPARAM;typedef uint64_t ULONGLONG;
typedef uint8_t BYTE;typedef void (*FARPROC)(void);typedef void *HANDLE;
typedef struct {DWORD dwLowDateTime,dwHighDateTime;} FILETIME;
typedef struct {DWORD dwFileAttributes;FILETIME ftCreationTime,ftLastAccessTime,ftLastWriteTime;
    DWORD nFileSizeHigh,nFileSizeLow;} WIN32_FILE_ATTRIBUTE_DATA;
#define GetFileExInfoStandard 0
#define ERROR_FILE_NOT_FOUND 2
#define ERROR_PATH_NOT_FOUND 3
BOOL GetFileAttributesExW(LPCWSTR,int,WIN32_FILE_ATTRIBUTE_DATA *);
DWORD GetLastError(void);
extern unsigned sapi_test_dictionary_calls;
#ifdef RELOQ_SAPI_SETTINGS_TEST
typedef void *HKEY;typedef LONG LSTATUS;
#define HKEY_CURRENT_USER ((HKEY)(uintptr_t)1)
#define ERROR_SUCCESS 0
#define ERROR_INVALID_DATA 13
#define REG_BINARY 3
#define REG_DWORD 4
#define RRF_RT_REG_BINARY 8
#define RRF_RT_REG_DWORD 16
#define KEY_SET_VALUE 2
#define KEY_READ 0x20019
#define S_FALSE ((HRESULT)1)
#define E_POINTER ((HRESULT)0x80004003)
DWORD GetModuleFileNameW(HMODULE,WCHAR *,DWORD);
LSTATUS RegGetValueW(HKEY,LPCWSTR,LPCWSTR,DWORD,DWORD *,void *,DWORD *);
LSTATUS RegOpenKeyExW(HKEY,LPCWSTR,DWORD,DWORD,HKEY *);
LSTATUS RegCreateKeyExW(HKEY,LPCWSTR,DWORD,void *,DWORD,DWORD,void *,HKEY *,DWORD *);
LSTATUS RegSetValueExW(HKEY,LPCWSTR,DWORD,DWORD,const BYTE *,DWORD);
LSTATUS RegCloseKey(HKEY);
#endif
typedef struct {int state;} INIT_ONCE,*PINIT_ONCE;
#define INIT_ONCE_STATIC_INIT {0}
#define WINAPI
#define CALLBACK
#define ECICALL
#define TRUE 1
#define FALSE 0
#define S_OK ((HRESULT)0)
#define E_FAIL ((HRESULT)0x80004005)
#define E_INVALIDARG ((HRESULT)0x80070057)
#define E_OUTOFMEMORY ((HRESULT)0x8007000e)
#define E_NOTIMPL ((HRESULT)0x80004001)
#define SUCCEEDED(x) ((HRESULT)(x)>=0)
#define FAILED(x) ((HRESULT)(x)<0)
#define HRESULT_FROM_WIN32(x) ((HRESULT)(0x80070000u|(x)))
#define ERROR_BUSY 170
#define WAIT_OBJECT_0 0
#define WAIT_TIMEOUT 258
#define INFINITE 0xffffffffu
#define LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR 0x100
#define LOAD_LIBRARY_SEARCH_SYSTEM32 0x800
#define GET_MODULE_HANDLE_EX_FLAG_PIN 1
#define GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS 4
#define CP_UTF8 65001
#define WC_ERR_INVALID_CHARS 0x80
enum {SPVA_Speak,SPVA_Silence,SPVA_Pronounce,SPVA_Bookmark,SPVA_SpellOut,SPVA_Section,SPVA_ParseUnknownTag};
enum {SPVES_CONTINUE=0,SPVES_ABORT=1,SPVES_SKIP=2,SPVES_RATE=4,SPVES_VOLUME=8};
enum {SPET_LPARAM_IS_UNDEFINED,SPET_LPARAM_IS_TOKEN,SPET_LPARAM_IS_OBJECT,SPET_LPARAM_IS_POINTER,SPET_LPARAM_IS_STRING};
enum {SPEI_TTS_BOOKMARK=4,SPEI_WORD_BOUNDARY=5,SPEI_SENTENCE_BOUNDARY=7};
typedef enum {SPVST_SENTENCE=1} SPVSKIPTYPE;
typedef struct {int eAction;WORD LangID,wReserved;LONG EmphAdj,RateAdj;ULONG Volume;
    struct {LONG MiddleAdj,RangeAdj;} PitchAdj;ULONG SilenceMSecs;WORD *pPhoneIds;
    int ePartOfSpeech;struct {LPCWSTR a,b,c;} Context;} SPVSTATE;
typedef struct SPVTEXTFRAG {struct SPVTEXTFRAG *pNext;SPVSTATE State;LPCWSTR pTextStart;ULONG ulTextLen,ulTextSrcOffset;} SPVTEXTFRAG;
typedef struct {WORD eEventId,elParamType;ULONG ulStreamNum;ULONGLONG ullAudioStreamOffset;WPARAM wParam;LPARAM lParam;} SPEVENT;
typedef struct {WORD wFormatTag,nChannels;DWORD nSamplesPerSec,nAvgBytesPerSec;WORD nBlockAlign,wBitsPerSample,cbSize;} WAVEFORMATEX;
typedef struct ISpTTSEngineSite ISpTTSEngineSite;
typedef struct {HRESULT (*qi)(void);ULONG (*addref)(void),(*release)(void);
    HRESULT (*AddEvents)(ISpTTSEngineSite *,const SPEVENT *,ULONG);
    HRESULT (*GetEventInterest)(ISpTTSEngineSite *,ULONGLONG *);
    DWORD (*GetActions)(ISpTTSEngineSite *);
    HRESULT (*Write)(ISpTTSEngineSite *,const void *,ULONG,ULONG *);
    HRESULT (*GetRate)(ISpTTSEngineSite *,LONG *);
    HRESULT (*GetVolume)(ISpTTSEngineSite *,USHORT *);
    HRESULT (*GetSkipInfo)(ISpTTSEngineSite *,SPVSKIPTYPE *,LONG *);
    HRESULT (*CompleteSkip)(ISpTTSEngineSite *,LONG);
} ISpTTSEngineSiteVtbl;
struct ISpTTSEngineSite {const ISpTTSEngineSiteVtbl *lpVtbl;};
LONG InterlockedIncrement(volatile LONG *),InterlockedCompareExchange(volatile LONG *,LONG,LONG),InterlockedExchange(volatile LONG *,LONG);
PVOID InterlockedCompareExchangePointer(PVOID volatile *,PVOID,PVOID),InterlockedExchangePointer(PVOID volatile *,PVOID);
BOOL InitOnceExecuteOnce(PINIT_ONCE,BOOL (*)(PINIT_ONCE,PVOID,PVOID *),PVOID,PVOID *);
HMODULE LoadLibraryExW(LPCWSTR,HANDLE,DWORD);FARPROC GetProcAddress(HMODULE,const char *);
BOOL FreeLibrary(HMODULE),GetModuleHandleExW(DWORD,LPCWSTR,HMODULE *);
HANDLE CreateSemaphoreW(void *,LONG,LONG,LPCWSTR),CreateEventW(void *,BOOL,BOOL,LPCWSTR);
HANDLE CreateThread(void *,size_t,DWORD (*)(void *),void *,DWORD,DWORD *);
DWORD WaitForSingleObject(HANDLE,DWORD);
BOOL ReleaseSemaphore(HANDLE,LONG,LONG *),SetEvent(HANDLE),CloseHandle(HANDLE);
int WideCharToMultiByte(unsigned,DWORD,LPCWSTR,int,char *,int,const char *,BOOL *);
extern const char *sapi_test_library;
#endif
