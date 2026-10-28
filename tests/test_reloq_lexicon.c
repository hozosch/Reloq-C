#include "reloq_lexicon.h"
#include "reloq_german_lexicon.h"
#include "reloq_german_phone_sequence.h"
#include "reloq_german_text.h"

#include <string.h>

#define CHECK(test) do { if (!(test)) return __LINE__; } while (0)

/* All keys, references and pronunciations below are synthetic test data,
 * not extracted dictionary records or asserted lexical pronunciations. */
static const unsigned char k_bach[] = "Bach";
static const unsigned char k_maus[] = "Maus";
static const unsigned char k_maeuse[] = "M\xe4use";
static const unsigned char k_schnee[] = "Schnee";
static const unsigned char spr_maus[] = "`[.1ma]";
static const unsigned char spr_maeuse[] = "`[.1m'aw's]";

static const struct reloq_lexicon_action actions[] = {
    {2, k_bach, sizeof k_bach - 1, {1, 2}, 14},
    {2, k_maus, sizeof k_maus - 1, {2, 0}, 37},
    {2, k_schnee, sizeof k_schnee - 1, {0, 0}, 77},
    {9, k_maus, sizeof k_maus - 1, {3, 1}, 8}
};
static const struct reloq_lexicon_override overrides[] = {
    {k_maus, sizeof k_maus - 1, spr_maus, sizeof spr_maus - 1},
    {k_maeuse, sizeof k_maeuse - 1, spr_maeuse,
     sizeof spr_maeuse - 1}
};

static int permissive_validator(const unsigned char *spr, size_t length,
                                void *context)
{
    (void)spr;
    (void)length;
    (void)context;
    return 1;
}

static int bound_lookup(void)
{
    struct reloq_lexicon view = {0};
    CHECK(reloq_lexicon_bind(&view, actions, 4, overrides, 2,
                             reloq_german_lexicon_validate_spr, NULL) == 0);
    const struct reloq_lexicon_action *action = NULL;
    CHECK(reloq_lexicon_find_action(&view, 2, k_maus, 4,
                                    &action) == 1 && action == actions + 1 &&
          action->action_ref == 37 && action->context_counts[0] == 2);
    CHECK(reloq_lexicon_find_action(&view, 9, k_maus, 4,
                                    &action) == 1 && action == actions + 3);
    CHECK(reloq_lexicon_find_action(&view, 3, k_maus, 4,
                                    &action) == 0 && action == actions + 3);
    CHECK(reloq_lexicon_find_action(&view, 9, k_maus, 3,
                                    &action) == 0 && action == actions + 3);
    CHECK(reloq_lexicon_find_action(&view, 9, k_maus, 0,
                                    &action) == -1 && action == actions + 3);

    const struct reloq_lexicon_override *override = NULL;
    CHECK(reloq_lexicon_find_override(&view, k_maus, 4,
                                      &override) == 1 &&
          override == overrides);
    CHECK(reloq_lexicon_find_override(&view, k_bach, 4,
                                      &override) == 0 &&
          override == overrides);
    CHECK(reloq_lexicon_find_override(&view, k_maus, 3,
                                      &override) == 0 &&
          override == overrides);
    return 0;
}

static int rejects_bad_data(void)
{
    struct reloq_lexicon view = {0};
    CHECK(reloq_lexicon_bind(&view, actions, 4, overrides, 2,
                             reloq_german_lexicon_validate_spr, NULL) == 0);
    const struct reloq_lexicon_action *previous_actions = view.actions;
    const struct reloq_lexicon_override *previous_overrides = view.overrides;
    CHECK(reloq_lexicon_bind(&view, actions, 4, overrides, 2,
                             NULL, NULL) == -1);
    CHECK(reloq_lexicon_bind(&view, NULL, 1, NULL, 0,
                             NULL, NULL) == -1);
    struct reloq_lexicon_action bad_actions[] = {actions[1], actions[0]};
    CHECK(reloq_lexicon_bind(&view, bad_actions, 2, NULL, 0,
                             NULL, NULL) == -1);
    bad_actions[0] = actions[0];
    bad_actions[1] = actions[0];
    CHECK(reloq_lexicon_bind(&view, bad_actions, 2, NULL, 0,
                             NULL, NULL) == -1);
    bad_actions[0].key = (const unsigned char *)"A\0B";
    bad_actions[0].key_length = 3;
    CHECK(reloq_lexicon_bind(&view, bad_actions, 1, NULL, 0,
                             NULL, NULL) == -1);
    struct reloq_lexicon_override bad_overrides[] = {overrides[1], overrides[0]};
    CHECK(reloq_lexicon_bind(&view, NULL, 0, bad_overrides, 2,
                             reloq_german_lexicon_validate_spr, NULL) == -1);
    bad_overrides[0] = overrides[0];
    bad_overrides[1] = overrides[0];
    CHECK(reloq_lexicon_bind(&view, NULL, 0, bad_overrides, 2,
                             reloq_german_lexicon_validate_spr, NULL) == -1);
    bad_overrides[0].spr = (const unsigned char *)"[.1?]";
    bad_overrides[0].spr_length = 5;
    CHECK(reloq_lexicon_bind(&view, NULL, 0, bad_overrides, 1,
                             reloq_german_lexicon_validate_spr, NULL) == -1);
    bad_overrides[0].spr = NULL;
    CHECK(reloq_lexicon_bind(&view, NULL, 0, bad_overrides, 1,
                             reloq_german_lexicon_validate_spr, NULL) == -1);
    CHECK(view.actions == previous_actions && view.action_count == 4 &&
          view.overrides == previous_overrides && view.override_count == 2);
    return 0;
}

static int priority_and_transaction(void)
{
    struct reloq_lexicon view = {0};
    CHECK(reloq_lexicon_bind(&view, actions, 4, overrides, 2,
                             reloq_german_lexicon_validate_spr, NULL) == 0);
    unsigned char output[64] = "unchanged";
    size_t length = 99;
    CHECK(reloq_german_lexicon_word(&view, k_maus, 4,
                                    output, 4, &length) == -2);
    CHECK(length == sizeof spr_maus - 1 &&
          strcmp((const char *)output, "unchanged") == 0);
    CHECK(reloq_german_lexicon_word(&view, k_maus, 4,
                                    NULL, 0, &length) == -2 &&
          length == sizeof spr_maus - 1);
    CHECK(reloq_german_lexicon_word(&view, k_maus, 4,
                                    output, sizeof output, &length) == 1 &&
          length == sizeof spr_maus - 1 &&
          strcmp((const char *)output, (const char *)spr_maus) == 0);

    /* A matching action alone must NOT resolve an uncovered word. */
    memcpy(output, "unchanged", 10);
    length = 99;
    CHECK(reloq_german_lexicon_word(&view, k_bach, 4,
                                    output, sizeof output, &length) == 0 &&
          length == 99 && strcmp((const char *)output, "unchanged") == 0);
    /* A matching action alone must NOT alter a generated pronunciation. */
    CHECK(reloq_german_lexicon_word(&view, k_schnee, 6,
                                    output, sizeof output, &length) == 1 &&
          strcmp((const char *)output, "`[.1Sne]") == 0);
    length = 99;
    CHECK(reloq_german_lexicon_word(&view, (const unsigned char *)"x`p3", 4,
                                    output, sizeof output, &length) == -1 &&
          length == 99);

    /* A malformed view and an override accepted by an unrelated language
     * validator must not enter the German annotation or change caller data. */
    struct reloq_lexicon broken = {0};
    broken.override_count = 1;
    CHECK(reloq_german_lexicon_word(&broken, k_maus, 4,
                                    output, sizeof output, &length) == -1 &&
          length == 99);
    const unsigned char invalid_spr[] = "[.1ma]"; /* missing control prefix */
    struct reloq_lexicon_override invalid = {
        k_maus, 4, invalid_spr, sizeof invalid_spr - 1
    };
    CHECK(reloq_lexicon_bind(&broken, NULL, 0, &invalid, 1,
                             permissive_validator, NULL) == 0);
    memcpy(output, "unchanged", 10);
    CHECK(reloq_german_lexicon_word(&broken, k_maus, 4,
                                    output, sizeof output, &length) == -1 &&
          length == 99 && strcmp((const char *)output, "unchanged") == 0);
    return 0;
}

static int text_adapter(void)
{
    struct reloq_lexicon view = {0};
    CHECK(reloq_lexicon_bind(&view, actions, 4, overrides, 2,
                             reloq_german_lexicon_validate_spr, NULL) == 0);
    const unsigned char input[] = "M\xc3\xa4use, Schnee!";
    const unsigned char expected[] = "`[.1m'aw's], `[.1Sne]!";
    unsigned char output[96] = {0};
    size_t length = 0, bad = 72;
    CHECK(reloq_german_text_annotate_with_lexicon(
              input, sizeof input - 1, output, sizeof output,
              &length, &bad, &view) == RELOQ_DE_TEXT_OK);
    CHECK(length == sizeof expected - 1 && bad == 72 &&
          memcmp(output, expected, sizeof expected) == 0);
    struct reloq_german_phone_node nodes[8] = {{0}};
    struct reloq_german_phone_sequence_info info = {0};
    CHECK(reloq_german_phone_sequence_build(output, length, 0,
                                             nodes, 8, &info) == 0);
    CHECK(info.word_count == 2 && info.phone_count == 6);

    /* Missing overrides leave the old API's results and errors intact. */
    const unsigned char fallback[] = "Schnee!";
    unsigned char original[96] = {0};
    size_t original_length = 0;
    CHECK(reloq_german_text_annotate(fallback, sizeof fallback - 1,
                                      original, sizeof original,
                                      &original_length, NULL) ==
          RELOQ_DE_TEXT_OK);
    CHECK(reloq_german_text_annotate_with_lexicon(
              fallback, sizeof fallback - 1, output, sizeof output,
              &length, NULL, &view) == RELOQ_DE_TEXT_OK);
    CHECK(length == original_length &&
          memcmp(output, original, length + 1) == 0);
    const unsigned char uncovered[] = "M\xc3\xa4use Bach";
    memcpy(output, "unchanged", 10);
    length = 99;
    CHECK(reloq_german_text_annotate_with_lexicon(
              uncovered, sizeof uncovered - 1, output, sizeof output,
              &length, &bad, &view) == RELOQ_DE_TEXT_UNCOVERED);
    CHECK(length == 99 && bad == 7 &&
          strcmp((const char *)output, "unchanged") == 0);
    return 0;
}

static int long_override(void)
{
    unsigned char spr[265];
    memcpy(spr, "`[.1", 4);
    memset(spr + 4, 'a', 260);
    spr[264] = ']';
    const unsigned char key[] = "Langwort";
    const struct reloq_lexicon_override entry = {
        key, sizeof key - 1, spr, sizeof spr
    };
    struct reloq_lexicon view = {0};
    CHECK(reloq_lexicon_bind(&view, NULL, 0, &entry, 1,
                             reloq_german_lexicon_validate_spr, NULL) == 0);
    unsigned char output[270];
    memset(output, 0xa5, sizeof output);
    size_t length = 17;
    CHECK(reloq_german_text_annotate_with_lexicon(
              key, sizeof key - 1, output, sizeof spr,
              &length, NULL, &view) == RELOQ_DE_TEXT_CAPACITY);
    CHECK(length == sizeof spr && output[0] == 0xa5 &&
          output[sizeof output - 1] == 0xa5);
    CHECK(reloq_german_text_annotate_with_lexicon(
              key, sizeof key - 1, output, sizeof output,
              &length, NULL, &view) == RELOQ_DE_TEXT_OK);
    CHECK(length == sizeof spr && memcmp(output, spr, sizeof spr) == 0 &&
          output[sizeof spr] == 0);
    return 0;
}

int main(void)
{
    int result = bound_lookup();
    if (result) return result;
    result = rejects_bad_data();
    if (result) return result;
    result = priority_and_transaction();
    if (result) return result;
    result = text_adapter();
    if (result) return result;
    return long_override();
}
