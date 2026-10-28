/* Batch diagnostic for whole-text German frontend coverage, without audio.
 * One UTF-8 input line yields one status, error byte, approximate group
 * count, complete SPR group count and annotation length. This uses the
 * same authored best-effort adapter and optional .rlex as generic ECI. */
#define _POSIX_C_SOURCE 200809L

#include "reloq_german_best_effort_text.h"
#include "reloq_german_lexicon_file.h"

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
        fputs("invalid or unreadable lexicon\n", stderr);
        return 2;
    }
    char *line = NULL;
    size_t capacity = 0;
    ssize_t read;
    while ((read = getline(&line, &capacity, stdin)) >= 0) {
        if (read && line[read - 1] == '\n') --read;
        if (read && line[read - 1] == '\r') --read;
        if (memchr(line, 0, (size_t)read)) {
            fputs("NUL in input\n", stderr);
            free(line);
            reloq_german_lexicon_file_clear(&lexicon);
            return 2;
        }
        unsigned char *annotation = NULL;
        size_t length = 0, guessed = 0, bad = 0, groups = 0;
        int status = reloq_german_best_effort_annotate(
            (const unsigned char *)line, (size_t)read,
            argc == 2 ? &lexicon.view : NULL,
            &annotation, &length, &guessed, &bad);
        if (!status) {
            for (size_t i = 0; i + 1 < length; ++i)
                if (annotation[i] == '`' && annotation[i + 1] == '[')
                    ++groups;
        }
        free(annotation);
        if (printf("%d\t%zu\t%zu\t%zu\t%zu\n",
                   status, bad, guessed, groups, length) < 0) {
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
