"""Keep English decimal points through Persian NVDA symbol processing."""
from __future__ import annotations

import re
import unicodedata

from speech.commands import CharacterModeCommand, IndexCommand, LangChangeCommand

from ._reloq_languages import language_for
from ._reloq_persian_joiners import _dictionary_untouched
from ._speech_tags import NativeDictionaryWord
from ._persian.pinglish_frontend import english_exception, latin_spans

_FA, _PINGLISH = 0x7f0000, 0x7f0001
_DECIMAL = re.compile(r'(?<![0-9_])(?:[0-9]{1,3}(?:,[0-9]{3})+|[0-9]+)(?:\.[0-9]+)+')


def decimal_pending(sequence):
    text = ''.join(item if isinstance(item, str) else item.text if isinstance(item, NativeDictionaryWord) else ''
                   for item in sequence)
    return '.' in text and any('0' <= char <= '9' for char in text)


def _english_number(text, start, end, language):
    if language == _FA:
        letters = [(index, unicodedata.name(char, '').startswith('LATIN '))
                   for index, char in enumerate(text) if char.isalpha()]
        previous = next((english for index, english in reversed(letters) if index < start), None)
        following = next((english for index, english in letters if index >= end), None)
        return previous if previous is not None else bool(following)
    if language == _PINGLISH:
        words = latin_spans(text)
        previous = next(((left, right) for left, right in reversed(words) if right <= start), None)
        following = next(((left, right) for left, right in words if left >= end), None)
        return bool((previous and english_exception(text[previous[0]:previous[1]]) and
                     not text[previous[1]:start].strip()) or
                    (previous is None and following and english_exception(text[following[0]:following[1]]) and
                     not text[end:following[0]].strip()))
    return False


def protect_decimals(driver, sequence, *, symbol_level=None):
    """Pass English decimals intact; leave dictionaries and spelling in charge."""
    if not decimal_pending(sequence):
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

    selected, spelling, group, intervals = home, False, [], {}

    def flush():
        if not group or spelling or selected not in (_FA, _PINGLISH):
            group.clear()
            return
        text = ''.join(value for _, value, _ in group)
        if not _dictionary_untouched(text, text):
            group.clear()
            return
        for match in _DECIMAL.finditer(text):
            if not _english_number(text, match.start(), match.end(), selected):
                continue
            try:
                if characterProcessing.processSpeechSymbols('en_US', match.group(), level) != match.group():
                    continue
            except (AttributeError, KeyError, LookupError, TypeError, ValueError):
                continue
            position = 0
            for item_index, value, is_text in group:
                end = position + len(value)
                left, right = max(position, match.start()), min(end, match.end())
                if is_text and left < right:
                    intervals.setdefault(item_index, []).append((left - position, right - position))
                position = end
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
        elif isinstance(item, str):
            group.append((index, item, True))
        elif isinstance(item, NativeDictionaryWord):
            group.append((index, item.text, False))
        elif not isinstance(item, IndexCommand):
            flush()
    flush()

    result = []
    for index, item in enumerate(sequence):
        if index not in intervals:
            result.append(item)
            continue
        cursor = 0
        for left, right in intervals[index]:
            if left > cursor:
                result.append(item[cursor:left])
            result.append(NativeDictionaryWord(item[left:right]))
            cursor = right
        if cursor < len(item):
            result.append(item[cursor:])
    return result
