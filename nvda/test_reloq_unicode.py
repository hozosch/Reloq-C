"""Pure Python Unicode admission tests; no NVDA installation is required."""

from __future__ import annotations

from _reloq_unicode import normalize_generic_text, normalize_spelling_text


def test_german_letters_and_punctuation_stay_verbatim():
    text = '„Ärger, Straße – Œuvre: 12,50 €!“\r\nMäuse.'
    assert normalize_generic_text(text) == text.replace("Œuvre", "OEuvre")


def test_combining_and_extended_latin():
    assert normalize_generic_text('Ba\u0308r') == 'Bär'
    assert normalize_generic_text('Sāmi, Łódź und ﬃ.') == 'Sami, Lodz und ffi.'
    assert normalize_generic_text('ẞ') == 'ß'


def test_greek_spelling_not_false_greek_pronunciation():
    assert normalize_generic_text('Γειά, κόσμος!') == (
        'Gamma Epsilon Iota Alpha, Kappa Omikron Sigma My Omikron Sigma!')
    assert normalize_generic_text('ἄ') == 'Alpha'


def test_emoji_clusters_and_neighboring_words():
    assert normalize_generic_text('Hallo😀Welt.') == 'Hallo Welt.'
    assert normalize_generic_text('👩\u200d💻👍🏽 🇬🇷 1️⃣') == (
        '  ')
    assert normalize_generic_text('A🙂🙃B') == 'A B'


def test_script_runs_and_symbols():
    assert normalize_generic_text('Hallo漢字Welt.') == (
        'Hallo Welt.')
    assert normalize_generic_text('東京 カタカナ 한국어') == (
        '  ')
    assert normalize_generic_text('a∑b') == 'a b'
    assert normalize_generic_text('A，B。') == 'A,B.'


def test_non_latin_digits_controls_and_surrogates():
    assert normalize_generic_text('١٢') == '12'
    assert normalize_generic_text('Hallo\x00Welt') == 'Hallo Welt'
    assert normalize_generic_text('A\u200bB') == 'A B'
    assert normalize_generic_text('\ud800') == ''


def test_result_is_always_encodable_and_has_no_nul():
    samples = ['A😀漢字Ω', 'e\u0304', '\ufeff', '\U0010ffff', 'A\0B',
               '👩\u200d💻', '١٢', '𐍈']
    for sample in samples:
        result = normalize_generic_text(sample)
        result.encode('cp1252', errors='strict')
        assert '\0' not in result


def test_character_navigation_preserves_codepoints():
    assert normalize_spelling_text("\uf6ec") == "Zeichen F 6 E C"
    assert normalize_spelling_text("Ж") == "kyrillisches Zeichen 4 1 6"
    assert normalize_spelling_text("Ωµϕ") == "Omega My Phi"
    assert normalize_spelling_text("𐍈") == "gotisches Zeichen 1 0 3 4 8"
    assert normalize_spelling_text("\U0010ffff") == "Zeichen 1 0 F F F F"
    assert normalize_spelling_text("A\u0308") == "Ä"
    assert normalize_spelling_text("`") == "Gravis"
    assert normalize_spelling_text("\0") == "Zeichen 0"

def test_unknowns_are_silent_only_in_running_text():
    for text in ["\uf6ec", "Ж", "𐍈", "\U0010ffff", "😀"]:
        assert normalize_generic_text(text) == ""
        assert normalize_spelling_text(text)
    assert normalize_generic_text("α=30; β=45") == "Alpha=30; Beta=45"


def test_latin_accents_are_named_only_in_spelling():
    assert normalize_generic_text("Łódź, français, déjà, naïve, Æsir, Straße.") == (
        "Lodz, francais, deja, naive, AEsir, Straße.")
    assert normalize_spelling_text("ó") == "o mit Akut"
    assert normalize_spelling_text("Ł") == "L mit Querstrich"
    assert normalize_spelling_text("ą") == "a mit Ogonek"
    assert normalize_spelling_text("e\u0301") == "e mit Akut"
    assert normalize_spelling_text("q\u0301") == "q mit Akut"
    assert normalize_generic_text("ÄÖÜäöüßẞ") == "ÄÖÜäöüßß"
    assert normalize_spelling_text("ö") == "ö"


if __name__ == '__main__':
    for name, function in sorted(globals().items()):
        if name.startswith('test_'):
            function()
    print('Unicode fallback checks passed')
