"""Conservative Unicode admission for the experimental German NVDA voice.

The current native ECI takes CP1252, not UTF-8. This adapter preserves German letters and punctuation, folds other Latin
letters for running speech and names their accents in spelling. Unknown
characters are silent in ordinary text and named by code point in
character mode. Greek uses letter names; this is not a multilingual voice.
"""

from __future__ import annotations

import unicodedata


_GREEK_NAMES = {
    "ALPHA": "Alpha", "BETA": "Beta", "GAMMA": "Gamma",
    "DELTA": "Delta", "EPSILON": "Epsilon", "ZETA": "Zeta",
    "ETA": "Eta", "THETA": "Theta", "IOTA": "Iota",
    "KAPPA": "Kappa", "LAMDA": "Lambda", "LAMBDA": "Lambda",
    "MU": "My", "NU": "Ny", "XI": "Xi", "OMICRON": "Omikron",
    "PI": "Pi", "RHO": "Rho", "SIGMA": "Sigma",
    "FINAL SIGMA": "Sigma", "TAU": "Tau", "UPSILON": "Ypsilon",
    "PHI": "Phi", "CHI": "Chi", "PSI": "Psi", "OMEGA": "Omega",
    "DIGAMMA": "Digamma", "STIGMA": "Stigma", "KOPPA": "Koppa",
    "SAMPI": "Sampi", "ARCHAIC KOPPA": "Koppa", "ARCHAIC SAMPI": "Sampi",
}


def _encodable(value: str) -> bool:
    try:
        value.encode("cp1252", errors="strict")
        return True
    except UnicodeEncodeError:
        return False


def _greek_name(value: str) -> str | None:
    # Greek polytonic letters are canonically decomposed; accents and
    # breathings are not separately named in this interim German voice.
    base = "".join(c for c in unicodedata.normalize("NFD", unicodedata.normalize("NFKC", value))
                   if not unicodedata.category(c).startswith("M"))
    if len(base) != 1:
        return None
    name = unicodedata.name(base, "")
    if name.startswith("GREEK ") and " LETTER " in name:
        return _GREEK_NAMES.get(name.split(" LETTER ", 1)[1])
    return None


_GERMAN_LETTERS = frozenset("äöüÄÖÜßẞ")
_LATIN_BASES = {"AE": "ae", "OE": "oe", "IJ": "ij", "ETH": "d",
                "THORN": "th", "ENG": "n", "DOTLESS I": "i", "LONG S": "s"}
_ACCENTS = {
    "ACUTE": "Akut", "GRAVE": "Gravis", "CIRCUMFLEX": "Zirkumflex",
    "TILDE": "Tilde", "DIAERESIS": "Trema", "MACRON": "Makron",
    "BREVE": "Breve", "CARON": "Hatschek", "CEDILLA": "Cedille",
    "OGONEK": "Ogonek", "DOUBLE ACUTE": "Doppelakut",
    "DOUBLE GRAVE": "Doppelgravis", "INVERTED BREVE": "umgekehrtem Breve",
    "DOT ABOVE": "Punkt oben", "DOT BELOW": "Punkt unten",
    "RING ABOVE": "Ring oben", "RING BELOW": "Ring unten",
    "HOOK ABOVE": "Haken oben", "LINE BELOW": "Strich unten",
    "STROKE": "Querstrich", "BAR": "Querstrich", "HOOK": "Haken",
}


def _latin_spelling(value: str) -> str | None:
    if value in _GERMAN_LETTERS:
        return "ß" if value == "ẞ" else value
    base = _latin_fold(value)
    if not base:
        return None
    name = unicodedata.name(value, "")
    marks = [unicodedata.name(c, "").removeprefix("COMBINING ").removesuffix(" ACCENT")
             for c in unicodedata.normalize("NFD", value)
             if unicodedata.category(c).startswith("M")]
    if not marks and " WITH " in name:
        marks = name.split(" WITH ", 1)[1].split(" AND ")
    if marks:
        accents = [_ACCENTS.get(mark) for mark in marks]
        if not all(accents):
            return _code_name(value)
        return base + " mit " + " und ".join(accents)
    if "LIGATURE" in name or len(base) > 1:
        return "Ligatur " + " ".join(base)
    return base


def _latin_fold(value: str) -> str | None:
    if value in _GERMAN_LETTERS:
        return "ß" if value == "ẞ" else value
    name = unicodedata.name(value, "")
    if not name.startswith("LATIN "):
        return None
    decomposed = unicodedata.normalize("NFKD", value)
    letters = "".join(c for c in decomposed if c.isascii() and c.isalpha())
    if letters and all(c.isascii() and c.isalpha() or
                       unicodedata.category(c).startswith("M")
                       for c in decomposed):
        return letters
    if " LETTER " in name:
        base = name.split(" LETTER ", 1)[1].split(" WITH ", 1)[0]
        if base in _LATIN_BASES:
            folded = _LATIN_BASES[base]
            return folded if " SMALL " in name else folded.upper()
        if len(base) == 1 and base.isascii() and base.isalpha():
            return base.lower() if " SMALL " in name else base
    if " LIGATURE " in name:
        base = name.split(" LIGATURE ", 1)[1].split(" WITH ", 1)[0]
        if 1 <= len(base) <= 3 and base.isascii() and base.isalpha():
            return base.lower() if " SMALL " in name else base
    return None


def _emoji_base(value: str) -> bool:
    code = ord(value)
    return 0x1F000 <= code <= 0x1FAFF or 0x2600 <= code <= 0x27BF


def _emoji_end(value: str, start: int) -> int:
    """Consume one base plus variation/modifier/ZWJ joined bases."""
    end = start + 1
    if (0x1F1E6 <= ord(value[start]) <= 0x1F1FF and end < len(value) and
            0x1F1E6 <= ord(value[end]) <= 0x1F1FF):
        end += 1  # Two regional indicators form one flag grapheme.
    while end < len(value):
        code = ord(value[end])
        if code in (0xFE0E, 0xFE0F) or 0x1F3FB <= code <= 0x1F3FF:
            end += 1
        elif code == 0x200D and end + 1 < len(value) and _emoji_base(value[end + 1]):
            end += 2
        else:
            break
    return end


def _script(value: str) -> str | None:
    name = unicodedata.name(value, "")
    for prefix in ("CJK UNIFIED IDEOGRAPH", "CJK COMPATIBILITY IDEOGRAPH",
                   "HIRAGANA", "KATAKANA", "HANGUL", "BOPOMOFO",
                   "CYRILLIC", "ARABIC", "HEBREW", "DEVANAGARI",
                   "THAI", "GEORGIAN"):
        if name.startswith(prefix):
            return prefix
    if unicodedata.category(value).startswith("L"):
        return "OTHER LETTER"
    return None


def _punctuation_fold(value: str) -> str | None:
    if not unicodedata.category(value).startswith("P"):
        return None
    name = unicodedata.name(value, "")
    for kind, replacement in (("FULL STOP", "."), ("COMMA", ","),
                              ("QUESTION MARK", "?"),
                              ("EXCLAMATION MARK", "!"),
                              ("SEMICOLON", ";"), ("COLON", ":")):
        if kind in name:
            return replacement
    return None


_SCRIPT_NAMES = {
    "CYRILLIC": "kyrillisches", "GREEK": "griechisches", "LATIN": "lateinisches",
    "ARABIC": "arabisches", "HEBREW": "hebräisches", "DEVANAGARI": "Devanagari",
    "THAI": "thailändisches", "GEORGIAN": "georgisches", "ARMENIAN": "armenisches",
    "BENGALI": "bengalisches", "GURMUKHI": "Gurmukhi", "GUJARATI": "Gujarati",
    "ORIYA": "Oriya", "TAMIL": "tamilisches", "TELUGU": "Telugu",
    "KANNADA": "Kannada", "MALAYALAM": "Malayalam", "SINHALA": "singhalesisches",
    "LAO": "laotisches", "TIBETAN": "tibetisches", "MYANMAR": "birmanisches",
    "ETHIOPIC": "äthiopisches", "CHEROKEE": "Cherokee", "KHMER": "Khmer",
    "HIRAGANA": "Hiragana", "KATAKANA": "Katakana", "HANGUL": "koreanisches",
    "CJK": "chinesisches", "BOPOMOFO": "Bopomofo", "SYRIAC": "syrisches",
    "THAANA": "Thaana", "NKO": "Nko", "TIFINAGH": "Tifinagh",
    "OGHAM": "Ogham", "RUNIC": "Runen", "GLAGOLITIC": "glagolitisches",
    "COPTIC": "koptisches", "GOTHIC": "gotisches", "DESERET": "Deseret",
}


def _code_name(value: str) -> str:
    name = unicodedata.name(value, "")
    script = _SCRIPT_NAMES.get(name.split(" ", 1)[0], "")
    # Code points are spelled digit by digit, including supplementary-plane
    # characters and private-use signs. Never transliterate another script.
    prefix = (script + " Zeichen") if script else "Zeichen"
    return prefix + " " + " ".join(format(ord(value), "X"))


def normalize_spelling_text(text: str) -> str:
    """Names for character navigation, distinct from ordinary admission.

    NFC composes a base and combining marks before naming it. Every unknown
    character remains individually identifiable, including in script runs.
    """
    if not isinstance(text, str):
        raise TypeError("expected Unicode text")
    words = []
    for value in unicodedata.normalize("NFC", text):
        greek = _greek_name(value)
        if greek:
            words.append(greek)
        elif value == "`":
            words.append("Gravis")
        elif value == "\0":
            words.append(_code_name(value))
        elif _latin_spelling(value):
            words.append(_latin_spelling(value))
        elif unicodedata.category(value).startswith("M"):
            mark = unicodedata.name(value, "").removeprefix("COMBINING ").removesuffix(" ACCENT")
            words.append("mit " + _ACCENTS[mark] if mark in _ACCENTS else _code_name(value))
        elif _encodable(value):
            words.append(value)
        else:
            words.append(_code_name(value))
    output = ""
    for word in words:
        if output and word and not output[-1].isspace() and not word[0].isspace():
            output += " "
        output += word
    return output


def normalize_generic_text(text: str) -> str:
    """Admit ordinary text; unnamed Unicode contributes only a boundary.

    Known Greek letters use their names in running text as well as spelling.
    Supported Latin compatibility forms and punctuation keep their existing
    behavior. No placeholder is spoken for an unknown character or script.
    """
    if not isinstance(text, str):
        raise TypeError("expected Unicode text")
    if text.isascii() and "\0" not in text:
        return text
    text = unicodedata.normalize("NFC", text)
    output: list[str] = []
    def add_word(word: str, next_at: int) -> None:
        if output and output[-1] and output[-1][-1].isalnum():
            output.append(" ")
        output.append(word)
        if next_at < len(text) and text[next_at].isalnum():
            output.append(" ")
    def boundary() -> None:
        if output and output[-1] and not output[-1][-1].isspace():
            output.append(" ")
    at = 0
    while at < len(text):
        value = text[at]
        if value in "0123456789#*" and at+1 < len(text) and ord(text[at+1]) in (0xFE0F,0x20E3):
            at += 1
            if at < len(text) and ord(text[at]) == 0xFE0F: at += 1
            if at < len(text) and ord(text[at]) == 0x20E3: at += 1
            boundary(); continue
        if _emoji_base(value):
            at = _emoji_end(text, at); boundary(); continue
        greek = _greek_name(value)
        if greek:
            add_word(greek, at+1)
        elif value == "\0":
            boundary()
        elif _latin_fold(value):
            output.append(_latin_fold(value))
        elif _encodable(value):
            output.append(value)
        else:
            latin = _latin_fold(value)
            compatible = unicodedata.normalize("NFKC", value)
            punctuation = _punctuation_fold(value)
            if latin:
                output.append(latin)
            elif compatible != value and "\0" not in compatible and _encodable(compatible):
                output.append(compatible)
            elif punctuation:
                output.append(punctuation)
            elif unicodedata.category(value) == "Nd":
                digit = unicodedata.digit(value, None)
                if digit is not None: output.append(str(digit))
            elif unicodedata.category(value).startswith("M"):
                pass
            else:
                boundary()
        at += 1
    converted = "".join(output)
    assert "\0" not in converted and _encodable(converted)
    return converted
