#include "reloq_german_connected_words.h"
#include "reloq_german_connected_cvc_index.h"

#include <string.h>

#define CHECK(x) do { if (!(x)) return __LINE__; } while (0)

int main(void)
{
    static const unsigned char phrase[] = "Pack Tipp.";
    static const unsigned char expected[] = " `[.1pAk] `2 `[.1tIp].";
    struct reloq_german_connected_plan plan = {0};
    size_t bytes = 0, bad = 999;
    CHECK(reloq_german_connected_words_prepare(
              phrase, sizeof phrase - 1, NULL, 0,
              &bytes, &bad, &plan) == 1 &&
          bytes == sizeof expected - 1 && !plan.phone_count);
    unsigned char annotation[128] = "unchanged";
    bytes = 137;
    CHECK(reloq_german_connected_words_prepare(
              phrase, sizeof phrase - 1, annotation, sizeof expected - 1,
              &bytes, &bad, &plan) == -2 &&
          bytes == sizeof expected - 1 &&
          !strcmp((char *)annotation, "unchanged") && !plan.phone_count);
    CHECK(reloq_german_connected_words_prepare(
              phrase, sizeof phrase - 1, annotation, sizeof annotation,
              &bytes, &bad, &plan) == 1 &&
          bytes == sizeof expected - 1 &&
          memcmp(annotation, expected, sizeof expected) == 0 &&
          plan.phone_count == 6 && plan.word_phone_count[0] == 3 &&
          plan.word_phone_count[1] == 3 &&
          plan.word_input_offset[0] == 0 &&
          plan.word_input_length[0] == 4 &&
          plan.word_input_offset[1] == 5 &&
          plan.word_input_length[1] == 4 &&
          plan.group_offset[0] == 1 &&
          plan.group_offset[1] == 13 &&
          plan.group_length[0] == 8 &&
          plan.group_length[1] == 8 &&
          plan.boundary_offset == 10 &&
          !plan.stress_explicitly_specified &&
          plan.reference_spr_equivalent &&
          plan.phones[2].right_phone == RELOQ_DE_PHONE_t &&
          !plan.phones[2].same_word_right &&
          plan.phones[3].left_phone == RELOQ_DE_PHONE_k &&
          !plan.phones[3].same_word_left &&
          plan.phones[3].word_index == 1);

    struct reloq_german_connected_plan explicit = {0};
    CHECK(reloq_german_connected_annotation_prepare(
              expected, sizeof expected - 1, &explicit) == 1 &&
          explicit.stress_explicitly_specified &&
          !explicit.reference_spr_equivalent &&
          explicit.phone_count == plan.phone_count &&
          explicit.boundary_offset == plan.boundary_offset &&
          explicit.phones[3].source_offset == plan.phones[3].source_offset);
    CHECK(reloq_german_connected_annotation_prepare(
              expected + 1, sizeof expected - 2, &explicit) == 1 &&
          explicit.group_offset[0] == 0 && explicit.boundary_offset == 9);
    explicit.phone_count = 88;
    static const unsigned char bad_control[] =
        " `[.1pAk] `1 `[.1tIp].";
    CHECK(reloq_german_connected_annotation_prepare(
              bad_control, sizeof bad_control - 1, &explicit) == 0 &&
          explicit.phone_count == 88);
    static const unsigned char stress_zero[] =
        " `[.0pAk] `2 `[.1tIp].";
    CHECK(reloq_german_connected_annotation_prepare(
              stress_zero, sizeof stress_zero - 1, &explicit) == 0 &&
          explicit.phone_count == 88);

    static const unsigned char hiss[] = "Hass Hiss.";
    CHECK(reloq_german_connected_words_prepare(
              hiss, sizeof hiss - 1, annotation, sizeof annotation,
              &bytes, &bad, &plan) == 1 &&
          strcmp((char *)annotation, " `[.1hAs] `2 `[.1hIs].") == 0 &&
          !plan.reference_spr_equivalent);
    static const unsigned char reduced[] = "Hat Hut.";
    CHECK(reloq_german_connected_words_prepare(
              reduced, sizeof reduced - 1, annotation, sizeof annotation,
              &bytes, &bad, &plan) == 1 &&
          !plan.stress_explicitly_specified &&
          !plan.reference_spr_equivalent);
    /* Natural 'Hat Hut.' actually reduces the first word and requires a
     * separate stress-context gate before it can be spoken as this plan. */
    static const unsigned char measured[] = "Hut Pack.";
    CHECK(reloq_german_connected_words_prepare(
              measured, sizeof measured - 1, annotation, sizeof annotation,
              &bytes, &bad, &plan) == 1 &&
          plan.reference_spr_equivalent &&
          !plan.stress_explicitly_specified);
    static const unsigned char tat[] = "Tat Pack.";
    CHECK(reloq_german_connected_words_prepare(
              tat, sizeof tat - 1, annotation, sizeof annotation,
              &bytes, &bad, &plan) == 1 &&
          !plan.reference_spr_equivalent);
    static const unsigned char second_tat[] = "Pack Tat.";
    CHECK(reloq_german_connected_words_prepare(
              second_tat, sizeof second_tat - 1, annotation,
              sizeof annotation, &bytes, &bad, &plan) == 1 &&
          plan.reference_spr_equivalent);
    CHECK(reloq_german_connected_cvc_reference_pair(
              (const unsigned char *)"Ob", 2,
              (const unsigned char *)"Hut", 3) == 0);
    CHECK(reloq_german_connected_cvc_reference_pair(
              (const unsigned char *)"Hut", 3,
              (const unsigned char *)"Ob", 2) == 1);
    CHECK(reloq_german_connected_cvc_reference_pair(
              (const unsigned char *)"Pack", 4,
              (const unsigned char *)"Hieb", 4) == 1);
    /* Every positive k→t pair is independently captured and compared as a
     * full natural Apple callback. The product list is reference data, not
     * a branch in the text-to-phoneme rule. */
    static const char *kt_pairs[][2] = {
        {"Pack", "Tipp"}, {"Hack", "Tipp"}, {"Pek", "Tip"},
        {"Hak", "Tep"}, {"Pik", "Tup"}, {"Hok", "Tap"},
        {"Kok", "Top"}, {"Tik", "Tuk"}, {"Ek", "Tib"},
        {"Uk", "Tut"}, {"Pak", "Tep"}, {"Ak", "Tap"},
        {"Hek", "Tup"}, {"Huk", "Tip"}, {"Puk", "Tak"},
        {"Tak", "Tip"}, {"Kak", "Tip"},
        {"Tick", "Tipp"}, {"Kick", "Tipp"}
    };
    for (size_t i = 0; i < sizeof kt_pairs / sizeof kt_pairs[0]; ++i) {
        const char *first = kt_pairs[i][0], *second = kt_pairs[i][1];
        unsigned char text[32];
        size_t n = strlen(first), m = strlen(second);
        CHECK(n + m + 3 <= sizeof text);
        memcpy(text, first, n);
        text[n] = ' ';
        memcpy(text + n + 1, second, m);
        text[n + m + 1] = '.';
        text[n + m + 2] = 0;
        CHECK(reloq_german_connected_words_prepare(
                  text, n + m + 2, annotation, sizeof annotation,
                  &bytes, &bad, &plan) == 1 &&
              plan.reference_spr_equivalent);
    }
    /* These finite p→t pairs were gated by the complete Apple callback,
     * including the single `2 and period. A lexical phone match alone is
     * insufficient to certify natural phrase stress. */
    static const char *pt_pairs[][2] = {
        {"Tipp","Tipp"}, {"Tipp","Top"}, {"Papp","Tipp"},
        {"Piep","Tipp"}, {"App","Tipp"}, {"Hopp","Tipp"},
        {"Pup","Tipp"}, {"Kapp","Tipp"}, {"Pip","Tipp"},
        {"Papp","Top"}, {"Tipp","Tap"}, {"Hipp","Tipp"},
        {"Tipp","Tup"}, {"Topp","Tipp"}, {"Popp","Tipp"},
        {"Tipp","Tapp"}
    };
    for (size_t i = 0; i < sizeof pt_pairs / sizeof pt_pairs[0]; ++i) {
        const char *first = pt_pairs[i][0], *second = pt_pairs[i][1];
        unsigned char text[32];
        size_t n = strlen(first), m = strlen(second);
        CHECK(n + m + 3 <= sizeof text);
        memcpy(text,first,n);
        text[n] = ' ';
        memcpy(text + n + 1,second,m);
        text[n + m + 1] = '.';
        text[n + m + 2] = 0;
        CHECK(reloq_german_connected_words_prepare(
                  text,n + m + 2,annotation,sizeof annotation,
                  &bytes,&bad,&plan) == 1 &&
              plan.reference_spr_equivalent);
    }
    static const unsigned char pt_unmeasured[] = "Tipp Tip.";
    CHECK(reloq_german_connected_words_prepare(
              pt_unmeasured,sizeof pt_unmeasured - 1,annotation,
              sizeof annotation,&bytes,&bad,&plan) == 1 &&
          !plan.reference_spr_equivalent);
    static const char *pt_prosody_traps[] = {
        "Tipp, Tipp.", "Tipp. Tipp.", "Tipp Tipp?",
        "Tipp Tipp Tipp.", "Papp-Tipp."
    };
    for (size_t i = 0; i < sizeof pt_prosody_traps /
                            sizeof pt_prosody_traps[0]; ++i) {
        const char *text = pt_prosody_traps[i];
        CHECK(reloq_german_connected_words_prepare(
                  (const unsigned char *)text,strlen(text),annotation,
                  sizeof annotation,&bytes,&bad,&plan) == 0);
    }
    static const unsigned char unmeasured[] = "Pek Tep.";
    CHECK(reloq_german_connected_words_prepare(
              unmeasured, sizeof unmeasured - 1, annotation,
              sizeof annotation, &bytes, &bad, &plan) == 1 &&
          !plan.reference_spr_equivalent);

    static const unsigned char other[] = "Pack Maus.";
    strcpy((char *)annotation, "unchanged");
    bytes = 333;
    CHECK(reloq_german_connected_words_prepare(
              other, sizeof other - 1, annotation, sizeof annotation,
              &bytes, &bad, &plan) == 0 && bad == 5 &&
          bytes == 333 && !strcmp((char *)annotation, "unchanged"));
    static const unsigned char third[] = "Pack Tipp Hack.";
    CHECK(reloq_german_connected_words_prepare(
              third, sizeof third - 1, annotation, sizeof annotation,
              &bytes, &bad, &plan) == 0);
    static const unsigned char punctuation[] = "Pack, Tipp.";
    CHECK(reloq_german_connected_words_prepare(
              punctuation, sizeof punctuation - 1,
              annotation, sizeof annotation, &bytes, &bad, &plan) == 0);
    static const unsigned char malformed[] = {'P', 'a', 0xc3, ' ', 'X', '.'};
    CHECK(reloq_german_connected_words_prepare(
              malformed, sizeof malformed, annotation, sizeof annotation,
              &bytes, &bad, &plan) == -1 && bad == 2);
    return 0;
}
