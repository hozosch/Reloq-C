/* Inspect an ECI-compatible Windows x64 DLL's public symbols without
 * pretending that the current partial frontend can synthesize arbitrary text.
 *
 * Build on Windows x64 or with MinGW-w64:
 *   x86_64-w64-mingw32-gcc -std=c11 -Wall -Wextra -Werror -O2 \
 *     -o eci_windows_x64_abi_probe.exe eci_windows_x64_abi_probe.c
 * Run in Windows:
 *   eci_windows_x64_abi_probe.exe path\\to\\eci.dll [--ibmtts]
 *
 * The optional --ibmtts checks every ECI symbol used by the inspected
 * IBMTTS driver. The default checks a minimal audio-and-index subset.
 * This does not invoke speech or establish implementation semantics.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN32) && defined(_WIN64)
#include <windows.h>

typedef void *ECIHand;
typedef int (__stdcall *ECICallback)(ECIHand, int, int, void *);
typedef ECIHand (__stdcall *ECINewEx)(int);
typedef ECIHand (__stdcall *ECIDelete)(ECIHand);
typedef void (__stdcall *ECIRegisterCallback)(ECIHand, ECICallback, void *);
typedef int (__stdcall *ECISetOutputBuffer)(ECIHand, int, int16_t *);
typedef int (__stdcall *ECIAddText)(ECIHand, const void *);
typedef int (__stdcall *ECIInsertIndex)(ECIHand, int);
typedef int (__stdcall *ECISynthesize)(ECIHand);
typedef int (__stdcall *ECIStop)(ECIHand);
typedef int (__stdcall *ECISetParam)(ECIHand, int, int);
typedef int (__stdcall *ECIGetParam)(ECIHand, int);

_Static_assert(sizeof(ECIHand) == 8, "ECI handles must retain 64 bits");
_Static_assert(sizeof(int) == 4, "ECI's parameter count must be 32-bit");
_Static_assert(sizeof(int16_t) == 2, "ECI PCM samples must be 16-bit");
_Static_assert(sizeof(ECICallback) == 8, "callback pointer must retain 64 bits");
_Static_assert(sizeof(ECINewEx) == sizeof(FARPROC), "PE function pointer width");
_Static_assert(sizeof(ECIDelete) == sizeof(FARPROC), "PE function pointer width");
_Static_assert(sizeof(ECIRegisterCallback) == sizeof(FARPROC), "PE function pointer width");
_Static_assert(sizeof(ECISetOutputBuffer) == sizeof(FARPROC), "PE function pointer width");
_Static_assert(sizeof(ECIAddText) == sizeof(FARPROC), "PE function pointer width");
_Static_assert(sizeof(ECIInsertIndex) == sizeof(FARPROC), "PE function pointer width");
_Static_assert(sizeof(ECISynthesize) == sizeof(FARPROC), "PE function pointer width");
_Static_assert(sizeof(ECIStop) == sizeof(FARPROC), "PE function pointer width");
_Static_assert(sizeof(ECISetParam) == sizeof(FARPROC), "PE function pointer width");
_Static_assert(sizeof(ECIGetParam) == sizeof(FARPROC), "PE function pointer width");

static const char *const audio_symbols[] = {
    "eciNewEx", "eciDelete", "eciRegisterCallback", "eciSetOutputBuffer",
    "eciAddText", "eciInsertIndex", "eciSynthesize", "eciStop"
};

static const char *const ibmtts_additional_symbols[] = {
    "eciVersion", "eciGetAvailableLanguages", "eciGetParam", "eciSetParam",
    "eciGetVoiceParam", "eciSetVoiceParam", "eciCopyVoice",
    "eciNewDict", "eciLoadDict", "eciSetDict"
};

static unsigned check_symbols(HMODULE lib, const char *const *names, size_t count)
{
    unsigned missing = 0;
    for (size_t i = 0; i < count; ++i) {
        FARPROC symbol = GetProcAddress(lib, names[i]);
        if (!symbol) {
            fprintf(stderr, "missing undecorated export: %s\n", names[i]);
            ++missing;
        }
    }
    return missing;
}

int main(int argc, char **argv)
{
    if (argc != 2 && (argc != 3 || strcmp(argv[2], "--ibmtts") != 0)) {
        fprintf(stderr, "usage: %s path-to-x64-eci.dll [--ibmtts]\n", argv[0]);
        return 2;
    }
    HMODULE lib = LoadLibraryA(argv[1]);
    if (!lib) {
        fprintf(stderr, "cannot load x64 DLL (Win32 error %lu)\n",
                (unsigned long)GetLastError());
        return 1;
    }
    unsigned missing = check_symbols(lib, audio_symbols,
                                     sizeof audio_symbols / sizeof audio_symbols[0]);
    if (argc == 3)
        missing += check_symbols(lib, ibmtts_additional_symbols,
                                 sizeof ibmtts_additional_symbols /
                                 sizeof ibmtts_additional_symbols[0]);
    FreeLibrary(lib);
    if (missing) {
        fprintf(stderr, "%u required ECI export(s) absent\n", missing);
        return 1;
    }
    puts("x64 ECI export names and pointer widths pass (no speech executed)");
    return 0;
}
#else
int main(void)
{
    fputs("This ABI probe must be compiled as a Windows x64 executable.\n",
          stderr);
    return 2;
}
#endif
