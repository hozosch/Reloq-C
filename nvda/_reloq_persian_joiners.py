"""Preserve Persian in-word joiners through NVDA's symbol processing."""
from __future__ import annotations

import unicodedata

import speech.commands as commands
from speech.commands import CharacterModeCommand, LangChangeCommand, SynthCommand

from ._reloq_languages import language_for

_FA_PROFILES = frozenset((0x7f0000, 0x7f0001))
_MARKERS = {'\u200c': '\ue070', '\u200d': '\ue071'}
_RESTORE = str.maketrans({value: key for key, value in _MARKERS.items()})


class ReloqPersianJoinerGuard(SynthCommand):
    """Mark only a temporarily escaped text item for restoration."""
    def __init__(self, enabled):
        self.enabled = bool(enabled)

    def __repr__(self):
        return f'ReloqPersianJoinerGuard({self.enabled!r})'


if not hasattr(commands, 'ReloqPersianJoinerGuard'):
    commands.ReloqPersianJoinerGuard = ReloqPersianJoinerGuard


def restore_joiners(text):
    return text.translate(_RESTORE)


def joiner_pending(sequence):
    return any(isinstance(item, str) and ('\u200c' in item or '\u200d' in item) for item in sequence)


def _persian_letter(char):
    cp = ord(char)
    return ((0x0600 <= cp <= 0x08ff or 0xfb50 <= cp <= 0xfeff) and
            unicodedata.category(char)[0] in ('L', 'M'))


def _escape(text):
    # Literal private-use characters remain user text, never metadata.
    if any(marker in text for marker in _MARKERS.values()):
        return text
    chars = list(text)
    for index in range(1, len(chars) - 1):
        if (chars[index] in _MARKERS and _persian_letter(chars[index - 1]) and
                _persian_letter(chars[index + 1])):
            chars[index] = _MARKERS[chars[index]]
    return ''.join(chars)


def _dictionary_untouched(original, escaped):
    """Avoid hiding either a user dictionary match or a new marker match."""
    try:
        import speechDictHandler
        if hasattr(speechDictHandler, 'listAvailableSpeechDictDefinitions'):
            dictionaries = (entry.dictionary for entry in speechDictHandler.listAvailableSpeechDictDefinitions()
                            if entry.enabled)
        else:
            dictionaries = speechDictHandler.dictionaries.values()
        for dictionary in dictionaries:
            if not dictionary:
                continue
            for entry in dictionary:
                if entry.compiled.search(original) or entry.compiled.search(escaped):
                    return False
        return True
    except (ImportError, AttributeError, TypeError, ValueError):
        return False


def protect_joiners(driver, sequence, *, symbol_level=None):
    """Protect only internal Persian joiners, retaining NVDA dictionaries."""
    if not joiner_pending(sequence):
        return sequence
    try:
        import characterProcessing
        import config
        level = symbol_level
        if level is None or level == characterProcessing.SymbolLevel.UNCHANGED:
            level = characterProcessing.SymbolLevel(config.conf['speech']['symbolLevel'])
        if level == characterProcessing.SymbolLevel.CHAR:
            return sequence
        with driver._submit_lock:
            home = driver._selectedLanguage
            languages = driver._languages
    except (ImportError, AttributeError, KeyError, TypeError, ValueError):
        return sequence
    selected, spelling, result = home, False, []
    for item in sequence:
        if isinstance(item, LangChangeCommand):
            chosen = language_for(item.lang, home, languages)
            if chosen is not None:
                selected = chosen
        elif isinstance(item, CharacterModeCommand):
            spelling = bool(item.state)
        elif isinstance(item, str) and not spelling and selected in _FA_PROFILES:
            escaped = _escape(item)
            if escaped != item and _dictionary_untouched(item, escaped):
                try:
                    processed = characterProcessing.processSpeechSymbols(languages[selected].locale, escaped, level)
                    stable = all(processed.count(marker) == escaped.count(marker) for marker in _MARKERS.values())
                except (AttributeError, KeyError, TypeError, ValueError):
                    stable = False
                if stable:
                    result.extend((ReloqPersianJoinerGuard(True), escaped, ReloqPersianJoinerGuard(False)))
                    continue
        result.append(item)
    return result
