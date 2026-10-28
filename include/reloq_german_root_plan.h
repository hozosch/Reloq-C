#ifndef RELOQ_GERMAN_ROOT_PLAN_H
#define RELOQ_GERMAN_ROOT_PLAN_H
#include <stddef.h>
#include "reloq_lexicon.h"
/* A spelling-to-phone plan with separate root, ending, syllable and stress
 * decisions. Returns zero outside its audited loan-root domain. */
int reloq_german_root_plan_word(const struct reloq_lexicon *,const unsigned char *,size_t,unsigned char *,size_t,size_t *);
size_t reloq_german_feminine_ending(const unsigned char *,size_t);
int reloq_german_unclassified_root_word(const struct reloq_lexicon *,const unsigned char *,size_t,unsigned char *,size_t,size_t *);
/* Authored -os adjective class, including ordinary adjective inflections. */
int reloq_german_root_plan_os_adjective(const unsigned char *,size_t);
#endif
