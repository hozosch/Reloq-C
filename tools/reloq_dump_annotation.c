/* Produce hexadecimal SPR bytes for offline differential diagnostics. */
#define _POSIX_C_SOURCE 200809L
#include "reloq_german_best_effort_text.h"
#include "reloq_german_lexicon_file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
int main(int argc, char **argv) {
    struct reloq_german_lexicon_file lexicon = {0};
    if (argc>2) return 2;
    if (argc==2 && reloq_german_lexicon_file_load_path(&lexicon,argv[1])) return 2;
    char *line=NULL;size_t cap=0;ssize_t n;
    while ((n=getline(&line,&cap,stdin))>=0) {
        if(n>0 && line[n-1]=='\n')--n;
        if(n>0 && line[n-1]=='\r')--n;
        unsigned char *spr=NULL;size_t k=0,approx=0,bad=0;
        int status=reloq_german_best_effort_annotate((unsigned char *)line,(size_t)n,
            argc==2?&lexicon.view:NULL,&spr,&k,&approx,&bad);
        printf("%d\t%zu\t",status,approx);
        if(status==0) for(size_t i=0;i<k;++i)printf("%02x",spr[i]);
        puts("");free(spr);
    }
    free(line);reloq_german_lexicon_file_clear(&lexicon);return 0;
}
