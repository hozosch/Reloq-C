#include "reloq_german_phone_sequence.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* SPR quantity classes before context-dependent phonetic adjustment. */
static unsigned vowel_quantity(enum reloq_german_phone p)
{
    switch(p) {
    case RELOQ_DE_PHONE_a: case RELOQ_DE_PHONE_e: case RELOQ_DE_PHONE_i:
    case RELOQ_DE_PHONE_o: case RELOQ_DE_PHONE_u: case RELOQ_DE_PHONE_y:
    case RELOQ_DE_PHONE_Q_E_COLON: case RELOQ_DE_PHONE_Q_oe:
        return 2;
    case RELOQ_DE_PHONE_A: case RELOQ_DE_PHONE_E: case RELOQ_DE_PHONE_I:
    case RELOQ_DE_PHONE_O: case RELOQ_DE_PHONE_U: case RELOQ_DE_PHONE_Y:
    case RELOQ_DE_PHONE_AT: case RELOQ_DE_PHONE_R: case RELOQ_DE_PHONE_Q_OE:
    case RELOQ_DE_PHONE_Q_aj: case RELOQ_DE_PHONE_Q_aw: case RELOQ_DE_PHONE_Q_oj:
    case RELOQ_DE_PHONE_Q_a_TILDE: case RELOQ_DE_PHONE_Q_E_TILDE:
    case RELOQ_DE_PHONE_Q_o_TILDE: case RELOQ_DE_PHONE_Q_oe_TILDE:
        return 1;
    default: return 0;
    }
}

int reloq_german_phone_sequence_build_checked(
    const unsigned char *input, size_t input_length, uint32_t max_pause_ms,
    struct reloq_german_phone_node *nodes, size_t capacity,
    struct reloq_german_phone_sequence_info *info,
    reloq_german_cancel_check cancelled, void *cancel_context)
{
    if (!input || !info || (capacity && !nodes)) return -1;

    /* Validate the entire callback, including groups after the last phone,
     * before modifying caller storage. The parser sees every control and
     * pause between words, even though those are not phone nodes. */
    struct reloq_german_annotation_cursor scan = {0};
    struct reloq_german_annotation_event event;
    struct reloq_german_phone_sequence_info counts = {0};
    int state;
    for (;;) {
        if (cancelled && cancelled(cancel_context)) return 1;
        state = reloq_german_annotation_next(input, input_length, &scan,
                                              max_pause_ms, &event);
        if (state != 1) break;
        if (event.kind != RELOQ_DE_ANNOTATION_SPR) continue;
        size_t *counter = NULL;
        if (event.spr.kind == RELOQ_DE_SPR_PHONE)
            counter = &counts.phone_count;
        else if (event.spr.kind == RELOQ_DE_SPR_WORD_OPEN)
            counter = &counts.word_count;
        else if (event.spr.kind == RELOQ_DE_SPR_SYLLABLE)
            counter = &counts.syllable_count;
        if (counter) {
            if (*counter == SIZE_MAX) return -1;
            ++*counter;
        }
    }
    if (state != 0 || counts.phone_count > capacity) return -1;

    /* A cancelled second pass must never leave half-written caller nodes.
     * The original NULL-predicate path avoids this extra allocation. */
    struct reloq_german_phone_node *staged = NULL;
    struct reloq_german_phone_node *target = nodes;
    if (cancelled && counts.phone_count) {
        if (counts.phone_count > SIZE_MAX / sizeof *staged) return -1;
        staged = malloc(counts.phone_count * sizeof *staged);
        if (!staged) return -2;
        target = staged;
    }

    scan = (struct reloq_german_annotation_cursor){0};
    size_t count = 0, word = 0, syllable = 0, first_in_syllable = 0;
    uint8_t stress = 0, stress_explicit = 0;
    uint8_t pending_weak = 0, weak_word = 0;
    uint8_t pending_prosody = RELOQ_DE_WORD_CONTROL_NONE;
    uint8_t word_prosody = RELOQ_DE_WORD_CONTROL_NONE;
    uint8_t pending_tail=0,word_tail=0;
    for (;;) {
        if (cancelled && cancelled(cancel_context)) {
            free(staged);
            return 1;
        }
        state = reloq_german_annotation_next(input, input_length, &scan,
                                              max_pause_ms, &event);
        if (state != 1) break;
        if (event.kind == RELOQ_DE_ANNOTATION_CONTROL) {
            if(event.length==3 && !memcmp(input+event.offset,"`10",3)) {
                pending_tail=1;continue;
            }
            pending_weak = event.length == 3 &&
                !memcmp(input + event.offset, "`00", 3);
            pending_prosody = RELOQ_DE_WORD_CONTROL_NONE;
            if (event.length == 2 && input[event.offset] == '`') {
                switch (input[event.offset + 1]) {
                case '0': pending_prosody = RELOQ_DE_WORD_CONTROL_0; break;
                case '1': pending_prosody = RELOQ_DE_WORD_CONTROL_1; break;
                case '2': pending_prosody = RELOQ_DE_WORD_CONTROL_2; break;
                case '3': pending_prosody = RELOQ_DE_WORD_CONTROL_3; break;
                case '4': pending_prosody = RELOQ_DE_WORD_CONTROL_4; break;
                }
            } else if (pending_weak)
                pending_prosody = RELOQ_DE_WORD_CONTROL_00;
            continue;
        }
        if (event.kind == RELOQ_DE_ANNOTATION_TEXT) {
            for (size_t i = 0; i < event.length; ++i) {
                const unsigned char c = input[event.offset + i];
                if (c != ' ' && (c < '\t' || c > '\r')) {
                    pending_weak = 0;
                    pending_prosody = RELOQ_DE_WORD_CONTROL_NONE;
                    pending_tail=0;
                    break;
                }
            }
            continue;
        }
        if (event.kind != RELOQ_DE_ANNOTATION_SPR) {
            pending_weak = 0;
            pending_prosody = RELOQ_DE_WORD_CONTROL_NONE;
            pending_tail=0;
            continue;
        }
        switch (event.spr.kind) {
        case RELOQ_DE_SPR_FEATURES:
            break;
        case RELOQ_DE_SPR_WORD_OPEN:
            /* The checked pass guarantees a matching nonempty group. */
            stress = stress_explicit = 0;
            weak_word = pending_weak;
            word_prosody = pending_prosody;
            word_tail=pending_tail;pending_tail=0;
            pending_weak = 0;
            pending_prosody = RELOQ_DE_WORD_CONTROL_NONE;
            break;
        case RELOQ_DE_SPR_SYLLABLE:
            first_in_syllable = count;
            stress = stress_explicit = 0;
            ++syllable;
            break;
        case RELOQ_DE_SPR_STRESS:
            stress = event.spr.stress;
            stress_explicit = 1;
            /* A stress marker may follow the first phone of its syllable. */
            for (size_t i = first_in_syllable; i < count; ++i) {
                if (cancelled && (i & 255) == 0 &&
                    cancelled(cancel_context)) {
                    free(staged);
                    return 1;
                }
                target[i].stress = stress;
                target[i].stress_explicit = 1;
            }
            break;
        case RELOQ_DE_SPR_PHONE: {
            struct reloq_german_phone_node node = {0};
            node.phone = event.spr.phone;
            node.source_offset = event.offset;
            node.source_length = event.length;
            node.word_index = word;
            node.syllable_index = syllable - 1;
            node.syllable_in_word = event.spr.syllable;
            node.stress = stress;
            node.stress_explicit = stress_explicit;
            node.weak_word_control = weak_word;
            node.prosody_control = word_prosody;
            node.prosody_tail=word_tail;
            if (count) {
                struct reloq_german_phone_node *left = &target[count - 1];
                node.left_phone = left->phone;
                node.same_word_left = left->word_index == word;
                node.same_syllable_left = left->syllable_index ==
                                          node.syllable_index;
                left->right_phone = node.phone;
                left->same_word_right = node.same_word_left;
                left->same_syllable_right = node.same_syllable_left;
            }
            target[count++] = node;
            break;
        }
        case RELOQ_DE_SPR_WORD_CLOSE:
            ++word;
            break;
        }
    }
    if (state != 0 || (cancelled && cancelled(cancel_context))) {
        free(staged);
        return state == 0 ? 1 : -1;
    }
    uint8_t primary_ahead=0, primary_long_closed=0;
    unsigned coda=0;
    for(size_t i=count;i>0;) {
        --i;
        if(cancelled && !(i&255) && cancelled(cancel_context)) {
            free(staged);return 1;
        }
        if(!target[i].same_word_right)primary_ahead=primary_long_closed=0;
        if(!target[i].same_syllable_right)coda=0;
        target[i].primary_ahead=primary_ahead;
        target[i].primary_long_closed=primary_long_closed;
        if(target[i].stress==1)primary_ahead=1;
        unsigned quantity=vowel_quantity(target[i].phone);
        if(quantity) {
            if(target[i].stress==1)
                primary_long_closed=quantity==2 && coda>=2;
            coda=0;
        } else if(coda<2)++coda;
    }
    /* The second pass is deterministic for immutable input; all errors and
     * capacity failures were checked before writing. */
    if (staged) {
        memcpy(nodes, staged, count * sizeof *staged);
        free(staged);
    }
    *info = counts;
    return 0;
}

int reloq_german_phone_sequence_build(
    const unsigned char *input, size_t input_length, uint32_t max_pause_ms,
    struct reloq_german_phone_node *nodes, size_t capacity,
    struct reloq_german_phone_sequence_info *info)
{
    return reloq_german_phone_sequence_build_checked(
        input, input_length, max_pause_ms, nodes, capacity, info,
        NULL, NULL);
}
