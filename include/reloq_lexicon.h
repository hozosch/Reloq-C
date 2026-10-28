#ifndef RELOQ_LEXICON_H
#define RELOQ_LEXICON_H

#include <stddef.h>
#include <stdint.h>

/* One independently authored, decoded key in a language's action table.
 * The two context counts and action reference are opaque lookup metadata:
 * neither an action number nor a matching key specifies pronunciation.
 * table_id distinguishes dictionaries within ONE caller-selected language.
 */
struct reloq_lexicon_action {
    uint32_t table_id;
    const unsigned char *key;
    size_t key_length;
    uint8_t context_counts[2];
    uint16_t action_ref;
};

/* A separately verified, explicit word-to-SPR replacement. A language
 * frontend supplies both the key encoding and a validator for its SPR
 * alphabet. This record does not derive from an action reference. */
struct reloq_lexicon_override {
    const unsigned char *key;
    size_t key_length;
    const unsigned char *spr;
    size_t spr_length;
};

struct reloq_lexicon {
    const struct reloq_lexicon_action *actions;
    size_t action_count;
    const struct reloq_lexicon_override *overrides;
    size_t override_count;
};

/* Return 1 only for a complete, valid pronunciation in the selected
 * language. Linguistic accuracy and rights must also be checked by the data
 * author; a syntax validator alone cannot establish either. */
typedef int (*reloq_lexicon_spr_validator)(const unsigned char *spr,
                                           size_t length, void *context);

/* Bind immutable, caller-owned arrays without allocation or copying.
 * Actions must be strictly ordered by (table_id, key), overrides strictly
 * ordered by key. Keys are nonempty, length-bounded byte strings without
 * NUL; no normalization, case folding or character conversion occurs.
 * A validator is mandatory when overrides are supplied. Empty arrays are
 * allowed with NULL pointers. All arrays and their byte spans must remain
 * alive and immutable while the view is used. Return 0 on success, -1 for
 * invalid data; on failure *view remains unchanged. */
int reloq_lexicon_bind(struct reloq_lexicon *view,
                      const struct reloq_lexicon_action *actions,
                      size_t action_count,
                      const struct reloq_lexicon_override *overrides,
                      size_t override_count,
                      reloq_lexicon_spr_validator validate_spr,
                      void *validator_context);

/* Exact bounded lookup in a successfully bound view. Return 1 on a hit, 0
 * on a miss, -1 on invalid arguments. *found changes only on a hit. A query
 * is in the same caller-chosen encoding as the stored keys. An action hit
 * is metadata only; clients must never interpret it as an SPR override. */
int reloq_lexicon_find_action(
    const struct reloq_lexicon *view, uint32_t table_id,
    const unsigned char *key, size_t key_length,
    const struct reloq_lexicon_action **found);
int reloq_lexicon_find_override(
    const struct reloq_lexicon *view,
    const unsigned char *key, size_t key_length,
    const struct reloq_lexicon_override **found);

/* Worker-scoped immutable lexical data supplied by an optional dictionary.
 * The resolver shares the standard lookup path, including morphology. Its
 * records must stay alive for the entire scope; nested scopes restore the
 * preceding resolver. No text normalization or utterance result is retained. */
struct reloq_lexicon_resolver {
    void *context;
    const struct reloq_lexicon_override *(*find)(void *,const unsigned char *,size_t);
};
const struct reloq_lexicon_resolver *reloq_lexicon_resolver_scope(
    const struct reloq_lexicon_resolver *);
/* Whether this exact record came from the current optional lexical view.
 * Native source-word facts are not metadata of these SPR-only entries. */
int reloq_lexicon_override_from_resolver(const struct reloq_lexicon_override *);

#endif
