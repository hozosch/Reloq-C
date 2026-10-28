#ifndef RELOQ61_FOREIGN_BANK_ZHCN61_H
#define RELOQ61_FOREIGN_BANK_ZHCN61_H
#include <stdint.h>
int reloq_zhcn61_bank_init(void);
int reloq_zhcn61_bank_property(void*,int32_t);
int reloq_zhcn61_bank_pending(void*);
void reloq_zhcn61_bank_set_property(void*,int);
int32_t reloq_zhcn61_bank_letters(void*,int32_t,int32_t);
int reloq_zhcn61_bank_dot(void);
int reloq_zhcn61_bank_word(void*,int32_t);
int reloq_zhcn61_bank_assign(void*,int32_t,int32_t);
int reloq_zhcn61_bank_speech(void*,int32_t);
int reloq_zhcn61_bank_restore(void*,int32_t,int32_t);
int reloq_zhcn61_bank_print(void*,int32_t,int32_t);
#endif
