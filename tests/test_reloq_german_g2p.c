#include "reloq_german_g2p.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void same(const unsigned char *word, const char *expected)
{
    unsigned char out[128] = {0};
    size_t length = 999;
    int status = reloq_german_g2p_word(word, strlen((const char *)word),
                                       out, sizeof out, &length);
    if (status != 1)
        fprintf(stderr, "Unexpectedly uncovered: %s (%d)\n", word, status);
    assert(status == 1);
    assert(length == strlen(expected));
    if (strcmp((const char *)out, expected) != 0)
        fprintf(stderr, "Mismatch %s: C=%s expected=%s\n", word, out, expected);
    assert(strcmp((const char *)out, expected) == 0);
}

static void uncovered(const unsigned char *word)
{
    unsigned char out[32] = "unchanged";
    size_t length = 79;
    int status = reloq_german_g2p_word(word, strlen((const char *)word),
                                        out, sizeof out, &length);
    if (status) fprintf(stderr, "Unexpectedly covered: %s (%d)\n", word, status);
    assert(status == 0);
    assert(strcmp((const char *)out, "unchanged") == 0);
    assert(length == 79);
}

static void approximation(const unsigned char *word, const char *expected,
                          int expected_flag)
{
    unsigned char out[256] = {0};
    size_t length = 999;
    int estimated = -1;
    assert(reloq_german_g2p_word_best_effort(
               word, strlen((const char *)word), out, sizeof out,
               &length, &estimated) == 1);
    if (length != strlen(expected))
        fprintf(stderr, "Approx mismatch %s: C=%s expected=%s\n",
                word, out, expected);
    assert(length == strlen(expected));
    if (strcmp((const char *)out, expected) != 0)
        fprintf(stderr, "Approx mismatch %s: C=%s expected=%s\n", word, out, expected);
    assert(strcmp((const char *)out, expected) == 0);
    if (estimated != expected_flag)
        fprintf(stderr, "Approx flag mismatch %s: %d expected %d\n",
                word, estimated, expected_flag);
    assert(estimated == expected_flag);
}

static void avoid_false_prefix(const unsigned char *word,
                               const char *bad_syllable)
{
    unsigned char out[256] = {0};
    size_t length = 0;
    int estimated = -1;
    assert(reloq_german_g2p_word_best_effort(
               word, strlen((const char *)word), out, sizeof out,
               &length, &estimated) == 1);
    assert(length == strlen((const char *)out));
    assert(strstr((const char *)out, bad_syllable) == NULL);
}

int main(void)
{
    /* Expected strings are independently observed Apple ECI annotations for
     * words; the rules cover spellings shared with unseen combinations. */
    same((const unsigned char *)"Maus", "`[.1m'aw's]");
    same((const unsigned char *)"Schwein", "`[.1Sv'aj'n]");
    same((const unsigned char *)"Brause", "`[.1br'aw'.0z@]");
    same((const unsigned char *)"Pfeil", "`[.1P'aj'l]");
    same((const unsigned char *)"Kaufen", "`[.1k'aw'.0f@n]");
    same((const unsigned char *)"Mieten", "`[.1mi.0t@n]");
    same((const unsigned char *)"Dicken", "`[.1dI.0k@n]");
    same((const unsigned char *)"Quelle", "`[.1kvE.0l@]");
    same((const unsigned char *)"Lafen", "`[.1la.0f@n]");
    same((const unsigned char *)"Laffen", "`[.1lA.0f@n]");
    same((const unsigned char *)"Baur", "`[.1b'aw'.0R]");
    same((const unsigned char *)"Biren", "`[.0bi.1ren]");
    same((const unsigned char *)"Schnee", "`[.1Sne]");
    same((const unsigned char *)"B\xe4ume", "`[.1b'oj'.0m@]");
    same((const unsigned char *)"B\xe4ren", "`[.1b'E:'.0r@n]");
    same((const unsigned char *)"B\xf6tten", "`[.1b'OE'.0t@n]");
    same((const unsigned char *)"H\xe4user", "`[.1h'oj'.0zR]");
    same((const unsigned char *)"Pass", "`[.1pAs]");
    same((const unsigned char *)"Tiss", "`[.1tIs]");
    same((const unsigned char *)"Kuss", "`[.1kUs]");

    /* Unstressed prefixes before a separately analyzed -en infinitive;
     * complete public ECI word annotations pin both phones and stress. */
    same((const unsigned char *)"Bepacken", "`[.0b@.1pA.0k@n]");
    same((const unsigned char *)"Gefallen", "`[.0g@.1fA.0l@n]");
    same((const unsigned char *)"Verlesen", "`[.0fEr.1le.0z@n]");
    same((const unsigned char *)"Erkennen", "`[.0Er.1kE.0n@n]");
    same((const unsigned char *)"Gewesen", "`[.0g@.1ve.0z@n]");

    /* Regular -te/-ten inherits the exact stressed vowel and consonant
     * from an independently covered -en infinitive. These expected SPR
     * groups were also obtained through Apple's public word callback. */
    same((const unsigned char *)"Packte", "`[.1pAk.0t@]");
    same((const unsigned char *)"Packten", "`[.1pAk.0t@n]");
    same((const unsigned char *)"Hackte", "`[.1hAk.0t@]");
    same((const unsigned char *)"Hackten", "`[.1hAk.0t@n]");
    same((const unsigned char *)"Pickte", "`[.1pIk.0t@]");
    same((const unsigned char *)"Tippten", "`[.1tIp.0t@n]");
    same((const unsigned char *)"Stoppte", "`[.1StOp.0t@]");
    same((const unsigned char *)"F\xfcllten", "`[.1fYl.0t@n]");
    same((const unsigned char *)"Rammte", "`[.1rAm.0t@]");
    same((const unsigned char *)"G\xf6nnten", "`[.1g'OE'n.0t@n]");
    same((const unsigned char *)"Setzte", "`[.1zET.0t@]");
    same((const unsigned char *)"Flitzten", "`[.1flIT.0t@n]");
    /* A productive final voiced stop devoices against all four t endings.
     * The base's covered -en form fixes vowel length and onset. */
    same((const unsigned char *)"Liegt", "`[.1likt]");
    same((const unsigned char *)"Liegst", "`[.1likst]");
    same((const unsigned char *)"Liegte", "`[.1lik.0t@]");
    same((const unsigned char *)"Liegten", "`[.1lik.0t@n]");
    same((const unsigned char *)"Siebt", "`[.1zipt]");
    same((const unsigned char *)"Siebten", "`[.1zip.0t@n]");
    same((const unsigned char *)"Leidt", "`[.1l'aj't]");
    same((const unsigned char *)"Leidst", "`[.1l'aj'tst]");
    same((const unsigned char *)"Leidten", "`[.1l'aj'.0t@n]");
    same((const unsigned char *)"Sagte", "`[.1zak.0t@]");

    /* Productive un- keeps the base's phones and weak syllables while
     * promoting the negative prefix to primary stress. */
    same((const unsigned char *)"Unfrei", "`[.1Un.2fr'aj']");
    same((const unsigned char *)"Unfreiheit",
         "`[.1Un.2fr'aj'.0h'aj't]");
    same((const unsigned char *)"Unlesung", "`[.1Un.2le.0zUG]");
    same((const unsigned char *)"Unsauberkeit",
         "`[.1Un.2z'aw'.0bR.0k'aj't]");
    same((const unsigned char *)"Unsch\xf6nem",
         "`[.1Un.2S'oe'.0n@m]");
    same((const unsigned char *)"Unfreies", "`[.1Un.2fr'aj'.0@s]");
    same((const unsigned char *)"Unheil", "`[.1Un.2h'aj'l]");
    same((const unsigned char *)"Unfall", "`[.1Un.2fAl]");

    /* -ig inherits the base's -en syllable boundary and lenition: the
     * independent -en result's final @n is replaced by IX. */
    same((const unsigned char *)"Heilig", "`[.1h'aj'.0lIX]");
    same((const unsigned char *)"Steinig", "`[.1St'aj'.0nIX]");
    same((const unsigned char *)"Eisig", "`[.1'aj'.0zIX]");
    same((const unsigned char *)"Einig", "`[.1'aj'.0nIX]");
    same((const unsigned char *)"Wenig", "`[.1ve.0nIX]");
    same((const unsigned char *)"Leibig", "`[.1l'aj'.0bIX]");
    same((const unsigned char *)"Gl\xe4ubig", "`[.1gl'oj'.0bIX]");
    same((const unsigned char *)"Saumig", "`[.1z'aw'.0mIX]");
    same((const unsigned char *)"Feurig", "`[.1f'oj'.0rIX]");
    same((const unsigned char *)"Schmutzig", "`[.1SmU.0TIX]");
    same((const unsigned char *)"Wellig", "`[.1vE.0lIX]");
    same((const unsigned char *)"Freudig", "`[.1fr'oj'.0dIX]");
    same((const unsigned char *)"Staubig", "`[.1St'aw'.0bIX]");
    same((const unsigned char *)"Unheilig",
         "`[.1Un.2h'aj'.0lIX]");
    same((const unsigned char *)"Unsteinig",
         "`[.1Un.2St'aj'.0nIX]");

    /* Productive weak -em/-es/-et and -ei-e/-ei-en families. */
    same((const unsigned char *)"Meinem", "`[.1m'aj'.0n@m]");
    same((const unsigned char *)"Deines", "`[.1d'aj'.0n@s]");
    same((const unsigned char *)"Dieses", "`[.1di.0z@s]");
    same((const unsigned char *)"Sch\xf6nem", "`[.1S'oe'.0n@m]");
    same((const unsigned char *)"Gutes", "`[.1gu.0t@s]");
    same((const unsigned char *)"Mietet", "`[.1mi.0t@t]");
    same((const unsigned char *)"Freie", "`[.1fr'aj'.0@]");
    same((const unsigned char *)"Freien", "`[.1fr'aj'.0@n]");
    same((const unsigned char *)"Freiem", "`[.1fr'aj'.0@m]");
    same((const unsigned char *)"Freies", "`[.1fr'aj'.0@s]");
    same((const unsigned char *)"Dies", "`[.1dis]");

    /* Diminutives derived from a covered, single stressed stem syllable. */
    same((const unsigned char *)"B\xe4umchen", "`[.1b'oj'm.0X@n]");
    same((const unsigned char *)"H\xe4uschen", "`[.1h'oj'.0S@n]");
    same((const unsigned char *)"F\xfcsschen", "`[.1fYs.0X@n]");
    same((const unsigned char *)"Steinchen", "`[.1St'aj'n.0X@n]");
    same((const unsigned char *)"H\xe4uslein", "`[.1h'oj's.0l'aj'n]");
    same((const unsigned char *)"Pfeillein", "`[.1P'aj'l.0l'aj'n]");

    /* Held-out public-reference -ung nouns from the transparent -en class.
     * The first syllable and its stress survive the nominal ending. */
    same((const unsigned char *)"Lesung", "`[.1le.0zUG]");
    same((const unsigned char *)"Messung", "`[.1mE.0sUG]");
    same((const unsigned char *)"Rettung", "`[.1rE.0tUG]");
    same((const unsigned char *)"Teilung", "`[.1t'aj'.0lUG]");
    same((const unsigned char *)"L\xf6sung", "`[.1l'oe'.0zUG]");
    same((const unsigned char *)"K\xfchlung", "`[.1ky.0lUG]");
    same((const unsigned char *)"Heizung", "`[.1h'aj'.0TUG]");

    /* Productive plural noun syllables, derived from the covered singular. */
    same((const unsigned char *)"Lesungen", "`[.1le.0zUG.0@n]");
    same((const unsigned char *)"Rettungen", "`[.1rE.0tUG.0@n]");
    same((const unsigned char *)"L\xf6sungen", "`[.1l'oe'.0zUG.0@n]");
    same((const unsigned char *)"K\xfchlungen", "`[.1ky.0lUG.0@n]");
    same((const unsigned char *)"Heizungen", "`[.1h'aj'.0TUG.0@n]");

    /* Productive abstract nouns add an unstressed syllable to a known base. */
    same((const unsigned char *)"Freiheit", "`[.1fr'aj'.0h'aj't]");
    same((const unsigned char *)"Reinheit", "`[.1r'aj'n.0h'aj't]");
    same((const unsigned char *)"Dummheit", "`[.1dUm.0h'aj't]");
    same((const unsigned char *)"Mehrheit", "`[.1mer.0h'aj't]");
    same((const unsigned char *)"Mattheit", "`[.1mAt.0h'aj't]");
    same((const unsigned char *)"Trockenheit", "`[.1trO.0k@n.0h'aj't]");
    same((const unsigned char *)"Gleichheit", "`[.1gl'aj'X.0h'aj't]");
    same((const unsigned char *)"Neuheit", "`[.1n'oj'.0h'aj't]");
    same((const unsigned char *)"Sauberkeit", "`[.1z'aw'.0bR.0k'aj't]");
    same((const unsigned char *)"Sauberheit", "`[.1z'aw'.0bR.0h'aj't]");
    same((const unsigned char *)"Freiheiten", "`[.1fr'aj'.0h'aj'.0t@n]");
    same((const unsigned char *)"Reinheiten", "`[.1r'aj'n.0h'aj'.0t@n]");
    same((const unsigned char *)"Dummheiten", "`[.1dUm.0h'aj'.0t@n]");
    same((const unsigned char *)"Trockenheiten",
         "`[.1trO.0k@n.0h'aj'.0t@n]");
    same((const unsigned char *)"Sauberkeiten",
         "`[.1z'aw'.0bR.0k'aj'.0t@n]");

    /* Productive weak -lich syllables from transparently covered stems.
     * Final /X/ becomes an onset before a following inflectional schwa. */
    same((const unsigned char *)"Steinlich",
         "`[.1St'aj'n.0lIX]");
    same((const unsigned char *)"Steinliche",
         "`[.1St'aj'n.0lI.0X@]");
    same((const unsigned char *)"Steinlichen",
         "`[.1St'aj'n.0lI.0X@n]");
    same((const unsigned char *)"Steinlichem",
         "`[.1St'aj'n.0lI.0X@m]");
    same((const unsigned char *)"Steinliches",
         "`[.1St'aj'n.0lI.0X@s]");
    same((const unsigned char *)"Steinlicher",
         "`[.1St'aj'n.0lI.0XR]");
    same((const unsigned char *)"Blitzlich",
         "`[.1blIT.0lIX]");
    same((const unsigned char *)"Zeitlich",
         "`[.1T'aj't.0lIX]");

    /* No invented phoneme for lexically unpredictable vowel length/stress. */
    uncovered((const unsigned char *)"Bach");
    uncovered((const unsigned char *)"Kuchen");
    uncovered((const unsigned char *)"Praktikum");
    uncovered((const unsigned char *)"Internet");
    uncovered((const unsigned char *)"mu\xdf");
    uncovered((const unsigned char *)"spien");
    uncovered((const unsigned char *)"Spleen");
    uncovered((const unsigned char *)"boom");
    uncovered((const unsigned char *)"fair");
    /* be- changes stress (Bebung); /ie/ and final /ng/ can resyllabify. */
    uncovered((const unsigned char *)"Bebung");
    uncovered((const unsigned char *)"Tielung");
    uncovered((const unsigned char *)"Laungung");
    uncovered((const unsigned char *)"Schiebung");
    uncovered((const unsigned char *)"Missheit");
    uncovered((const unsigned char *)"Zielheit");
    uncovered((const unsigned char *)"Lesungheit");
    uncovered((const unsigned char *)"Bebungen");
    uncovered((const unsigned char *)"Tielungen");
    uncovered((const unsigned char *)"Missheiten");
    uncovered((const unsigned char *)"freiet");
    uncovered((const unsigned char *)"haet");
    uncovered((const unsigned char *)"Frauchen");
    uncovered((const unsigned char *)"Beuchen");
    uncovered((const unsigned char *)"raallein");
    uncovered((const unsigned char *)"Unklar");
    uncovered((const unsigned char *)"Unsterblich");
    uncovered((const unsigned char *)"Farbig");
    uncovered((const unsigned char *)"Lustig");
    /* Other codas can shift across this suffix (and can even change stress).
     * The strict class keeps them outside its present evidence. */
    uncovered((const unsigned char *)"Hauslich");
    uncovered((const unsigned char *)"W\xfcrdig");
    uncovered((const unsigned char *)"Unser");
    uncovered((const unsigned char *)"Unsere");
    uncovered((const unsigned char *)"Unserem");
    uncovered((const unsigned char *)"Undneunz");
    uncovered((const unsigned char *)"Unauff");
    uncovered((const unsigned char *)"Ungutes");
    /* A prefix-like initial spelling in a loan is not a verified prefix. */
    uncovered((const unsigned char *)"Bestie");
    uncovered((const unsigned char *)"Genie");
    uncovered((const unsigned char *)"bellvue");
    /* The stem spelling alone is not enough for these neighboring
     * syllable shapes: -ss- yields /st/ in the weak syllable, written -tt-
     * contracts to one /t/, and unrelated endings are not this past rule. */
    uncovered((const unsigned char *)"Passte");
    uncovered((const unsigned char *)"Passten");
    uncovered((const unsigned char *)"Rettte");
    uncovered((const unsigned char *)"Rettten");
    uncovered((const unsigned char *)"Kasten");
    uncovered((const unsigned char *)"Kosten");
    uncovered((const unsigned char *)"Tauschte");
    uncovered((const unsigned char *)"Machte");
    uncovered((const unsigned char *)"Cafe");
    uncovered((const unsigned char *)"Eloquence");

    /* The separate research approximation covers unseen syllable shapes
     * without changing the conservative result or silently entering ECI. */
    approximation((const unsigned char *)"Gewesen",
                  "`[.0g@.1ve.0z@n]", 0);
    approximation((const unsigned char *)"Internet",
                  "`[.1In.0tR.0nEt]", 1);
    approximation((const unsigned char *)"Praktikum",
                  "`[.1prAk.0ti.0kUm]", 1);
    /* Productive -heit/-keit is a weak abstract-noun suffix. An approximate
     * compound parse can still supply its phones; the final accent is weak
     * while an earlier real compound accent remains untouched. */
    approximation((const unsigned char *)"Krankheit",
                  "`[.1krAGk.0h'aj't]", 1);
    approximation((const unsigned char *)"Buntheit",
                  "`[.1bUnt.0h'aj't]", 1);
    approximation((const unsigned char *)"M\xf6glichkeit",
                  "`[.1m'oe'k.0lIX.0k'aj't]", 1);
    approximation((const unsigned char *)"Schwierigkeit",
                  "`[.1Svi.0rIX.0k'aj't]", 1);
    approximation((const unsigned char *)"Zuverl\xe4ssigkeit",
                  "`[.1Tu.0fR.2lE.0sIX.0k'aj't]", 1);
    /* Nominalized -bar keeps its long vowel and weak stress, including
     * after a prefixed adjective; examples come from a disjoint public
     * Apple ECI factorial development set. */
    approximation((const unsigned char *)"Tragbarkeit",
                  "`[.1trak.0bar.0k'aj't]", 1);
    approximation((const unsigned char *)"Sichtbarkeit",
                  "`[.1zIXt.0bar.0k'aj't]", 1);
    approximation((const unsigned char *)"Bezahlbarkeit",
                  "`[.0b@.1Tal.0bar.0k'aj't]", 1);
    approximation((const unsigned char *)"Achtsam",
                  "`[.1Axt.0zam]", 1);
    approximation((const unsigned char *)"Heilsamkeit",
                  "`[.1h'aj'l.0zam.0k'aj't]", 1);
    approximation((const unsigned char *)"Sorgsamkeit",
                  "`[.1zOrk.0zam.0k'aj't]", 1);
    /* Negative un- preserves the analysis of a full productive base, even
     * when it begins with weak ge- and the main accent is later. Opaque
     * un... spellings stay with the ordinary analyzer. */
    approximation((const unsigned char *)"Undankbar",
                  "`[.1Un.2dAGk.0bar]", 1);
    approximation((const unsigned char *)"Ungesehen",
                  "`[.1Un.0g@.2ze.0@n]", 1);
    approximation((const unsigned char *)"Ungl\xfc" "cklich",
                  "`[.1Un.2glYk.0lIX]", 1);
    approximation((const unsigned char *)"Unserem",
                  "`[.1Un.0z@.0r@m]", 1);
    approximation((const unsigned char *)"Uniform",
                  "`[.1u.0ni.0fOrm]", 1);
    approximation((const unsigned char *)"Ungeimpft",
                  "`[.1Un.2g'aj'mPt]", 1);
    /* In opaque unter- spellings, or with a speculative ge- base that
     * lacks the expected weak-prefix/stem structure, decline composition
     * rather than pinning today's approximate fallback phones. */
    avoid_false_prefix((const unsigned char *)"Unterhaltsam", ".2ter");
    avoid_false_prefix((const unsigned char *)"Unterscheidbar", ".2ter");
    avoid_false_prefix((const unsigned char *)"Unterschreibbar", ".2ter");
    avoid_false_prefix((const unsigned char *)"Ungeplant", ".2lAnt");
    avoid_false_prefix((const unsigned char *)"Ungeheuerlich", ".2ge");
    avoid_false_prefix((const unsigned char *)"Ungeordnet", ".2Ort");
    approximation((const unsigned char *)"Unsere",
                  "`[.1Un.0z@.0r@]", 1);
    approximation((const unsigned char *)"Bevorzugen",
                  "`[.0b@.1for.0Tu.0g@n]", 1);
    /* Productive closed syllables and atomic written digraph onsets in the
     * opt-in spelling approximation. The negative controls keep long /e/
     * before r and the separable ent- distinct from an ordinary -tsch-. */
    approximation((const unsigned char *)"Morgen",
                  "`[.1mOr.0g@n]", 1);
    approximation((const unsigned char *)"Lampe",
                  "`[.1lAm.0p@]", 1);
    approximation((const unsigned char *)"Kirche",
                  "`[.1kIr.0X@]", 1);
    approximation((const unsigned char *)"W\xf6rter",
                  "`[.1v'OE'r.0tR]", 1);
    /* This scanner has no lexicon. The full frontend retains the observed
     * long vowel; unmarked closed e+r codas are short in letter analysis. */
    approximation((const unsigned char *)"Erde",
                  "`[.1Er.0d@]", 1);
    approximation((const unsigned char *)"Entscheidung",
                  "`[.0Ent.1S'aj'.0dUG]", 1);
    approximation((const unsigned char *)"Pantschen",
                  "`[.1pAn.0C@n]", 1);
    /* Written weak -el and verb-like -et retain reduced /@/; a loan
     * ending in -net with three syllables remains full-vowel. */
    approximation((const unsigned char *)"Vogel",
                  "`[.1fo.0g@l]", 1);
    approximation((const unsigned char *)"Schl\xfcssel",
                  "`[.1SlY.0s@l]", 1);
    approximation((const unsigned char *)"Wartet",
                  "`[.1vAr.0t@t]", 1);
    approximation((const unsigned char *)"Zeichnet",
                  "`[.1T'aj'X.0n@t]", 1);
    approximation((const unsigned char *)"Sagte",
                  "`[.1zak.0t@]", 0);
    approximation((const unsigned char *)"Bach", "`[.1bAx]", 1);
    /* Opt-in productive spelling classes, checked against independent
     * isolated Apple callbacks. These do not alter strict ECI coverage. */
    approximation((const unsigned char *)"Am", "`[.1Am]", 1);
    approximation((const unsigned char *)"Es", "`[.1Es]", 1);
    approximation((const unsigned char *)"Im", "`[.1Im]", 1);
    approximation((const unsigned char *)"Den", "`[.1den]", 1);
    approximation((const unsigned char *)"Von", "`[.1fOn]", 1);
    approximation((const unsigned char *)"Mai", "`[.1m'aj']", 1);
    approximation((const unsigned char *)"Kaffee", "`[.0kA.1fe]", 1);
    approximation((const unsigned char *)"Notiz", "`[.0no.1tiT]", 1);
    approximation((const unsigned char *)"Entwurf", "`[.0Ent.1vUrf]", 1);
    approximation((const unsigned char *)"Wichtig", "`[.1vIX.0tIX]", 1);
    approximation((const unsigned char *)"Lustig", "`[.1lU.0stIX]", 1);
    approximation((const unsigned char *)"Kasten", "`[.1kA.0st@n]", 1);
    approximation((const unsigned char *)"Kosten", "`[.1kO.0st@n]", 1);
    /* The measured dt stem class now belongs to the conservative path. */
    approximation((const unsigned char *)"Schmidt", "`[.1SmIt]", 0);
    approximation((const unsigned char *)"Trinkt", "`[.1trIGkt]", 1);
    approximation((const unsigned char *)"Kommt", "`[.1kOmt]", 1);
    approximation((const unsigned char *)"Bevor", "`[.0b@.1for]", 1);
    approximation((const unsigned char *)"Nebenbei",
                  "`[.0ne.0b@n.1b'aj']", 1);
    approximation((const unsigned char *)"Dazu", "`[.0da.1Tu]", 1);
    approximation((const unsigned char *)"Zitierter",
                  "`[.0Ti.1tir.0tR]", 1);
    approximation((const unsigned char *)"Machen", "`[.1mA.0x@n]", 1);
    approximation((const unsigned char *)"Sprache", "`[.1Spra.0x@]", 1);
    /* Weak er- is a syllable, not the first two bytes of a stressed stem.
     * After an actual prefix, initial sp/st of the stem has a new /S/
     * onset; reduced -ern/-ert retains vocalic /R/ before n/t. */
    approximation((const unsigned char *)"Kindern", "`[.1kIn.0dRn]", 1);
    approximation((const unsigned char *)"Leitern", "`[.1l'aj'.0tRn]", 1);
    approximation((const unsigned char *)"\304ndert", "`[.1En.0dRt]", 1);
    approximation((const unsigned char *)"Erfindet", "`[.0Er.1fIn.0d@t]", 1);
    approximation((const unsigned char *)"Erz\344hlt", "`[.0Er.1T'E:'lt]", 1);
    approximation((const unsigned char *)"Erobert", "`[.0Er.1o.0bRt]", 1);
    approximation((const unsigned char *)"Erspielt", "`[.0Er.1Spilt]", 1);
    approximation((const unsigned char *)"Gespart", "`[.0g@.1SpArt]", 1);
    approximation((const unsigned char *)"Bestellt", "`[.0b@.1StElt]", 1);
    approximation((const unsigned char *)"Versteht", "`[.0fEr.1Stet]", 1);
    approximation((const unsigned char *)"Entspart", "`[.0Ent.1SpArt]", 1);
    approximation((const unsigned char *)"Erden", "`[.1Er.0d@n]", 1);
    /* Separable prefixes compose a complete covered -en base. In particular
     * the entire stem onset /Sr/, /St/, /tr/ stays after the boundary. These
     * outputs were checked across the productive prefix×infinitive class by
     * the public Apple phoneme callback, not added as lexical exceptions. */
    approximation((const unsigned char *)"Aufschreiben",
                  "`[.1'aw'f.2Sr'aj'.0b@n]", 1);
    approximation((const unsigned char *)"Ausschreiben",
                  "`[.1'aw's.2Sr'aj'.0b@n]", 1);
    approximation((const unsigned char *)"Abschreiben",
                  "`[.1Ap.2Sr'aj'.0b@n]", 1);
    approximation((const unsigned char *)"Aufstellen",
                  "`[.1'aw'f.2StE.0l@n]", 1);
    approximation((const unsigned char *)"Abstellen",
                  "`[.1Ap.2StE.0l@n]", 1);
    approximation((const unsigned char *)"Auftragen",
                  "`[.1'aw'f.2tra.0g@n]", 1);
    approximation((const unsigned char *)"Anbauen",
                  "`[.1An.2b'aw'.0@n]", 1);
    approximation((const unsigned char *)"Einfragen",
                  "`[.1'aj'n.2fra.0g@n]", 1);
    approximation((const unsigned char *)"Aufschreibt",
                  "`[.1'aw'f.2Sr'aj'pt]", 1);
    approximation((const unsigned char *)"Ausschreibst",
                  "`[.1'aw's.2Sr'aj'pst]", 1);
    approximation((const unsigned char *)"Abschreibe",
                  "`[.1Ap.2Sr'aj'.0b@]", 1);
    approximation((const unsigned char *)"Anfragt",
                  "`[.1An.2frakt]", 1);
    approximation((const unsigned char *)"Einbaute",
                  "`[.1'aj'n.2b'aw'.0t@]", 1);
    /* The productive OOV path composes a complete estimated base after a
     * transparent separable prefix. These five readings were captured via
     * public Apple ECI in isolation and two sentence contexts; their base
     * spellings are not independently strict C dictionary entries. */
    approximation((const unsigned char *)"Abgrenzen",
                  "`[.1Ap.2grEn.0T@n]", 1);
    approximation((const unsigned char *)"Abgeschaltet",
                  "`[.1Ap.0g@.2SAl.0t@t]", 1);
    approximation((const unsigned char *)"Aufgewandt",
                  "`[.1'aw'f.0g@.2vAnt]", 1);
    approximation((const unsigned char *)"Ausgeblieben",
                  "`[.1'aw's.0g@.2bli.0b@n]", 1);
    approximation((const unsigned char *)"Ausstand",
                  "`[.1'aw's.2StAnt]", 1);
    /* Opaque loan stems and embedded un-/zu- accents require their own
     * analysis; a plausible prefix spelling alone must not promote them. */
    avoid_false_prefix((const unsigned char *)"Abstraktion", ".2StrAk");
    avoid_false_prefix((const unsigned char *)"Absolut", ".2zo");
    avoid_false_prefix((const unsigned char *)"Abzugeben", ".2Tu");
    avoid_false_prefix((const unsigned char *)"Unabweislich", ".2Ap");

    /* A separable particle plus infinitival zu keeps the base's complete
     * verified phone/stress sequence. Several prefixes and a weak er-/ge-
     * base exercise the same construction; no word-specific rule is used. */
    approximation((const unsigned char *)"Abzugeben",
                  "`[.1Ap.0Tu.2ge.0b@n]", 1);
    approximation((const unsigned char *)"Abzuerkennen",
                  "`[.1Ap.0Tu.0Er.2kE.0n@n]", 1);
    approximation((const unsigned char *)"Anzuflehen",
                  "`[.1An.0Tu.2fle.0@n]", 1);
    approximation((const unsigned char *)"Aufzulesen",
                  "`[.1'aw'f.0Tu.2le.0z@n]", 1);
    approximation((const unsigned char *)"Auszumalen",
                  "`[.1'aw's.0Tu.2ma.0l@n]", 1);
    approximation((const unsigned char *)"Einzuteilen",
                  "`[.1'aj'n.0Tu.2t'aj'.0l@n]", 1);
    approximation((const unsigned char *)"Beizuwohnen",
                  "`[.1b'aj'.0Tu.2vo.0n@n]", 1);
    approximation((const unsigned char *)"Hinzuladen",
                  "`[.1hIn.0Tu.2la.0d@n]", 1);
    approximation((const unsigned char *)"Mitzugeben",
                  "`[.1mIt.0Tu.2ge.0b@n]", 1);
    approximation((const unsigned char *)"Umzuleiten",
                  "`[.1Um.0Tu.2l'aj'.0t@n]", 1);
    approximation((const unsigned char *)"Vorzusehen",
                  "`[.1for.0Tu.2ze.0@n]", 1);
    approximation((const unsigned char *)"Nachzulesen",
                  "`[.1nax.0Tu.2le.0z@n]", 1);
    approximation((const unsigned char *)"Wegzugeben",
                  "`[.1vEk.0Tu.2ge.0b@n]", 1);
    approximation((const unsigned char *)"Herzuleiten",
                  "`[.1hEr.0Tu.2l'aj'.0t@n]", 1);
    approximation((const unsigned char *)"Zuzulegen",
                  "`[.1Tu.0Tu.2le.0g@n]", 1);
    /* A conservatively accepted base can still have a false prefix parse.
     * Do not replace the correct written eu diphthong with weak be- + /u/. */
    approximation((const unsigned char *)"Vorzubeugen",
                  "`[.1for.0Tu.0b'oj'.0g@n]", 1);

    /* Prefix composition preserves the independently covered stem quantity
     * and final devoicing, observed through normal text (not SPR replay). */
    approximation((const unsigned char *)"Gesagt", "`[.0g@.1zakt]", 0);
    approximation((const unsigned char *)"Gefragt", "`[.0g@.1frakt]", 0);
    approximation((const unsigned char *)"Verlebt", "`[.0fEr.1lept]", 0);

    /* Productive adjectival -los lengthens only the terminal fallback
     * syllable; ordinary short /O/ roots and standalone los are controls. */
    approximation((const unsigned char *)"Rastlos",
                  "`[.1rAst.0los]", 1);
    approximation((const unsigned char *)"Seelenlos",
                  "`[.1ze.0l@n.0los]", 1);
    approximation((const unsigned char *)"Fassungslos",
                  "`[.1fA.0sUGs.0los]", 1);
    approximation((const unsigned char *)"Los",
                  "`[.1lOs]", 1);
    approximation((const unsigned char *)"Schloss",
                  "`[.1SlOs]", 0);
    /* The meaning of um- can change its stress: Umbauen has an unstressed
     * prefix, Umstellen has a stressed one. No generic um- rule is inferred. */
    approximation((const unsigned char *)"Umbauen",
                  "`[.1Um.0b'aw'.0@n]", 1);
    /* A complete two-part spelling permits secondary stress when its C
     * phone stream already has the same boundary as both constituents. */
    approximation((const unsigned char *)"Autobahn", "`[.1'aw'.0to.2ban]", 1);
    approximation((const unsigned char *)"Luftraum", "`[.1lUft.2r'aw'm]", 1);
    /* A multisyllabic, conservatively analyzed head can retain secondary
     * stress when the constituent analyses have the complete same phones
     * as the unsegmented word. Phonotactically impossible splits such as
     * Straßen+nbahnen are rejected; -licht/-tor remain lexically ambiguous. */
    approximation((const unsigned char *)"Autobahnen",
                  "`[.1'aw'.0to.2ba.0n@n]", 1);
    approximation((const unsigned char *)"Stra\xdf" "enbahnen",
                  "`[.1Stra.0s@n.2ba.0n@n]", 1);
    approximation((const unsigned char *)"Morgensonne",
                  "`[.1mOr.0g@n.2zO.0n@]", 1);
    approximation((const unsigned char *)"Wasserleitung",
                  "`[.1vA.0sR.2l'aj'.0tUG]", 1);
    approximation((const unsigned char *)"H\xe4lftezeichen",
                  "`[.1hElf.0t@.2T'aj'.0X@n]", 1);
    approximation((const unsigned char *)"Baumtor",
                  "`[.1b'aw'm.0tor]", 1);
    /* Two independently covered complete components are segmented before
     * spelling-to-phone conversion. A later accent repair cannot recover
     * the /bl/, /pl/ and /tr/ onsets after a mistaken consonant split. */
    approximation((const unsigned char *)"Weinblatt",
                  "`[.1v'aj'n.2blAt]", 1);
    approximation((const unsigned char *)"Neuplatz",
                  "`[.1n'oj'.2plAT]", 1);
    approximation((const unsigned char *)"Weintraube",
                  "`[.1v'aj'n.2tr'aw'.0b@]", 1);
    approximation((const unsigned char *)"Zeithaus",
                  "`[.1T'aj't.2h'aw's]", 1);
    /* A third covered root keeps its whole onset and independent secondary
     * accent; otherwise the flat syllabifier swallows /St/ or /bl/. */
    approximation((const unsigned char *)"Neubaustelle",
                  "`[.1n'oj'.2b'aw'.2StE.0l@]", 1);
    approximation((const unsigned char *)"Baumhausbau",
                  "`[.1b'aw'm.2h'aw's.2b'aw']", 1);
    approximation((const unsigned char *)"Feuerwehrhaus",
                  "`[.1f'oj'.0R.2ver.2h'aw's]", 1);
    approximation((const unsigned char *)"Seilbahnwagen",
                  "`[.1z'aj'l.2ban.2va.0g@n]", 1);
    approximation((const unsigned char *)"Sonnenblumenhaus",
                  "`[.1zO.0n@n.2blu.0m@n.2h'aw's]", 1);
    /* -tier is a counterclass: the noun head bears primary stress. */
    approximation((const unsigned char *)"Baumtier",
                  "`[.0b'aw'm.1tir]", 1);
    /* Loan letters and dense consonant clusters are restricted to the
     * optional approximator; the strict reference-matched rules stay shut. */
    approximation((const unsigned char *)"Cafe", "`[.1ka.0f@]", 1);
    approximation((const unsigned char *)"Eloquence",
                  "`[.1e.0lO.0kv@n.0T@]", 1);
    approximation((const unsigned char *)"Sprachsynthesizer",
                  "`[.1SprAx.0zYnt.0h@.0zi.0TR]", 1);
    /* In the optional approximation a uniquely composable first stem and
     * second independently stressed stem retain lexical secondary stress.
     * Suffixes and invented competing boundaries remain conservative. */
    approximation((const unsigned char *)"Allbarm",
                  "`[.1Al.2bArm]", 1);
    approximation((const unsigned char *)"Hauptraum",
                  "`[.1h'aw'pt.2r'aw'm]", 1);
    approximation((const unsigned char *)"Kaufnahm",
                  "`[.1k'aw'f.2nam]", 1);
    approximation((const unsigned char *)"Probenunter",
                  "`[.1pro.0b@.2nUn.0tR]", 1);
    approximation((const unsigned char *)"Zeitraum",
                  "`[.1T'aj't.2r'aw'm]", 1);
    approximation((const unsigned char *)"Treuhand",
                  "`[.1tr'oj'.2hAnt]", 1);
    approximation((const unsigned char *)"Niederung",
                  "`[.1ni.0d@.0rUG]", 1);
    approximation((const unsigned char *)"Raufas",
                  "`[.1r'aw'.0fAs]", 1);
    /* Secondary accent in long -ieren loans; the short form is an important
     * counterclass and keeps its weak opening syllable. */
    approximation((const unsigned char *)"Akzeptieren",
                  "`[.2Ak.0TEp.1ti.0r@n]", 1);
    approximation((const unsigned char *)"Kontaktieren",
                  "`[.2kOn.0tAk.1ti.0r@n]", 1);
    approximation((const unsigned char *)"Probieren",
                  "`[.0pro.1bi.0r@n]", 1);
    /* An opaque an... start must not be mistaken for a productive an-
     * prefix. A legal following root onset and inflection allow the
     * independently analyzed participle or infinitive to compose. */
    approximation((const unsigned char *)"Angekommen",
                  "`[.1An.0g@.2kO.0m@n]", 1);
    approximation((const unsigned char *)"Anschrien",
                  "`[.1An.2Srin]", 1);
    approximation((const unsigned char *)"Antworten",
                  "`[.1Ant.0vOr.0t@n]", 1);

    unsigned char out[32] = "untouched";
    size_t needed = 111;
    const unsigned char word[] = "Maus";
    assert(reloq_german_g2p_word(word, 4, out, 4, &needed) == -2);
    assert(needed == strlen("`[.1m'aw's]"));
    assert(strcmp((const char *)out, "untouched") == 0);
    needed = 111;
    assert(reloq_german_g2p_word((const unsigned char *)"Packte", 6,
                                 out, 4, &needed) == -2);
    assert(needed == strlen("`[.1pAk.0t@]"));
    assert(strcmp((const char *)out, "untouched") == 0);
    needed = 111;
    int estimated = 9;
    assert(reloq_german_g2p_word_best_effort(
               (const unsigned char *)"Internet", 8,
               out, 4, &needed, &estimated) == -2);
    assert(needed == strlen("`[.1In.0tR.0nEt]"));
    assert(estimated == 9 && strcmp((const char *)out, "untouched") == 0);
    needed = 111;
    assert(reloq_german_g2p_word_best_effort(
               (const unsigned char *)"Tragbarkeit", strlen("Tragbarkeit"),
               out, 4, &needed, &estimated) == -2);
    assert(needed == strlen("`[.1trak.0bar.0k'aj't]"));
    assert(estimated == 9 && strcmp((const char *)out, "untouched") == 0);
    needed = 111;
    assert(reloq_german_g2p_word_best_effort(
               (const unsigned char *)"Weinblatt", 9,
               out, 4, &needed, &estimated) == -2);
    assert(needed == strlen("`[.1v'aj'n.2blAt]"));
    assert(strcmp((const char *)out, "untouched") == 0);
    needed = 111;
    assert(reloq_german_g2p_word_best_effort(
               (const unsigned char *)"Aufschreiben", strlen("Aufschreiben"),
               out, 4, &needed, &estimated) == -2);
    assert(needed == strlen("`[.1'aw'f.2Sr'aj'.0b@n]"));
    assert(estimated == 9 && strcmp((const char *)out, "untouched") == 0);
    needed = 111;
    assert(reloq_german_g2p_word((const unsigned char *)"Unfreiheit", 10,
                                 out, 4, &needed) == -2);
    assert(needed == strlen("`[.1Un.2fr'aj'.0h'aj't]"));
    assert(strcmp((const char *)out, "untouched") == 0);
    needed = 111;
    assert(reloq_german_g2p_word((const unsigned char *)"Steinig", 7,
                                 out, 4, &needed) == -2);
    assert(needed == strlen("`[.1St'aj'.0nIX]"));
    assert(strcmp((const char *)out, "untouched") == 0);
    needed = 111;
    assert(reloq_german_g2p_word((const unsigned char *)"Lesungen", 8,
                                 out, 4, &needed) == -2);
    assert(needed == strlen("`[.1le.0zUG.0@n]"));
    assert(strcmp((const char *)out, "untouched") == 0);
    needed = 111;
    assert(reloq_german_g2p_word((const unsigned char *)"Freiheit", 8,
                                 out, 5, &needed) == -2);
    assert(needed == strlen("`[.1fr'aj'.0h'aj't]"));
    assert(strcmp((const char *)out, "untouched") == 0);
    needed = 111;
    assert(reloq_german_g2p_word((const unsigned char *)"Lesung", 6,
                                 out, 4, &needed) == -2);
    assert(needed == strlen("`[.1le.0zUG]"));
    assert(strcmp((const char *)out, "untouched") == 0);
    needed = 111;
    assert(reloq_german_g2p_word((const unsigned char *)"H\xe4uschen", 8,
                                 out, 4, &needed) == -2);
    assert(needed == strlen("`[.1h'oj'.0S@n]"));
    assert(strcmp((const char *)out, "untouched") == 0);
    needed = 111;
    assert(reloq_german_g2p_word((const unsigned char *)"x\xff", 2,
                                 out, sizeof out, &needed) == -1);
    assert(needed == 111);
    assert(strcmp((const char *)out, "untouched") == 0);
    puts("German partial rule-based G2P: OK");
    return 0;
}
