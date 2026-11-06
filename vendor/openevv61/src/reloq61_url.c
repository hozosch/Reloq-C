#include <stdint.h>
#include <string.h>
#include "delta.h"
#include "reloq61_url.h"

void reloq61_url_optional_s(void *state, unsigned char *scratch)
{
    /* The same URL rule is used by several languages, whose character
     * numberings need not agree. Resolve the letter in the active language's
     * input field, then let the native scanner consume it when present. */
    const delta_stmt *input = &vstmtbl[1];
    if (!input->fields || input->nfields < 1 || !input->fields[0].values)
        return;
    const delta_fielddesc *field = &input->fields[0];
    const char *const *names = field->values;
    for (int pass = 0; pass < 2; ++pass) {
        const char *letter = pass ? "S" : "s";
        for (int i = 0; i < field->nvalues && i < 256; ++i) {
            if (names[i] && strcmp(names[i], letter) == 0) {
                *scratch = (uint8_t)i;
                if (test_string_s(state, 1, 1, scratch) == 0)
                    return;
                break;
            }
        }
    }
}
