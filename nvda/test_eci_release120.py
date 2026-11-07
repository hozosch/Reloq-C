"""Regressions for Greek spacing, question contour and short PCM peaks."""
import argparse
from array import array
from pathlib import Path

from _reloq_eci_direct import (DirectEciEngine, EciIndex, GENDER, NATIVE_SPEED,
                               NATIVE_VOLUME, ORIGINAL_QUESTIONS, PROCESS_TAGS, SAMPLE_RATE)
from test_eci_german_clock_native import phonemes, render


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    args = parser.parse_args()
    library = args.library.resolve()
    greek = DirectEciEngine(library, dialect=0x120000, encoding='utf8', native_voice=True)
    try:
        greek.set_parameter(PROCESS_TAGS, 1)
        # Preserve the existing sentence analysis rather than emphasizing
        # every lexical content word in ordinary connected Greek.
        cases = [('Γλώσσα Ελληνικά αναδυόμενο κουτί', '[.0a.0na.0di.1o.0me.0no]'),
                 ('Αυτόματη εναλλαγή γλώσσας ενεργή', '[.1glo.0sas]')]
        for text, stressed in cases:
            phones = phonemes(greek, text)
            assert '`0 `' + stressed in phones, phones
            whole = render(greek, [text, EciIndex(7)])
            assert whole[0] and whole[1] == [(7, len(whole[0]))]
            for cut in range(len(text) + 1):
                assert render(greek, [text[:cut], text[cut:], EciIndex(7)]) == whole, (text, cut)
        # NVDA can join a control label and text with multiple spaces.  A
        # spacing difference must not synthesize a comma or invent a pause.
        title = 'Αλλαγή Διαμόρφωσης Γλώσσας  παράθυρο διαλόγου'
        body = 'Πρέπει να γίνει επανεκκίνηση του NVDA, για να τεθεί σε ισχύ η νέα γλώσσα.'
        # A long preceding title used to leave the whole subordinate clause
        # without a normal word accent, even though its lexical vowels were
        # marked. Restore the middle content word while keeping the nearby
        # function words weak and the core's other reductions intact.
        phones = phonemes(greek, title + '  ' + body)
        assert '`0 `[.1glo.0sas]' not in phones and '`[.1glo.0sas]' in phones, phones
        clause = phones.split(',', 1)[1]
        assert '`0 `[.0te.1Ti]' in clause, clause
        assert '`0 `[.0is.1ji]' not in clause and '`[.0is.1ji]' in clause, clause
        assert '`0 `[.1ne.0a]' in clause, clause
        assert '`[.0La]' in clause and '`[.0na]' in clause and '`[.0se]' in clause, clause
        # At most one of two consecutively reduced content words receives a
        # normal accent; do not force both into full prominence.
        ordinary = phonemes(greek, 'Το μεγάλο πράσινο παράθυρο είναι ανοιχτό.')
        assert '`[.1pra.0si.0no] `0 `[.0pa.1ra.0Ti.0ro]' in ordinary, ordinary
        assert '`0 `[.1pra.0si.0no]' not in ordinary, ordinary
        network = phonemes(greek, 'Ο έλεγχος σύνδεσης δικτύου ολοκληρώθηκε.')
        assert '`[.1sin.0de.0sis] `0 `[.0dik.1ti.0u]' in network, network
        assert '`0 `[.1sin.0de.0sis]' not in network, network
        assert phonemes(greek, title + '  ' + body) == phonemes(greek, title + ' ' + body)
        mixed_title = 'Αλλαγή γλώσσας για το NVDA'
        assert phonemes(greek, mixed_title + '  ' + body) == phonemes(greek, mixed_title + ' ' + body)
        whole = render(greek, [title + '  ' + body, EciIndex(11)])
        assert whole[0] and whole[1] == [(11, len(whole[0]))]
        assert whole == render(greek, [title + ' ' + body, EciIndex(11)])
        assert len(render(greek, [title + ', ' + body, EciIndex(11)])[0]) > len(whole[0])
        for cut in range(len(title) - 1, len(title) + 4):
            text = title + '  ' + body
            assert render(greek, [text[:cut], text[cut:], EciIndex(11)]) == whole
        # A double space after an ordinary preposition is not a new phrase.
        assert phonemes(greek, 'Αυτός έρχεται από  Αθήνα.') == phonemes(greek, 'Αυτός έρχεται από Αθήνα.')
        assert '`[.0a] `[.0po]' in phonemes(greek, 'από Αθήνα')
        assert '[.0a.1po]' in phonemes(greek, 'από')
        assert render(greek, ['`0 Αθήνα και Θεσσαλονίκη']) != render(greek, ['`1 Αθήνα και Θεσσαλονίκη'])
        # Injecting a prominence tag directly after a hyphen caused the
        # Castilian core to read a spurious punctuation name between words.
        for joiner in ('-', '‐', '‑', '−'):
            assert '[.1pa.0bla]' not in phonemes(greek, 'Καλός καιρός' + joiner + 'ήρθε η καταιγίδα.')
    finally:
        greek.close()

    for language, question, statement in (
        (0x10000, 'What would you like to do?', 'This is a sentence.'),
        (0x40000, 'Was möchtest du machen?', 'Das ist ein Satz.'),
    ):
        engine = DirectEciEngine(library, dialect=language, encoding='utf8', native_voice=True)
        try:
            for rate in (1, 6):
                engine.set_parameter(SAMPLE_RATE, rate)
                engine.set_parameter(ORIGINAL_QUESTIONS, 0)
                rising = render(engine, [question, EciIndex(1)])
                plain = render(engine, [statement, EciIndex(2)])
                engine.set_parameter(ORIGINAL_QUESTIONS, 1)
                original = render(engine, [question, EciIndex(1)])
                assert original[0] != rising[0] and len(original[0]) == len(rising[0])
                assert original[1] == rising[1]
                assert render(engine, [statement, EciIndex(2)]) == plain
                engine.set_parameter(ORIGINAL_QUESTIONS, 0)
                assert render(engine, [question, EciIndex(1)]) == rising
        finally:
            engine.close()

    german = DirectEciEngine(library, dialect=0x40000, encoding='utf8', native_voice=True)
    try:
        german.set_parameter(NATIVE_VOLUME, 100)
        german.set_parameter(NATIVE_SPEED, 98)
        for gender in (0, 1):
            german.set_parameter(GENDER, gender)
            for rate in range(7):
                german.set_parameter(SAMPLE_RATE, rate)
                data, marks = render(german, ['48', EciIndex(5)])
                pcm = array('h');pcm.frombytes(data)
                assert pcm and 0 < max(abs(s) for s in pcm) < 28001, (gender, rate)
                assert marks == [(5, len(data))], (gender, rate, marks)
    finally:
        german.close()
    print('Greek spacing and existing prominence, selectable question contour and short peaks passed')


if __name__ == '__main__':
    main()
