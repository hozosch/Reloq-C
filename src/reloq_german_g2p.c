#include "reloq_german_g2p.h"
#include "reloq_german_morphology.h"

#include <stdint.h>
#include <string.h>

enum vowel_kind { A, E, I, O, U, UML_A, UML_O, UML_U,
                  DIP_AW, DIP_AJ, DIP_OJ };

static int is_vowel(unsigned char c)
{
    return c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' ||
           c == 0xe4 || c == 0xf6 || c == 0xfc;
}

static int normalize(unsigned char c, unsigned char *out)
{
    if (c >= 'A' && c <= 'Z') c = (unsigned char)(c - 'A' + 'a');
    if (c == 0xc4) c = 0xe4;
    if (c == 0xd6) c = 0xf6;
    if (c == 0xdc) c = 0xfc;
    if ((c >= 'a' && c <= 'z') || c == 0xe4 || c == 0xf6 ||
        c == 0xfc || c == 0xdf) {
        *out = c;
        return 1;
    }
    return 0;
}

/* Phones are copied from independently observed public-API SPR spellings;
 * no engine data or executable rule tables are included. */
static const char *vowel_spelling(enum vowel_kind kind, int is_long)
{
    switch (kind) {
    case A: return is_long ? "a" : "A";
    case E: return is_long ? "e" : "E";
    case I: return is_long ? "i" : "I";
    case O: return is_long ? "o" : "O";
    case U: return is_long ? "u" : "U";
    case UML_A: return is_long ? "'E:'" : "E";
    case UML_O: return is_long ? "'oe'" : "'OE'";
    case UML_U: return is_long ? "y" : "Y";
    case DIP_AW: return "'aw'";
    case DIP_AJ: return "'aj'";
    case DIP_OJ: return "'oj'";
    }
    return NULL;
}

/* strip_t checks a lexical stem class before assign_ger_vowel_diaphons.
 * These independently measured quantities describe that class; unknown
 * unmarked VC+t spellings retain the closed-syllable decision. Written
 * dt is coalesced before that decision. Explicit length marks and complex
 * codas are owned by the existing scanner. */
static int t_quantity(const unsigned char *text,size_t root)
{
    if (root<2 || !is_vowel(text[root-2]) || is_vowel(text[root-1]) ||
        text[root-1]=='h' || (root>=3 && is_vowel(text[root-3]))) return -1;
    static const struct {const char *stem;int long_vowel;} classes[]={
#include "reloq_german_t_quantity.inc"
    };
    for (size_t i=0;i<sizeof classes/sizeof *classes;++i) {
        size_t n=strlen(classes[i].stem);
        if (root>=n && !memcmp(text+root-n,classes[i].stem,n))
            return classes[i].long_vowel;
    }
    /* Public t/te/ten controls retain long i before written g, even for
     * unseen stems. Keep this quantity distinct from the short -ig suffix. */
    return text[root-1]=='d' ||
        (text[root-2]=='i' && text[root-1]=='g');
}
static int primary_quantity(unsigned char *spr,size_t *size,int long_vowel)
{
    if (long_vowel<0) return 1;
    for (size_t i=4;i<*size && spr[i]!='.' && spr[i]!=']';++i) {
        enum vowel_kind v;size_t width=1;
        switch(spr[i]) {
        case 'a':case 'A':v=A;break;case 'e':case 'E':v=E;break;
        case 'i':case 'I':v=I;break;case 'o':case 'O':v=O;break;
        case 'u':case 'U':v=U;break;case 'y':case 'Y':v=UML_U;break;
        case '\'':
            if (i+4<=*size && !memcmp(spr+i,"'E:'",4)) {v=UML_A;width=4;}
            else if (i+4<=*size && (!memcmp(spr+i,"'oe'",4) || !memcmp(spr+i,"'OE'",4))) {v=UML_O;width=4;}
            else return 0;
            break;
        default:continue;
        }
        const char *phone=vowel_spelling(v,long_vowel);size_t n=strlen(phone);
        if (*size-width+n>=256) return 0;
        memmove(spr+i+n,spr+i+width,*size-i-width+1);
        memcpy(spr+i,phone,n);*size=*size-width+n;return 1;
    }
    return 0;
}

static int append(char *target, size_t *length, const char *part)
{
    size_t n = strlen(part);
    if (*length + n >= 256) return 0;
    memcpy(target + *length, part, n);
    *length += n;
    return 1;
}

/* un- is a semantic prefix, not merely the first two letters of a word.
 * Without a full lexicon, only reuse a base with a transparent length mark
 * or a separately analyzed nominal ending.  This prevents the prefix from
 * taking the place of the function-word stem in unser[e...].  Reject an
 * impossible d+consonant onset and doubled coda after a diphthong, which
 * otherwise let truncated "und..." and "unauff" corpus labels through.
 * These are productive spelling gates; the lexicon still has precedence. */
static int eligible_un_base(const unsigned char *base, size_t length)
{
    int explicit_shape = 0;
    if (length < 2 || (base[0] == 'd' && !is_vowel(base[1]) &&
                       base[1] != 'r')) return 0;
    if ((length >= 3 && !memcmp(base + length - 3, "ung", 3)) ||
        (length >= 4 && (!memcmp(base + length - 4, "heit", 4) ||
                         !memcmp(base + length - 4, "keit", 4))))
        explicit_shape = 1;
    for (size_t i = 0; i < length; ++i) {
        unsigned char c = base[i];
        if (c == 0xe4 || c == 0xf6 || c == 0xfc) explicit_shape = 1;
        if (i + 1 == length) continue;
        unsigned char next = base[i + 1];
        int diphthong = (c == 'a' && next == 'u') ||
                        (c == 'e' && (next == 'i' || next == 'u')) ||
                        (c == 0xe4 && next == 'u') ||
                        (c == 'i' && next == 'e');
        if (diphthong && i + 3 < length &&
            base[i + 2] == base[i + 3] && !is_vowel(base[i + 2]))
            return 0;
        if (diphthong || (is_vowel(c) && next == 'h') ||
            ((c == 'a' || c == 'e' || c == 'o') && c == next) ||
            (!is_vowel(c) && c == next) ||
            (c == 'c' && next == 'k') || (c == 't' && next == 'z'))
            explicit_shape = 1;
    }
    return explicit_shape;
}

/* A productive negative un- prefix carries the first stress while the
 * independently analyzed base retains its phones and weak syllables.  The
 * base's former primary stress becomes secondary: frei -> Unfrei,
 * Freiheit -> Unfreiheit.  A base outside this frontend's grammar remains
 * outside the prefixed grammar too.  In particular this does not guess
 * whether an opaque spelling merely happens to begin with "un". */
static int negative_un_from_base(const unsigned char *text,
                                 size_t word_length,
                                 unsigned char *output, size_t capacity,
                                 size_t *byte_length)
{
    static const char prefix[] = "`[.1Un.2";
    unsigned char base[256];
    size_t n = 0, required;
    int status;

    if (word_length < 4 || text[0] != 'u' || text[1] != 'n' ||
        !eligible_un_base(text + 2, word_length - 2)) return 0;
    status = reloq_german_g2p_word(text + 2, word_length - 2,
                                   base, sizeof base, &n);
    if (status != 1 || n < 5 || memcmp(base, "`[.1", 4) ||
        base[n - 1] != ']') return 0;
    required = sizeof prefix - 1 + n - 4;
    if (!output || capacity <= required) {
        *byte_length = required;
        return -2;
    }
    memcpy(output, prefix, sizeof prefix - 1);
    memcpy(output + sizeof prefix - 1, base + 4, n - 4);
    output[required] = 0;
    *byte_length = required;
    return 1;
}

/* Productive unstressed verbal prefixes preserve an independently covered
 * base word's complete stressed syllable sequence. The verified classes
 * are -en infinitives and final voiced-stop + t forms (sagt, fragt, lebt).
 * Restricting the base shapes avoids falsely segmenting names and loans
 * such as Bestie and Genie into be-/ge- plus an unrelated spelling.
 * The public German SPR
 * changes only the first segment to .0b@/.0g@/.0fEr/.0Er, then resumes the
 * base's .1 onset and later weak syllables. Requiring a complete, otherwise
 * covered base avoids trying to guess where a prefix ends in opaque words.
 * Requiring that base to start with primary stress avoids unverified stacking
 * of several weak prefixes. Explicit lexical overrides still take priority
 * in reloq_german_lexicon_word. */
static int weak_prefix_from_base(const unsigned char *text,
                                 size_t word_length,
                                 unsigned char *output, size_t capacity,
                                 size_t *byte_length)
{
    const char *phone;
    size_t prefix_length;
    if (word_length >= 5 && text[0] == 'b' && text[1] == 'e') {
        prefix_length = 2; phone = "`[.0b@";
    } else if (word_length >= 5 && text[0] == 'g' && text[1] == 'e') {
        prefix_length = 2; phone = "`[.0g@";
    } else if (word_length >= 5 && text[0] == 'e' && text[1] == 'r') {
        prefix_length = 2; phone = "`[.0Er";
    } else if (word_length >= 6 && !memcmp(text, "ver", 3)) {
        prefix_length = 3; phone = "`[.0fEr";
    } else return 0;

    size_t base_length = word_length - prefix_length;
    const int infinitive=text[word_length-2]=='e' && text[word_length-1]=='n';
    const int stop_t=text[word_length-1]=='t' &&
        (text[word_length-2]=='b' || text[word_length-2]=='d' || text[word_length-2]=='g');
    if (base_length<4 || (!infinitive && !stop_t)) return 0;

    unsigned char base[256];
    size_t n = 0;
    int status = reloq_german_g2p_word(text + prefix_length,
                                       base_length,
                                       base, sizeof base, &n);
    if (status != 1 || n < 5 || memcmp(base, "`[.1", 4) ||
        base[n - 1] != ']') return 0;
    /* `[' from the base is replaced by `[..prefix; retain its .1 marker. */
    size_t leader = strlen(phone), required = leader + n - 2;
    if (!output || capacity <= required) {
        *byte_length = required;
        return -2;
    }
    memcpy(output, phone, leader);
    memcpy(output + leader, base + 2, n - 2);
    output[required] = 0;
    *byte_length = required;
    return 1;
}

/* A separable verbal prefix is one spoken component, followed by a
 * separately covered verb. Composing the two complete analyses
 * preserves the initial /S/ in auf+schreiben and aus+stellen, and the /tr/
 * onset in auf+tragen; an orthographic syllable splitter would otherwise
 * move the first consonant into the prefix.  The selected prefix classes
 * have an independently observed public-ECI composition across distinct
 * infinitive stems and regular finite forms.  A finite form needs both its
 * covered pronunciation and the corresponding -en spelling to be covered;
 * this is a morphological gate, not arbitrary substring splitting.  Do not
 * infer um- or durch-: stress can depend on meaning.  Conservative whole
 * words retain precedence. */
static int separable_prefix_from_verb(const unsigned char *text,
                                      size_t word_length,
                                      unsigned char *output, size_t capacity,
                                      size_t *byte_length)
{
    static const struct {
        const char *spelling;
        size_t length;
        const char *spr;
    } prefixes[] = {
        {"auf", 3, "`[.1'aw'f"},
        {"aus", 3, "`[.1'aw's"},
        {"ab", 2, "`[.1Ap"},
        {"an", 2, "`[.1An"},
        {"ein", 3, "`[.1'aj'n"},
    };
    if (word_length < 6) return 0;
    for (size_t i = 0; i < sizeof prefixes / sizeof prefixes[0]; ++i) {
        size_t p = prefixes[i].length;
        if (word_length < p + 4 ||
            memcmp(text, prefixes[i].spelling, p)) continue;
        const unsigned char *base = text + p;
        size_t base_length = word_length - p;
        if (base[base_length - 2] != 'e' || base[base_length - 1] != 'n') {
            static const char *const endings[] = {"ten", "st", "te", "t", "e"};
            int verified_lemma = 0;
            for (size_t k = 0; k < sizeof endings / sizeof endings[0]; ++k) {
                size_t n_end = strlen(endings[k]);
                if (base_length <= n_end + 1 ||
                    memcmp(base + base_length - n_end, endings[k], n_end))
                    continue;
                unsigned char lemma[128], checked[256];
                size_t root = base_length - n_end, checked_length = 0;
                memcpy(lemma, base, root);
                memcpy(lemma + root, "en", 2);
                if (reloq_german_g2p_word(lemma, root + 2,
                                          checked, sizeof checked,
                                          &checked_length) == 1) {
                    verified_lemma = 1;
                    break;
                }
            }
            if (!verified_lemma) return 0;
        }
        unsigned char stem[256];
        size_t n = 0;
        if (reloq_german_g2p_word(base, base_length,
                                  stem, sizeof stem, &n) != 1 ||
            n < 5 || memcmp(stem, "`[.1", 4) || stem[n - 1] != ']' ||
            memchr(stem + 4, '2', n - 5)) return 0;
        size_t first = strlen(prefixes[i].spr);
        size_t required = first + 2 + n - 4;
        if (required >= 256) return 0;
        if (!output || capacity <= required) {
            *byte_length = required;
            return -2;
        }
        memcpy(output, prefixes[i].spr, first);
        memcpy(output + first, ".2", 2);
        memcpy(output + first + 2, stem + 4, n - 4);
        output[required] = 0;
        *byte_length = required;
        return 1;
    }
    return 0;
}

/* A separable verb places infinitival zu between the particle and its
 * independently analyzed base: auf+zu+lesen, ab+zu+erkennen. The particle
 * bears primary stress, zu is weak, and the base's one primary accent is
 * retained as secondary. A verified -en infinitive supplies the phones and
 * the actual base stress; splitting the whole spelling into syllables loses
 * both the consonant onset and the accent after zu.  This construction also
 * distinguishes separable um+zu+... from the inseparable um- reading. */
static int separable_prefix_with_zu(const unsigned char *text,
                                    size_t word_length,
                                    unsigned char *output, size_t capacity,
                                    size_t *byte_length)
{
    static const struct {
        const char *spelling;
        size_t length;
        const char *spr;
    } particles[] = {
        {"ab", 2, "`[.1Ap"},
        {"an", 2, "`[.1An"},
        {"auf", 3, "`[.1'aw'f"},
        {"aus", 3, "`[.1'aw's"},
        {"ein", 3, "`[.1'aj'n"},
        {"bei", 3, "`[.1b'aj'"},
        {"hin", 3, "`[.1hIn"},
        {"mit", 3, "`[.1mIt"},
        {"um", 2, "`[.1Um"},
        {"vor", 3, "`[.1for"},
        {"nach", 4, "`[.1nax"},
        {"weg", 3, "`[.1vEk"},
        {"her", 3, "`[.1hEr"},
        {"zu", 2, "`[.1Tu"},
    };
    for (size_t i = 0; i < sizeof particles / sizeof particles[0]; ++i) {
        size_t p = particles[i].length;
        if (word_length < p + 6 || memcmp(text, particles[i].spelling, p) ||
            memcmp(text + p, "zu", 2) ||
            memcmp(text + word_length - 2, "en", 2)) continue;
        unsigned char base[256];
        size_t n = 0, primary = SIZE_MAX;
        if (reloq_german_g2p_word(text + p + 2, word_length - p - 2,
                                  base, sizeof base, &n) != 1 ||
            n < 5 || memcmp(base, "`[", 2) || base[n - 1] != ']')
            continue;
        /* The conservative transducer can still misread an otherwise
         * covered infinitive. In particular, a written eu/äu diphthong
         * must not be decomposed as a weak be- plus a stressed /u/ by the
         * base analyzer. Keep the older whole-word fallback when its
         * purported base loses this visible vowel nucleus. */
        const unsigned char *root = text + p + 2;
        size_t root_length = word_length - p - 2;
        int rounded_diphthong = 0;
        for (size_t j = 0; j + 1 < root_length; ++j)
            if ((root[j] == 'e' || root[j] == 0xe4) && root[j + 1] == 'u')
                rounded_diphthong = 1;
        if (rounded_diphthong && !strstr((const char *)base, "'oj'"))
            continue;
        for (size_t j = 2; j + 1 < n; ++j) {
            if (base[j] != '.') continue;
            if (base[j + 1] == '2') { primary = SIZE_MAX; break; }
            if (base[j + 1] == '1') {
                if (primary != SIZE_MAX) { primary = SIZE_MAX; break; }
                primary = j + 1;
            }
        }
        if (primary == SIZE_MAX) continue;
        size_t leader = strlen(particles[i].spr);
        size_t required = leader + 4 + n - 2;
        if (required >= sizeof base) continue;
        if (!output || capacity <= required) {
            *byte_length = required;
            return -2;
        }
        memcpy(output, particles[i].spr, leader);
        memcpy(output + leader, ".0Tu", 4);
        memcpy(output + leader + 4, base + 2, n - 2);
        output[leader + 4 + primary - 2] = '2';
        output[required] = 0;
        *byte_length = required;
        return 1;
    }
    return 0;
}

/* The productive -ig ending uses the same stem/weak-syllable split as -en:
 * steinen -> steinig (/St'aj'.n@n/ -> /St'aj'.nIX/).  Reusing that already
 * verified analysis also keeps intervocalic /z/, consonant resyllabification
 * and vowel length together instead of appending a syllable to an isolated
 * stem's final devoiced consonant.  Irregular or unavailable -en analyses
 * remain outside this rule. */
static int ig_from_en(const unsigned char *text, size_t word_length,
                      unsigned char *output, size_t capacity,
                      size_t *byte_length)
{
    unsigned char en[128], spr[256];
    size_t stem_length, n = 0;

    if (word_length < 4 || text[word_length - 2] != 'i' ||
        text[word_length - 1] != 'g') return 0;
    stem_length = word_length - 2;
    memcpy(en, text, stem_length);
    en[stem_length] = 'e';
    en[stem_length + 1] = 'n';
    if (reloq_german_g2p_word(en, stem_length + 2,
                              spr, sizeof spr, &n) != 1 ||
        n < 7 || memcmp(spr, "`[.1", 4) ||
        memcmp(spr + n - 3, "@n]", 3)) return 0;
    if (!output || capacity <= n) {
        *byte_length = n;
        return -2;
    }
    memcpy(output, spr, n - 3);
    memcpy(output + n - 3, "IX]", 3);
    output[n] = 0;
    *byte_length = n;
    return 1;
}

/* Regular weak past -te/-ten from an independently covered -en infinitive.
 * A written doubled consonant or ck/tz marks the same short stressed vowel
 * in both forms: packen /pA.k@n/ -> packte /pAk.t@/. Reuse the complete
 * checked onset and vowel rather than repeating spelling guesses. Only a
 * single stressed syllable followed by one consonant and /@n/ qualifies;
 * an arbitrary word ending in -ten is not evidence of a past tense. Other
 * roots (including simple written s/r, sch, and written tt/ss whose /st/
 * placement differs in the public reference) remain uncovered here. */
static int past_from_verified_en(const unsigned char *text,
                                 size_t word_length,
                                 unsigned char *output, size_t capacity,
                                 size_t *byte_length)
{
    size_t ending, root, n = 0, boundary;
    unsigned char base[128], spr[256], c;
    const char *cluster;
    const char *tail;
    size_t required;

    if (word_length >= 5 && !memcmp(text + word_length - 2, "te", 2))
        ending = 2;
    else if (word_length >= 6 &&
             !memcmp(text + word_length - 3, "ten", 3))
        ending = 3;
    else return 0;
    root = word_length - ending;
    if (root < 3) return 0;
    cluster = (const char *)text + root - 2;
    if (memcmp(cluster, "ck", 2) && memcmp(cluster, "pp", 2) &&
        memcmp(cluster, "ff", 2) &&
        memcmp(cluster, "ll", 2) && memcmp(cluster, "mm", 2) &&
        memcmp(cluster, "nn", 2) &&
        memcmp(cluster, "tz", 2)) return 0;
    memcpy(base, text, root);
    memcpy(base + root, "en", 2);
    if (reloq_german_g2p_word(base, root + 2,
                              spr, sizeof spr, &n) != 1 ||
        n < 10 || memcmp(spr, "`[.1", 4) ||
        memcmp(spr + n - 3, "@n]", 3)) return 0;
    boundary = n - 6;
    if (spr[boundary] != '.' || spr[boundary + 1] != '0' ||
        memchr(spr + 4, '.', boundary - 4)) return 0;
    c = spr[boundary + 2];
    if (!((!memcmp(cluster, "ck", 2) && c == 'k') ||
          (!memcmp(cluster, "pp", 2) && c == 'p') ||
          (!memcmp(cluster, "ff", 2) && c == 'f') ||
          (!memcmp(cluster, "ll", 2) && c == 'l') ||
          (!memcmp(cluster, "mm", 2) && c == 'm') ||
          (!memcmp(cluster, "nn", 2) && c == 'n') ||
          (!memcmp(cluster, "tz", 2) && c == 'T'))) return 0;
    tail = ending == 2 ? ".0t@]" : ".0t@n]";
    required = boundary + 1 + strlen(tail);
    if (!output || capacity <= required) {
        *byte_length = required;
        return -2;
    }
    memcpy(output, spr, boundary);
    output[boundary] = c;
    memcpy(output + boundary + 1, tail, strlen(tail));
    output[required] = 0;
    *byte_length = required;
    return 1;
}

/* A regular inflection of an independently analyzed -en stem whose final
 * voiced stop becomes voiceless before -t/-st.  The same stem coda belongs
 * to the stressed syllable in all four endings: liegen -> liegt/liegst,
 * lieben -> liebte/liebten.  Written -dt coalesces into one /t/ in -t/-te,
 * while -dst retains the /tst/ cluster.  Require a complete two-syllable
 * infinitive with that very stem-final consonant; no unrelated spelling is
 * admitted just because it ends in -gt, -bt or -dt. */
static int voiced_stop_inflection_from_en(const unsigned char *text,
                                         size_t word_length,
                                         unsigned char *output,
                                         size_t capacity,
                                         size_t *byte_length)
{
    size_t ending, root, n = 0, boundary, required, tail_length;
    unsigned char base[128], spr[256], voiced;
    const char *tail;

    if (word_length >= 6 && !memcmp(text + word_length - 3, "ten", 3))
        ending = 3;
    else if (word_length >= 5 && !memcmp(text + word_length - 2, "te", 2))
        ending = 2;
    else if (word_length >= 5 && !memcmp(text + word_length - 2, "st", 2))
        ending = 2;
    else if (word_length >= 4 && text[word_length - 1] == 't')
        ending = 1;
    else return 0;
    root = word_length - ending;
    voiced = text[root - 1];
    if (voiced != 'b' && voiced != 'd' && voiced != 'g') return 0;
    /* ger_d coalesces written dt before vowel-quantity assignment. The
     * old explicit-length gate incorrectly treated every unmarked d+t
     * cluster as two consonants. Exact lexical exceptions are resolved
     * by the lexicon before this grammatical path. */
    memcpy(base, text, root);
    memcpy(base + root, "en", 2);
    if (reloq_german_g2p_word(base, root + 2, spr, sizeof spr, &n) != 1 ||
        n < 10 || memcmp(spr, "`[.1", 4) ||
        memcmp(spr + n - 3, "@n]", 3)) return 0;
    if (!(ending == 2 && text[word_length - 1] == 't') &&
        !primary_quantity(spr, &n, t_quantity(text, root))) return 0;
    boundary = n - 6;
    if (spr[boundary] != '.' || spr[boundary + 1] != '0' ||
        spr[boundary + 2] != voiced ||
        memchr(spr + 4, '.', boundary - 4)) return 0;

    if (ending == 3) tail = voiced == 'd' ? ".0t@n]" :
                              voiced == 'b' ? "p.0t@n]" : "k.0t@n]";
    else if (text[word_length - 1] == 'e')
        tail = voiced == 'd' ? ".0t@]" :
               voiced == 'b' ? "p.0t@]" : "k.0t@]";
    else if (ending == 2) tail = voiced == 'd' ? "tst]" :
                                 voiced == 'b' ? "pst]" : "kst]";
    else tail = voiced == 'd' ? "t]" :
                voiced == 'b' ? "pt]" : "kt]";
    tail_length = strlen(tail);
    required = boundary + tail_length;
    if (!output || capacity <= required) {
        *byte_length = required;
        return -2;
    }
    memcpy(output, spr, boundary);
    memcpy(output + boundary, tail, tail_length);
    output[required] = 0;
    *byte_length = required;
    return 1;
}

int reloq_german_g2p_v_voiced(const unsigned char *root,size_t length,int one_nucleus)
{
    /* Membership observed at apply_ger_v_rules, after root/ending analysis.
     * These classes change only v; they contain no word pronunciation. */
    static const char *const voiced[]={"frevl","nerv","salv","slav","vag","val","vill","vis","volt"};
    static const char *const unvoiced[]={"hannover","vati","vegesack","vettel","vielleicht","villing"};
    int voice=!one_nucleus;
    if(one_nucleus)for(size_t i=0;i<sizeof voiced/sizeof *voiced;++i)
        if(strlen(voiced[i])==length&&!memcmp(root,voiced[i],length))voice=1;
    if(voice)for(size_t i=0;i<sizeof unvoiced/sizeof *unvoiced;++i)
        if(strlen(unvoiced[i])==length&&!memcmp(root,unvoiced[i],length))voice=0;
    return voice;
}

/* Convert one consonant span. Local spelling, position and preceding vowel
 * determine German initial s, final devoicing, and back/front ch. */
static int consonants_impl(const unsigned char *text, size_t length,
                           int initial, int final, int back_vowel,
                           int approximate, char *target, size_t *used)
{
    size_t pos = 0;
    while (pos < length) {
        size_t left = length - pos;
        unsigned char c = text[pos];
        if (initial && pos==0 && left>=4 && !memcmp(text,"dsch",4)) {
            /* Original root-onset diaphon generation selects /dZ/ as one
             * affricate. A coda d followed by a new sch onset stays split. */
            if (!append(target,used,"J")) return 0;
            pos+=4;
        } else if (left >= 4 && memcmp(text + pos, "tsch", 4) == 0) {
            if (!append(target, used, "C")) return 0;
            pos += 4;
        } else if (left >= 3 && memcmp(text + pos, "tch", 3) == 0) {
            if (!append(target, used, "C")) return 0;
            pos += 3;
        } else if (left >= 3 && memcmp(text + pos, "sch", 3) == 0) {
            if (!append(target, used, "S")) return 0;
            pos += 3;
        } else if (left >= 2 && memcmp(text + pos, "ch", 2) == 0) {
            /* A root-initial ch sees its following letter, not the vowel
             * of a preceding syllable. Onset spans retain the following
             * nucleus in the caller's word buffer. */
            unsigned char next=initial && pos==0?text[2]:0;
            const char *phone=initial && pos==0 ?
                (next=='e'||next=='i'||next==0xe4||next==0xf6||next==0xfc?"X":"k") :
                back_vowel && pos==0?"x":"X";
            if (!append(target, used, phone)) return 0;
            pos += 2;
        } else if (left >= 2 && memcmp(text + pos, "pf", 2) == 0) {
            if (!append(target, used, "P")) return 0;
            pos += 2;
        } else if (left >= 2 && memcmp(text + pos, "qu", 2) == 0) {
            if (!append(target, used, "kv")) return 0;
            pos += 2;
        } else if (left >= 2 && memcmp(text + pos, "ck", 2) == 0) {
            if (!append(target, used, "k")) return 0;
            pos += 2;
        } else if (left >= 2 && memcmp(text + pos, "tz", 2) == 0) {
            if (!append(target, used, "T")) return 0;
            pos += 2;
        } else if (left >= 2 && memcmp(text + pos, "ng", 2) == 0) {
            if (!append(target, used, "G")) return 0;
            pos += left >= 3 && text[pos+2]=='r' ? 1 : 2;
        } else if (left >= 2 && c=='r' &&
                   (text[pos+1]=='h' || text[pos+1]=='r')) {
            /* The r rule consumes the following h within its root span.
             * A separately analysed h-initial root keeps its own onset. */
            if (!append(target, used, "r")) return 0;
            pos += 2;
            if (text[pos-1]=='r' && pos<length && text[pos]=='h') ++pos;
        } else if (left >= 2 && memcmp(text + pos, "th", 2) == 0) {
            if (!append(target, used, "t")) return 0;
            pos += left>=3 && text[pos+2]=='t'?3:2;
        } else if (left >= 2 && memcmp(text + pos, "gh", 2) == 0) {
            if (!append(target, used, final?"k":"g")) return 0;
            pos += 2;
        } else if (!final && left >= 2 && memcmp(text + pos, "kv", 2) == 0) {
            if (!append(target, used, "kv")) return 0;
            pos += 2;
        } else if (left >= 2 && memcmp(text + pos, "sh", 2) == 0) {
            if (!append(target, used, "S")) return 0;
            pos += 2;
        } else if (left >= 2 && memcmp(text + pos, "wh", 2) == 0) {
            if (!append(target, used, "v")) return 0;
            pos += 2;
        } else if (left >= 2 && memcmp(text + pos, "ph", 2) == 0) {
            if (!append(target, used, "f")) return 0;
            pos += 2;
        } else {
            const char *p = NULL;
            switch (c) {
            case 'b': p = final ? "p" : "b"; break;
            /* Opaque loan spellings reach this mapper only in the opt-in
             * approximation. German <c> before front vowels is usually
             * /ts/, otherwise /k/; <y> can be /j/ before a vowel. The
             * strict frontend rejects these ambiguous spellings earlier. */
            case 'c':
                if (approximate) p = pos + 1 < length &&
                    strchr("eiy", text[pos + 1]) ? "T" : "k";
                break;
            case 'd': p = final ? "t" : "d"; break;
            case 'g': p = final ? "k" : "g"; break;
            case 'f': p = "f"; break;
            case 'h': p = "h"; break;
            case 'j': p = "j"; break;
            case 'k': p = "k"; break;
            case 'l': p = "l"; break;
            case 'm': p = "m"; break;
            case 'n': p = "n"; break;
            case 'p': p = "p"; break;
            case 'q': if (approximate) p = "k"; break;
            case 'r': p = "r"; break;
            case 's':
                /* The reference voices a single s before a vowel, not
                 * merely because a consonant belongs to an onset. This
                 * covers unfamiliar clusters as well as German /skr/. */
                p = final || (left>=2 && !is_vowel(text[pos+1]) &&
                    text[pos+1]!='y') ? "s" : "z";
                break;
            case 't': p = "t"; break;
            case 'v': p = !final && (approximate&2) ? "v" : "f"; break;
            case 'w': p = "v"; break;
            case 'x': p = "ks"; break;
            case 'y': if (approximate) p = "j"; break;
            case 'z': p = "T"; break;
            case 0xdf: p = "s"; break;
            default: return 0;
            }
            if (initial && pos == 0 && left >= 2 && c == 's' &&
                (text[1] == 'p' || text[1] == 't')) p = "S";
            if (!p) return 0;
            if (!append(target, used, p)) return 0;
            ++pos;
        }
    }
    return 1;
}

static int consonants(const unsigned char *text, size_t length,
                      int initial, int final, int back_vowel,
                      char *target, size_t *used)
{
    return consonants_impl(text, length, initial, final, back_vowel,
                           0, target, used);
}

/* Productive nominal -ung in the transparent stem class.  A verified
 * spelling-to-SPR verb candidate ending in -en supplies its onset, vowel,
 * coda, syllable break and stress.  The derivation changes only the final
 * unstressed /@n/ to /UG/ (lesen -> Lesung, retten -> Rettung).  This is a
 * phonological rule, not an imported dictionary action.  Prefixes and
 * adjacent-vowel /ng/ contexts have other observed parses, so decline them
 * instead of asserting a pronunciation for them. */
static int ung_from_en(const unsigned char *text, size_t word_length,
                       unsigned char *output, size_t capacity,
                       size_t *byte_length)
{
    unsigned char verb[128], spr[256];
    size_t stem_length, n = 0;
    int status;

    if (word_length < 5 || memcmp(text + word_length - 3, "ung", 3)) return 0;
    stem_length = word_length - 3;
    if ((stem_length >= 2 &&
         ((!memcmp(text, "be", 2)) || (!memcmp(text, "ge", 2)) ||
          (!memcmp(text, "er", 2)))) ||
        (stem_length >= 3 && !memcmp(text, "ver", 3)) ||
        text[0] == 'v' ||
        (stem_length >= 2 && !memcmp(text + stem_length - 2, "ng", 2)))
        return 0;
    for (size_t i = 0; i + 1 < stem_length; ++i)
        if (text[i] == 'i' && text[i + 1] == 'e') return 0;
    memcpy(verb, text, stem_length);
    verb[stem_length] = 'e';
    verb[stem_length + 1] = 'n';
    status = reloq_german_g2p_word(verb, stem_length + 2,
                                   spr, sizeof spr, &n);
    if (status != 1 || n < 3 || memcmp(spr + n - 3, "@n]", 3)) return 0;
    if (!output || capacity <= n) {
        *byte_length = n;
        return -2;
    }
    memcpy(output, spr, n - 3);
    memcpy(output + n - 3, "UG]", 3);
    output[n] = 0;
    *byte_length = n;
    return 1;
}

/* Transparent abstract-noun endings keep the base syllables and add a weak
 * /haj t/ or /kaj t/ syllable.  This rule is deliberately restricted to
 * stems whose independent C reading is already known.  Prefix, -ie- and
 * nested ending spellings have observed lexical stress or hiatus changes. */
static int heit_keit_from_base(const unsigned char *text, size_t word_length,
                               unsigned char *output, size_t capacity,
                               size_t *byte_length)
{
    static const char heit[] = ".0h'aj't]";
    static const char keit[] = ".0k'aj't]";
    unsigned char spr[256];
    const char *ending;
    size_t stem_length, n = 0, required;
    int status;

    if (word_length < 6) return 0;
    if (!memcmp(text + word_length - 4, "heit", 4)) ending = heit;
    else if (!memcmp(text + word_length - 4, "keit", 4)) ending = keit;
    else return 0;
    stem_length = word_length - 4;
    if ((stem_length >= 2 &&
         (!memcmp(text, "be", 2) || !memcmp(text, "ge", 2) ||
          !memcmp(text, "er", 2))) ||
        (stem_length >= 3 && !memcmp(text, "ver", 3)) ||
        (stem_length >= 4 && !memcmp(text, "miss", 4)) ||
        text[0] == 'v' ||
        text[stem_length - 1] == 'e' ||
        (stem_length >= 3 &&
         !memcmp(text + stem_length - 3, "ung", 3)) ||
        (stem_length >= 4 &&
         (!memcmp(text + stem_length - 4, "heit", 4) ||
          !memcmp(text + stem_length - 4, "keit", 4))))
        return 0;
    for (size_t i = 0; i + 1 < stem_length; ++i)
        if (text[i] == 'i' && text[i + 1] == 'e') return 0;
    status = reloq_german_g2p_word(text, stem_length,
                                   spr, sizeof spr, &n);
    if (status != 1 || n < 2 || spr[n - 1] != ']') return 0;
    required = n - 1 + strlen(ending);
    if (!output || capacity <= required) {
        *byte_length = required;
        return -2;
    }
    memcpy(output, spr, n - 1);
    memcpy(output + n - 1, ending, required - n + 1);
    output[required] = 0;
    *byte_length = required;
    return 1;
}

/* The three transparent noun plurals reuse a pronunciation already derived
 * for their singular. In public ECI SPR, -ung adds a weak /@n/ syllable;
 * -heit and -keit move their final /t/ to that new weak syllable. This rule
 * does not infer a new lexical stress or repair an ambiguous singular. */
static int nominal_plural_from_singular(const unsigned char *text,
                                        size_t word_length,
                                        unsigned char *output,
                                        size_t capacity,
                                        size_t *byte_length)
{
    enum { UNG, HEIT, KEIT } kind;
    if (word_length >= 7 &&
        memcmp(text + word_length - 5, "ungen", 5) == 0) kind = UNG;
    else if (word_length >= 8 &&
             memcmp(text + word_length - 6, "heiten", 6) == 0) kind = HEIT;
    else if (word_length >= 8 &&
             memcmp(text + word_length - 6, "keiten", 6) == 0) kind = KEIT;
    else return 0;

    unsigned char singular[128], spr[256];
    size_t singular_length = word_length - 2, n = 0;
    memcpy(singular, text, singular_length);
    int status = reloq_german_g2p_word(singular, singular_length,
                                        spr, sizeof spr, &n);
    if (status != 1 || n >= sizeof spr - 4) return 0;

    const char *ending;
    size_t retained;
    if (kind == UNG) {
        if (n < 4 || memcmp(spr + n - 3, "UG]", 3) != 0) return 0;
        retained = n - 1;
        ending = ".0@n]";
    } else {
        const char *expected = kind == HEIT ? "h'aj't]" : "k'aj't]";
        if (n < 8 || memcmp(spr + n - 7, expected, 7) != 0) return 0;
        retained = n - 2;
        ending = ".0t@n]";
    }
    size_t extra = strlen(ending), required = retained + extra;
    if (!output || capacity <= required) {
        *byte_length = required;
        return -2;
    }
    memcpy(output, spr, retained);
    memcpy(output + retained, ending, extra);
    output[required] = 0;
    *byte_length = required;
    return 1;
}

/* Transparent diminutives preserve the covered monosyllabic stem and add
 * an unstressed syllable. A single written s + chen coalesces to /S/,
 * whereas ss + chen retains /s.X/. Bare -au/-aa, other vowel hiatus and
 * prefixed readings require lexical decisions. */
static int diminutive_from_base(const unsigned char *text, size_t word_length,
                                unsigned char *output, size_t capacity,
                                size_t *byte_length)
{
    int chen;
    unsigned char spr[256];
    const char *ending;
    size_t stem_length, n = 0, retained, required;
    if (word_length < 6) return 0;
    if (memcmp(text + word_length - 4, "chen", 4) == 0) chen = 1;
    else if (memcmp(text + word_length - 4, "lein", 4) == 0) chen = 0;
    else return 0;
    stem_length = word_length - 4;
    if ((stem_length >= 2 &&
         (!memcmp(text, "be", 2) || !memcmp(text, "ge", 2) ||
          !memcmp(text, "er", 2))) ||
        (stem_length >= 3 && !memcmp(text, "ver", 3)) ||
        text[0] == 'v' ||
        (chen && stem_length >= 2 &&
         (!memcmp(text + stem_length - 2, "au", 2) ||
          !memcmp(text + stem_length - 2, "aa", 2))) ||
        (chen && stem_length >= 2 &&
         !memcmp(text + stem_length - 2, "ts", 2)) ||
        (!chen && stem_length >= 3 &&
         !memcmp(text + stem_length - 3, "aal", 3)))
        return 0;

    if (reloq_german_g2p_word(text, stem_length, spr, sizeof spr, &n) != 1 ||
        n < 5 || memcmp(spr, "`[.1", 4) != 0 || spr[n - 1] != ']' ||
        memchr(spr + 4, '.', n - 5))
        return 0;

    retained = n - 1;
    if (!chen) ending = ".0l'aj'n]";
    else if (text[stem_length - 1] == 's' &&
             text[stem_length - 2] != 's') {
        if (n < 6 || spr[n - 2] != 's') return 0;
        --retained;
        ending = ".0S@n]";
    } else ending = ".0X@n]";

    required = retained + strlen(ending);
    if (!output || capacity <= required) {
        *byte_length = required;
        return -2;
    }
    memcpy(output, spr, retained);
    memcpy(output + retained, ending, required - retained);
    output[required] = 0;
    *byte_length = required;
    return 1;
}

/* Suffixal -lich and its ordinary weak adjective endings attach to an
 * independently covered stem.  The public German annotations place final
 * /X/ in a new weak syllable; before an ending, that /X/ starts a second
 * weak syllable. Restrict the orthographic stem coda to the classes whose
 * boundary survives unchanged: other codas can move to the suffix or create
 * secondary stress, so a guessed parse there is worse than no strict parse. */
static int lich_from_base(const unsigned char *text, size_t word_length,
                          unsigned char *output, size_t capacity,
                          size_t *byte_length)
{
    static const struct {
        const char *letters;
        const char *phones;
    } endings[] = {
        { "lichen", ".0lI.0X@n]" },
        { "lichem", ".0lI.0X@m]" },
        { "liches", ".0lI.0X@s]" },
        { "licher", ".0lI.0XR]" },
        { "liche", ".0lI.0X@]" },
        { "lich", ".0lIX]" }
    };
    size_t stem_length = 0, n = 0, required;
    const char *tail = NULL;
    unsigned char base[256];
    if (word_length < 6) return 0;
    for (size_t i = 0; i < sizeof endings / sizeof endings[0]; ++i) {
        size_t suffix_length = strlen(endings[i].letters);
        if (word_length > suffix_length + 1 &&
            !memcmp(text + word_length - suffix_length,
                    endings[i].letters, suffix_length)) {
            stem_length = word_length - suffix_length;
            tail = endings[i].phones;
            break;
        }
    }
    if (!tail) return 0;
    if ((stem_length >= 2 &&
         (!memcmp(text, "be", 2) || !memcmp(text, "ge", 2) ||
          !memcmp(text, "er", 2))) ||
        (stem_length >= 3 && !memcmp(text, "ver", 3))) return 0;
    unsigned char c = text[stem_length - 1];
    if (c != 'l' && c != 'n' && c != 't' &&
        !(stem_length >= 2 &&
          ((!memcmp(text + stem_length - 2, "ss", 2)) ||
           (!memcmp(text + stem_length - 2, "tt", 2)) ||
           (!memcmp(text + stem_length - 2, "tz", 2))))) return 0;
    if (reloq_german_g2p_word(text, stem_length,
                              base, sizeof base, &n) != 1 ||
        n < 5 || memcmp(base, "`[.1", 4) || base[n - 1] != ']') return 0;
    required = n - 1 + strlen(tail);
    if (!output || capacity <= required) {
        *byte_length = required;
        return -2;
    }
    memcpy(output, base, n - 1);
    memcpy(output + n - 1, tail, strlen(tail));
    output[required] = 0;
    *byte_length = required;
    return 1;
}

static size_t approximate_gap_split(const unsigned char *,size_t,size_t,size_t,int *);

int reloq_german_g2p_word(const unsigned char *word, size_t word_length,
                          unsigned char *output, size_t capacity,
                          size_t *byte_length)
{
    unsigned char text[128];
    char result[256];
    size_t used = 0;
    size_t suffix_length = 0;
    const char *suffix_phone = NULL;
    size_t vowel_at = SIZE_MAX, vowel_width = 1;
    enum vowel_kind vowel = A;
    int explicit_length = 0, short_vowel = 0, back_vowel = 0;
    int double_s = 0, stressed_iren = 0;

    if (!word || !byte_length || (!output && capacity)) return -1;
    if (word_length < 2 || word_length >= sizeof text) return 0;
    for (size_t i = 0; i < word_length; ++i) {
        if (!normalize(word[i], &text[i])) return -1;
    }
    if (text[0] == 'u' && text[1] == 'n') {
        int status = negative_un_from_base(text, word_length,
                                           output, capacity, byte_length);
        if (status) return status;
    }
    if ((text[0] == 'b' && text[1] == 'e') ||
        (text[0] == 'g' && text[1] == 'e') ||
        (text[0] == 'e' && text[1] == 'r') ||
        (word_length >= 3 && !memcmp(text, "ver", 3))) {
        int status = weak_prefix_from_base(text, word_length,
                                           output, capacity, byte_length);
        if (status) return status;
    }
    if (text[word_length - 1] == 't' || text[word_length - 1] == 'e' ||
        (text[word_length - 1] == 'n' && word_length >= 3 &&
         !memcmp(text + word_length - 3, "ten", 3))) {
        int status = voiced_stop_inflection_from_en(
            text, word_length, output, capacity, byte_length);
        if (status) return status;
    }
    if (word_length >= 5 &&
        (text[word_length - 1] == 'e' ||
         (text[word_length - 1] == 'n' &&
          text[word_length - 2] == 'e'))) {
        int status = past_from_verified_en(text, word_length,
                                           output, capacity, byte_length);
        if (status) return status;
    }
    if (word_length >= 4 && text[word_length - 2] == 'i' &&
        text[word_length - 1] == 'g') {
        int status = ig_from_en(text, word_length,
                                output, capacity, byte_length);
        if (status) return status;
    }
    if (word_length >= 7 &&
        (memcmp(text + word_length - 5, "ungen", 5) == 0 ||
         (word_length >= 8 &&
          (memcmp(text + word_length - 6, "heiten", 6) == 0 ||
           memcmp(text + word_length - 6, "keiten", 6) == 0))))
        return nominal_plural_from_singular(text, word_length,
                                            output, capacity, byte_length);
    if (word_length >= 5 && !memcmp(text + word_length - 3, "ung", 3))
        return ung_from_en(text, word_length, output, capacity, byte_length);
    if (word_length >= 6 &&
        (!memcmp(text + word_length - 4, "heit", 4) ||
         !memcmp(text + word_length - 4, "keit", 4)))
        return heit_keit_from_base(text, word_length,
                                   output, capacity, byte_length);
    if (word_length >= 6 &&
        (!memcmp(text + word_length - 4, "chen", 4) ||
         !memcmp(text + word_length - 4, "lein", 4))) {
        int status = diminutive_from_base(text, word_length,
                                          output, capacity, byte_length);
        if (status) return status;
    }
    if (word_length >= 6) {
        int status = lich_from_base(text, word_length,
                                    output, capacity, byte_length);
        if (status) return status;
    }
    /* These consonant spellings are frequently borrowed with phonemes
     * selected by the lexicon. Do not present the regular German reading
     * as known for such words. */
    if ((text[0] == 'c' && text[1] == 'h') ||
        (text[0] == 's' && text[1] == 'k') ||
        text[word_length - 1] == 'w') return 0;
    if (word_length >= 4 &&
        memcmp(text + word_length - 4, "iend", 4) == 0) return 0;
    /* Final -ien can be one syllable (dien) or add a reduced syllable
     * (knien), except after a complete -ei- diphthong (freien).
     * Final -een can be regular (Seen) or a loan (Spleen). */
    if (word_length >= 3 &&
        ((memcmp(text + word_length - 3, "ien", 3) == 0 &&
          !(word_length >= 4 && text[word_length - 4] == 'e')) ||
         (memcmp(text + word_length - 3, "een", 3) == 0))) return 0;
    if (word_length >= 3 && text[word_length - 2] == 'e' &&
        (text[word_length - 3] != 'i' ||
         (word_length >= 4 && text[word_length - 4] == 'e' &&
          text[word_length - 1] != 't')) &&
        text[word_length - 3] != 'e' &&
        (text[word_length - 1] == 'n' || text[word_length - 1] == 'm' ||
         text[word_length - 1] == 's' || text[word_length - 1] == 't')) {
        /* In the verified weak-ending family the final consonant stays in
         * the reduced syllable: -en/-em/-es/-et -> /@n/@m/@s/@t/.
         * After written -ie/-ee, the e belongs to the stem vowel instead
         * (dies/Tee); let the vowel scanner handle it. */
        suffix_length = 2;
        switch (text[word_length - 1]) {
        case 'n': suffix_phone = "@n"; break;
        case 'm': suffix_phone = "@m"; break;
        case 's': suffix_phone = "@s"; break;
        default: suffix_phone = "@t"; break;
        }
    } else if (word_length >= 3 && text[word_length - 2] == 'e' &&
               text[word_length - 1] == 'r') {
        suffix_length = 2; suffix_phone = "R";
    } else if (text[word_length - 1] == 'e' &&
               text[word_length - 2] != 'e' &&
               (text[word_length - 2] != 'i' ||
                (word_length >= 3 && text[word_length - 3] == 'e'))) {
        suffix_length = 1; suffix_phone = "@";
    }
    /* A final -ier/-eer belongs to the same stem syllable (Tier/leer); it
     * differs from the vocalic -er after a consonant (Kater). */
    if (word_length >= 3 && suffix_length == 2 &&
        text[word_length - 1] == 'r' &&
        (text[word_length - 3] == 'i' || text[word_length - 3] == 'e')) {
        suffix_length = 0; suffix_phone = NULL;
    }
    size_t stem_end = word_length - suffix_length;
    for (size_t i = 0; i < stem_end; ++i) {
        if (text[i] == 'u' && i && text[i - 1] == 'q') continue;
        if (!is_vowel(text[i])) continue;
        if (vowel_at != SIZE_MAX) return 0;
        vowel_at = i;
        switch (text[i]) {
        case 'a': vowel = A; break;
        case 'e': vowel = E; break;
        case 'i': vowel = I; break;
        case 'o': vowel = O; break;
        case 'u': vowel = U; break;
        case 0xe4: vowel = UML_A; break;
        case 0xf6: vowel = UML_O; break;
        case 0xfc: vowel = UML_U; break;
        }
        if (i + 1 < stem_end) {
            unsigned char next = text[i + 1];
            if (text[i] == 'a' && next == 'u') vowel = DIP_AW;
            else if (text[i] == 'e' && next == 'i') vowel = DIP_AJ;
            else if (text[i] == 'a' && next == 'i') return 0;
            else if ((text[i] == 'e' || text[i] == 0xe4) &&
                     next == 'u') vowel = DIP_OJ;
            else if (text[i] == 'o' && next == 'u' &&
                     !reloq_german_morphology_is_root_boundary(text,word_length,i+1)) {
                vowel = U; explicit_length = 2;
            } else if (text[i] == 'i' && next == 'e') {
                vowel = I; explicit_length = 1;
            } else if (next == text[i] &&
                       (next == 'a' || next == 'e' || next == 'o')) {
                /* ai and oo depend strongly on lexical origin in the
                 * observed German module (Mai/fair, Boot/boom). */
                if (next == 'o') return 0;
                explicit_length = 1;
            } else if (next == 'h') {
                explicit_length = 1;
            } else continue;
            vowel_width = 2;
            ++i;
        }
    }
    if (vowel_at == SIZE_MAX) return 0;
    size_t coda_at = vowel_at + vowel_width;
    if (suffix_length == 2 && text[word_length - 1] == 'r' &&
        vowel_width == 2 && text[vowel_at + 1] == 'h' &&
        coda_at == stem_end) return 0;
    if (coda_at < stem_end && text[coda_at] == 'h' &&
        explicit_length) ++coda_at; /* silent h after ie/diphthong */
    size_t coda_length = stem_end - coda_at;
    /* When no consonant separates the stem from written -et, the reference
     * has competing hiatus and stress readings (haet/noet versus neuet).
     * Do not impose a weak syllable on that class. */
    if (suffix_phone && strcmp(suffix_phone, "@t") == 0 && !coda_length)
        return 0;
    stressed_iren = suffix_length == 2 && word_length >= 4 &&
                   memcmp(text + word_length - 4, "iren", 4) == 0 &&
                   vowel == I && vowel_width == 1 &&
                   coda_length == 1 && text[coda_at] == 'r';
    if (coda_length == 2 &&
        ((text[coda_at] == 'z' && text[coda_at + 1] == 'z') ||
         (text[coda_at] == 'r' && text[coda_at + 1] == 'r' &&
          vowel == I && explicit_length) ||
         (text[coda_at] == 't' && text[coda_at + 1] == 'h' &&
          suffix_length == 2 && text[word_length - 1] == 'r')))
        return 0;
    if (vowel == E && explicit_length && vowel_width == 2 &&
        text[vowel_at + 1] == 'e' &&
        ((coda_length == 1 && text[coda_at] == 'n') ||
         (coda_length == 2 && text[coda_at] == 'c' &&
          text[coda_at + 1] == 'h')))
        return 0;
    if (vowel == DIP_OJ && vowel_at >= 2 &&
        text[0] == 'q' && text[1] == 'u') return 0;
    if (coda_length >= 2 && text[coda_at] == text[coda_at + 1] &&
        coda_length == 2) short_vowel = 1;
    if (coda_length == 2 &&
        ((text[coda_at] == 'c' && text[coda_at + 1] == 'k') ||
         (text[coda_at] == 't' && text[coda_at + 1] == 'z')))
        short_vowel = 1;
    if (coda_length && text[coda_at]=='x') short_vowel=1;
    if (explicit_length) short_vowel = 0;
    /* ou is one nucleus. The original o rule chooses its quantity from
     * the next two written consonants, before diaphone contraction. */
    if (explicit_length==2 && coda_length>=2 &&
        !is_vowel(text[coda_at]) && !is_vowel(text[coda_at+1])) short_vowel=1;
    if (coda_length >= 2 && text[coda_at] == 's' &&
        text[coda_at + 1] == 's') double_s = 1;
    if (suffix_length) {
        if (coda_length > 2 && !(coda_length == 3 &&
                                 memcmp(text + coda_at, "sch", 3) == 0))
            return 0;
        if (!explicit_length && vowel <= UML_U &&
            (coda_length > 1 && !short_vowel)) return 0;
        /* 'ch' between a first vowel and schwa has unpredictable vowel
         * length in this small orthographic grammar (Kuchen/wachen). */
        if (!explicit_length && coda_length == 2 &&
            memcmp(text + coda_at, "ch", 2) == 0) return 0;
        if (!explicit_length && coda_length == 1 &&
            text[coda_at] == 0xdf) return 0;
    } else if (!explicit_length && vowel <= UML_U && !short_vowel &&
               !(coda_length == 1 && text[coda_at] == 0xdf)) {
        return 0;
    }
    if (!suffix_length && coda_length > 3) return 0;
    /* Older spellings with ß after a short vowel coexist with long-vowel
     * spellings (muß/Fuß). The spelling alone cannot choose between them. */
    if (!suffix_length && !explicit_length && coda_length == 1 &&
        text[coda_at] == 0xdf) return 0;
    back_vowel = vowel == A || vowel == O || vowel == U || vowel == DIP_AW;
    if (stressed_iren) {
        /* A single written -iren syllable is stressed on -ren in the
         * observed rule family (Biren, Piren, Viren, griren). Written
         * -ieren is a different three-syllable pattern, unsupported here. */
        if (!append(result, &used, "`[.0")) return 0;
        if (!consonants(text, vowel_at, 1, 0, 0, result, &used)) return 0;
        if (!append(result, &used, "i.1ren")) return 0;
    } else {
        if (!append(result, &used, "`[.1")) return 0;
        if (!consonants(text, vowel_at, 1, 0, back_vowel, result, &used))
            return 0;
        if (!append(result, &used,
                    vowel_spelling(vowel, !short_vowel))) return 0;
        if (suffix_length) {
            const int split_x=text[coda_at]=='x' &&
                (coda_length==1 || (coda_length==2 && text[coda_at+1]=='t'));
            if (split_x && !append(result,&used,"k")) return 0;
            /* The same right-to-left onset assignment applies before a
             * weak ending. Moving the complete written gap into that
             * ending invented ft/nt/rd onsets after marked long vowels. */
            int doubled=0;
            size_t coda=split_x?0:approximate_gap_split(text,word_length,
                coda_at,stem_end,&doubled);
            if(coda && !consonants(text+coda_at,coda,0,1,back_vowel,result,&used))return 0;
            coda_at+=coda;coda_length-=coda;
            if (!append(result, &used, ".0")) return 0;
            if (coda_length == 2 &&
                text[coda_at] == text[coda_at + 1]) coda_length = 1;
            if (double_s || split_x) {
                if (!append(result, &used, split_x && coda_length==2?"st":"s")) return 0;
            } else if (!consonants(text + coda_at, coda_length, 0, 0,
                                   back_vowel, result, &used)) return 0;
            if (!append(result, &used, suffix_phone)) return 0;
        } else if (coda_length == 1 && text[coda_at] == 'r' &&
                   vowel >= DIP_AW) {
            if (!append(result, &used, ".0R")) return 0;
        } else {
            if (coda_length == 2 &&
                text[coda_at] == text[coda_at + 1]) coda_length = 1;
            if (!consonants(text + coda_at, coda_length, 0, 1, back_vowel,
                            result, &used)) return 0;
        }
    }
    if (!append(result, &used, "]")) return 0;
    if (!output || capacity <= used) {
        *byte_length = used;
        return -2;
    }
    memcpy(output, result, used);
    output[used] = 0;
    *byte_length = used;
    return 1;
}

struct approximate_nucleus {
    size_t at, end;
    enum vowel_kind kind;
    int explicit_length, short_marker;
};

/* Estimate the regular -en -> -te/-ten change from an already analyzed
 * infinitive. The consonant between its stem vowel and schwa closes the
 * stressed syllable and devoices before /t/. Other orthographic contexts
 * are intentionally left for the generic approximation below. */
static int approximate_past_from_en(const unsigned char *text, size_t length,
                                    unsigned char *output, size_t capacity,
                                    size_t *byte_length)
{
    size_t ending;
    if (length > 4 && !memcmp(text + length - 2, "te", 2)) ending = 2;
    else if (length > 5 && !memcmp(text + length - 3, "ten", 3)) ending = 3;
    else return 0;
    size_t root = length - ending;
    int marked=0;
    for (size_t i=0;i+1<root;++i) {
        unsigned char a=text[i],b=text[i+1];
        if ((is_vowel(a) && (b=='h' || a==b)) ||
            (a=='i' && b=='e') || (a=='a' && (b=='u' || b=='i')) ||
            ((a=='e' || a==0xe4) && (b=='u' || b=='i')) ||
            (!is_vowel(a) && a==b) || (a=='c' && b=='k') || (a=='t' && b=='z')) marked=1;
    }
    const int quantity=marked?-1:t_quantity(text,root);
    if (!marked && quantity<0) return 0;
    /* Bare -sten is frequently a noun/adjective suffix rather than past
     * -ten (Kasten, Kosten, besten). Let the general st syllabifier decide.
     * Actual s-stem pasts such as reisten share that syllable shape. */
    if (text[root - 1] == 's') return 0;
    unsigned char infinitive[128], spr[256];
    size_t n = 0;
    memcpy(infinitive, text, root);
    memcpy(infinitive + root, "en", 2);
    if (reloq_german_g2p_word(infinitive, root + 2,
                              spr, sizeof spr, &n) != 1 || n < 9 ||
        memcmp(spr, "`[.1", 4) || memcmp(spr + n - 3, "@n]", 3))
        return 0;
    if (!primary_quantity(spr,&n,quantity)) return 0;
    size_t boundary = n - 6;
    if (spr[boundary] != '.' || spr[boundary + 1] != '0') return 0;
    unsigned char c = spr[boundary + 2];
    if (!strchr("bdfgklmnprstxzX", c)) return 0;
    if (c == 'b') c = 'p';
    else if (c == 'd') c = 't';
    else if (c == 'g') c = 'k';
    else if (c == 'z') c = 's';
    const char *tail = ending == 2 ? ".0t@]" : ".0t@n]";
    size_t required = boundary + 1 + strlen(tail);
    if (!output || capacity <= required) {
        *byte_length = required;
        return -2;
    }
    memcpy(output, spr, boundary);
    output[boundary] = c;
    memcpy(output + boundary + 1, tail, strlen(tail));
    output[required] = 0;
    *byte_length = required;
    return 1;
}

static int approximate_vowels(const unsigned char *text, size_t length,
                               struct approximate_nucleus *vowels,
                               size_t *count)
{
    size_t total = 0;
    for (size_t i = 0; i < length; ++i) {
        if (text[i] == 'u' && i && text[i - 1] == 'q') continue;
        /* <y> is vocalic in many loans (System, Synthese), but behaves as
         * a consonantal glide before another vowel (Yacht, Yoga). */
        if (!is_vowel(text[i]) &&
            !(text[i] == 'y' && !(i + 1 < length && is_vowel(text[i + 1]))))
            continue;
        if (total == 32) return 0;
        struct approximate_nucleus *v = &vowels[total++];
        v->at = i; v->end = i + 1;
        v->explicit_length = 0; v->short_marker = 0;
        switch (text[i]) {
        case 'a': v->kind = A; break;
        case 'e': v->kind = E; break;
        case 'i': v->kind = I; break;
        case 'o': v->kind = O; break;
        case 'u': v->kind = U; break;
        case 0xe4: v->kind = UML_A; break;
        case 0xf6: v->kind = UML_O; break;
        case 'y': v->kind = i+1==length?I:UML_U; break;
        default: v->kind = UML_U; break;
        }
        if (i + 1 >= length) continue;
        unsigned char next = text[i + 1];
        if (text[i] == 'a' && next == 'u') v->kind = DIP_AW;
        /* The established be-/ge- domain is decided before ey can
         * consume its boundary. A shorter residual spelling does not
         * license prefix stripping in the original. */
        else if ((text[i]=='e' || text[i]=='a') &&
                 (next=='i' || (next=='y' &&
                  !(i==1 && length>=5 &&
                    (!memcmp(text,"be",2) || !memcmp(text,"ge",2))))))
            v->kind=DIP_AJ;
        else if ((text[i] == 'e' || text[i] == 0xe4) && next == 'u')
            v->kind = DIP_OJ;
        /* The original o rule excludes n, st and sm before forming oi.
         * This decision precedes stress and weak-ending syllabification. */
        else if (text[i]=='o' && next=='i' &&
                 !(i+2<length && (text[i+2]=='n' ||
                   (text[i+2]=='s' && i+3<length && strchr("tm",text[i+3])))) &&
                 !reloq_german_morphology_is_root_boundary(text,length,i+1))
            v->kind=DIP_OJ;
        else if (text[i] == 'o' && next == 'u' &&
                 !reloq_german_morphology_is_root_boundary(text,length,i+1)) {
            v->kind=U;v->explicit_length=2;
        } else if (text[i] == 'i' && next == 'e') v->explicit_length = 1;
        else if (text[i] == next &&
                 (next == 'a' || next == 'e' || next == 'o'))
            v->explicit_length = 1;
        else if (next == 'h') v->explicit_length = 1;
        else continue;
        v->end = ++i + 1;
    }
    *count = total;
    return total > 0;
}

/* Independently formulated from assign_ger_vowel_diaphons and checked
 * against diaphone-stage observations. A valid next-syllable onset does
 * not by itself imply a long preceding vowel (gn/kn/kv); tl/dl can keep
 * a long vowel despite splitting the consonants between syllables. */
int reloq_german_g2p_cluster_quantity(const unsigned char *span,size_t length)
{
    if (!span) return -1;
    /* ph/th are normalized before quantity and onset assignment. Their
     * following consonants do not turn the written digraph into two phones. */
    if (length>=2 && span[1]=='h' && (span[0]=='p' || span[0]=='t' || span[0]=='g')) return 1;
    if (length==2) {
        if((span[0]=='c' && span[1]=='k') || (span[0]=='t' && span[1]=='z'))return 0;
        if(span[0]=='c' && span[1]=='c')return 0;
        if ((span[1]=='r' && strchr("ptkbdgf",span[0])) ||
            (span[1]=='l' && strchr("ptkbdgf",span[0])) ||
            (span[1]=='h' && (span[0]=='p' || span[0]=='t'))) return 1;
        if ((strchr("bdf",span[0]) && strchr("mnv",span[1])) ||
            (span[0]=='g' && (span[1]=='n' || span[1]=='m' || span[1]=='v')) ||
            (span[0]=='k' && strchr("nvw",span[1])) ||
            (span[0]=='q' && span[1]=='u') ||
            (span[0]=='s' && span[1]=='t')) return 0;
    }
    if (length>=3 && !memcmp(span,"sch",3)) return 0;
    return -1;
}

/* A stripped ending is outside the diaphone quantity domain even when
 * its vowel later takes a root-final consonant into its syllable. */
static int weak_morph_tail(const unsigned char *text,size_t length)
{
    static const char *const tails[]={"e","en","em","er","es","et","est",
        "end","ende","enden","endem","ender","endes","ern","erisch",
        "erische","erischen","erischer","erisches","ete","eten","eter",
        "etes","ung","ungen","isch","ische","ischen","ischer","isches",
        "ischem","ig","ige","igen","igem","iger","iges","lich","liche","lichen","licher","liches","lichem"};
    for (size_t i=0;i<sizeof tails/sizeof *tails;++i)
        if (strlen(tails[i])==length && !memcmp(text,tails[i],length)) return 1;
    return 0;
}

static int weak_v_domain_tail(const unsigned char *text,size_t length)
{
    if(weak_morph_tail(text,length))return 1;
    /* The reduced root ending survives a further weak inflection. */
    return length>=2 && (!memcmp(text,"er",2)||!memcmp(text,"el",2)||
        !memcmp(text,"en",2)) && (length==2 || weak_morph_tail(text+2,length-2));
}

/* generate_ger_diaphons supplies a nucleus to a stranded final nasal or
 * lateral after an obstruent (and between unlike nasals). This precedes
 * syllabification and final devoicing. r/l do not license this insertion. */
static size_t terminal_sonorant(const unsigned char *s,size_t n)
{
    if(n<2)return 0;
    size_t at=n-1;
    if(strchr("st",s[at]) && at) --at;
    if(!at || !strchr("mnl",s[at]))return 0;
    if(at==1 && strchr("bdgptkfvwsmn",s[0]) && s[0]!=s[at])return at;
    if(at==2 && !memcmp(s,"ch",2))return at;
    if(at==3 && !memcmp(s,"sch",3))return at;
    return 0;
}

static size_t approximate_gap_split(const unsigned char *text, size_t word_length,
                                    size_t start, size_t end, int *doubled)
{
    size_t length = end - start;
    *doubled = 0;
    if (length < 2) return 0;
    /* The original phonemic syllabifier excludes /G/ from a single-phone
     * onset (assign_ger_syll_init_con, name 23). An intrastem ng therefore
     * closes the preceding syllable. Prefix/base boundaries are resolved
     * earlier; this branch does not reinterpret an already composed base. */
    if (length == 2 && !memcmp(text + start, "ng", 2)) return length;
    if ((length==2 && !memcmp(text+start,"rh",2)) ||
        (length==3 && !memcmp(text+start,"rrh",3))) {
        /* Root-local rh cannot cross an already classified compound edge. */
        if(reloq_german_morphology_is_root_boundary(text,word_length,end-1) ||
           (reloq_german_morphology_constituent_class(text,end-1)&1u) ||
           (reloq_german_morphology_constituent_class(text+end-1,word_length-end+1)&16u))
            return length-1;
        *doubled=2; /* Short quantity, but no repeated letter to discard. */
        return 0;
    }
    if (length == 2 && text[start] == text[start + 1]) {
        /* Two c letters are two diaphones before a front vowel: /k.ts/.
         * The ordinary written-double contraction would erase /k/. */
        if(text[start]=='c' && end<word_length && strchr("eiy",text[end]))return 1;
        *doubled = 1;
        return 0;
    }
    /* The consonant pass consumes gh, and adjacent identical t phones
     * coalesce after th has become t. Keep the whole span until that pass;
     * splitting its spelling first invents an extra consonant or h onset. */
    if((length==2 && !memcmp(text+start,"gh",2)) ||
       (length==3 && !memcmp(text+start,"tht",3)))return 0;
    /* The last two/three/four written consonants can be a *single* German
     * onset. Splitting an rch/dch/nsch gap before the last h invents /k.h/
     * or /s.k.h/ instead of the existing ch/sch/tsch phone mapper. */
    /* Initial ent- is a separable weak prefix: ent+scheiden has /nt.S/.
     * In a root such as pantschen, the same letters are /n.C/ instead;
     * restricting this branch to the prefix keeps the affricate intact. */
    if (start == 1 && text[0] == 'e' && length >= 5 &&
        !memcmp(text + end - 5, "ntsch", 5))
        return length - 3;
    /* Independently expressed branches of assign_ger_syll_init_con:
     * stop/f/pf + l/r (except tl/dl), g + m/n, k + n/v, ts + v,
     * and the S + consonant groups, optionally ending in l/r after p/t.
     * Match their written spans together so a digraph is never split.
     * Plain intervocalic sp is deliberately absent: the original selects
     * p there, whereas st belongs together. Earlier morphology supplies
     * separately identified prefix boundaries before this fallback. */
    static const char *const onsets[] = {
        "schpl", "schpr", "schtr", "schl", "schr", "schm", "schn",
        "schw", "schp", "scht", "pfl", "pfr", "bl", "br", "pl",
        "pr", "dr", "tr", "gl", "gr", "kl", "kr", "fl", "fr", "phl", "phr", "thr",
        "gn", "gm", "kn", "kv", "kw", "zw"
    };
    for (size_t i=0;i<sizeof onsets/sizeof *onsets;++i) {
        size_t n=strlen(onsets[i]);
        /* Keep each diaphone intact when extending the next onset. */
        if (n>length || memcmp(text+end-n,onsets[i],n)) continue;
        /* Syllable onset selection scans phone classes from the right;
         * a preceding root consonant is not limited to a nasal. Earlier
         * morphology has already supplied any established root boundary. */
        /* ng is one diaphone, not n plus the beginning of gl/gr/gn.
         * A morphological kv coda devoices before ending resyllabification. */
        if (n<length && onsets[i][0]=='g' && text[end-n-1]=='n') continue;
        if (!strcmp(onsets[i],"kv") && weak_morph_tail(text+end,word_length-end)) continue;
        if (n<length && !strcmp(onsets[i],"zw")) continue;
        /* The original stops at an already established syllable boundary
         * before trying clusters. This fallback has no lexical morphology
         * boundary map. Leave its previous behavior for unresolved l-initial
         * derivational shapes, including further inflection/compounding;
         * spelling alone cannot distinguish moeg+lich from unknown aglich.
         * This is a coverage guard, not a claim that every such spelling
         * has a suffix boundary. */
        size_t tail=word_length-(end-1);
        /* strip_outer_noun_forming_sffx recognizes -reich before the
         * later ending. Its /r/ starts a suffix, so a preceding d/t must
         * not be pulled into dr/tr across that established boundary. */
        if (onsets[i][n-1]=='r' && tail>=5 &&
            !memcmp(text+end-1,"reich",5)) continue;
        if (onsets[i][n-1]=='l' &&
            ((tail>=4 && (!memcmp(text+end-1,"lich",4) ||
                          !memcmp(text+end-1,"ling",4))) ||
             (tail>=3 && !memcmp(text+end-1,"los",3)))) continue;
        if (n<length) {
            /* A phonotactic onset cannot absorb a consonant across a
             * separately recognized lexical root boundary. */
            for (size_t boundary=end-n+1;boundary<end;++boundary)
                if (reloq_german_morphology_is_root_boundary(text,word_length,boundary))
                    return boundary-start;
        }
        return length-n;
    }
    if (length >= 4 && !memcmp(text + end - 4, "tsch", 4))
        return length - 4;
    if (length >= 3 && (!memcmp(text + end - 3, "sch", 3) ||
                       !memcmp(text + end - 3, "tch", 3)))
        return length - 3;
    /* A written double is consumed before a following h. The second
     * letter cannot also begin sh/ph/wh in the next syllable. */
    if(length>=3 && text[end-1]=='h' && text[end-2]==text[end-3] &&
       strchr("spw",text[end-2]))return length-1;
    if (length >= 2 && (!memcmp(text + end - 2, "ch", 2) ||
        !memcmp(text + end - 2, "sh", 2) || !memcmp(text + end - 2, "ph", 2) ||
        !memcmp(text + end - 2, "wh", 2))) return length - 2;
    if ((length == 2 && (!memcmp(text + start, "ch", 2) ||
                         !memcmp(text + start, "ck", 2) ||
                         !memcmp(text + start, "pf", 2) ||
                         !memcmp(text + start, "ph", 2) ||
                         !memcmp(text + start, "th", 2) ||
                         !memcmp(text + start, "qu", 2) ||
                         !memcmp(text + start, "tz", 2) ||
                         !memcmp(text + start, "st", 2))) ||
        (length == 3 && !memcmp(text + start, "sch", 3))) return 0;
    if (length >= 3 && !memcmp(text + end - 2, "st", 2))
        return length - 2;
    return length - 1;
}

/* Written st immediately after er- can also be part of the unrelated
 * stressed stem erst- (erste, ersten).  A marked root after st, such as
 * stell-/stick-/statt-, supplies independent orthographic evidence of a
 * separable stem.  This still cannot replace a lexical morphology. */
static int marked_er_st_root(const unsigned char *text, size_t length)
{
    for (size_t i = 4; i + 1 < length; ++i) {
        unsigned char a = text[i], b = text[i + 1];
        if ((!is_vowel(a) && a == b) ||
            (a == 'c' && b == 'k') || (a == 't' && b == 'z') ||
            (is_vowel(a) && b == 'h') ||
            (a == 'i' && b == 'e') ||
            (a == 'e' && (b == 'i' || b == 'u')) ||
            (a == 'a' && b == 'u')) return 1;
    }
    return 0;
}

/* A stem-initial sp/st and their r/l clusters are one onset after a
 * separable, unstressed prefix.  The ordinary intervocalic cluster splitter
 * otherwise strands s in the preceding syllable and voices the following
 * German stem incorrectly. */
static int german_sp_st_onset(const unsigned char *text, size_t length)
{
    if (length < 2 || text[0] != 's' ||
        (text[1] != 'p' && text[1] != 't')) return 0;
    return length == 2 ||
        (length == 3 && (text[2] == 'r' || text[2] == 'l'));
}

/* Written nk, and n just before an onset k, have one velar nasal; written
 * dt has one /t/. Keep this local to the explicitly approximate renderer:
 * conservative pronunciations were checked independently and must not
 * change through the shared consonant mapper. */
static int approximate_coda(const unsigned char *text, size_t length,
                            int next_consonant, int back_vowel, int keep_final_voice,
                            char *target, size_t *used)
{
    size_t start = 0;
    for (size_t i = 0; i < length; ++i) {
        const char *replacement = NULL;
        size_t skip = 1;
        if (text[i] == 'n' && i + 1 < length && text[i + 1] == 'k')
            replacement = "Gk", skip = 2;
        else if (text[i] == 'n' && i + 1 == length &&
                 (next_consonant=='k' || next_consonant=='g'))
            replacement = "G";
        else if (text[i] == 'n' && i + 1 == length && next_consonant=='n')
            replacement = "";
        else if (text[i]=='w' && (!keep_final_voice || i+1<length))
            replacement = "f";
        else if (text[i] == 'd' && i + 1 < length && text[i + 1] == 't')
            replacement = "t", skip = 2;
        if (!replacement) continue;
        if (i > start && !consonants_impl(text + start, i - start, 0, !keep_final_voice,
                                          back_vowel, 1, target, used)) return 0;
        if (!append(target, used, replacement)) return 0;
        i += skip - 1;
        start = i + 1;
    }
    if (keep_final_voice && length>start &&
        (strchr("bdg",text[length-1]) || (length==1 && text[0]=='s')) &&
        !(length>=2 && !memcmp(text+length-2,"ng",2))) {
        if (length-start>1 && !consonants_impl(text+start,length-start-1,0,1,
                                              back_vowel,1,target,used)) return 0;
        return consonants_impl(text+length-1,1,0,0,back_vowel,1,target,used);
    }
    return start == length ||
        consonants_impl(text + start, length - start, 0, 1, back_vowel,
                        1, target, used);
}

static int possible_compound_onset(const unsigned char *word, size_t n)
{
    static const char *const clusters[] = {
        "bl", "br", "dr", "fl", "fr", "gl", "gr", "kl", "kr",
        "pf", "pl", "pr", "qu", "sch", "sp", "st", "th", "tr",
        "tsch", "wr", "zw"
    };
    if (n < 3 || is_vowel(word[0])) return 0;
    if (is_vowel(word[1]) || word[1] == 'y') return 1;
    for (size_t i = 0; i < sizeof clusters / sizeof clusters[0]; ++i) {
        size_t len = strlen(clusters[i]);
        if (n >= len && !memcmp(word, clusters[i], len)) return 1;
    }
    return 0;
}

/* The complete /bl/, /pl/, /tr/ etc. onset precedes a vowel and outranks a
 * spurious split that peels its first consonant into a putative root coda.
 * This phonotactic priority does not prefer either of two equally plausible
 * root boundaries; those remain ambiguous. */
static size_t compound_onset_width(const unsigned char *word, size_t n)
{
    static const char *const clusters[] = {
        "bl", "br", "dr", "fl", "fr", "gl", "gr", "kl", "kr",
        "pf", "pl", "pr", "qu", "sch", "sp", "st", "th", "tr",
        "tsch", "wr", "zw"
    };
    size_t width = 1;
    if (!possible_compound_onset(word, n)) return 0;
    /* Modern German <th> denotes /t/ in a few lexical spellings but also
     * arises across an ordinary t+h compound boundary (Zeit+haus). The
     * latter cannot be outranked as though <th> were a productive onset. */
    if (n >= 2 && word[0] == 't' && word[1] == 'h') return 0;
    for (size_t i = 0; i < sizeof clusters / sizeof clusters[0]; ++i) {
        size_t k = strlen(clusters[i]);
        if (n >= k && !memcmp(word, clusters[i], k) && k > width)
            width = k;
    }
    return width;
}

/* Productive noun/adjective endings are not evidence for another stressed
 * lexical root. The strict single-word grammar can coincidentally pronounce
 * a fragment such as a reduced suffix as if it were a standalone word.
 * Without a verified lexical/morphological stem, keep the existing full-word
 * analysis for these endings. */
static int derivational_or_inflectional_tail(const unsigned char *text,
                                             size_t length)
{
    static const char *const endings[] = {
        "ung", "ungen", "bar", "bare", "baren", "barer", "bares", "barem",
        "ere", "eren", "erer", "eres", "erem"
    };
    for (size_t i = 0; i < sizeof endings / sizeof endings[0]; ++i) {
        size_t n = strlen(endings[i]);
        if (length >= n + 3 && !memcmp(text + length - n,
                                        endings[i], n)) return 1;
    }
    return 0;
}

/* Decide a transparent two-root compound before the whole spelling is
 * syllabified.  A later stress-only pass cannot restore a cluster that the
 * whole-word scanner has already split (Wein+blatt -> n.p.l, Neu+platz ->
 * p.l). Both complete constituent readings must be independently covered
 * by the conservative grammar; exactly one orthographic split may qualify.
 * There is no pronunciation table or spelling-specific replacement here.
 * A first root ending in written -e can have a linking element and different
 * stress (Sonne+...), and thus requires a richer morphological analysis. */
static int compose_checked_roots(const unsigned char *text, size_t length,
                                 unsigned char *output, size_t capacity,
                                 size_t *byte_length)
{
    unsigned char selected[256];
    size_t selected_length = 0;
    size_t best_onset = 0;
    int matches = 0;
    int closed_stop_cluster = 0;
    if (length < 6 ||
        derivational_or_inflectional_tail(text, length)) return 0;
    for (size_t split = 3; split + 3 <= length; ++split) {
        unsigned char first[256], second[256];
        size_t left = 0, right = 0, required, onset;
        /* Final -bei/-zu particles and the -tier noun head can receive
         * primary rather than secondary accent.  Twelve independently
         * combined first roots plus -tier all used that other template in
         * the public German callback. Do not impose this template there. */
        onset = compound_onset_width(text + split, length - split);
        if (text[split - 1] == 'e' ||
            (length - split == 3 &&
             !memcmp(text + split, "bei", 3)) ||
            (length - split >= 4 &&
             !memcmp(text + split, "tier", 4)) ||
            !onset ||
            /* Internal <sp/st> need not use the word-initial /Sp/St/ of a
             * standalone second root. Separable verbs were resolved by a
             * stronger morphological rule before this compound pass. */
            (text[split] == 's' && length - split >= 2 &&
             (text[split + 1] == 'p' || text[split + 1] == 't')) ||
            reloq_german_g2p_word(text, split,
                                  first, sizeof first, &left) != 1 ||
            reloq_german_g2p_word(text + split, length - split,
                                  second, sizeof second, &right) != 1 ||
            left < 5 || right < 5 ||
            memcmp(first, "`[.1", 4) || memcmp(second, "`[.1", 4) ||
            first[left - 1] != ']' || second[right - 1] != ']' ||
            memchr(first + 4, '2', left - 5) ||
            memchr(second + 4, '2', right - 5)) continue;

        required = left - 1 + right - 2;
        if (required >= sizeof selected) continue;
        /* At a vowel-final first root the reference can reassign /tr/ to
         * the preceding syllable.  The bare component join has no evidence
         * for choosing between those readings. */
        if (is_vowel(text[split - 1]) && length - split >= 2 &&
            text[split] == 't' && text[split + 1] == 'r') continue;
        /* Both Haupt+raum and Haup+traum can satisfy the tiny C word
         * grammar. A real closed /pt/ or /kt/ root coda prevents selecting
         * the other parse by onset length alone. Leave that ambiguity to
         * the existing whole-word analysis. */
        if (onset == 1 && text[split] == 'r' && split >= 2 &&
            text[split - 1] == 't' &&
            (text[split - 2] == 'p' || text[split - 2] == 'k'))
            closed_stop_cluster = 1;
        if (onset < best_onset) continue;
        if (onset == best_onset && ++matches > 1) continue;
        if (onset > best_onset) {
            best_onset = onset;
            matches = 1;
        }
        memcpy(selected, first, left - 1);
        memcpy(selected + left - 1, second + 2, right - 2);
        selected[left] = '2';
        selected_length = required;
    }
    if (matches != 1 || closed_stop_cluster) return 0;
    if (!output || capacity <= selected_length) {
        *byte_length = selected_length;
        return -2;
    }
    memcpy(output, selected, selected_length);
    output[selected_length] = 0;
    *byte_length = selected_length;
    return 1;
}

/* Three separately verified roots retain their own accents. The generic
 * whole-word scanner has no representation for a third stressed component
 * and can even turn its onset into the preceding coda. Keep the search
 * bounded and choose only a single parse with the strongest legitimate
 * consonant onsets at both boundaries. No recursive substring guessing or
 * embedded reference data are needed. */
static int compose_three_checked_roots(const unsigned char *text,
                                       size_t length,
                                       unsigned char *output,
                                       size_t capacity,
                                       size_t *byte_length)
{
    unsigned char selected[256];
    size_t selected_length = 0, best_onsets = 0;
    int matches = 0;
    if (length < 9 || length > 48 ||
        derivational_or_inflectional_tail(text, length)) return 0;
    for (size_t first_end = 3; first_end + 6 <= length; ++first_end) {
        unsigned char first[256];
        size_t first_len = 0;
        if (text[first_end - 1] == 'e' ||
            reloq_german_g2p_word(text, first_end, first,
                                  sizeof first, &first_len) != 1 ||
            first_len < 5 || memcmp(first, "`[.1", 4) ||
            first[first_len - 1] != ']' ||
            memchr(first + 4, '2', first_len - 5)) continue;

        for (size_t second_end = first_end + 3;
             second_end + 3 <= length; ++second_end) {
            size_t middle_size = second_end - first_end;
            size_t last_size = length - second_end;
            size_t onset_middle = compound_onset_width(
                text + first_end, middle_size);
            size_t onset_last = compound_onset_width(
                text + second_end, last_size);
            size_t middle_len = 0, last_len = 0, required;
            unsigned char middle[256], last[256];
            if (!onset_middle || !onset_last ||
                text[second_end - 1] == 'e' ||
                (last_size >= 4 && !memcmp(text + second_end, "tier", 4)) ||
                (last_size == 3 && !memcmp(text + second_end, "bei", 3)) ||
                (is_vowel(text[first_end - 1]) && middle_size >= 2 &&
                 text[first_end] == 't' && text[first_end + 1] == 'r') ||
                (is_vowel(text[second_end - 1]) && last_size >= 2 &&
                 text[second_end] == 't' && text[second_end + 1] == 'r') ||
                reloq_german_g2p_word(text + first_end, middle_size,
                                      middle, sizeof middle,
                                      &middle_len) != 1 ||
                reloq_german_g2p_word(text + second_end, last_size,
                                      last, sizeof last, &last_len) != 1 ||
                middle_len < 5 || last_len < 5 ||
                memcmp(middle, "`[.1", 4) ||
                memcmp(last, "`[.1", 4) ||
                middle[middle_len - 1] != ']' || last[last_len - 1] != ']' ||
                memchr(middle + 4, '2', middle_len - 5) ||
                memchr(last + 4, '2', last_len - 5)) continue;
            required = first_len + middle_len + last_len - 6;
            if (required >= sizeof selected) continue;
            size_t score = onset_middle + onset_last;
            if (score < best_onsets) continue;
            if (score == best_onsets && ++matches > 1) continue;
            if (score > best_onsets) {
                best_onsets = score;
                matches = 1;
            }
            memcpy(selected, first, first_len - 1);
            memcpy(selected + first_len - 1,
                   middle + 2, middle_len - 3);
            memcpy(selected + first_len + middle_len - 4,
                   last + 2, last_len - 2);
            selected[first_len] = '2';
            selected[first_len + middle_len - 3] = '2';
            selected_length = required;
        }
    }
    if (matches != 1) return 0;
    if (!output || capacity <= selected_length) {
        *byte_length = selected_length;
        return -2;
    }
    memcpy(output, selected, selected_length);
    output[selected_length] = 0;
    *byte_length = selected_length;
    return 1;
}

/* Compare segment streams, ignoring only the bracket and syllable/stress
 * notation. The candidate surface and the two component analyses must have
 * precisely the same *phones*. This never substitutes an approximate vowel,
 * an invented coda, or a changed consonant at a guessed compound boundary. */
static int next_phone(const unsigned char *spr, size_t n, size_t *at)
{
    while (*at < n) {
        unsigned char c = spr[(*at)++];
        if (c == '`' || c == '[' || c == ']') continue;
        if (c == '.' && *at < n && spr[*at] >= '0' && spr[*at] <= '2') {
            ++*at;
            continue;
        }
        return c;
    }
    return -1;
}

static int same_phone_stream(const unsigned char *a, size_t an,
                             const unsigned char *b, size_t bn)
{
    size_t ai = 0, bi = 0;
    int x, y;
    do {
        x = next_phone(a, an, &ai);
        y = next_phone(b, bn, &bi);
        if (x != y) return 0;
    } while (x != -1);
    return 1;
}

static size_t syllable_count(const unsigned char *spr, size_t n)
{
    size_t count = 0;
    for (size_t i = 0; i + 1 < n; ++i)
        if (spr[i] == '.' && spr[i + 1] >= '0' && spr[i + 1] <= '2')
            ++count;
    return count;
}

static int best_effort_impl(
    const unsigned char *word, size_t word_length,
    unsigned char *output, size_t capacity, size_t *byte_length,
    int *approximation_used, int compound_stress, int terminal_complete);

/* A separable an-/ab-/auf-/aus- prefix also composes with a productive base
 * outside the conservative one-word grammar.  Keep the complete base
 * analysis, including weak ge-/ver- and final reductions, and demote only
 * its one primary accent.  Restrict the estimate to a consonant-initial
 * base: an apparent prefix before a vowel can be part of a loan spelling
 * (Abakus, Austern versus a genuine aus+... boundary), and a base with its
 * own secondary accent needs a separate compound analysis.  Strict words
 * and strict prefix+base composition have already had precedence. */
static int separable_prefix_from_estimated_base(
    const unsigned char *text, size_t word_length,
    unsigned char *output, size_t capacity, size_t *byte_length)
{
    static const struct {
        const char *spelling;
        size_t length;
        const char *spr;
    } prefixes[] = {
        {"an", 2, "`[.1An"},
        {"ab", 2, "`[.1Ap"},
        {"auf", 3, "`[.1'aw'f"},
        {"aus", 3, "`[.1'aw's"},
    };
    for (size_t i = 0; i < sizeof prefixes / sizeof prefixes[0]; ++i) {
        size_t p = prefixes[i].length;
        if (word_length < p + 4 || memcmp(text, prefixes[i].spelling, p) ||
            is_vowel(text[p])) continue;
        /* Prefix+zu+verb has the base accent after the intervening zu;
         * a separately estimated reading of "zugeben" does not establish
         * that stress position for "abzugeben". */
        if (text[p] == 'z' && text[p + 1] == 'u') continue;
        /* Initial an- is also found in opaque stems.  An independently
         * analyzed root must have a legal German word onset before an-
         * can be inferred; a t+w cluster is not such an onset. */
        if (p == 2 && text[1] == 'n' &&
            !possible_compound_onset(text + p, word_length - p)) continue;
        if (p == 2) {
            /* Initial ab-/an- also occur in opaque loan stems.  A
             * transparent verbal or nominal ending supplies independent
             * evidence of a separable stem. */
            size_t stem_length = word_length - p;
            int participle = stem_length >= 5 &&
                !memcmp(text + p, "ge", 2);
            int infinitive = stem_length >= 5 &&
                !memcmp(text + word_length - 2, "en", 2);
            int nominalization = stem_length >= 6 &&
                (!memcmp(text + word_length - 3, "ung", 3) ||
                 !memcmp(text + word_length - 4, "ungs", 4));
            if (!participle && !infinitive && !nominalization) continue;
        }
        unsigned char base[256];
        size_t n = 0, main_at = SIZE_MAX;
        int base_estimated = 0;
        if (reloq_german_g2p_word_best_effort(
                text + p, word_length - p, base, sizeof base,
                &n, &base_estimated) != 1 ||
            n < 5 || memcmp(base, "`[", 2) || base[n - 1] != ']')
            continue;
        for (size_t j = 2; j + 1 < n; ++j) {
            if (base[j] != '.') continue;
            if (base[j + 1] == '2') { main_at = SIZE_MAX; break; }
            if (base[j + 1] == '1') {
                if (main_at != SIZE_MAX) { main_at = SIZE_MAX; break; }
                main_at = j + 1;
            }
        }
        if (main_at == SIZE_MAX) continue;
        size_t leader = strlen(prefixes[i].spr);
        size_t required = leader + n - 2;
        if (required >= 256) continue;
        if (!output || capacity <= required) {
            *byte_length = required;
            return -2;
        }
        memcpy(output, prefixes[i].spr, leader);
        memcpy(output + leader, base + 2, n - 2);
        output[leader + main_at - 2] = '2';
        output[required] = 0;
        *byte_length = required;
        return 1;
    }
    return 0;
}

/* An un- form with a productive adjectival/participial base has the prefix
 * as primary accent and keeps the base's existing main accent as secondary.
 * The visible morphology gate deliberately excludes opaque initial un...
 * spellings such as unser, unter, Uni- and names. The base is analyzed as a
 * whole word first, so weak ge-/be- and inner root accents stay intact. */
static int productive_negative_un(const unsigned char *text, size_t length,
                                   unsigned char *output, size_t capacity,
                                   size_t *byte_length)
{
    static const char *const endings[] = {
        "ig", "lich", "bar", "sam", "haft", "los", "voll", "heit", "keit"
    };
    unsigned char base[256];
    size_t base_length = length - 2, n = 0, required, main_at = SIZE_MAX;
    int eligible = 0, base_estimated = 0;
    if (length < 6 || text[0] != 'u' || text[1] != 'n') return 0;
    /* unter- is itself a productive lexical prefix and an initial five
     * letters cannot identify an un- boundary (Unterhaltbar is not an
     * accented Un+terhaltbar compound). An actual un+ter... adjective is
     * ambiguous in spelling too, so leave that class undecided. */
    if (length >= 5 && !memcmp(text, "unter", 5)) return 0;
    for (size_t i = 0; i < sizeof endings / sizeof endings[0]; ++i) {
        size_t k = strlen(endings[i]);
        if (base_length >= k + 2 &&
            !memcmp(text + length - k, endings[i], k)) eligible = 1;
    }
    if (base_length >= 6 && text[2] == 'g' && text[3] == 'e' &&
        (text[length - 1] == 't' ||
         (text[length - 2] == 'e' && text[length - 1] == 'n')))
        eligible = 1;
    if (!eligible ||
        reloq_german_g2p_word_best_effort(text + 2, base_length,
                                           base, sizeof base, &n,
                                           &base_estimated) != 1 ||
        n < 5 || memcmp(base, "`[", 2) || base[n - 1] != ']') return 0;
    /* A separately analyzed weak verbal prefix must actually remain weak.
     * If the small fallback has swallowed ge-/ver-/be-/er-/ent- into the
     * root, copying it under un- locks in a false vowel and root boundary.
     * In that case leave the spelling to the ordinary fallback instead. */
    const unsigned char *stem = text + 2;
    int weak_ge = base_length >= 2 && !memcmp(stem, "ge", 2) &&
                  n >= 8 && !memcmp(base, "`[.0g@.1", 8);
    /* Written gei- is a single stressed diphthong in Geimpft/Geiz. It is
     * structurally distinct from a weak ge- prefix before another onset. */
    int stressed_gei = base_length >= 3 &&
                       !memcmp(stem, "gei", 3) &&
                       n >= 9 && !memcmp(base, "`[.1g'aj'", 9);
    /* A speculative ge...et analysis often keeps a full /Et/ instead of
     * the weak participial /@t/ (geordnet, geöffnet, gerettet). The prefix
     * structure alone cannot certify its final stem and vowel quality. */
    if (base_estimated && weak_ge && base_length >= 5 &&
        !memcmp(text + length - 2, "et", 2)) return 0;
    if ((base_length >= 2 && !memcmp(stem, "ge", 2) &&
         !weak_ge && !stressed_gei) ||
        (base_length >= 3 && !memcmp(stem, "ver", 3) &&
         (n < 9 || memcmp(base, "`[.0fEr.1", 9))) ||
        (base_length >= 2 && !memcmp(stem, "be", 2) &&
         (n < 8 || memcmp(base, "`[.0b@.1", 8))) ||
        (base_length >= 2 && !memcmp(stem, "er", 2) &&
         (n < 8 || memcmp(base, "`[.0Er.1", 8))) ||
        (base_length >= 3 && !memcmp(stem, "ent", 3) &&
         (n < 9 || memcmp(base, "`[.0Ent.1", 9)))) return 0;
    for (size_t i = 2; i + 1 < n; ++i)
        if (base[i] == '.' && base[i + 1] == '1') {
            if (main_at != SIZE_MAX) return 0;
            main_at = i + 1;
        }
    if (main_at == SIZE_MAX) return 0;
    required = 6 + n - 2;
    if (required >= sizeof base) return 0;
    if (!output || capacity <= required) {
        *byte_length = required;
        return -2;
    }
    memcpy(output, "`[.1Un", 6);
    memcpy(output + 6, base + 2, n - 2);
    output[6 + main_at - 2] = '2';
    output[required] = 0;
    *byte_length = required;
    return 1;
}

/* The adjectival -bar retains its long /a/ when nominalized with -keit.
 * Derive this from a whole independently analyzed adjective, rather than
 * scanning the expanded spelling: the generic medial-cluster heuristic
 * would shorten the vowel and can mistake -bar for an accented noun head.
 * This also works after a weak prefix (bezahlbar -> Bezahlbarkeit). */
static int barkeit_from_bar(const unsigned char *text, size_t length,
                            unsigned char *output, size_t capacity,
                            size_t *byte_length)
{
    static const char ending[] = ".0k'aj't]";
    unsigned char base[256];
    size_t stem_length, n = 0, required;
    if (length < 8 || memcmp(text + length - 7, "barkeit", 7)) return 0;
    stem_length = length - 4;
    if (best_effort_impl(text, stem_length, base, sizeof base,
                         &n, NULL, 0, 1) != 1 || n < 8 ||
        memcmp(base + n - 6, ".0bar]", 6)) return 0;
    required = n - 1 + sizeof ending - 1;
    if (required >= sizeof base) return 0;
    if (!output || capacity <= required) {
        *byte_length = required;
        return -2;
    }
    memcpy(output, base, n - 1);
    memcpy(output + n - 1, ending, sizeof ending - 1);
    output[required] = 0;
    *byte_length = required;
    return 1;
}

/* An independently covered multisyllabic second component sometimes keeps
 * secondary stress despite a generic orthographic syllabifier assigning .0.
 * Only use one unambiguous spelling boundary, a plausible stem onset, a
 * complete primary accent in both components, and an unchanged whole-word
 * phone stream.  One-syllable heads and ambiguous derivational endings need
 * lexical prosody: e.g. -licht is usually weak and -tor varies by compound.
 * They are deliberately not inferred here. */
static void compose_checked_multisyllabic_head(
    const unsigned char *text, size_t word_length,
    unsigned char *candidate, size_t used)
{
    if (word_length < 8 || used < 6 || memcmp(candidate, "`[.1", 4) ||
        memchr(candidate + 2, '2', used - 2)) return;

    unsigned char chosen[256];
    size_t chosen_length = 0;
    int count = 0;
    for (size_t split = 3; split + 4 <= word_length; ++split) {
        if (!possible_compound_onset(text + split, word_length - split))
            continue;
        unsigned char second[256];
        size_t right = 0;
        if (reloq_german_g2p_word(text + split, word_length - split,
                                  second, sizeof second, &right) != 1 ||
            right < 8 || memcmp(second, "`[.1", 4) ||
            syllable_count(second, right) < 2 ||
            memchr(second + 4, '2', right - 5)) continue;

        unsigned char first[256];
        size_t left = 0;
        int first_checked = reloq_german_g2p_word(
            text, split, first, sizeof first, &left) == 1;
        if (!first_checked &&
            best_effort_impl(text, split, first, sizeof first,
                             &left, NULL, 0, 1) != 1) continue;
        if (left < 5 || memcmp(first, "`[.1", 4) ||
            memchr(first + 4, '2', left - 5)) continue;
        /* Unknown first components ending in a stop, or with more than two
         * syllables, are easy to mistake for a fragment of the second one. */
        if (!first_checked &&
            ((text[split - 1] == 'b' || text[split - 1] == 'd' ||
              text[split - 1] == 'g') ||
             syllable_count(first, left) > 2))
            continue;
        /* A partial separable prefix (Aussch+reiben, Abt+ragen) is not a
         * complete constituent. The verified verb rule above handles it. */
        if (!first_checked && split < 6 &&
            ((!memcmp(text, "auf", 3) || !memcmp(text, "aus", 3)) ||
             !memcmp(text, "ab", 2) || !memcmp(text, "an", 2) ||
             !memcmp(text, "ein", 3) || !memcmp(text, "um", 2)))
            continue;
        size_t length = left - 1 + right - 2;
        if (length != used || length >= sizeof chosen) continue;
        unsigned char composed[256];
        memcpy(composed, first, left - 1);
        memcpy(composed + left - 1, second + 2, right - 2);
        if (!same_phone_stream(candidate, used, composed, length)) continue;
        composed[left] = '2';
        if (++count > 1) return;
        memcpy(chosen, composed, length);
        chosen_length = length;
    }
    if (count == 1 && chosen_length == used)
        memcpy(candidate, chosen, used);
}

static int best_effort_impl(
    const unsigned char *word, size_t word_length,
    unsigned char *output, size_t capacity, size_t *byte_length,
    int *approximation_used, int compound_stress, int terminal_complete)
{
    int strict = reloq_german_g2p_word(word, word_length,
                                       output, capacity, byte_length);
    if (strict != 0) {
        if (strict == 1 && approximation_used) *approximation_used = 0;
        return strict;
    }
    if (!word || !byte_length || word_length < 2 || word_length >= 128)
        return 0;
    unsigned char text[128];
    for (size_t i = 0; i < word_length; ++i)
        if (!normalize(word[i], &text[i])) return -1;

    text[word_length]=0;

    /* Reuse a complete adjective stem before decomposing the expanded
     * spelling. This is the same independently checked suffix operation
     * as the lexical path; an estimated base remains explicitly estimated.
     * The recursive input is shorter by at least one byte. */
    size_t adjective_base=reloq_german_adjective_base(text,word_length);
    if (adjective_base) {
        unsigned char stem[256];size_t stem_length=0;
        if (best_effort_impl(text,adjective_base,stem,sizeof stem,&stem_length,NULL,1,0)==1) {
            int formed=reloq_german_adjective_form(text,word_length,stem,stem_length,
                output,capacity,byte_length);
            if (formed) {
                if (formed==1 && approximation_used) *approximation_used=1;
                return formed;
            }
        }
    }

    int derived = productive_negative_un(text, word_length, output, capacity,
                                         byte_length);
    if (derived) {
        if (derived == 1 && approximation_used) *approximation_used = 1;
        return derived;
    }

    derived = separable_prefix_with_zu(text, word_length, output, capacity,
                                       byte_length);
    if (derived) {
        if (derived == 1 && approximation_used) *approximation_used = 1;
        return derived;
    }

    derived = barkeit_from_bar(text, word_length, output, capacity,
                                   byte_length);
    if (derived) {
        if (derived == 1 && approximation_used) *approximation_used = 1;
        return derived;
    }

    int composed = separable_prefix_from_verb(text, word_length,
                                              output, capacity, byte_length);
    if (composed) {
        if (composed == 1 && approximation_used) *approximation_used = 1;
        return composed;
    }

    if (compound_stress) {
        /* Do not promote the estimated prefix inside another derivation:
         * e.g. negative un+ab... has a different accent hierarchy. */
        composed = separable_prefix_from_estimated_base(
            text, word_length, output, capacity, byte_length);
        if (composed) {
            if (composed == 1 && approximation_used) *approximation_used = 1;
            return composed;
        }
    }

    composed = compose_three_checked_roots(text, word_length,
                                           output, capacity, byte_length);
    if (composed) {
        if (composed == 1 && approximation_used) *approximation_used = 1;
        return composed;
    }

    composed = compose_checked_roots(text, word_length,
                                     output, capacity, byte_length);
    if (composed) {
        if (composed == 1 && approximation_used) *approximation_used = 1;
        return composed;
    }

    unsigned char candidate[256];
    size_t needed = 0;
    int past = approximate_past_from_en(text, word_length, candidate,
                                        sizeof candidate, &needed);
    if (past == 1) {
        if (!output || capacity <= needed) {
            *byte_length = needed;
            return -2;
        }
        memcpy(output, candidate, needed + 1);
        *byte_length = needed;
        if (approximation_used) *approximation_used = 1;
        return 1;
    }

    struct approximate_nucleus v[32];
    size_t count = 0;
    if (!approximate_vowels(text, word_length, v, &count)) return 0;
    int voiced_v=0;
    if(memchr(text,'v',word_length)) {
        size_t v_start=0,v_root=word_length,root_nuclei=count;
        /* The existing weak-prefix and independently measured compound
         * boundaries precede the root-local v rule. Do not count their
         * nuclei as part of an opaque loan stem. */
        if(count>1 && word_length>=5 && v[0].end==2 &&
           (!memcmp(text,"be",2)||!memcmp(text,"ge",2)))v_start=2;
        for(size_t at=v_start+2;v_start && at<v_root;++at)
            if(reloq_german_morphology_is_root_boundary(text,word_length,at)) {
                v_root=at;break;
            }
        if(!v_start && word_length>3 && !memcmp(text,"vor",3) &&
           reloq_german_morphology_is_root_boundary(text,word_length,3))v_root=3;
        for(size_t at=v_start+2;at+3<=v_root;++at)
            if(!memcmp(text+at,"ung",3) && (at+3==v_root ||
               weak_morph_tail(text+at+3,v_root-at-3))) {v_root=at;break;}
        struct approximate_nucleus root_v[32];
        if(approximate_vowels(text+v_start,v_root-v_start,root_v,&root_nuclei))
            for(size_t k=1;k<root_nuclei;++k) {
                size_t at=v_start+root_v[k].at;
                if(weak_v_domain_tail(text+at,v_root-at)) {
                    v_root=at;root_nuclei=k;break;
                }
            }
        voiced_v=reloq_german_g2p_v_voiced(text+v_start,v_root-v_start,root_nuclei==1);
        /* A further weak ending does not erase a resolved initial root or
         * prefix. Reuse that shorter analysis before classifying the entire
         * spelling as a new multi-nucleus loan domain. */
        if(text[0]=='v')for(size_t at=word_length-1;at>=3;--at) {
            if(is_vowel(text[at-1]) || !weak_morph_tail(text+at,word_length-at))continue;
            unsigned char base[256];size_t size=0;
            if(best_effort_impl(text,at,base,sizeof base,&size,NULL,0,0)==1 &&
               size>4 && (base[4]=='f'||base[4]=='v')) {voiced_v=base[4]=='v';break;}
        }
        if(voiced_v && !memcmp(text,"ver",3))for(size_t at=4;at+3<=word_length;++at) {
            if(memcmp(text+at,"ung",3) ||
               (at+3<word_length && !weak_morph_tail(text+at+3,word_length-at-3)))continue;
            unsigned char infinitive[128],base[256];size_t size=0;
            memcpy(infinitive,text,at);memcpy(infinitive+at,"en",2);
            if(reloq_german_g2p_word(infinitive,at+2,base,sizeof base,&size)==1 &&
               size>7 && !memcmp(base,"`[.0fEr",7)) {voiced_v=0;break;}
        }
        /* Original inseparable-prefix exclusions are stem-prefix classes,
         * tested before the v rule (observed table 151). An unresolved ver-
         * still belongs to the prefix domain unless such a class protects it. */
        if(voiced_v && word_length>3 && !memcmp(text,"ver",3)) {
            static const char *const opaque[]={"verand","verena","veritab","veron",
                "verbal","verben","verbum","verdun","versatil","version","vertikal"};
            int protected=0;
            for(size_t k=0;k<sizeof opaque/sizeof *opaque;++k) {
                size_t n=strlen(opaque[k]);
                if(word_length>=n && !memcmp(text,opaque[k],n)) {protected=1;break;}
            }
            if(!protected)voiced_v=0;
        }
    }
    const int consonant_mode=1|(voiced_v?2:0);
    /* The observed loan ending is one nucleus /jon/, including its plural.
     * Establish it before syllabification and quantity assignment; treating
     * its written i and o as two vowels creates a spurious stress domain. */
    size_t ion = SIZE_MAX, ion_stress=SIZE_MAX;
    for (size_t j=1;j+1<count;++j) {
        size_t at=v[j].at;
        if (text[at]!='i' || v[j].end!=at+1 || v[j+1].at!=at+1 ||
            text[at+1]!='o' || !strchr("tsg",text[at-1])) continue;
        size_t tail=word_length-at;
        int nominal=((tail==3 && !memcmp(text+at,"ion",3)) ||
              (tail==4 && !memcmp(text+at,"ions",4)) ||
              (tail==5 && !memcmp(text+at,"ionen",5)));
        int adjective=tail>=5 && !memcmp(text+at,"ion",3) &&
            ((!memcmp(text+at+3,"al",2)) ||
             (tail>=6 && !memcmp(text+at+3,"ell",3)) ||
             (text[at+3]==0xe4 && text[at+4]=='r'));
        if (adjective) {
            size_t stem=text[at+3]=='e'?6:5;
            size_t remaining=tail-stem;
            if (remaining>2 || (remaining && text[at+stem]!='e') ||
                (remaining==2 && !strchr("nmrs",text[at+stem+1]))) adjective=0;
        }
        if (!nominal && !adjective) continue;
        ion=j;v[j].end=v[j+1].end;v[j].kind=O;
        ion_stress=ion+(adjective?1:0);
        v[j].explicit_length=1;v[j].short_marker=0;
        memmove(v+j+1,v+j+2,(count-j-2)*sizeof *v);--count;
        break;
    }
    size_t stress = 0;
    /* Unaccented verbal prefixes can precede nouns, participles and
     * adjectives as well as infinitives. The conservative grammar only
     * accepts independently checked -en and stop+t bases; the opt-in path estimates
     * the same common prefix pattern for other complete word shapes. */
    if (count > 1 && word_length >= 5 && v[0].kind == E &&
        v[0].end == 2 &&
        ((!memcmp(text, "be", 2)) || (!memcmp(text, "ge", 2)) ||
         (word_length > 5 && !memcmp(text, "ver", 3)))) stress = 1;
    /* In er- the first vowel ends at byte one, unlike be-/ge-/ver-.
     * Only infer a separate weak prefix for a verb-shaped word.  A short
     * unprefixed stem such as ern-ten or ers-ten is not evidence of one:
     * the putative n+t/st onset cannot supply the same er- boundary. */
    if (count > 1 && !memcmp(text, "er", 2) && v[0].at == 0 &&
        v[0].end == 1 &&
        (text[word_length - 1] == 't' ||
         (word_length >= 2 && !memcmp(text + word_length - 2, "en", 2)) ||
         (count > 2 && word_length >= 3 &&
          !memcmp(text + word_length - 3, "ern", 3)) ||
         (count > 2 && text[word_length - 1] == 'e')) &&
        !((count == 2 &&
           ((text[2] == 'n' && text[3] == 't') ||
            ((text[2] == 's' && text[3] == 't') &&
             !marked_er_st_root(text, word_length)) ||
            text[2] == 'd')) ||
          (text[2] == 'd' && text[3] == 'i' && text[4] == 'g')))
        stress = 1;
    if (count > 2 && word_length >= 5 &&
        !memcmp(text + word_length - 5, "ieren", 5)) stress = count - 2;
    /* Ent- is generally an unstressed verbal prefix; require a following
     * consonant and a second nucleus to avoid guessing at bare syllables. */
    if (count > 1 && word_length >= 6 &&
        !memcmp(text, "ent", 3) && v[1].at >= 4)
        stress = 1;
    /* Productive loan endings keep their final stressed vowel. This is a
     * pronunciation guess, used only after the conservative grammar fails. */
    if (count > 1 && word_length >= 2 &&
        ((!memcmp(text + word_length - 2, "ee", 2)) ||
         (!memcmp(text + word_length - 2, "iz", 2))))
        stress = count - 1;
    /* Productive German -ier stress remains on the long /i/ when suffixes
     * follow it (zitiert, zitierter, probieren). */
    for (size_t i = 1; i < count; ++i)
        if (v[i].end == v[i].at + 2 &&
            v[i].at + 2 < word_length &&
            text[v[i].at] == 'i' && text[v[i].at + 1] == 'e' &&
            text[v[i].at + 2] == 'r') stress = i;
    /* Adverbial particles -bei/-zu normally bear the primary stress in
     * forms such as herbei, nebenbei, hinzu and dazu. */
    if (count > 1 && word_length >= 4 &&
        ((word_length >= 5 &&
          !memcmp(text + word_length - 3, "bei", 3)) ||
         !memcmp(text + word_length - 2, "zu", 2)))
        stress = count - 1;

    if (ion!=SIZE_MAX) stress=ion_stress;

    /* Written oi has the reference's incomplete stress-class marker.
     * Before a weak ending its single root nucleus therefore remains
     * unaccented. A following full root vowel and adjacent vowel hiatus
     * do not take this path. This is distinct from eu/aeu's ordinary oj. */
    int missing_oi_accent=0;
    if(stress==0 && count>1 && v[0].kind==DIP_OJ &&
       text[v[0].at]=='o' && text[v[0].at+1]=='i')
        for(size_t at=v[0].end+1;at<=v[1].at;++at)
            if(weak_morph_tail(text+at,word_length-at))missing_oi_accent=1;

    size_t used = 0;
    if (!append((char *)candidate, &used, "`[")) return 0;
    size_t onset_start = 0;
    size_t reduce_at[32]={0}, stress_at[32]={0};
    int previous_double = 0;
    int prefix_sp_st = 0;
    for (size_t i = 0; i < count; ++i) {
        size_t next_at = i + 1 < count ? v[i + 1].at : word_length;
        size_t gap_start = v[i].end, gap_length = next_at - gap_start;
        int doubled = 0;
        /* x represents two consonants. Its k closes the preceding nucleus
         * even when another coda consonant precedes it; s (or st) starts
         * the next syllable. A spelling boundary cannot bisect that letter. */
        size_t x_at=next_at && text[next_at-1]=='x'?next_at-1:
            next_at>=2 && text[next_at-2]=='x' && text[next_at-1]=='t'?next_at-2:SIZE_MAX;
        const int split_x=i+1<count && x_at>=gap_start && x_at<next_at;
        size_t split = i + 1 < count ?
            approximate_gap_split(text, word_length, gap_start, next_at,
                                  &doubled) : gap_length;
        if (ion!=SIZE_MAX && gap_length>2 &&
            (text[next_at-1]=='l' || text[next_at-1]=='r') &&
            strchr("bpkfg",text[next_at-2])) split=gap_length-2;
        if (i == 0 && stress == 1 && text[0] == 'e' &&
            text[1] == 'r' && v[0].end == 1 &&
            gap_start == 1 && gap_length && text[gap_start] == 'r') {
            /* A productive er- ends after r even when the following stem
             * starts with a consonant cluster or a second written r. */
            split = 1;
            doubled = 0;
        }
        if (i == 0 && stress == 1 && count > 1) {
            size_t prefix_coda = SIZE_MAX;
            if (v[0].at == 1 && v[0].end == 2 &&
                (!memcmp(text, "be", 2) || !memcmp(text, "ge", 2)))
                prefix_coda = 0;
            else if (v[0].at == 1 && v[0].end == 2 &&
                     !voiced_v && !memcmp(text, "ver", 3))
                prefix_coda = 1;
            else if (v[0].at == 0 && v[0].end == 1 &&
                     !memcmp(text, "er", 2))
                prefix_coda = 1;
            else if (v[0].at == 0 && v[0].end == 1 &&
                     !memcmp(text, "ent", 3))
                prefix_coda = 2;
            if (prefix_coda != SIZE_MAX && gap_length >= prefix_coda) {
                split = prefix_coda;
                doubled = 0;
                prefix_sp_st = german_sp_st_onset(text + gap_start + prefix_coda,
                                                gap_length - prefix_coda);
            }
        }
        if (split_x) split=x_at-gap_start;
        size_t next_onset = gap_start + split;
        size_t coda_length = split;
        const size_t syllabic=terminal_complete && i+1==count?
            terminal_sonorant(text+gap_start,gap_length):0;
        int syllabic_long=0;
        stress_at[i]=used+1;
        if (!append((char *)candidate, &used, i == stress && !missing_oi_accent ? ".1" :
                    ion_stress>=2 && ion!=SIZE_MAX && i==0 ? ".2" : ".0"))
            return 0;
        if (previous_double==1 && onset_start + 1 < v[i].at) ++onset_start;
        if (i==ion) {
            size_t end=v[i].at-1;
            if (end>onset_start && !consonants_impl(text+onset_start,
                end-onset_start,0,0,0,consonant_mode,(char *)candidate,&used)) return 0;
            const char *glide=text[end]=='t'?"Tj":text[end]=='g'?"gj":
                end>0 && text[end-1]=='s'?"sj":"zj";
            if (!append((char *)candidate,&used,glide)) return 0;
        } else if (ion!=SIZE_MAX && v[i].at==onset_start+1 &&
                   text[onset_start]=='v') {
            if (!append((char *)candidate,&used,"v")) return 0;
        } else if (i && text[onset_start]=='x' &&
                   (v[i].at-onset_start==1 ||
                    (v[i].at-onset_start==2 && text[onset_start+1]=='t'))) {
            if (!append((char *)candidate,&used,v[i].at-onset_start==1?"s":"st")) return 0;
        } else if (v[i].at>onset_start && text[v[i].at-1]=='c' &&
                   (text[v[i].at]=='e'||text[v[i].at]=='i'||text[v[i].at]=='y')) {
            if (!consonants_impl(text+onset_start,v[i].at-onset_start-1,
                i==0,0,0,consonant_mode,(char *)candidate,&used) ||
                !append((char *)candidate,&used,"T")) return 0;
        } else if (i && v[i].kind==E && !v[i].explicit_length && i+1==count &&
                   onset_start && (text[onset_start-1]=='k' ||
                    (onset_start>=2 && !memcmp(text+onset_start-2,"ch",2) &&
                     !reloq_german_morphology_is_root_boundary(text,word_length,onset_start))) &&
                   onset_start+1==v[i].at && text[onset_start]=='s') {
            if (!append((char *)candidate,&used,"s")) return 0;
        } else if (previous_double && text[onset_start] == 's' &&
            onset_start + 1 == v[i].at) {
            if (!append((char *)candidate, &used, "s")) return 0;
        } else if (i && onset_start + 1 < v[i].at &&
                   text[onset_start] == 's' &&
                   (text[onset_start + 1] == 't' ||
                    text[onset_start + 1] == 'p')) {
            /* Internal st/sp retains /s/; the shared mapper's standalone
             * intervocalic s would otherwise turn it into voiced /z/. */
            if (!append((char *)candidate, &used,
                        i == 1 && prefix_sp_st ? "S" : "s") ||
                !consonants_impl(text + onset_start + 1,
                            v[i].at - onset_start - 1, 0, 0,
                            i && (v[i - 1].kind == A ||
                                  v[i - 1].kind == O ||
                                  v[i - 1].kind == U ||
                                  v[i - 1].kind == DIP_AW),
                            consonant_mode, (char *)candidate, &used)) return 0;
        } else if (!consonants_impl(text + onset_start, v[i].at - onset_start,
                               i == 0, 0,
                               i && onset_start==v[i-1].end && (v[i - 1].kind == A ||
                                     v[i - 1].kind == O ||
                                     v[i - 1].kind == U ||
                                     v[i - 1].kind == DIP_AW),
                               consonant_mode, (char *)candidate, &used))
            return 0;
        previous_double = doubled;

        int weak = i != stress && v[i].kind == E && !v[i].explicit_length &&
            (ion==SIZE_MAX || i>ion);
        int vocalic_r = weak && coda_length >= 1 &&
            text[gap_start] == 'r' &&
            (coda_length == 1 ||
             (i == count - 1 && coda_length == 2 &&
              (text[gap_start + 1] == 'n' ||
               text[gap_start + 1] == 't')));
        if (vocalic_r) {
            /* Initial unstressed er-/ver- keeps the reference's /Er/;
             * internal and final unaccented -er is usually syllabic /R/. */
            if (!append((char *)candidate, &used,
                        i == 0 && stress == 1 ? "Er" : "R")) return 0;
            ++gap_start;
            --coda_length;
        } else if (weak && ((i>0 && word_length-v[i].at==7 &&
                            !memcmp(text+v[i].at,"entlich",7)) ||
                           (i == count - 1 &&
                            (gap_length == 0 ||
                             (gap_length == 2 && !memcmp(text+gap_start,"nd",2)) ||
                             (gap_length == 1 &&
                              (strchr("nmsl", text[gap_start]) ||
                               /* Weak -et in regular verb forms keeps
                                * schwa: wartet, meldet, zeichnet, atmet.
                                * A final -et by spelling alone is not
                                * enough: loan nouns such as Internet keep
                                * a full vowel in their final syllable. */
                               (text[gap_start] == 't' && v[i].at > 0 &&
                                (strchr("dt", text[v[i].at - 1]) ||
                                 (count == 2 &&
                                  strchr("nm", text[v[i].at - 1])))))))) ||
                           (i + 1 < count &&
                            ((coda_length == 0 &&
                              (i > 0 ||
                               (v[0].end == 2 &&
                                (!memcmp(text, "be", 2) ||
                                 !memcmp(text, "ge", 2))))) ||
                             (coda_length == 1 &&
                              text[gap_start] == 'n'))))) {
            if (!append((char *)candidate, &used, "@")) return 0;
        } else {
            int short_vowel = !v[i].explicit_length &&
                (v[i].short_marker || doubled || coda_length >= 2 ||
                 ((v[i].kind==E || v[i].kind==I || v[i].kind==O ||
                   v[i].kind==UML_A || v[i].kind==UML_O) && gap_length>=2 &&
                  !memcmp(text+gap_start,"ch",2)) ||
                 (i + 1 < count && coda_length == 1 &&
                  (strchr("nkpts", text[gap_start]) ||
                   (strchr("lmr", text[gap_start]) &&
                    /* Productive vor- retains /o:/ before its /r/ in
                     * vor+zug, be+vor+zug etc.; this is not the closed
                     * short-vowel root pattern of Wort/Morgen. */
                    !(v[i].kind == O && text[gap_start] == 'r' &&
                      v[i].at > 0 && text[v[i].at - 1] == 'v')))) ||
                 (i == count - 1 && count > 1 && coda_length == 1 &&
                  strchr("tkpm", text[gap_start])) ||
                 (i == count - 1 && coda_length == 1 &&
                  (text[gap_start] == 's' ||
                   (v[i].kind != E &&
                    strchr("mn", text[gap_start])))) ||
                 (i == count - 1 && word_length >= 4 &&
                  text[word_length - 2] == 'i' &&
                  text[word_length - 1] == 'g') ||
                 (i == 0 && i + 1 < count && v[i].kind == A &&
                  v[i].at < 3 && gap_length == 2 &&
                  !memcmp(text + gap_start, "ch", 2) &&
                  v[i + 1].kind == E) ||
                 (i + 1 < count && gap_length >= 2 &&
                  !memcmp(text + next_at - 2, "st", 2)));
            /* Quantity and syllable ownership are separate decisions:
             * x still closes a vowel before its s moves to the next onset;
             * gm/kw retain short quantity despite being legal onsets. */
            if (!v[i].explicit_length &&
                ((gap_length && text[gap_start]=='x') ||
                 (gap_length==2 && (!memcmp(text+gap_start,"gm",2) ||
                                    !memcmp(text+gap_start,"kw",2))))) short_vowel=1;
            if (!v[i].explicit_length && v[i].kind<=UML_U &&
                (i+1<count || (gap_length>=2 && text[gap_start+1]=='h' &&
                 strchr("ptg",text[gap_start])))) {
                int quantity=reloq_german_g2p_cluster_quantity(text+gap_start,gap_length);
                if (gap_length==2 && text[gap_start+1]=='n' &&
                    strchr("bdgkptf",text[gap_start]) &&
                    weak_morph_tail(text+next_at,word_length-next_at)) quantity=1;
                if(gap_length==2 && strchr("lnm",text[gap_start+1]) &&
                   strchr("wv",text[gap_start]))quantity=0;
                if(gap_length==2 && text[gap_start]=='s' &&
                   strchr("ln",text[gap_start+1]) &&
                   weak_morph_tail(text+next_at,word_length-next_at))quantity=1;
                if (((gap_length==4 && !memcmp(text+gap_start,"tsch",4)) ||
                     (gap_length==3 && !memcmp(text+gap_start,"tch",3))) &&
                    weak_morph_tail(text+next_at,word_length-next_at)) quantity=0;
                if (quantity>=0) short_vowel=!quantity;
            }
            if (v[i].explicit_length==2)
                short_vowel=gap_length>=2 && !is_vowel(text[gap_start]) &&
                    !is_vowel(text[gap_start+1]);
            if(syllabic && !v[i].explicit_length) {
                const unsigned char *cluster=text+gap_start;
                short_vowel=!(syllabic==1 && strchr("bdgptkfs",cluster[0]) &&
                    (cluster[1]=='l' || (cluster[1]=='n' &&
                     !(gap_length>2 && cluster[2]=='t'))));
                if(gap_length==3 && cluster[0]=='s' && cluster[2]=='t')short_vowel=1;
            }
            syllabic_long=!short_vowel;
            const char *phone = vowel_spelling(v[i].kind, !short_vowel);
            /* Final unstressed ay/ey contract to /e/ and /i/ respectively.
             * An accented pair keeps /aj/, including initial and medial
             * positions; the letters are consumed as one nucleus. */
            if(i!=stress && v[i].end==word_length &&
               v[i].end==v[i].at+2 && text[v[i].at+1]=='y' &&
               (text[v[i].at]=='a' || text[v[i].at]=='e'))
                phone=text[v[i].at]=='a'?"e":"i";
            if (v[i].kind==A && !v[i].explicit_length && !short_vowel) {
                /* -bar retains its own long nucleus through inflection.
                 * Other unmarked a nuclei are reduced only after compound
                 * accents have been resolved from the unreduced reading. */
                const unsigned char *tail=text+v[i].at+1;
                int bar=v[i].at && text[v[i].at-1]=='b' &&
                    (!strcmp((const char *)tail,"r") || !strcmp((const char *)tail,"re") ||
                     !strcmp((const char *)tail,"ren") || !strcmp((const char *)tail,"rem") ||
                     !strcmp((const char *)tail,"rer") || !strcmp((const char *)tail,"res") ||
                     !strcmp((const char *)tail,"rkeit") || !strcmp((const char *)tail,"rkeiten"));
                if (!bar && (i>stress || ion!=SIZE_MAX)) reduce_at[i]=used;
            }
            if (!phone || !append((char *)candidate, &used, phone)) return 0;
        }
        /* Phonetic syllable boundaries are not morphological word ends.
         * The original keeps a voiced root coda before root-final l/n
         * when a stripped weak ending follows that sonorant. */
        const int keep_final_voice=i+1<count && coda_length+1==gap_length &&
            strchr("ln",text[gap_start+coda_length]) &&
            weak_morph_tail(text+next_at,word_length-next_at);
        if(syllabic) {
            const unsigned char *cluster=text+gap_start;
            if(!append((char *)candidate,&used,".0"))return 0;
            if(syllabic==1 && cluster[0]=='s') {
                if(!append((char *)candidate,&used,syllabic_long?"z":"s"))return 0;
            } else if(!consonants_impl(cluster,syllabic,0,0,
                v[i].kind==A || v[i].kind==O || v[i].kind==U || v[i].kind==DIP_AW,
                1,(char *)candidate,&used))return 0;
            if(!append((char *)candidate,&used,"@") ||
               !consonants_impl(cluster+syllabic,gap_length-syllabic,0,1,0,consonant_mode,
                    (char *)candidate,&used))return 0;
        } else if (split_x) {
            if (coda_length && !approximate_coda(text+gap_start,coda_length,0,
                v[i].kind==A || v[i].kind==O || v[i].kind==U || v[i].kind==DIP_AW,0,
                (char *)candidate,&used)) return 0;
            if (!append((char *)candidate,&used,"k")) return 0;
        } else if (i == count - 1 && word_length >= 4 &&
            text[word_length - 2] == 'i' &&
            text[word_length - 1] == 'g' &&
            coda_length == 1 && text[gap_start] == 'g') {
            /* Productive final -ig has voiceless /IX/ in standard German.
             * Strict verified -ig derivations were returned above. */
            if (!append((char *)candidate, &used, "X")) return 0;
        } else if (coda_length == 2 && !memcmp(text+gap_start,"ch",2) &&
                   i+1<count && text[next_onset]=='s' &&
                   !reloq_german_morphology_is_root_boundary(text,word_length,next_onset)) {
            /* Internal chs maps to /ks/ before syllabification. The /k/
             * closes this nucleus; the written s belongs to the next one. */
            if (!append((char *)candidate,&used,"k")) return 0;
        } else if (coda_length == 3 && !memcmp(text + gap_start, "chs", 3)) {
            if (!append((char *)candidate, &used, "ks")) return 0;
        } else if (coda_length >= 2 &&
                   text[gap_start] == text[gap_start + 1]) {
            /* Written geminates mark a short vowel but one consonant sound.
             * Other coda consonants remain, e.g. kommt -> /kOmt/. */
            if (!approximate_coda(text + gap_start + 1, coda_length - 1,
                                   i + 1 < count &&
                                   gap_start + coda_length < word_length ?
                                   text[gap_start + coda_length] : 0,
                                   v[i].kind == A || v[i].kind == O ||
                                   v[i].kind == U || v[i].kind == DIP_AW,
                                   keep_final_voice && text[gap_start]!='s',
                                   (char *)candidate, &used)) return 0;
        } else if (coda_length &&
                   !approximate_coda(text + gap_start, coda_length,
                                      i + 1 < count &&
                                      gap_start + coda_length < word_length ?
                                      text[gap_start + coda_length] : 0,
                                      v[i].kind == A || v[i].kind == O ||
                                      v[i].kind == U || v[i].kind == DIP_AW,keep_final_voice,
                                      (char *)candidate, &used)) return 0;
        if (i + 1 < count) onset_start = next_onset;
    }
    if (!append((char *)candidate, &used, "]")) return 0;
    /* A German compound often preserves the first component's main accent
     * and the next component's lexical accent as .2. Infer this only when
     * the *entire* previously generated phone sequence has a unique exact
     * syllable-boundary match from two independent word analyses, at least
     * one conservative.  If the first is itself approximate, the second
     * must be a single syllable.  Ordinary weak -lich/-ung endings are not
     * compounds.  Component analysis disables this pass, bounding recursion.
     * No individual word or observed reference pronunciation is consulted. */
    if (compound_stress && word_length >= 7 &&
        !(word_length >= 4 &&
          !memcmp(text + word_length - 4, "lich", 4)) &&
        !(word_length >= 3 &&
          !memcmp(text + word_length - 3, "ung", 3)) &&
        !memchr(candidate + 2, '2', used - 2)) {
        size_t match_at = SIZE_MAX;
        int ambiguous = 0;
        for (size_t split = 3; split + 3 <= word_length; ++split) {
            unsigned char first[256], second[256];
            size_t left = 0, right = 0;
            int first_checked = reloq_german_g2p_word(
                text, split, first, sizeof first, &left) == 1;
            unsigned char strict_second[256];
            size_t strict_second_length = 0;
            int second_checked = reloq_german_g2p_word(
                text + split, word_length - split,
                strict_second, sizeof strict_second,
                &strict_second_length) == 1;
            if (!first_checked && !second_checked) continue;
            if (!first_checked &&
                best_effort_impl(text, split, first, sizeof first,
                                 &left, NULL, 0, 1) != 1) continue;
            if (left < 5 || memcmp(first, "`[.1", 4) ||
                memchr(first + 4, '2', left - 4)) continue;
            /* A weak suffix or an opaque longer second stem is too easily
             * mistaken for a compound.  The one-syllable component class
             * admits independent open/closed word readings such as -bahn
             * and -raum, while preserving a unique exact boundary. */
            if (word_length - split < 4 && !second_checked) continue;
            if (best_effort_impl(text + split, word_length - split,
                                 second, sizeof second, &right, NULL, 0, 1) != 1 ||
                right < 5 || memcmp(second, "`[.1", 4) ||
                memchr(second + 4, '2', right - 4) ||
                (!first_checked &&
                 memchr(second + 4, '.', right - 5))) continue;
            if (left - 1 + right - 2 != used ||
                memcmp(candidate, first, left - 1) ||
                candidate[left - 1] != '.' ||
                candidate[left] != '0' ||
                memcmp(candidate + left + 1, second + 4, right - 4))
                continue;
            if (match_at != SIZE_MAX) { ambiguous = 1; break; }
            match_at = left;
        }
        if (!ambiguous && match_at != SIZE_MAX)
            candidate[match_at] = '2';
    }
    if (compound_stress)
        compose_checked_multisyllabic_head(text, word_length,
                                            candidate, used);
    for (size_t i=0;i<count;++i)
        if (reduce_at[i] && candidate[stress_at[i]]=='0' && candidate[reduce_at[i]]=='a')
            candidate[reduce_at[i]]='A';
    if (!output || capacity <= used) {
        *byte_length = used;
        return -2;
    }
    memcpy(output, candidate, used);
    output[used] = 0;
    *byte_length = used;
    if (approximation_used) *approximation_used = 1;
    return 1;
}

int reloq_german_g2p_open_stem_best_effort(
    const unsigned char *word,size_t word_length,
    unsigned char *output,size_t capacity,size_t *byte_length,
    int *approximation_used)
{
    return best_effort_impl(word,word_length,output,capacity,byte_length,
        approximation_used,1,0);
}

int reloq_german_g2p_word_best_effort(
    const unsigned char *word, size_t word_length,
    unsigned char *output, size_t capacity, size_t *byte_length,
    int *approximation_used)
{
    int used_fallback = 0;
    int status = best_effort_impl(word, word_length, output, capacity,
                                  byte_length, &used_fallback, 1, 1);
    if (status == 1 && used_fallback && output && word_length > 3 &&
        *byte_length >= 6 &&
        output[*byte_length - 6] == '.' &&
        output[*byte_length - 5] >= '0' &&
        output[*byte_length - 5] <= '2' &&
        !memcmp(output + *byte_length - 4, "lOs]", 4)) {
        /* Productive German adjectival -los has a long final /o/. Only
         * change the terminal, separately syllabified fallback suffix;
         * conservative and lexical readings retain precedence. An opaque
         * word ending in these letters without a whole /lOs/ syllable
         * remains untouched. */
        unsigned char suffix[3];
        int written_los = 1;
        for (size_t i = 0; i < 3; ++i)
            if (!normalize(word[word_length - 3 + i], &suffix[i]))
                written_los = 0;
        if (written_los && !memcmp(suffix, "los", 3))
            output[*byte_length - 3] = 'o';
    }
    if (status == 1 && used_fallback && word_length >= 6 && output &&
        *byte_length >= 10) {
        /* The abstract suffix is a weak derivation, even if the fallback
         * scanner or constituent composer happened to infer a noun root
         * with an independent accent at exactly this boundary. Preserve
         * earlier compound accents and all phones: only demote the suffix. */
        unsigned char normalized[128];
        if (word_length < sizeof normalized) {
            int valid = 1;
            for (size_t i = 0; i < word_length; ++i)
                if (!normalize(word[i], &normalized[i])) { valid = 0; break; }
            if (valid &&
                (!memcmp(normalized + word_length - 4, "heit", 4) ||
                 !memcmp(normalized + word_length - 4, "keit", 4))) {
                const char *ending = normalized[word_length - 4] == 'h' ?
                    ".2h'aj't]" : ".2k'aj't]";
                size_t n = strlen(ending);
                if (*byte_length >= n &&
                    !memcmp(output + *byte_length - n, ending, n))
                    output[*byte_length - n + 1] = '0';
            }
            if (valid) {
                /* Productive unstressed -sam has long /a/, both as an
                 * adjective and before nominalizing -keit. A plain closed
                 * syllable scan incorrectly shortens it to /A/. The exact
                 * terminal phone shape guards against a false root split. */
                const char *tail = NULL;
                if (!memcmp(normalized + word_length - 3, "sam", 3))
                    tail = ".0zAm]";
                else if (word_length >= 7 &&
                         !memcmp(normalized + word_length - 7,
                                 "samkeit", 7))
                    tail = ".0zAm.0k'aj't]";
                if (tail) {
                    size_t n = strlen(tail);
                    if (*byte_length >= n &&
                        !memcmp(output + *byte_length - n, tail, n))
                        output[*byte_length - n + 3] = 'a';
                }
            }
            if (valid && word_length >= 8 &&
                !memcmp(normalized + word_length - 5, "ieren", 5) &&
                *byte_length >= 12 && !memcmp(output, "`[.0", 4) &&
                !memcmp(output + *byte_length - 6, ".0r@n]", 6)) {
                /* Long -ieren verbs retain a secondary accent at their
                 * beginning before the principal -ier- accent. Apply only
                 * to the four-or-more-syllable generated shape: shorter
                 * forms such as probieren and diktieren are weak initially,
                 * and a verified lexical reading never enters this path.
                 * This changes stress, not phones or syllable boundaries. */
                unsigned syllables = 0, primaries = 0;
                for (size_t i = 2; i + 1 < *byte_length; ++i) {
                    if (output[i] == '.' && output[i + 1] >= '0' &&
                        output[i + 1] <= '2') {
                        ++syllables;
                        primaries += output[i + 1] == '1';
                    }
                }
                if (syllables >= 4 && primaries == 1)
                    output[3] = '2';
            }
        }
    }
    if (status == 1 && approximation_used) *approximation_used = used_fallback;
    return status;
}
