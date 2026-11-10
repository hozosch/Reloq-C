#ifndef RELOQ_UPDATE_DATA_H
#define RELOQ_UPDATE_DATA_H
#include <stddef.h>
#include <stdint.h>
#define RELOQ_UPDATE_LIMIT (128u * 1024u * 1024u)
struct reloq_update {
    char version[48], name[96], url[512], sha256[65];
    uint32_t size;
};
/* -1 malformed/untrusted, 0 no newer stable package, 1 verified metadata. */
int reloq_update_parse(const char *json, size_t length, const char *installed, struct reloq_update *out);
#endif
