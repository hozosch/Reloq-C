/* Run on Windows x64 against the independent C ECI DLL:
 *   eci_windows_x64_runtime_smoke.exe path\to\eci.dll
 *
 * The cross-build only compiles this program; it does not execute the DLL.
 * This checks an actual callback, a connected German phrase, Stop from the
 * callback, and recovery on one persistent ECI handle. It deliberately does
 * not imply that arbitrary text or other sample rates are covered. */
#include "reloq_eci.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN32) && defined(_WIN64)
#include <windows.h>

typedef ECIHand (ECICALL *FnNewEx)(int);
typedef ECIHand (ECICALL *FnDelete)(ECIHand);
typedef void (ECICALL *FnRegisterCallback)(ECIHand, ECICallback, void *);
typedef int (ECICALL *FnSetOutputBuffer)(ECIHand, int, short *);
typedef int (ECICALL *FnAddText)(ECIHand, const void *);
typedef int (ECICALL *FnInsertIndex)(ECIHand, int);
typedef int (ECICALL *FnSynthesize)(ECIHand);
typedef int (ECICALL *FnSynchronize)(ECIHand);
typedef int (ECICALL *FnStop)(ECIHand);
typedef int (ECICALL *FnClearInput)(ECIHand);

struct api {
    FnNewEx new_ex;
    FnDelete delete_engine;
    FnRegisterCallback register_callback;
    FnSetOutputBuffer set_output_buffer;
    FnAddText add_text;
    FnInsertIndex insert_index;
    FnSynthesize synthesize;
    FnSynchronize synchronize;
    FnStop stop;
    FnClearInput clear_input;
};

struct capture {
    struct api *api;
    short *buffer;
    size_t samples;
    size_t nonzero;
    size_t index_position;
    int index_count;
    int index_value;
    int callback_count;
    int stop_on_first_pcm;
};

static int ECICALL on_callback(ECIHand engine, int message, int param,
                              void *data)
{
    struct capture *c = data;
    if (message == RELOQ_ECI_WAVEFORM_BUFFER) {
        if (param < 1 || param > 1100) return 2;
        ++c->callback_count;
        for (int i = 0; i < param; ++i)
            c->nonzero += c->buffer[i] != 0;
        c->samples += (size_t)param;
        if (c->stop_on_first_pcm && c->callback_count == 1 &&
            !c->api->stop(engine)) return 2;
        return 1;
    }
    if (message == RELOQ_ECI_INDEX_REPLY) {
        ++c->index_count;
        c->index_value = param;
        c->index_position = c->samples;
        return 1;
    }
    return 2;
}

/* FARPROC is already a function pointer on Win64. Copy its representation to
 * the typed pointer without a nonportable function-pointer cast warning. */
#define LOAD(member, symbol) do {                                         \
    FARPROC proc = GetProcAddress(lib, symbol);                           \
    if (!proc || sizeof api.member != sizeof proc) {                      \
        fprintf(stderr, "required ECI export missing: %s\n", symbol);    \
        FreeLibrary(lib);                                                 \
        return 1;                                                         \
    }                                                                     \
    memcpy(&api.member, &proc, sizeof proc);                               \
} while (0)

int main(int argc, char **argv)
{
    if (argc != 2) {
        fputs("usage: eci_windows_x64_runtime_smoke.exe path-to-eci.dll\n",
              stderr);
        return 2;
    }
    HMODULE lib = LoadLibraryA(argv[1]);
    if (!lib) {
        fprintf(stderr, "could not load ECI DLL (Win32 error %lu)\n",
                (unsigned long)GetLastError());
        return 1;
    }
    struct api api = {0};
    LOAD(new_ex, "eciNewEx");
    LOAD(delete_engine, "eciDelete");
    LOAD(register_callback, "eciRegisterCallback");
    LOAD(set_output_buffer, "eciSetOutputBuffer");
    LOAD(add_text, "eciAddText");
    LOAD(insert_index, "eciInsertIndex");
    LOAD(synthesize, "eciSynthesize");
    LOAD(synchronize, "eciSynchronize");
    LOAD(stop, "eciStop");
    LOAD(clear_input, "eciClearInput");

    ECIHand engine = api.new_ex(RELOQ_ECI_GERMAN);
    if (!engine) {
        fputs("eciNewEx rejected German voice\n", stderr);
        FreeLibrary(lib);
        return 1;
    }
    int passed = 0;
    short buffer[1100];
    struct capture c = {.api = &api, .buffer = buffer};
    api.register_callback(engine, on_callback, &c);
    if (!api.set_output_buffer(engine, 1100, buffer)) goto done;

    if (!api.add_text(engine, "Pack Tipp.") ||
        !api.insert_index(engine, 10) ||
        !api.synthesize(engine) || !api.synchronize(engine) ||
        c.samples != 11220 || !c.nonzero || !c.callback_count ||
        c.index_count != 1 || c.index_value != 10 ||
        c.index_position != c.samples) goto done;

    /* An NVDA text field commonly supplies an unpunctuated word. The
     * native DLL, not the preview plugin, supplies final-period prosody. */
    static const struct { const char *word; size_t samples; } words[] = {
        {"Mett", 8338}, {"Bett", 8426}
    };
    for (size_t i = 0; i < sizeof words / sizeof words[0]; ++i) {
        memset(&c, 0, sizeof c);
        c.api = &api;
        c.buffer = buffer;
        if (!api.add_text(engine, words[i].word) ||
            !api.insert_index(engine, 12 + (int)i) ||
            !api.synthesize(engine) || !api.synchronize(engine) ||
            c.samples != words[i].samples || !c.nonzero ||
            c.index_count != 1 || c.index_value != 12 + (int)i ||
            c.index_position != c.samples) goto done;
    }

    memset(&c, 0, sizeof c);
    c.api = &api;
    c.buffer = buffer;
    if (!api.add_text(engine, "Maus.") || api.synthesize(engine) ||
        c.samples || c.index_count) goto done;
    if (!api.clear_input(engine)) goto done;

    memset(&c, 0, sizeof c);
    c.api = &api;
    c.buffer = buffer;
    c.stop_on_first_pcm = 1;
    if (!api.add_text(engine, "Hieb.") || api.synthesize(engine) ||
        c.callback_count != 1 || !c.samples || c.index_count) goto done;

    memset(&c, 0, sizeof c);
    c.api = &api;
    c.buffer = buffer;
    if (!api.add_text(engine, "Hass?") ||
        !api.insert_index(engine, 11) ||
        !api.synthesize(engine) || !api.synchronize(engine) ||
        !c.samples || !c.nonzero || c.index_count != 1 ||
        c.index_value != 11 || c.index_position != c.samples) goto done;
    passed = 1;
done:
    if (api.delete_engine(engine)) {
        fputs("eciDelete reported an active render\n", stderr);
        passed = 0;
    }
    FreeLibrary(lib);
    if (!passed) {
        fputs("x64 ECI runtime smoke failed\n", stderr);
        return 1;
    }
    puts("x64 ECI runtime: connected PCM, index, Stop, recovery OK");
    return 0;
}
#else
int main(void)
{
    fputs("This smoke test must run on Windows x64.\n", stderr);
    return 2;
}
#endif
