#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../vendor/openevv61/src/eci/lang/eci_rom.h"

extern EvvRom *reloq_chinese_romanizer(const char *dir);

int main(void)
{
    const char *text = "\xe5\xb0\x8f\xe7\xb1\xb3\xe4\xba\xa7\xe5\x93\x81";
    EvvRom *r = reloq_chinese_romanizer(NULL);
    if (!r) return 1;
    int32_t status = r->ops->addText(r, text, (int32_t)strlen(text), 0);
    char *out = NULL;
    status = r->ops->processSentence(r, &out, 0);
    if (status != 2 || !out) {
        fprintf(stderr, "status=%d out=%p\n", status, (void *)out);
        r->ops->release(r);
        return 1;
    }
    printf("%s\n", out);
    r->ops->release(r);
    return 0;
}
