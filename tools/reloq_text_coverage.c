/* Batch-query the actual portable German text frontend for coverage reports.
 * One UTF-8 input line gives one status, UTF-8 failure offset, and required
 * annotation length. No Apple modules or audio samples are loaded here. */
#define _POSIX_C_SOURCE 200809L

#include "reloq_german_lexicon_file.h"
#include "reloq_german_text.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

int main(int argc, char **argv)
{
    if (argc > 2) {
        fprintf(stderr, "usage: %s [german_apple_observed.rlex]\n", argv[0]);
        return 2;
    }
    struct reloq_german_lexicon_file lexicon = {0};
    if (argc == 2 && reloq_german_lexicon_file_load_path(&lexicon, argv[1])) {
        fprintf(stderr, "could not load validated German lexicon\n");
        return 2;
    }
    char *line = NULL;
    size_t capacity = 0;
    ssize_t length;
    while ((length = getline(&line, &capacity, stdin)) >= 0) {
        if (length && line[length - 1] == '\n') --length;
        if (length && line[length - 1] == '\r') --length;
        if (memchr(line, '\0', (size_t)length)) {
            fputs("NUL in query input\n", stderr);
            free(line);
            reloq_german_lexicon_file_clear(&lexicon);
            return 2;
        }
        size_t needed = 0, bad = 0;
        int status = reloq_german_text_annotate_with_lexicon(
            (const unsigned char *)line, (size_t)length, NULL, 0,
            &needed, &bad, argc == 2 ? &lexicon.view : NULL);
        if (printf("%d\t%zu\t%zu\n", status, bad, needed) < 0) {
            perror("stdout");
            free(line);
            reloq_german_lexicon_file_clear(&lexicon);
            return 2;
        }
    }
    int failed = ferror(stdin) || fflush(stdout) == EOF;
    free(line);
    reloq_german_lexicon_file_clear(&lexicon);
    return failed ? 2 : 0;
}
