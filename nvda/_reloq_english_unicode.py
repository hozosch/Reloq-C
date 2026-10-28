"""English preview Unicode admission and English character names.

The spelling readings are derived from the separately observed English
lexicon. Unknown running-text characters are ignored; spelling names their
script and hexadecimal code. Latin accents are named without transliteration
of other alphabets. This module never invokes the German text frontend.
"""
from __future__ import annotations
import unicodedata

# BEGIN OBSERVED READINGS -- generated from reloq_english_lexicon.inc
_READINGS = {'0': '`[.1zir.0o]', '1': '`[.1wHn]', '2': '`[.1tu]', '3': '`[.1Tri]', '4': '`[.1fcr]', '5': '`[.1fYv]', '6': '`[.1sIks]', '7': '`[.1sE.0vXn]', '8': '`[.1et]', '9': '`[.1nYn]', 'A': '`[.1e]', 'B': '`[.1bi]', 'C': '`[.1si]', 'D': '`[.1di]', 'E': '`[.1i]', 'F': '`[.1Ef]', 'G': '`[.1Ji]', 'H': '`[.1eC]', 'I': '`[.1Y]', 'J': '`[.1Je]', 'K': '`[.1ke]', 'L': '`[.1El]', 'M': '`[.1Em]', 'N': '`[.1En]', 'O': '`[.1o]', 'P': '`[.1pi]', 'Q': '`[.1kyu]', 'R': '`[.1ar]', 'S': '`[.1Es]', 'T': '`[.1ti]', 'U': '`[.1yu]', 'V': '`[.1vi]', 'W': '`[.1dH.0bxl.2yu]', 'X': '`[.1Eks]', 'Y': '`[.1wY]', 'Z': '`[.1zi]', 'above': '`[.0x.1bHv]', 'acute': '`[.0x.1kyut]', 'ampersand': '`[.1Am.0pR.2sAnd]', 'and': '`[.1And]', 'apostrophe': '`[.0x.1pa.0strx.0fi]', 'are': '`[.1ar]', 'at': '`[.1At]', 'bar': '`[.1bar]', 'below': '`[.0bX.1lo]', 'brace': '`[.1bres]', 'character': '`[.1ker.0Xk.0tR]', 'circumflex': '`[.1sR.0kxm.2flEks]', 'colon': '`[.1kol.0Xn]', 'd': '`[.1di]', 'dollar': '`[.1dal.0R]', 'dot': '`[.1dat]', 'double': '`[.1dH.0bxl]', 'equals': '`[.1i.0kwxlz]', 'euro': '`[.1yur.0o]', 'exclamation': '`[.2Ek.0sklx.1me.0SXn]', 'from': '`[.1frHm]', 'greek': '`[.1grik]', 'hook': '`[.1hUk]', 'i': '`[.1Y]', 'l': '`[.1El]', 'latin': '`[.1lA.0?N]', 'left': '`[.1lEft]', 'less': '`[.1lEs]', 'line': '`[.1lYn]', 'mark': '`[.1mark]', 'never': '`[.1nE.0vR]', 'new': '`[.1nu]', 'number': '`[.1nHm.0bR]', 'o': '`[.1o]', 'oe': '`[.1o]', 'of': '`[.1Hv]', 'other': '`[.1H.0DR]', 'percent': '`[.0pR.1sEnt]', 'period': '`[.1pir.0i.0Xd]', 'plus': '`[.1plHs]', 'pound': '`[.1pWnd]', 'preview': '`[.1pri.2vyu]', 'question': '`[.1kwEs.0CXn]', 'quote': '`[.1kwot]', 'right': '`[.1rYt]', 'ring': '`[.1rIG]', 'semicolon': '`[.1sE.0mi.2kol.0Xn]', 'sign': '`[.1sYn]', 'space': '`[.1spes]', 'stroke': '`[.1strok]', 'text': '`[.1tEkst]', 'than': '`[.1DAn]', 'the': '`[.1DH]', 'their': '`[.1Der]', 'underscore': '`[.1Hn.0dR.2skcr]', 'with': '`[.1wIT]', 'without': '`[.2wIT.1Wt]'}
# END OBSERVED READINGS

_SYMBOLS = {
    ' ': 'space', '\t': 'tab', '\n': 'new line', '.': 'period', ',': 'comma',
    '?': 'question mark', '!': 'exclamation mark', ':': 'colon', ';': 'semicolon',
    '-': 'dash', '_': 'underscore', '+': 'plus', '=': 'equals', '/': 'slash',
    '\\': 'backslash', '@': 'at', '#': 'number sign', '$': 'dollar sign',
    '%': 'percent', '&': 'ampersand', '*': 'asterisk', '(': 'left parenthesis',
    ')': 'right parenthesis', '[': 'left bracket', ']': 'right bracket',
    '{': 'left brace', '}': 'right brace', '<': 'less than', '>': 'greater than',
    "'": 'apostrophe', '"': 'quote', '`': 'backtick', '~': 'tilde',
    '^': 'caret', '|': 'vertical bar', '€': 'euro', '£': 'pound sign',
}
_GREEK = {word: word.lower() for word in (
    'ALPHA BETA GAMMA DELTA EPSILON ZETA ETA THETA IOTA KAPPA LAMBDA MU NU XI '
    'OMICRON PI RHO SIGMA TAU UPSILON PHI CHI PSI OMEGA').split()}
_GREEK.update(LAMDA='lambda', **{'FINAL SIGMA': 'sigma'})
_FOLDS = {'æ':'ae', 'œ':'oe', 'ø':'o', 'ł':'l', 'ð':'d', 'þ':'th', 'ß':'ss', 'ı':'i'}


def _latin(char):
    if not unicodedata.name(char, '').startswith('LATIN '):
        return None
    if char.lower() in _FOLDS:
        text = _FOLDS[char.lower()]
        return text.upper() if char.isupper() else text
    parts = unicodedata.normalize('NFKD', char)
    text = ''.join(c for c in parts if c.isascii() and c.isalpha())
    return text or None


def _greek(char):
    root = ''.join(c for c in unicodedata.normalize('NFD', char)
                   if not unicodedata.combining(c))
    if len(root) != 1:
        return None
    name = unicodedata.name(root, '')
    return _GREEK.get(name.split(' LETTER ', 1)[1]) if name.startswith('GREEK ') and ' LETTER ' in name else None


def normalize_generic_text(text):
    parts = []
    for char in text:
        if char.isascii():
            parts.append(char)
        elif char.isspace():
            parts.append(' ')
        elif _latin(char):
            parts.append(_latin(char))
        elif _greek(char):
            parts.append(' ' + _greek(char) + ' ')
        elif char in '“”„«»':
            parts.append('"')
        elif char in '‘’':
            parts.append("'")
        elif char in '–—−':
            parts.append('-')
        elif char in _SYMBOLS:
            parts.append(' ' + _SYMBOLS[char] + ' ')
        elif not unicodedata.combining(char):
            parts.append(' ')
    return ''.join(parts)


def _reading(words):
    result = []
    for word in words.split():
        if word in _READINGS:
            result.append(_READINGS[word])
        else:
            result.extend(_READINGS[c.upper()] for c in word if c.isascii() and c.isalnum())
    return ' '.join(result)


def normalize_spelling_text(text):
    result = []
    for char in text:
        if char.isascii() and char.isalnum():
            result.append(_READINGS[char.upper()])
        elif char in _SYMBOLS:
            result.append(_reading(_SYMBOLS[char]))
        elif _latin(char):
            result.extend(_READINGS[c.upper()] for c in _latin(char))
            name = unicodedata.name(char, '')
            if ' WITH ' in name:
                result.append(_reading('with ' + name.split(' WITH ', 1)[1].lower()))
        elif _greek(char):
            result.append(_reading(_greek(char)))
        else:
            name = unicodedata.name(char, '')
            script = name.split(' ', 1)[0].lower() if unicodedata.category(char).startswith('L') else ''
            result.append(_reading(script + ' character'))
            result.extend(_READINGS[c] for c in f'{ord(char):X}')
    return ' '.join(result)
