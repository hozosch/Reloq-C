#include "reloq_lexicon.h"

#include <stdint.h>
#include <string.h>

static int valid_key(const unsigned char *key, size_t length)
{
    if (!key || !length) return 0;
    for (size_t i = 0; i < length; ++i)
        if (!key[i]) return 0;
    return 1;
}

static int compare_key(const unsigned char *a, size_t a_length,
                       const unsigned char *b, size_t b_length)
{
    size_t common = a_length < b_length ? a_length : b_length;
    int result = memcmp(a, b, common);
    if (result) return result;
    return (a_length > b_length) - (a_length < b_length);
}

int reloq_lexicon_bind(struct reloq_lexicon *view,
                      const struct reloq_lexicon_action *actions,
                      size_t action_count,
                      const struct reloq_lexicon_override *overrides,
                      size_t override_count,
                      reloq_lexicon_spr_validator validate_spr,
                      void *validator_context)
{
    if (!view || (action_count && !actions) ||
        (override_count && (!overrides || !validate_spr)) ||
        action_count > PTRDIFF_MAX / sizeof *actions ||
        override_count > PTRDIFF_MAX / sizeof *overrides)
        return -1;

    for (size_t i = 0; i < action_count; ++i) {
        if (!valid_key(actions[i].key, actions[i].key_length)) return -1;
        if (i && (actions[i - 1].table_id > actions[i].table_id ||
                  (actions[i - 1].table_id == actions[i].table_id &&
                   compare_key(actions[i - 1].key,
                               actions[i - 1].key_length,
                               actions[i].key, actions[i].key_length) >= 0)))
            return -1;
    }

    for (size_t i = 0; i < override_count; ++i) {
        if (!valid_key(overrides[i].key, overrides[i].key_length) ||
            !overrides[i].spr || !overrides[i].spr_length ||
            validate_spr(overrides[i].spr, overrides[i].spr_length,
                         validator_context) != 1)
            return -1;
        if (i && compare_key(overrides[i - 1].key,
                             overrides[i - 1].key_length,
                             overrides[i].key,
                             overrides[i].key_length) >= 0)
            return -1;
    }

    struct reloq_lexicon bound = {actions, action_count,
                                  overrides, override_count};
    *view = bound;
    return 0;
}

int reloq_lexicon_find_action(
    const struct reloq_lexicon *view, uint32_t table_id,
    const unsigned char *key, size_t key_length,
    const struct reloq_lexicon_action **found)
{
    if (!view || !found || !valid_key(key, key_length) ||
        (view->action_count && !view->actions)) return -1;
    size_t low = 0, high = view->action_count;
    while (low < high) {
        size_t at = low + (high - low) / 2;
        const struct reloq_lexicon_action *entry = &view->actions[at];
        int order = entry->table_id < table_id ? -1 :
                    entry->table_id > table_id ? 1 :
                    compare_key(entry->key, entry->key_length,
                                key, key_length);
        if (!order) {
            *found = entry;
            return 1;
        }
        if (order < 0) low = at + 1;
        else high = at;
    }
    return 0;
}

static _Thread_local const struct reloq_lexicon_resolver *active_resolver;
const struct reloq_lexicon_resolver *reloq_lexicon_resolver_scope(
    const struct reloq_lexicon_resolver *next)
{
    const struct reloq_lexicon_resolver *previous=active_resolver;
    active_resolver=next;return previous;
}

int reloq_lexicon_override_from_resolver(const struct reloq_lexicon_override *entry)
{
    return entry&&active_resolver&&active_resolver->find&&
        active_resolver->find(active_resolver->context,entry->key,entry->key_length)==entry;
}

int reloq_lexicon_find_override(
    const struct reloq_lexicon *view,
    const unsigned char *key, size_t key_length,
    const struct reloq_lexicon_override **found)
{
    if (!view || !found || !valid_key(key, key_length) ||
        (view->override_count && !view->overrides)) return -1;
    if(active_resolver&&active_resolver->find) {
        const struct reloq_lexicon_override *entry=active_resolver->find(
            active_resolver->context,key,key_length);
        if(entry){*found=entry;return 1;}
    }
    size_t low = 0, high = view->override_count;
    while (low < high) {
        size_t at = low + (high - low) / 2;
        const struct reloq_lexicon_override *entry = &view->overrides[at];
        int order = compare_key(entry->key, entry->key_length,
                                key, key_length);
        if (!order) {
            *found = entry;
            return 1;
        }
        if (order < 0) low = at + 1;
        else high = at;
    }
    return 0;
}
