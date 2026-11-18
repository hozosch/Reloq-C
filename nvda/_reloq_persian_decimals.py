"""Keep English decimal points through Persian NVDA symbol processing."""
from __future__ import annotations

import re
import unicodedata

from speech.commands import CharacterModeCommand, IndexCommand, LangChangeCommand

from ._reloq_languages import language_for
from ._reloq_persian_joiners import _dictionary_untouched
from ._speech_tags import NativeDictionaryWord
from ._persian.pinglish_frontend import english_exception, latin_spans, technical_version_prefix

_FA, _PINGLISH = 0x7f0000, 0x7f0001
_DECIMAL = re.compile(r'(?<![0-9_])[+\-−]?(?:[0-9]{1,3}(?:,[0-9]{3})+|[0-9]+)(?:\.[0-9]+)+')


def dotted_version_word(selected, text, start, end):
    """Leave an acronym attached to a version for the decimal guard."""
    if selected not in (_FA, _PINGLISH) or not re.match(r'\.[0-9]', text[end:]):
        return False
    word = text[start:end]
    match = re.fullmatch(r'([A-Za-z]+)[0-9]+', word)
    return bool(match and (selected == _FA or technical_version_prefix(match.group(1))))


def decimal_pending(sequence):
    text = ''.join(item if isinstance(item, str) else item.text if isinstance(item, NativeDictionaryWord) else ''
                   for item in sequence)
    return '.' in text and any('0' <= char <= '9' for char in text)


def _builtin_split_prefixes(text, intervals):
    """Preserve NVDA's builtin result when guarding a decimal.

    A stock builtin rule inserts a space between a letter and a digit. Once
    the number is carried as a command that zero-width match would disappear;
    reproduce its space only if the complete dictionary result stays identical.
    Return None when an arbitrary rule cannot be preserved safely.
    """
    try:
        import speechDictHandler
        try:
            import globalVars
            if not getattr(globalVars, 'speechDictionaryProcessing', True):
                return {}
        except ImportError:
            pass
        if hasattr(speechDictHandler, 'listAvailableSpeechDictDefinitions'):
            dictionaries = tuple(definition.dictionary for definition in
                speechDictHandler.listAvailableSpeechDictDefinitions() if definition.enabled and
                getattr(definition.source, 'value', definition.source) == 'builtin')
        else:
            dictionaries = (speechDictHandler.dictionaries.get('builtin', ()),)
        if not any(dictionaries):
            return {}
        def process(value):
            for dictionary in dictionaries:
                for entry in dictionary:
                    if hasattr(entry, 'sub'):
                        value = entry.sub(value)
                    elif entry.compiled.search(value):
                        raise ValueError('dictionary replacement unavailable')
            return value
        expected, pieces, cursor = process(text), [], 0
        candidates = set()
        for left, right, replacement in sorted(intervals):
            if left < cursor or right < left:
                return None
            if (replacement is None and left and left < right and
                    re.fullmatch(r'[^\W\d]', text[left - 1]) and
                    '0' <= text[left] <= '9'):
                candidates.add(left)
            pieces.extend((process(text[cursor:left]), text[left:right]))
            cursor = right
        pieces.append(process(text[cursor:]))
        if ''.join(pieces) == expected:
            return {}
        if candidates:
            pieces, cursor = [], 0
            for left, right, _replacement in sorted(intervals):
                pieces.extend((process(text[cursor:left]), ' ' if left in candidates else '', text[left:right]))
                cursor = right
            pieces.append(process(text[cursor:]))
            if ''.join(pieces) == expected:
                return {left: ' ' for left in candidates}
        return None
    except Exception:
        # An unavailable or invalid NVDA dictionary must not block speech.
        return None


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
        return bool((previous and (english_exception(text[previous[0]:previous[1]]) or
                     (technical_version_prefix(text[previous[0]:previous[1]]) and previous[1] == start)) and
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

    selected, spelling, group, intervals, prefixes = home, False, [], {}, {}

    def flush():
        if not group or spelling or selected not in (_FA, _PINGLISH):
            group.clear()
            return
        text = ''.join(value for _, value, _ in group)
        if not _dictionary_untouched(text, text, ignored_sources=('builtin',)):
            group.clear()
            return
        group_intervals = {}
        for match in _DECIMAL.finditer(text):
            if not _english_number(text, match.start(), match.end(), selected):
                continue
            try:
                raw = match.group()
                signed = raw[0] in '+-−'
                number = raw[1:] if signed else raw
                # English treats a hyphen after a word character differently
                # from a leading minus sign. Supply that immediate context.
                prior = text[match.start() - 1:match.start()] if match.start() else ''
                prior = prior if prior and (prior.isalnum() or prior == '_') else ''
                processed = characterProcessing.processSpeechSymbols('en_US', prior + raw, level)
                if prior:
                    if not processed.startswith(prior):
                        continue
                    processed = processed[len(prior):]
                if not processed.endswith(number) or (not signed and processed != number):
                    continue
            except (AttributeError, KeyError, LookupError, TypeError, ValueError):
                continue
            # Keep NVDA's English handling of signs (e.g. "minus"), while
            # retaining the untouched decimal across intervening indexes.
            sign_end = match.start() + int(signed)
            regions = ([(match.start(), sign_end, processed[:-len(number)])] if signed else [])
            regions.append((sign_end, match.end(), None))
            position = 0
            for item_index, value, is_text in group:
                end = position + len(value)
                for region_start, region_end, replacement in regions:
                    left, right = max(position, region_start), min(end, region_end)
                    if is_text and left < right:
                        group_intervals.setdefault(item_index, []).append((left - position, right - position, replacement))
                position = end
        group_prefixes = {}
        for item_index, value, is_text in group:
            if is_text and item_index in group_intervals:
                found = _builtin_split_prefixes(value, group_intervals[item_index])
                if found is None:
                    group.clear()
                    return
                group_prefixes[item_index] = found
        for item_index, selected_intervals in group_intervals.items():
            intervals.setdefault(item_index, []).extend(selected_intervals)
        prefixes.update(group_prefixes)
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
        for left, right, replacement in intervals[index]:
            if left > cursor:
                result.append(item[cursor:left])
            protected = item[left:right] if replacement is None else replacement
            protected = prefixes.get(index, {}).get(left, '') + protected
            if protected:
                result.append(NativeDictionaryWord(protected))
            cursor = right
        if cursor < len(item):
            result.append(item[cursor:])
    return result
