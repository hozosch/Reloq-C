#ifndef RELOQ_TEST_SNAPSHOT_HOST_H
#define RELOQ_TEST_SNAPSHOT_HOST_H
#include <stdio.h>
typedef uintptr_t HCRYPTPROV,HCRYPTHASH;
typedef int SRWLOCK;
typedef struct {DWORD dwFileAttributes;WCHAR cFileName[260];} WIN32_FIND_DATAW;
#define SRWLOCK_INIT 0
#define PROV_RSA_AES 24
#define CRYPT_VERIFYCONTEXT 0
#define CALG_SHA_256 0
#define HP_HASHVAL 0
#define FILE_ATTRIBUTE_DIRECTORY 16
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#define ERROR_ALREADY_EXISTS 183
#define MOVEFILE_WRITE_THROUGH 8
BOOL CryptAcquireContextW(HCRYPTPROV *,const WCHAR *,const WCHAR *,DWORD,DWORD);
BOOL CryptCreateHash(HCRYPTPROV,DWORD,uintptr_t,DWORD,HCRYPTHASH *);
BOOL CryptHashData(HCRYPTHASH,const BYTE *,DWORD,DWORD);
BOOL CryptGetHashParam(HCRYPTHASH,DWORD,BYTE *,DWORD *,DWORD);
BOOL CryptDestroyHash(HCRYPTHASH),CryptReleaseContext(HCRYPTPROV,DWORD);
BOOL CreateDirectoryW(const WCHAR *,void *),DeleteFileW(const WCHAR *),RemoveDirectoryW(const WCHAR *);
BOOL MoveFileExW(const WCHAR *,const WCHAR *,DWORD);
DWORD GetFileAttributesW(const WCHAR *),GetEnvironmentVariableW(const WCHAR *,WCHAR *,DWORD);
DWORD GetCurrentProcessId(void),GetCurrentThreadId(void);
ULONGLONG GetTickCount64(void);
HANDLE FindFirstFileW(const WCHAR *,WIN32_FIND_DATAW *);
BOOL FindNextFileW(HANDLE,WIN32_FIND_DATAW *),FindClose(HANDLE);
void AcquireSRWLockShared(SRWLOCK *),ReleaseSRWLockShared(SRWLOCK *);
void AcquireSRWLockExclusive(SRWLOCK *),ReleaseSRWLockExclusive(SRWLOCK *);
FILE *test_wfopen(const WCHAR *,const WCHAR *);
#define _wfopen test_wfopen
/* These are supplied by the real SAPI adapter in Windows. */
DWORD sapi_dictionary_source(void);
int sapi_default_dictionary_path(WCHAR *,size_t);
HRESULT sapi_dictionary_files_load(struct sapi_dictionary_file *,size_t,size_t *);
#endif
