/* Handing out the one object the library exports, and the engine facade it
 * wraps.
 *
 * Three kinds can be asked for. One and two both answer with an
 * EngineWrapper, so the caller gets the same thing whichever it names;
 * three answers with the licence object itself. Anything else is refused.
 * Whatever comes back has had a reference added before it is handed over.
 *
 * The licence lives in a static built the first time somebody asks. Its
 * initialization must finish before another thread uses that static.
 */

#include <stdint.h>
#include <stdatomic.h>
#include "eci_synththread.h"
#include "evv_abi.h"


typedef STDCALL uint32_t (*AddRefFn)(void *self);

/* Slot one of anything's table adds a reference. */
#define ADD_REF(p) (((AddRefFn *)(*(void ***)(p)))[1](p))

/* Which kind the caller is asking for. */
#define OBJ_ENGINE_A 1
#define OBJ_ENGINE_B 2
#define OBJ_LICENCE  3

/* The engine facade, which is in eci_enginewrap.c. Only its size is
   wanted here, to ask for the room before constructing one. */
extern const uint32_t ew_bytes;

typedef struct RequestLicense {
    const void *vt;
    int32_t     granted;
} RequestLicense;

extern void *cpp_new(uint32_t n) MANGLED("??2@YAPAXI@Z");
extern THIS RequestLicense *rl_ctor(RequestLicense *r);
extern THIS int32_t rl_licenseGranted(RequestLicense *r);
extern THIS void *ew_ctor(void *e);

/* The licence is shared by all provider instances. Publish it only after its
 * constructor has finished; a second simultaneous voice must not observe a
 * partially initialized licence or spuriously fail to create its engine. */
static RequestLicense licence;
static atomic_int     licence_state;

static void ensureLicence(void)
{
    if (atomic_load_explicit(&licence_state,memory_order_acquire) == 2)
        return;
    int expected = 0;
    if (atomic_compare_exchange_strong_explicit(&licence_state,&expected,1,
            memory_order_acquire,memory_order_acquire)) {
        rl_ctor(&licence);
        atomic_store_explicit(&licence_state,2,memory_order_release);
    } else {
        while (atomic_load_explicit(&licence_state,memory_order_acquire) != 2) { }
    }
}

/* Nothing to do. Whatever this once set up is set up by the time anything
   calls it, and the original left the body empty too. */
void initDllLink(void)
{
}

int getObject(int32_t kind, void **out)
{
    ensureLicence();

    *out = 0;

    if (kind == OBJ_ENGINE_A || kind == OBJ_ENGINE_B) {
        if (rl_licenseGranted(&licence)) {
            void *p = cpp_new(ew_bytes);

            *out = p ? ew_ctor(p) : 0;
            if (*out != 0)
                ADD_REF(*out);
        }
    }

    if (kind == OBJ_LICENCE) {
        *out = &licence;
        ADD_REF(*out);
    }

    return *out != 0;
}

