#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../vendor/openevv61/src/eci/lang/eci_rom.h"

extern EvvRom *reloq_chinese_romanizer(const char *dir);

static int probe(EvvRom *r, const char *text)
{
    if(!r->ops->addText(r, text, (int32_t)strlen(text), 0))return 2;
    char *out = NULL;
    int32_t status = r->ops->processSentence(r, &out, 0);
    if(status != 2 || !out)return 3;
    puts(out);
    return 0;
}

int main(int argc, char **argv)
{
    EvvRom *r = reloq_chinese_romanizer(NULL);
    if(!r)return 1;
    if(argc > 2 && !strcmp(argv[1], "--file")) {
        FILE *file = fopen(argv[2], "rb");
        char *text = NULL;
        size_t size = 0;
        if(!file)return 4;
        fseek(file, 0, SEEK_END);
        long length = ftell(file);
        fseek(file, 0, SEEK_SET);
        if(length < 0 || length > 1024*1024)return 4;
        text = malloc((size_t)length+1);
        if(!text)return 4;
        if(fread(text, 1, (size_t)length, file) != (size_t)length) {free(text);return 4;}
        text[length] = 0;
        fclose(file);
        int status = probe(r, text);
        free(text);
        if(status)return status;
    } else {
        for(int i = 1; i < argc; ++i) {
            int status = probe(r, argv[i]);
            if(status)return status;
        }
    }
    r->ops->release(r);
    return 0;
}
