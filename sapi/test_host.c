#define _POSIX_C_SOURCE 200809L
#include "settings.h"
#include <pthread.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
const char *sapi_test_library;
HMODULE sapi_module;volatile LONG sapi_objects,sapi_locks;
LONG InterlockedIncrement(volatile LONG *p){return __atomic_add_fetch(p,1,__ATOMIC_SEQ_CST);}
LONG InterlockedExchange(volatile LONG *p,LONG n){return __atomic_exchange_n(p,n,__ATOMIC_SEQ_CST);}
LONG InterlockedCompareExchange(volatile LONG *p,LONG n,LONG old)
{__atomic_compare_exchange_n(p,&old,n,0,__ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST);return old;}
PVOID InterlockedExchangePointer(PVOID volatile *p,PVOID n){return __atomic_exchange_n(p,n,__ATOMIC_SEQ_CST);}
PVOID InterlockedCompareExchangePointer(PVOID volatile *p,PVOID n,PVOID old)
{__atomic_compare_exchange_n(p,&old,n,0,__ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST);return old;}
BOOL InitOnceExecuteOnce(PINIT_ONCE once,BOOL (*fn)(PINIT_ONCE,PVOID,PVOID *),PVOID arg,PVOID *out)
{static pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;pthread_mutex_lock(&lock);
    if(!once->state&&fn(once,arg,out))once->state=1;BOOL ok=once->state;pthread_mutex_unlock(&lock);return ok;}
HMODULE LoadLibraryExW(LPCWSTR p,HANDLE h,DWORD flags){(void)p;(void)h;(void)flags;return dlopen(sapi_test_library,RTLD_NOW|RTLD_LOCAL);}
FARPROC GetProcAddress(HMODULE h,const char *name){void *p=dlsym(h,name);FARPROC fn;memcpy(&fn,&p,sizeof fn);return fn;}
BOOL FreeLibrary(HMODULE h){return dlclose(h)==0;}
BOOL GetModuleHandleExW(DWORD flags,LPCWSTR address,HMODULE *out){(void)flags;(void)address;*out=(HMODULE)1;return TRUE;}
int sapi_module_path(WCHAR *p,size_t cap,const WCHAR *name){if(wcslen(name)>=cap)return 0;wcscpy(p,name);return 1;}
struct handle {int thread,event,manual,count,maximum;pthread_mutex_t lock;pthread_cond_t condition;pthread_t tid;DWORD (*fn)(void *);void *arg;};
static HANDLE new_handle(void){struct handle *h=calloc(1,sizeof *h);if(h){pthread_mutex_init(&h->lock,NULL);pthread_cond_init(&h->condition,NULL);}return h;}
HANDLE CreateSemaphoreW(void *a,LONG initial,LONG maximum,LPCWSTR name)
{(void)a;(void)name;struct handle *h=new_handle();if(h){h->count=initial;h->maximum=maximum;}return h;}
HANDLE CreateEventW(void *a,BOOL manual,BOOL initial,LPCWSTR name)
{struct handle *h=CreateSemaphoreW(a,initial,1,name);if(h){h->event=1;h->manual=manual;}return h;}
static void *thread_main(void *arg){struct handle *h=arg;h->fn(h->arg);return NULL;}
HANDLE CreateThread(void *security,size_t stack,DWORD (*fn)(void *),void *arg,DWORD flags,DWORD *id)
{(void)security;(void)stack;(void)flags;(void)id;struct handle *h=new_handle();if(!h)return NULL;
 h->thread=1;h->fn=fn;h->arg=arg;if(pthread_create(&h->tid,NULL,thread_main,h)){CloseHandle(h);return NULL;}return h;}
DWORD WaitForSingleObject(HANDLE handle,DWORD ms)
{struct handle *h=handle;if(h->thread){pthread_join(h->tid,NULL);h->thread=0;return WAIT_OBJECT_0;}
 pthread_mutex_lock(&h->lock);int rc=0;struct timespec until;clock_gettime(CLOCK_REALTIME,&until);
 until.tv_sec+=ms/1000;until.tv_nsec+=(long)(ms%1000)*1000000;if(until.tv_nsec>=1000000000){until.tv_sec++;until.tv_nsec-=1000000000;}
 while(!h->count&&!rc)rc=ms==INFINITE?pthread_cond_wait(&h->condition,&h->lock):pthread_cond_timedwait(&h->condition,&h->lock,&until);
 if(h->count){if(!h->manual)--h->count;rc=0;}pthread_mutex_unlock(&h->lock);return rc?WAIT_TIMEOUT:WAIT_OBJECT_0;}
BOOL ReleaseSemaphore(HANDLE handle,LONG count,LONG *previous)
{struct handle *h=handle;pthread_mutex_lock(&h->lock);if(previous)*previous=h->count;
 BOOL ok=count>0&&h->count<=h->maximum-count;if(ok){h->count+=count;pthread_cond_broadcast(&h->condition);}pthread_mutex_unlock(&h->lock);return ok;}
BOOL SetEvent(HANDLE handle){struct handle *h=handle;pthread_mutex_lock(&h->lock);h->count=1;pthread_cond_broadcast(&h->condition);pthread_mutex_unlock(&h->lock);return TRUE;}
BOOL CloseHandle(HANDLE handle){struct handle *h=handle;pthread_cond_destroy(&h->condition);pthread_mutex_destroy(&h->lock);free(h);return TRUE;}
int WideCharToMultiByte(unsigned code,DWORD flags,LPCWSTR input,int length,char *output,int capacity,const char *unused,BOOL *used)
{(void)code;(void)flags;(void)unused;(void)used;size_t n=0;int include_zero=length<0;if(include_zero)length=(int)wcslen(input)+1;
 for(int i=0;i<length;++i){uint32_t cp=(uint32_t)input[i];
  if(cp>=0xd800&&cp<=0xdbff){if(i+1>=length||input[i+1]<0xdc00||input[i+1]>0xdfff)return 0;cp=0x10000+((cp-0xd800)<<10)+(input[++i]-0xdc00);}
  else if(cp>=0xdc00&&cp<=0xdfff)return 0;if(cp>0x10ffff)return 0;
  unsigned char bytes[4];int count=cp<128?1:cp<0x800?2:cp<0x10000?3:4;
  for(int j=count-1;j>0;--j){bytes[j]=(unsigned char)(0x80|(cp&63));cp>>=6;}
  bytes[0]=(unsigned char)(count==1?cp:cp|(count==2?0xc0:count==3?0xe0:0xf0));
  if(output){if(n+(size_t)count>(size_t)capacity)return 0;memcpy(output+n,bytes,(size_t)count);}n+=(size_t)count;
 }return n>INT_MAX?0:(int)n;}
