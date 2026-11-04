/* Measure SAPI transport up to the first write, without a real audio device. */
#define _POSIX_C_SOURCE 200809L
#define main other_backend_tests
#include "test_backend.c"
#undef main
#include <time.h>

static double started, first;
static double clock_s(void)
{
    struct timespec t;
    assert(!clock_gettime(CLOCK_MONOTONIC, &t));
    return t.tv_sec + t.tv_nsec * 1e-9;
}
static HRESULT timed_write(ISpTTSEngineSite *p,const void *data,ULONG bytes,ULONG *written)
{
    if (!first) first = clock_s() - started;
    return write_pcm(p,data,bytes,written);
}
static const ISpTTSEngineSiteVtbl timing_vtable =
    {NULL,NULL,NULL,events,interest,actions,timed_write,rate,volume,skip,complete};

int main(int argc, char **argv)
{
    assert(argc == 2);
    sapi_test_library = argv[1];
    static const unsigned counts[] = {1, 100, 500, 1200};
    for (int language = 0; language < 2; ++language)
    for (int rate_index = 0; rate_index < 2; ++rate_index) {
        struct sapi_backend *backend = sapi_backend_new(language ? 0x40000 : 0x10000);
        assert(backend);
        struct sapi_settings config = settings();
        WAVEFORMATEX output = format(rate_index ? 48000 : 11025);
        for (int events_enabled = 0; events_enabled < 2; ++events_enabled)
        for (size_t k = 0; k < sizeof counts / sizeof *counts; ++k) {
            const WCHAR *word = language ? L"Beispiel " : L"Example ";
            size_t word_length = wcslen(word), count = counts[k];
            WCHAR *message = calloc(count * word_length + 2, sizeof *message);
            assert(message);
            for (size_t i=0; i<count; ++i)
                memcpy(message + i * word_length, word, word_length * sizeof *message);
            message[count * word_length] = L'.';
            SPVTEXTFRAG fragment = text(message, 0);
            for (int iteration = 0; iteration < 4; ++iteration) {
                struct site result = site_new();
                result.iface.lpVtbl = &timing_vtable;
                result.interest = events_enabled ? ~(ULONGLONG)0 : 0;
                result.abort_after = 1;
                first = 0;
                started = clock_s();
                HRESULT status = sapi_backend_speak(backend,&config,0,&output,&fragment,&result.iface);
                double elapsed = clock_s() - started;
                assert(SUCCEEDED(status) && result.count > 0 && first > 0);
                printf("language=%s hz=%u events=%d words=%u iteration=%d first_ms=%.3f total_ms=%.3f samples=%zu\n",
                    language ? "de" : "en", output.nSamplesPerSec, events_enabled,
                    counts[k], iteration, first*1000, elapsed*1000, result.count);
                free(result.pcm);
            }
            free(message);
        }
        sapi_backend_delete(backend);
    }
    return 0;
}
