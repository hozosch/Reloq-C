/* Schedule the worker before CreateThread returns unless it was requested
 * suspended. AddressSanitizer then exposes a premature task free reliably,
 * without relying on the host's thread scheduling. */
#include "windows.h"
#include "evv_port.h"
#include <assert.h>
#include <stdio.h>

static LPTHREAD_START_ROUTINE pending;
static LPVOID argument;
static int suspended, ran, closes, terminated, fail_create, fail_resume;

void evv_frame_done(void) {}
BOOL CloseHandle(HANDLE handle) { (void)handle; ++closes; return TRUE; }
HANDLE CreateThread(void *attributes, SIZE_T stack, LPTHREAD_START_ROUTINE start,
                    LPVOID arg, DWORD flags, DWORD *id)
{
    (void)attributes; (void)stack; (void)id;
    if (fail_create) return NULL;
    pending=start; argument=arg; suspended=(flags & CREATE_SUSPENDED)!=0;
    if (!suspended) start(arg);
    return (HANDLE)(uintptr_t)42;
}
DWORD ResumeThread(HANDLE handle)
{
    (void)handle;
    if (fail_resume || !suspended) return (DWORD)-1;
    pending(argument);
    return 1;
}
BOOL TerminateThread(HANDLE handle, DWORD code)
{
    (void)handle; (void)code; ++terminated; return TRUE;
}
static void immediate(void *arg) { (void)arg; ++ran; }

int main(void)
{
    fail_create=1;
    assert(!evv_task_start(immediate,NULL,0) && !ran && !closes);
    fail_create=0; fail_resume=1;
    assert(!evv_task_start(immediate,NULL,0) && !ran && closes==1 && terminated==1);
    fail_resume=0;
    assert(evv_task_start(immediate,NULL,0) && ran==1 && suspended && closes==2);
    puts("Passed Windows task-start ordering and failure cleanup.");
    return 0;
}
