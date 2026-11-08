#ifndef RELOQ61_GERMAN_FINAL_H
#define RELOQ61_GERMAN_FINAL_H
#include <stddef.h>
#include <stdint.h>
struct reloq61_german_final_entry { int32_t phone,end; };
struct reloq61_german_final_a {
    void *state;
    struct reloq61_german_final_entry *entries;
    size_t count,capacity;
    int failed;
};
struct reloq61_german_final_a reloq61_german_final_scope(struct reloq61_german_final_a);
void reloq61_german_final_dispose(struct reloq61_german_final_a);
void reloq61_german_final_note(void *,int32_t,int32_t);
int reloq61_german_final_matches(void *,int32_t,int32_t);
#endif
