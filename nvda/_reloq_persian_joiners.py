"""Preserve Persian joiners and sentence boundaries through NVDA."""
from __future__ import annotations

import re
import unicodedata

import speech.commands as commands
from speech.commands import CharacterModeCommand, IndexCommand, LangChangeCommand, SynthCommand

from ._reloq_languages import language_for

_FA_PROFILES = frozenset((0x7f0000, 0x7f0001))
_MARKERS = {'\u200c': '\ue070', '\u200d': '\ue071', '\u061f': '\ue072', ':': '\ue073'}
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
    return any(isinstance(item, str) and any(char in item for char in _MARKERS) for item in sequence)


def emoji_selector_pending(sequence):
    return any(isinstance(item, str) and '\ufe0f' in item for item in sequence)


def protect_emoji_selectors(driver, sequence, *, symbol_level=None):
    """Avoid announcing a redundant emoji presentation selector in speech.

    Preserve the original if NVDA treats the whole emoji sequence specially,
    a speech dictionary matches, or the user is examining characters.
    """
    if not emoji_selector_pending(sequence):
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
            home, languages = driver._selectedLanguage, driver._languages
    except (ImportError, AttributeError, KeyError, TypeError, ValueError):
        return sequence
    selected, spelling, result, group = home, False, list(sequence), []

    def flush():
        if not group:
            return
        text = ''.join(value for _, value in group)
        # Index commands do not interrupt the logical text. Keep them in
        # place while recognizing a base and its selector across fragments.
        chars = list(text)
        for i in range(1, len(chars)):
            code = ord(chars[i - 1]) if chars[i - 1] else 0
            if chars[i] == '\ufe0f' and (0x2600 <= code <= 0x27bf or 0x1f000 <= code <= 0x1faff):
                chars[i] = ''
        stripped = ''.join(chars)
        if stripped == text or not _dictionary_untouched(text, stripped):
            group.clear()
            return
        try:
            locale = languages[selected].locale
            at, changed = 0, []
            for index, value in group:
                replacement = ''.join(chars[at:at + len(value)])
                at += len(value)
                if value == replacement:
                    continue
                before = characterProcessing.processSpeechSymbols(locale, value, level)
                after = characterProcessing.processSpeechSymbols(locale, replacement, level)
                if before.replace('\ufe0f', '') != after:
                    group.clear()
                    return
                changed.append((index, replacement))
            for index, replacement in changed:
                result[index] = replacement
        except (AttributeError, KeyError, LookupError, TypeError, ValueError):
            pass
        group.clear()

    for index, item in enumerate(sequence):
        if isinstance(item, LangChangeCommand):
            flush()
            chosen = language_for(item.lang, home, languages)
            if chosen is not None:
                selected = chosen
        elif isinstance(item, CharacterModeCommand):
            flush()
            spelling = bool(item.state)
        elif isinstance(item, str) and not spelling and selected in _FA_PROFILES:
            group.append((index, item))
        elif not isinstance(item, IndexCommand):
            flush()
    flush()
    return result


def prepare_persian_symbols(locale):
    """Let NVDA load its locale data when a Persian voice is selected.

    This stores no speech results: NVDA owns and invalidates the symbol
    processor, including custom symbol definitions, in the usual way.
    """
    if locale not in ('fa_IR', 'fa_Latn'):
        return
    try:
        import characterProcessing
        characterProcessing.processSpeechSymbols(locale, '', characterProcessing.SymbolLevel.NONE)
    except Exception:
        # A missing or broken NVDA symbol dictionary must not prevent voice
        # selection. The ordinary speech path still handles it normally.
        pass


def _persian_letter(char):
    cp = ord(char)
    return ((0x0600 <= cp <= 0x08ff or 0xfb50 <= cp <= 0xfeff) and
            unicodedata.category(char)[0] in ('L', 'M'))


def _colon_is_prose(text, index, previous, following):
    """Match the driver's sentence-colon exclusions across NVDA indexes."""
    left = previous + text[:index]
    if not left.strip():
        return False
    before = left[-1:]
    after = (text[index + 1:] or following)[:1]
    if before and after and before.isdecimal() and after.isdecimal():
        return False
    if after in ('/', '\\'):
        prefix = re.search(r'(?<![A-Za-z0-9+.-])([A-Za-z][A-Za-z0-9+.-]*)$', left)
        tail = text[index + 1:] or following
        if prefix and (len(prefix.group(1)) == 1 or tail[:2] == '//' or
                       len(tail) > 1 and not tail[1].isspace() or
                       prefix.group(1).casefold() in ('file', 'http', 'https', 'ftp')):
            return False
    return True


def _escape(text, *, question=False, colon=False, previous=''):
    # Literal private-use characters remain user text, never metadata.
    if any(marker in text for marker in _MARKERS.values()):
        return text
    chars = list(text)
    for index in range(len(chars)):
        if (chars[index] in ('\u200c', '\u200d') and 0 < index < len(chars) - 1 and
                _persian_letter(text[index - 1]) and _persian_letter(text[index + 1])):
            chars[index] = _MARKERS[chars[index]]
        elif question and chars[index] == '\u061f':
            before = text[index - 1] if index else previous[-1:]
            after = text[index + 1] if index + 1 < len(chars) else ''
            if (before and (before.isalnum() or unicodedata.category(before).startswith('M') or
                            before in ')]}»') and
                    (not after or after.isspace() or after in ')]}»')):
                chars[index] = _MARKERS[chars[index]]
        elif colon and chars[index] == ':':
            # Let NVDA speak its configured name, then restore a separate
            # punctuation mark for the core's sentence contour.
            chars[index] = ':' + _MARKERS[':']
    return ''.join(chars)


def _dictionary_untouched(original, escaped, *, ignored_sources=()):
    """Avoid hiding either a user dictionary match or a new marker match."""
    try:
        import speechDictHandler
        if hasattr(speechDictHandler, 'listAvailableSpeechDictDefinitions'):
            dictionaries = (entry.dictionary for entry in speechDictHandler.listAvailableSpeechDictDefinitions()
                            if entry.enabled and (not ignored_sources or
                               getattr(getattr(entry, 'source', None), 'value',
                                       getattr(entry, 'source', None)) not in ignored_sources))
        else:
            dictionaries = (dictionary for name, dictionary in speechDictHandler.dictionaries.items()
                            if name not in ignored_sources)
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
    """Preserve in-word joiners and otherwise silent sentence questions."""
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
    selected, spelling, result, previous = home, False, [], ''
    for item_index, item in enumerate(sequence):
        if isinstance(item, LangChangeCommand):
            chosen = language_for(item.lang, home, languages)
            if chosen is not None:
                selected = chosen
            previous = ''
        elif isinstance(item, CharacterModeCommand):
            spelling = bool(item.state)
            previous = ''
        elif isinstance(item, str) and not spelling and selected in _FA_PROFILES:
            following = ''
            for later in sequence[item_index + 1:]:
                if isinstance(later, str) and later:
                    following = later
                    break
                if not isinstance(later, IndexCommand):
                    break
            try:
                # Only intervene when the active NVDA symbol settings would
                # remove the question mark silently. A custom spoken name or
                # preserved character must retain the user's own setting.
                question, colon = False, False
                locale = languages[selected].locale
                if '\u061f' in item:
                    silent = not characterProcessing.processSpeechSymbols(locale, '\u061f', level).strip()
                    if silent:
                        processed = characterProcessing.processSpeechSymbols(locale, item, level)
                        without = characterProcessing.processSpeechSymbols(locale, item.replace('\u061f', ''), level)
                        question = ' '.join(processed.split()) == ' '.join(without.split())
                if item.count(':') == 1 and _colon_is_prose(item, item.index(':'), previous, following):
                    original = characterProcessing.processSpeechSymbols(locale, item, level)
                    if ':' not in original:
                        at = item.index(':') + 1
                        trial = item[:at] + _MARKERS[':'] + item[at:]
                        processed = characterProcessing.processSpeechSymbols(locale, trial, level)
                        # A context-sensitive custom symbol rule must see the
                        # same text and produce the same spoken description.
                        colon = (processed.count(_MARKERS[':']) == 1 and
                                 ' '.join(processed.replace(_MARKERS[':'], '').split()) ==
                                 ' '.join(original.split()))
            except (AttributeError, KeyError, LookupError, TypeError, ValueError):
                question, colon = False, False
            escaped = _escape(item, question=question, colon=colon, previous=previous)
            previous = (previous + item)[-256:]
            if escaped != item and _dictionary_untouched(item, escaped):
                try:
                    processed = characterProcessing.processSpeechSymbols(languages[selected].locale, escaped, level)
                    stable = all(processed.count(marker) == escaped.count(marker) for marker in _MARKERS.values())
                except (AttributeError, KeyError, TypeError, ValueError):
                    stable = False
                if stable:
                    result.extend((ReloqPersianJoinerGuard(True), escaped, ReloqPersianJoinerGuard(False)))
                    continue
        elif not isinstance(item, IndexCommand):
            previous = ''
        result.append(item)
    return result
