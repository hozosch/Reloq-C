#include "reloq_german_spr.h"

#include <string.h>

static enum reloq_german_phone single_phone(unsigned char c)
{
    switch (c) {
#define SINGLE(byte, name) case byte: return RELOQ_DE_PHONE_##name
    SINGLE('a', a); SINGLE('A', A); SINGLE('e', e); SINGLE('E', E);
    SINGLE('i', i); SINGLE('I', I); SINGLE('o', o); SINGLE('O', O);
    SINGLE('u', u); SINGLE('U', U); SINGLE('y', y); SINGLE('Y', Y);
    SINGLE('b', b); SINGLE('C', C); SINGLE('d', d); SINGLE('f', f);
    SINGLE('g', g); SINGLE('G', G); SINGLE('h', h); SINGLE('j', j);
    SINGLE('J', J); SINGLE('k', k); SINGLE('l', l); SINGLE('m', m);
    SINGLE('n', n); SINGLE('p', p); SINGLE('P', P); SINGLE('r', r);
    SINGLE('R', R); SINGLE('s', s); SINGLE('S', S); SINGLE('t', t);
    SINGLE('T', T); SINGLE('v', v); SINGLE('w', w); SINGLE('x', x);
    SINGLE('X', X); SINGLE('z', z); SINGLE('Z', Z);
#undef SINGLE
    case '@': return RELOQ_DE_PHONE_AT;
    default: return RELOQ_DE_PHONE_NONE;
    }
}

static enum reloq_german_phone quoted_phone(const unsigned char *input,
                                             size_t remaining, size_t *width)
{
    /* These spellings are from the public German SPR table. The quotes are
     * part of the syntax; a colon or tilde inside them is not a modifier. */
    static const struct {
        const char *spelling;
        size_t length;
        enum reloq_german_phone phone;
    } names[] = {
        {"'E:'", 4, RELOQ_DE_PHONE_Q_E_COLON},
        {"'oe'", 4, RELOQ_DE_PHONE_Q_oe},
        {"'OE'", 4, RELOQ_DE_PHONE_Q_OE},
        {"'aj'", 4, RELOQ_DE_PHONE_Q_aj},
        {"'aw'", 4, RELOQ_DE_PHONE_Q_aw},
        {"'oj'", 4, RELOQ_DE_PHONE_Q_oj},
        {"'a~'", 4, RELOQ_DE_PHONE_Q_a_TILDE},
        {"'E~'", 4, RELOQ_DE_PHONE_Q_E_TILDE},
        {"'o~'", 4, RELOQ_DE_PHONE_Q_o_TILDE},
        {"'oe~'", 5, RELOQ_DE_PHONE_Q_oe_TILDE}
    };
    for (size_t i = 0; i < sizeof names / sizeof names[0]; ++i) {
        if (remaining >= names[i].length &&
            memcmp(input, names[i].spelling, names[i].length) == 0) {
            *width = names[i].length;
            return names[i].phone;
        }
    }
    return RELOQ_DE_PHONE_NONE;
}

static int feature_header(const unsigned char *s,size_t n,
    struct reloq_german_spr_event *event,size_t *used)
{
    uint64_t fields[4]={0};size_t at=1;
    for(unsigned k=0;k<4;++k) {
        const size_t start=at;unsigned base=k?16:10;
        while(at<n) {
            unsigned c=s[at],d=c>='0'&&c<='9'?c-'0':
                c>='a'&&c<='f'?c-'a'+10:c>='A'&&c<='F'?c-'A'+10:base;
            if(d>=base)break;
            if(fields[k]>(UINT64_MAX-d)/base)return -1;
            fields[k]=fields[k]*base+d;++at;
        }
        if(at==start || at>=n || s[at++]!=(k==3?';':','))return -1;
    }
    if(!fields[0] || fields[0]>64)return -1;
    uint64_t mask=fields[0]==64?UINT64_MAX:(UINT64_C(1)<<fields[0])-1;
    if((fields[1]|fields[2]|fields[3])&~mask)return -1;
    event->kind=RELOQ_DE_SPR_FEATURES;
    event->feature_syllables=(unsigned)fields[0];
    event->glottal_syllables=fields[1];
    event->full_quantity_syllables=fields[2];
    event->full_a_syllables=fields[3];
    event->length=at;*used=at;return 0;
}

int reloq_german_spr_next(const unsigned char *input, size_t length,
                          struct reloq_german_spr_cursor *cursor,
                          struct reloq_german_spr_event *event)
{
    if (!input || !cursor || !event || cursor->offset > length ||
        cursor->phase > 2)
        return -1;
    struct reloq_german_spr_cursor next = *cursor;
    struct reloq_german_spr_event out = {0};
    out.offset = next.offset;

    if (next.phase == 2)
        return next.offset == length ? 0 : -1;
    if (next.phase == 0) {
        size_t width = 0;
        if (length - next.offset >= 2 && input[next.offset] == '`' &&
            input[next.offset + 1] == '[')
            width = 2;
        else if (next.offset < length && input[next.offset] == '[')
            width = 1;
        if (!width)
            return -1;
        out.kind = RELOQ_DE_SPR_WORD_OPEN;
        out.length = width;
        next.offset += width;
        next.phase = 1;
    } else if (next.offset == length) {
        return -1;
    } else {
        unsigned char c = input[next.offset];
        if(c=='!' && !next.syllable_count && !next.has_features) {
            size_t used;
            if(feature_header(input+next.offset,length-next.offset,&out,&used))return -1;
            next.has_features=1;
            next.feature_syllables=(unsigned char)out.feature_syllables;
            next.offset+=used;
        } else if (c == '.') {
            if ((next.syllable_count && !next.has_phone) ||
                next.syllable_count == SIZE_MAX)
                return -1;
            out.kind = RELOQ_DE_SPR_SYLLABLE;
            out.length = 1;
            out.syllable = next.syllable_count++;
            next.has_phone = 0;
            next.has_stress = 0;
            ++next.offset;
        } else if (c == ']') {
            if (!next.syllable_count || !next.has_phone ||
                (next.has_features && next.feature_syllables !=
                 (next.syllable_count<64?next.syllable_count:64)))
                return -1;
            out.kind = RELOQ_DE_SPR_WORD_CLOSE;
            out.length = 1;
            next.phase = 2;
            ++next.offset;
        } else if (!next.syllable_count) {
            /* Dots are optional in authored SPR. Emit the syllable boundary
             * without consuming a byte, then interpret this same byte. */
            out.kind = RELOQ_DE_SPR_SYLLABLE;
            out.syllable = 0;
            next.syllable_count = 1;
        } else if (c >= '0' && c <= '2') {
            if (next.has_stress || (c == '1' && next.primary_count))
                return -1;
            out.kind = RELOQ_DE_SPR_STRESS;
            out.length = 1;
            out.syllable = next.syllable_count - 1;
            out.stress = (uint8_t)(c - '0');
            next.has_stress = 1;
            if (c == '1')
                ++next.primary_count;
            ++next.offset;
        } else {
            size_t width = 1;
            enum reloq_german_phone phone = c == '\''
                ? quoted_phone(input + next.offset, length - next.offset,
                               &width)
                : single_phone(c);
            if (phone == RELOQ_DE_PHONE_NONE)
                return -1;
            out.kind = RELOQ_DE_SPR_PHONE;
            out.length = width;
            out.syllable = next.syllable_count - 1;
            out.phone = phone;
            next.has_phone = 1;
            next.offset += width;
        }
    }
    *cursor = next;
    *event = out;
    return 1;
}
