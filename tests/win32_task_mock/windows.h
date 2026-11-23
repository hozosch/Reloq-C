#ifndef RELOQ_TEST_WINDOWS_H
#define RELOQ_TEST_WINDOWS_H
#include <stddef.h>
#include <stdint.h>
#define WINAPI
#define TRUE 1
#define FALSE 0
#define SRWLOCK_INIT 0
#define WAIT_OBJECT_0 0
#define WAIT_TIMEOUT 258
#define INFINITE ((DWORD)-1)
#define STACK_SIZE_PARAM_IS_A_RESERVATION 0x10000
#define CREATE_SUSPENDED 4
typedef void *HANDLE;
typedef void *LPVOID;
typedef size_t SIZE_T;
typedef uint32_t DWORD;
typedef int BOOL;
typedef int SRWLOCK;
typedef DWORD (*LPTHREAD_START_ROUTINE)(LPVOID);
void AcquireSRWLockExclusive(SRWLOCK *);
void ReleaseSRWLockExclusive(SRWLOCK *);
HANDLE CreateSemaphoreA(void *, int, int, const char *);
BOOL CloseHandle(HANDLE);
DWORD WaitForSingleObject(HANDLE, DWORD);
BOOL ReleaseSemaphore(HANDLE, int, int *);
HANDLE CreateEventA(void *, BOOL, BOOL, const char *);
BOOL SetEvent(HANDLE);
BOOL ResetEvent(HANDLE);
BOOL PulseEvent(HANDLE);
HANDLE CreateThread(void *, SIZE_T, LPTHREAD_START_ROUTINE, LPVOID, DWORD, DWORD *);
DWORD ResumeThread(HANDLE);
BOOL TerminateThread(HANDLE, DWORD);
DWORD GetCurrentThreadId(void);
void Sleep(DWORD);
DWORD GetTickCount(void);
#endif
