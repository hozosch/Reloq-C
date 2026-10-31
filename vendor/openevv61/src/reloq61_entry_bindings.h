/* Native C argument projections, no runtime instruction decoder. */
#ifndef RELOQ61_ENTRY_BINDINGS_H
#define RELOQ61_ENTRY_BINDINGS_H
#define RELOQ61_ENTRY_ARG(n) (nargs>(n)?args[n]:0)
#define RELOQ61_ENTRY1(name, call) \
extern int32_t call(void *, int32_t); \
static int32_t name(void *state, const int32_t *args, int nargs) \
{ return call(state, RELOQ61_ENTRY_ARG(0)); }
#define RELOQ61_ENTRY2(name, call) \
extern int32_t call(void *, int32_t, int32_t); \
static int32_t name(void *state, const int32_t *args, int nargs) \
{ return call(state, RELOQ61_ENTRY_ARG(0), RELOQ61_ENTRY_ARG(1)); }
#define RELOQ61_ENTRY3(name, call) \
extern int32_t call(void *, int32_t, int32_t, int32_t); \
static int32_t name(void *state, const int32_t *args, int nargs) \
{ return call(state, RELOQ61_ENTRY_ARG(0), RELOQ61_ENTRY_ARG(1), RELOQ61_ENTRY_ARG(2)); }
#define RELOQ61_ENTRY4(name, call) \
extern int32_t call(void *, int32_t, int32_t, int32_t, int32_t); \
static int32_t name(void *state, const int32_t *args, int nargs) \
{ return call(state, RELOQ61_ENTRY_ARG(0), RELOQ61_ENTRY_ARG(1), RELOQ61_ENTRY_ARG(2), RELOQ61_ENTRY_ARG(3)); }
#define RELOQ61_ENTRY5(name, call) \
extern int32_t call(void *, int32_t, int32_t, int32_t, int32_t, int32_t); \
static int32_t name(void *state, const int32_t *args, int nargs) \
{ return call(state, RELOQ61_ENTRY_ARG(0), RELOQ61_ENTRY_ARG(1), RELOQ61_ENTRY_ARG(2), RELOQ61_ENTRY_ARG(3), RELOQ61_ENTRY_ARG(4)); }
#define RELOQ61_ENTRY6(name, call) \
extern int32_t call(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t); \
static int32_t name(void *state, const int32_t *args, int nargs) \
{ return call(state, RELOQ61_ENTRY_ARG(0), RELOQ61_ENTRY_ARG(1), RELOQ61_ENTRY_ARG(2), RELOQ61_ENTRY_ARG(3), RELOQ61_ENTRY_ARG(4), RELOQ61_ENTRY_ARG(5)); }
#define RELOQ61_ENTRY7(name, call) \
extern int32_t call(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t); \
static int32_t name(void *state, const int32_t *args, int nargs) \
{ return call(state, RELOQ61_ENTRY_ARG(0), RELOQ61_ENTRY_ARG(1), RELOQ61_ENTRY_ARG(2), RELOQ61_ENTRY_ARG(3), RELOQ61_ENTRY_ARG(4), RELOQ61_ENTRY_ARG(5), RELOQ61_ENTRY_ARG(6)); }
#define RELOQ61_ENTRY8(name, call) \
extern int32_t call(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t); \
static int32_t name(void *state, const int32_t *args, int nargs) \
{ return call(state, RELOQ61_ENTRY_ARG(0), RELOQ61_ENTRY_ARG(1), RELOQ61_ENTRY_ARG(2), RELOQ61_ENTRY_ARG(3), RELOQ61_ENTRY_ARG(4), RELOQ61_ENTRY_ARG(5), RELOQ61_ENTRY_ARG(6), RELOQ61_ENTRY_ARG(7)); }
#define RELOQ61_ENTRY9(name, call) \
extern int32_t call(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t); \
static int32_t name(void *state, const int32_t *args, int nargs) \
{ return call(state, RELOQ61_ENTRY_ARG(0), RELOQ61_ENTRY_ARG(1), RELOQ61_ENTRY_ARG(2), RELOQ61_ENTRY_ARG(3), RELOQ61_ENTRY_ARG(4), RELOQ61_ENTRY_ARG(5), RELOQ61_ENTRY_ARG(6), RELOQ61_ENTRY_ARG(7), RELOQ61_ENTRY_ARG(8)); }
#define RELOQ61_ENTRY10(name, call) \
extern int32_t call(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t); \
static int32_t name(void *state, const int32_t *args, int nargs) \
{ return call(state, RELOQ61_ENTRY_ARG(0), RELOQ61_ENTRY_ARG(1), RELOQ61_ENTRY_ARG(2), RELOQ61_ENTRY_ARG(3), RELOQ61_ENTRY_ARG(4), RELOQ61_ENTRY_ARG(5), RELOQ61_ENTRY_ARG(6), RELOQ61_ENTRY_ARG(7), RELOQ61_ENTRY_ARG(8), RELOQ61_ENTRY_ARG(9)); }
#define RELOQ61_ENTRY11(name, call) \
extern int32_t call(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t); \
static int32_t name(void *state, const int32_t *args, int nargs) \
{ return call(state, RELOQ61_ENTRY_ARG(0), RELOQ61_ENTRY_ARG(1), RELOQ61_ENTRY_ARG(2), RELOQ61_ENTRY_ARG(3), RELOQ61_ENTRY_ARG(4), RELOQ61_ENTRY_ARG(5), RELOQ61_ENTRY_ARG(6), RELOQ61_ENTRY_ARG(7), RELOQ61_ENTRY_ARG(8), RELOQ61_ENTRY_ARG(9), RELOQ61_ENTRY_ARG(10)); }
#define RELOQ61_ENTRY12(name, call) \
extern int32_t call(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t); \
static int32_t name(void *state, const int32_t *args, int nargs) \
{ return call(state, RELOQ61_ENTRY_ARG(0), RELOQ61_ENTRY_ARG(1), RELOQ61_ENTRY_ARG(2), RELOQ61_ENTRY_ARG(3), RELOQ61_ENTRY_ARG(4), RELOQ61_ENTRY_ARG(5), RELOQ61_ENTRY_ARG(6), RELOQ61_ENTRY_ARG(7), RELOQ61_ENTRY_ARG(8), RELOQ61_ENTRY_ARG(9), RELOQ61_ENTRY_ARG(10), RELOQ61_ENTRY_ARG(11)); }
#define RELOQ61_ENTRY13(name, call) \
extern int32_t call(void *, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t); \
static int32_t name(void *state, const int32_t *args, int nargs) \
{ return call(state, RELOQ61_ENTRY_ARG(0), RELOQ61_ENTRY_ARG(1), RELOQ61_ENTRY_ARG(2), RELOQ61_ENTRY_ARG(3), RELOQ61_ENTRY_ARG(4), RELOQ61_ENTRY_ARG(5), RELOQ61_ENTRY_ARG(6), RELOQ61_ENTRY_ARG(7), RELOQ61_ENTRY_ARG(8), RELOQ61_ENTRY_ARG(9), RELOQ61_ENTRY_ARG(10), RELOQ61_ENTRY_ARG(11), RELOQ61_ENTRY_ARG(12)); }
#define RELOQ61_ENTRY_BOUND(name, call) \
static int32_t name(void *state, const int32_t *args, int nargs) \
{ return call(state, args, nargs); }
#endif
