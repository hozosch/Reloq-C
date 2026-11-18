"""Bounded, in-memory version of the accepted Persian sentence planner.

The native frame capture and replay own cancellation and voice settings. This
module has no NumPy, temporary files, global environment changes or helper
processes. The driver supplies verified source positions and Persian tags.
"""
from __future__ import annotations

from bisect import bisect_left, bisect_right
import ctypes
from dataclasses import dataclass
import re
import struct

if __package__:
    from .._reloq_eci_direct import EciIndex, EciText, SAMPLE_RATE, SAMPLE_RATES
    from .._reloq_persian_frames import FrameCapture, NativeFrame
    from .._reloq_persian_glide import TRACT, apply_u_a_glides, plan_u_a_glides
    from .text_frontend import native_sequence_accepted, native_tag_accepted
else:
    from _reloq_eci_direct import EciIndex, EciText, SAMPLE_RATE, SAMPLE_RATES
    from _reloq_persian_frames import FrameCapture, NativeFrame
    from _reloq_persian_glide import TRACT, apply_u_a_glides, plan_u_a_glides
    from text_frontend import native_sequence_accepted, native_tag_accepted

_TAG = re.compile(r'`\[([^]]+)\]')
_SYLLABLE = re.compile(r'\.([012])([^.]+)')
_FINAL_U = re.compile(r"(?:u|'u:')\]$")
_INITIAL_A = re.compile(r"^`\[\.[012](?:a|'a:')")
_EN_TO_FA = {**{c: c for c in 'aAeioubdfghjklmnoprstvzSZ'},
             'E': 'e', 'I': 'i', 'O': 'o', 'U': 'u', 'c': 'a',
             'H': 'A', 'x': 'A', 'X': 'e', 'Y': 'A', 'y': 'j',
             'w': 'v', 'G': 'n', 'D': 'd', 'T': 't', 'F': 't'}
_FA_TO_EN = {**{c: c for c in 'aAioubdfghklmnprstvzSZ'},
             'e': 'E', 'j': 'y', 'G': 'h', 'R': 'k', 'J': 'y', 'C': 'S'}
_EN_VOWELS = {'i': 'i', 'I': 'i', 'e': 'e', 'E': 'A', 'a': 'a', 'A': 'a',
              'o': 'o', 'O': 'o', 'u': 'u', 'U': 'u', 'x': 'A', 'X': 'A',
              'y': 'i', 'Y': 'a', '@': 'A', 'V': 'A'}
_GENERATED_FA_PROXY = re.compile(r'`\[(?:\.[012]m[Aaeiou])+\]')


def word_boundary(text, matches, word):
    """Place private word marks before weak controls or punctuation gaps."""
    start, end = matches[word - 1].end(), matches[word].start()
    gap = text[start:end]
    if any(char in gap for char in '.,?!:;()[]{}'):
        return start
    weak = re.search(r'`00\s*$', gap)
    return start + weak.start() if weak else end


def _f32(value):
    return struct.unpack('=f', struct.pack('=f', value))[0]


def _converted(text, table):
    result = []
    for match in _TAG.finditer(text):
        chars = []
        for ch in match.group(1):
            if ch in '.012':
                chars.append(ch)
            elif ch in "':":
                continue
            elif ch in table:
                chars.append(table[ch])
            else:
                return None
        result.append('`[' + ''.join(chars) + ']')
    return result or None


def _display_punctuation(display, tags):
    matches = list(_TAG.finditer(display))
    if len(matches) != len(tags):
        raise ValueError('proxy word count does not match native display')
    result, previous = [], 0
    for index, (match, tag) in enumerate(zip(matches, tags)):
        marks = ''.join(c for c in display[previous:match.start()] if c in '.,?!:;')
        result.append((marks + ' ' if index else marks) + tag)
        previous = match.end()
    result.append(''.join(c for c in display[previous:] if c in '.,?!:;'))
    return ''.join(result)


def _punctuation(text):
    found = re.search(r'([.?!,;:])\s*$', text)
    return found.group(1) if found else ''


def _bracketed_proxy(text, convert):
    """Keep the native bracket boundaries around each English proxy run."""
    if not any(char in text for char in '()[]{}'):
        return None
    parts = []
    for fragment in re.split(r'([()\[\]{}])', text):
        if not fragment:
            continue
        if len(fragment) == 1 and fragment in '()[]{}':
            parts.append(fragment)
            continue
        stripped = fragment.strip()
        if not stripped or not any(char.isalnum() for char in stripped):
            parts.append(fragment)
            continue
        left = len(fragment) - len(fragment.lstrip())
        right = len(fragment.rstrip())
        parts.extend((fragment[:left], convert(stripped), fragment[right:]))
    return ''.join(parts)


def english_proxy(en, text):
    bracketed = _bracketed_proxy(text, lambda value: english_proxy(en, value))
    if bracketed is not None:
        return bracketed
    shown = en._phonemes_for_probe(text)
    result = []
    for match in _TAG.finditer(shown):
        syllables = _SYLLABLE.findall(match.group(1))
        if not syllables:
            raise ValueError('English proxy has no syllables')
        result.append('`[' + ''.join('.' + stress + 'm' +
                                     next((_EN_VOWELS[c] for c in body if c in _EN_VOWELS), 'A')
                                     for stress, body in syllables) + ']')
    if not result:
        raise ValueError('English proxy has no words')
    return (_display_punctuation(shown, result) if re.search(r'[.?!,;:]\s*\S', text)
            else ' '.join(result) + _punctuation(text))


def english_phone_proxy(en, fa, text):
    bracketed = _bracketed_proxy(text, lambda value: english_phone_proxy(en, fa, value))
    if bracketed is not None:
        return bracketed
    shown = en._phonemes_for_probe(text)
    tags = _converted(shown, _EN_TO_FA)
    if tags:
        # One native parse can verify a whole generated proxy, avoiding a
        # separate synthesis for every letter of acronyms such as NVDA.
        # If context changes a display, retain the established per-tag test.
        combined = ' '.join(tags)
        accepted = native_sequence_accepted(combined, fa._phonemes_for_probe(combined))
        if not accepted:
            accepted = all(native_tag_accepted(tag, fa._phonemes_for_probe(tag)) for tag in tags)
    else:
        accepted = False
    if accepted:
        return (_display_punctuation(shown, tags) if re.search(r'[.?!,;:]\s*\S', text)
                else ' '.join(tags) + _punctuation(text))
    return english_proxy(en, text)


def persian_proxy_for_english(text, en, *, mark_weak=False):
    # The English pilot cannot time long spans composed entirely of .0
    # syllables reliably: they can stretch manyfold or fail synthesis. Give
    # each pilot-only word a uniform anchor. The separately captured Persian
    # speech keeps its unmarked syllables and its own timing.
    if not mark_weak and _TAG.search(text) and not re.search(r'\.[12]', text):
        text = _TAG.sub(lambda match: match.group().replace('.0', '.1', 1), text)
    def word_controls(value):
        if not mark_weak:
            return value
        # The US parser otherwise promotes an all-zero tag to a stressed
        # word. Its own weak-word control preserves the Persian phrase role.
        output, cursor = [], 0
        for match in _TAG.finditer(value):
            output.append(value[cursor:match.start()])
            marks = _SYLLABLE.findall(match.group(1))
            if marks and all(stress == '0' for stress, _ in marks) and not re.search(
                    r'`00\s*$', value[:match.start()]):
                output.append('`00 ')
            output.append(match.group())
            cursor = match.end()
        output.append(value[cursor:])
        return ''.join(output)
    matches = tuple(_TAG.finditer(text))
    if not matches:
        raise ValueError('Persian proxy has no words')
    def compatible(tag, displayed):
        expected = _SYLLABLE.findall(tag[2:-1])
        actual = _SYLLABLE.findall(displayed)
        if not expected or len(expected) != len(actual):
            return False
        for (stress, phones), (seen_stress, seen) in zip(expected, actual):
            # The native US reader may turn an intervocalic /d/ or /t/ into
            # its flap F. This keeps the same syllable and a stop-like timing;
            # it is the silent contour pilot, never the Persian pronunciation.
            if stress == '1' and seen_stress != '1':
                return False
            if len(phones) != len(seen) or any(a != b and not (a in 'dt' and b == 'F')
                                                for a, b in zip(phones, seen)):
                return False
        return True

    converted = []
    for match in matches:
        value = _converted(match.group(), _FA_TO_EN)
        converted.append(value[0] if value else None)
    # Keep each safe consonant-bearing word independently. One unsupported
    # word must not make every neighbouring word use a neutral m-vowel proxy.
    shown = (_TAG.findall(en._phonemes_for_probe(' '.join(converted)))
             if all(converted) else ())
    batch_ok = len(shown) == len(converted) and all(
        compatible(tag, item) for tag, item in zip(converted, shown))
    vowels = {'A': 'A', 'a': 'a', 'e': 'E', 'i': 'i', 'o': 'o', 'u': 'u'}
    result, weak_fallback = [], []
    for index, match in enumerate(matches):
        syllables = _SYLLABLE.findall(match.group(1))
        if not syllables:
            raise ValueError('Persian proxy has no syllables')
        tag = converted[index]
        if tag is not None and not batch_ok:
            display = _TAG.findall(en._phonemes_for_probe(tag))
            if len(display) != 1 or not compatible(tag, display[0]):
                tag = None
        if tag is None:
            # The US parser spells a multi-syllable tag which has no primary
            # stress, including a secondary-only number group. Promote one
            # pilot-only syllable to make the control syntactically valid;
            # a weak-word control retains its role in the phrase.
            needs_anchor = len(syllables) > 1 and not any(s == '1' for s, _ in syllables)
            anchor = next((i for i, (s, _) in enumerate(syllables) if s == '2'), 0)
            tag = '`[' + ''.join('.' + ('1' if needs_anchor and i == anchor else stress) + 'm' +
                                  next((vowels[c] for c in body if c in vowels), 'A')
                                  for i, (stress, body) in enumerate(syllables)) + ']'
            weak_fallback.append(needs_anchor)
        else:
            weak_fallback.append(False)
        result.append(tag)
    cursor, output = 0, []
    for match, tag, weak in zip(matches, result, weak_fallback):
        output.append(text[cursor:match.start()])
        if weak and mark_weak and not re.search(r'`00\s*$', text[:match.start()]):
            output.append('`00 ')
        output.append(tag)
        cursor = match.end()
    output.append(text[cursor:])
    return word_controls(''.join(output))


def _spelling_proxy(text, segment, spelling_spans):
    spans = [(start, end) for item, start, end in spelling_spans if item == segment]
    if not spans:
        return text
    # The proxy only supplies timing and contour. Actual English characters
    # are still sent to the English engine in its own spelling mode.
    return ''.join(char + ' ' if any(a <= pos < b for a, b in spans) and not char.isspace() else char
                   for pos, char in enumerate(text))


def _context(parts, language, en, fa, word_markers, source_marks=(), guarded_spans=(), spelling_spans=()):
    result = []
    for i, part in enumerate(parts):
        if i:
            result.extend((' ', EciIndex(i)))
        if part.language == language:
            inserts = [(offset, order, mark) for segment, offset, order, mark, _ in source_marks
                       if segment == i]
            if language == 'fa':
                matches = tuple(_TAG.finditer(part.text))
                for word, match in enumerate(matches):
                    if not word or (i, word) not in word_markers:
                        continue
                    boundary = word_boundary(part.text, matches, word)
                    inserts.append((boundary, -1, word_markers[i, word]))
            cursor = 0
            guarded = [(start, end) for segment, start, end in guarded_spans if segment == i]
            spelled = [(start, end) for segment, start, end in spelling_spans if segment == i]
            def append_text(start, end):
                if not guarded and not spelled:
                    result.append(part.text[start:end])
                    return
                cuts = sorted({start, end, *(p for a, b in (*guarded, *spelled) for p in (a, b)
                                            if start < p < end)})
                for left, right in zip(cuts, cuts[1:]):
                    piece = part.text[left:right]
                    protect = any(a <= left and right <= b for a, b in guarded)
                    spelling = any(a <= left and right <= b for a, b in spelled)
                    result.append(EciText(piece, character_mode=spelling, segmentation_guarded=protect)
                                  if protect or spelling else piece)
            for offset, _order, mark in sorted(inserts):
                if offset < cursor or offset > len(part.text):
                    raise ValueError('invalid source index offset')
                append_text(cursor, offset)
                result.append(EciIndex(mark))
                if (language == 'fa' and mark in word_markers.values() and offset < len(part.text) and
                        part.text[offset] in '.,?!:;()[]{}'):
                    result.append(' ')
                cursor = offset
            append_text(cursor, len(part.text))
        elif language == 'fa':
            result.append(english_phone_proxy(en, fa, _spelling_proxy(part.text, i, spelling_spans)))
        else:
            result.append(persian_proxy_for_english(part.text, en))
    return result


def _word_markers(parts):
    markers, next_index = {}, len(parts)
    for i, part in enumerate(parts):
        if part.language == 'fa':
            for word, _ in enumerate(_TAG.finditer(part.text)):
                if word:
                    markers[i, word] = next_index
                    next_index += 1
    return markers


def indexed_us_proxy(tagged, en, marks):
    """Make a US prosody proxy with marks before whole Persian words."""
    proxy = persian_proxy_for_english(tagged, en, mark_weak=True)
    # Some Unicode spaces are preserved by the Persian text projection but
    # named by the English parser. They are ordinary word gaps in a pilot.
    proxy = ''.join(' ' if char.isspace() and char not in '\t\r\n' else char for char in proxy)
    # The Persian voice's bracket pauses intentionally share the comma
    # contour. Preserve that choice in the US pilot without touching tags.
    proxy = re.sub(r'`\[[^]]+\]|[()\[\]{}]',
                   lambda match: match.group() if match.group().startswith('`[') else ',', proxy)
    matches = tuple(_TAG.finditer(proxy))
    if len(matches) != len(marks) + 1:
        raise ValueError('US proxy changed the Persian word count')
    result, previous = [], 0
    for i, (match, mark) in enumerate(zip(matches[1:], marks), 1):
        boundary = word_boundary(proxy, matches, i)
        result.extend((proxy[previous:boundary], EciIndex(mark)))
        if boundary < len(proxy) and proxy[boundary] in '.,?!:;()[]{}':
            # A space before punctuation leaves the sound unchanged but
            # keeps the provider's index callback at the preceding word.
            result.append(' ')
        previous = boundary
    result.append(proxy[previous:])
    return tuple(result)


def _last_audible(pcm, rate, *, native=None):
    if native is not None:
        sample = native(pcm, len(pcm))
        if sample < 0:
            raise ValueError('silent or invalid planning pass')
        return round(sample * 1000 / rate)
    for pos in range(len(pcm) - 2, -1, -2):
        if abs(struct.unpack_from('<h', pcm, pos)[0]) > 4:
            return round((pos // 2 + 1) * 1000 / rate)
    raise ValueError('silent planning pass')


def _interp(t, ticks, values):
    i = bisect_right(ticks, t)
    if not i:
        return float(values[0])
    if i >= len(ticks):
        return float(values[-1])
    a, b = i - 1, i
    return float(values[a]) + (float(values[b]) - float(values[a])) * (t - ticks[a]) / (ticks[b] - ticks[a])


def _interp_frame(t, item, column):
    """Interpolate a column without copying the entire sentence for every bridge."""
    i = bisect_right(item.ticks, t)
    if not i:
        return float(item.frames[0].controls[column])
    if i >= len(item.ticks):
        return float(item.frames[-1].controls[column])
    a, b = i - 1, i
    first = float(item.frames[a].controls[column])
    last = float(item.frames[b].controls[column])
    return first + (last - first) * (t - item.ticks[a]) / (item.ticks[b] - item.ticks[a])


def _frame_coordinates(item, positions):
    """Locate shared interpolation intervals once for all formants."""
    result = []
    for t in positions:
        index = bisect_right(item.ticks, t)
        if not index:
            result.append((0, 0, 0, 1))
        elif index >= len(item.ticks):
            last = len(item.ticks) - 1
            result.append((last, last, 0, 1))
        else:
            first = index - 1
            result.append((first, index, t - item.ticks[first], item.ticks[index] - item.ticks[first]))
    return result


def _frame_value(item, column, coordinates):
    first, last, offset, span = coordinates
    a = float(item.frames[first].controls[column])
    if first == last:
        return a
    b = float(item.frames[last].controls[column])
    # Keep the same evaluation order as the original native-frame bridge.
    return a + (b - a) * offset / span


@dataclass
class CapturedRows:
    pcm: bytes
    indexes: dict[int, int]
    frames: object
    ticks: list[int]
    original_pitch: object = None
    syllables: tuple[tuple[int, int], ...] = ()

    @classmethod
    def from_capture(cls, value, *, native=None):
        if value is None:
            raise RuntimeError('native planning was cancelled')
        frames = value.frames()
        if native is not None and frames:
            ticks = (ctypes.c_int64 * len(frames))()
            if native(frames, len(frames), ticks):
                raise RuntimeError('invalid native planning clock')
        else:
            ticks = [row.tick for row in frames]
            if not ticks or ticks[0] < 0 or any(a >= b for a, b in zip(ticks, ticks[1:])):
                raise RuntimeError('invalid native planning clock')
        indexes = {}
        for index, sample in value.indexes:
            if index in indexes:
                raise RuntimeError('duplicate planning mark')
            indexes[index] = sample
        return cls(value.pcm, indexes, frames, ticks, syllables=value.syllables)


def _edges(item, count):
    result = [0] + [item.indexes[i] for i in range(1, count)] + [len(item.pcm) // 2]
    if any(a >= b for a, b in zip(result, result[1:])):
        raise RuntimeError('invalid native context boundaries')
    return tuple(result)


def _dense(item, column, check=None, *, native=None):
    if native is not None:
        if check:
            check()
        result = (ctypes.c_float * (item.ticks[-1] + 1))()
        if native(item.frames, len(item.frames), column, result, len(result)):
            raise RuntimeError('invalid native dense contour')
        if check:
            check()
        return result
    values = [row.controls[column] for row in item.frames]
    result, ticks, cursor = [], item.ticks, 0
    for i in range(item.ticks[-1] + 1):
        if check and not i % 1024:
            check()
        while cursor < len(ticks) and ticks[cursor] <= i:
            cursor += 1
        if not cursor:
            value = values[0]
        elif cursor >= len(ticks):
            value = values[-1]
        else:
            a, b = cursor - 1, cursor
            value = float(values[a]) + (float(values[b]) - float(values[a])) * (i - ticks[a]) / (ticks[b] - ticks[a])
        result.append(_f32(value))
    return result


def _bracket_boundary(parts, edge):
    """Recognize source brackets without confusing them with phone tags."""
    left = _TAG.sub('x', parts[edge - 1].text).rstrip()
    right = _TAG.sub('x', parts[edge].text).lstrip()
    return bool((left and left[-1] in '()[]{}') or (right and right[0] in '()[]{}'))


def _fit_context_pitch(item, bounds, pilot_pitch, pilot_bounds, parts, language, rate, check=None, *,
                       native=None, native_edges=None, only=None, retain_brackets=True):
    av = None
    for i, part in enumerate(parts):
        if check:
            check()
        if part.language != language or only is not None and i not in only:
            continue
        # Brackets already supply native phrase boundaries and pauses. When
        # both of this segment's language boundaries are bracket boundaries,
        # retain its own language's native pitch rather than fitting it to
        # the other language. A connected edge continues to use the shared
        # contour, including the previously selected NVDA-to-Ach transition.
        adjoining = tuple(edge for edge in (i, i + 1) if 0 < edge < len(parts))
        if retain_brackets and adjoining and all(_bracket_boundary(parts, edge) for edge in adjoining):
            continue
        start = round(bounds[i] * 1000 / rate)
        end = min(round(bounds[i + 1] * 1000 / rate), item.ticks[-1] + 1)
        if not 0 <= start < end <= item.ticks[-1] + 1:
            raise RuntimeError('invalid native pitch interval')
        a, b = pilot_bounds[i], pilot_bounds[i + 1]
        if not 0 <= a < b < len(pilot_pitch):
            raise RuntimeError('invalid shared pitch interval')
        # The Finnish-based pilot remains the Persian contour. Keep English's
        # own word accents and blend only around actual, unbracketed switches.
        edges = 0
        if language == 'en-us':
            if i and parts[i - 1].language != language and not _bracket_boundary(parts, i):
                edges |= 1
            if i + 1 < len(parts) and parts[i + 1].language != language and not _bracket_boundary(parts, i + 1):
                edges |= 2
        if edges and item.original_pitch is None:
            item.original_pitch = (ctypes.c_float * len(item.frames))(*(row.controls[1] for row in item.frames))
        if edges and native_edges is not None:
            status = native_edges(item.frames, len(item.frames), pilot_pitch, len(pilot_pitch),
                                  item.original_pitch, start, end, a, b, edges, 45)
            if status:
                raise RuntimeError('no voiced pitch anchor' if status == 1 else 'invalid native pitch interval')
            if check:
                check()
            continue
        if native is not None and not edges:
            status = native(item.frames, len(item.frames), pilot_pitch, len(pilot_pitch), start, end, a, b)
            if status:
                raise RuntimeError('no voiced pitch anchor' if status == 1 else 'invalid native pitch interval')
            if check:
                check()
            continue
        # Only the first and last voiced milliseconds are needed. Search
        # from each edge instead of allocating two dense sentence arrays.
        if av is None:
            av = [row.controls[2] for row in item.frames]
        first = next((j for j in range(start, end) if _interp(j, item.ticks, av) > 0), None)
        last = next((j for j in range(end - 1, start - 1, -1) if _interp(j, item.ticks, av) > 0), None)
        if first is None or last is None or first >= last:
            raise RuntimeError('no voiced pitch anchor')
        for number, row in enumerate(item.frames):
            if check and not number % 1024:
                check()
            tick = row.tick
            if tick < start or tick >= end:
                continue
            t = a if tick < first else b if tick > last else a + (b - a) * (tick - first) / (last - first)
            lo = int(t)
            target = (pilot_pitch[lo] if lo >= len(pilot_pitch) - 1 else
                      pilot_pitch[lo] + (pilot_pitch[lo + 1] - pilot_pitch[lo]) * (t - lo))
            if edges:
                weight = 0.0
                if edges & 1:
                    x = min(1.0, max(0.0, (tick - first) / 45))
                    weight = 1.0 - x * x * (3.0 - 2.0 * x)
                if edges & 2:
                    x = min(1.0, max(0.0, (last - tick) / 45))
                    weight = max(weight, 1.0 - x * x * (3.0 - 2.0 * x))
                original = item.original_pitch[number]
                target = original + weight * (target - original)
            row.controls[1] = _f32(target)


def _syllable_edges(item, start, end, expected):
    """Return observed internal boundaries only for a verifiable word span."""
    if expected < 2 or not item.syllables:
        return ()
    spans = item.syllables
    # A later sentence can reset the provider's local clock. Its ambiguous
    # offset must not invalidate the already verified first sentence, whose
    # syllables retain the original request clock.
    for i, (a, b) in enumerate(spans):
        if a < 0 or a >= b:
            return ()
        if i and a < spans[i-1][1]:
            spans = spans[:i]
            break
    selected = [(a, b) for a, b in spans if start * 20000 <= a + b < end * 20000]
    if len(selected) != expected:
        return ()
    return tuple(round((left[1] + right[0]) / 20000)
                 for left, right in zip(selected, selected[1:]))


def _syllable_word_edges(item, bounds, counts):
    """Resolve sentence-local clocks against independently captured word marks."""
    if (not item.syllables or len(bounds) != len(counts) + 1 or
            any(n < 1 for n in counts) or sum(counts) != len(item.syllables)):
        return None
    groups, pos, offset, previous = [], 0, 0, None
    for i, count in enumerate(counts):
        group = []
        for j, (a, b) in enumerate(item.syllables[pos:pos + count]):
            if a < 0 or a >= b:
                return None
            if previous is not None and a < previous:
                if j or i + 1 >= len(counts):
                    return None
                # A mark before punctuation can precede the inter-sentence
                # pause, not the following word. The next word boundary and
                # the measured end of this first word locate the new clock.
                # A final single-word sentence has no such mark and falls
                # back to the established word-level transfer.
                offset = bounds[i + 1] * 10000 - item.syllables[pos + count - 1][1]
                if offset < 0:
                    return None
            previous = b
            a, b = a + offset, b + offset
            if not bounds[i] * 20000 <= a + b < bounds[i + 1] * 20000:
                return None
            group.append((a, b))
        groups.append(tuple(round((left[1] + right[0]) / 20000)
                            for left, right in zip(group, group[1:])))
        pos += count
    return tuple(groups)


def fit_us_word_pitch(item, bounds, donor, donor_bounds, target_markers, donor_markers,
                      rate, capture_api, check=None, *, syllable_counts=()):
    """Fit complete US word contours, including each final voiced vowel.

    Markers belong to the same generated Persian words in both captures.
    The source language supplies the frame timing and all acoustic controls
    other than F0. If a word boundary is unavailable, leave the caller's
    existing phrase-level fit intact rather than guessing an alignment.
    """
    if capture_api.fit_pitch is None or capture_api.dense_curve is None:
        return False
    count = len(target_markers) + 1
    if len(donor_markers) + 1 != count:
        raise ValueError('US/Persian word markers differ')
    target = [round(bounds[0] * 1000 / rate)]
    target.extend(round(item.indexes[m] * 1000 / rate) for m in target_markers)
    target.append(min(round(bounds[1] * 1000 / rate), item.ticks[-1] + 1))
    source = [round(donor_bounds[0] * 1000 / rate)]
    source.extend(round(donor.indexes[m] * 1000 / rate) for m in donor_markers)
    source.append(min(round(donor_bounds[1] * 1000 / rate), donor.ticks[-1] + 1))
    if (any(a >= b for a, b in zip(target, target[1:])) or
            any(a >= b for a, b in zip(source, source[1:]))):
        return False
    curve = _dense(donor, 1, check, native=capture_api.dense_curve)
    planned = []
    for i in range(count):
        left = bisect_left(donor.ticks, source[i])
        right = bisect_left(donor.ticks, source[i + 1])
        voiced = [row.tick for row in donor.frames[left:right]
                  if row.controls[2] > 0 and row.controls[1] > 0]
        if len(voiced) < 2 or voiced[0] >= voiced[-1]:
            return False
        planned.append((target[i], target[i + 1], voiced[0], voiced[-1]))
    if len(syllable_counts) != count:
        syllable_counts = ()
    target_groups = _syllable_word_edges(item, target, syllable_counts) if syllable_counts else None
    source_groups = _syllable_word_edges(donor, source, syllable_counts) if syllable_counts else None
    for i, (start, end, first, last) in enumerate(planned):
        if check:
            check()
        fitter = getattr(capture_api, 'fit_pitch_anchors', None)
        if fitter is not None and syllable_counts:
            expected = syllable_counts[i]
            target_edges = (target_groups[i] if target_groups is not None else
                            _syllable_edges(item, start, end, expected))
            donor_edges = (source_groups[i] if source_groups is not None else
                           _syllable_edges(donor, source[i], source[i+1], expected))
            if target_edges and len(target_edges) == len(donor_edges):
                n = len(target_edges)
                target_array = (ctypes.c_int64 * n)(*target_edges)
                donor_array = (ctypes.c_int64 * n)(*donor_edges)
                status = fitter(item.frames, len(item.frames), curve, len(curve), start, end,
                                first, last, target_array, donor_array, n)
                if status == 0:
                    continue
                if status < 0:
                    raise RuntimeError('invalid native syllable pitch alignment')
        status = capture_api.fit_pitch(item.frames, len(item.frames), curve, len(curve),
                                       start, end, first, last)
        if status:
            raise RuntimeError('could not align US pitch to Persian word')
    return True


@dataclass
class SentencePlan:
    frames: object
    counts: tuple[int, ...]
    contexts: dict[str, CapturedRows]
    edges: dict[str, tuple[int, ...]]
    word_markers: dict[tuple[int, int], int]
    pilot: CapturedRows | None
    pilot_bounds: tuple[int, ...]
    source_marks: tuple[tuple[int, int, int], ...] = ()
    _timeline: list[float] | None = None
    _native_timeline: object = None
    _native_frication: object = None
    _native_bridge_continuation: object = None

    def timeline(self):
        """Reuse frame times while only resonance or source values change."""
        if self._timeline is None:
            if self._native_timeline is not None:
                times = (ctypes.c_double * (len(self.frames) + 1))()
                if self._native_timeline(self.frames, len(self.frames), times):
                    raise RuntimeError('invalid native frame timeline')
                self._timeline = times
            else:
                self._timeline = _starts(self.frames)
        return self._timeline


def context_plan(parts, fa, en, rate, *, maximum=60000, source_events=(), guarded_spans=(),
                 spelling_spans=(), check=None, trusted_fa=False, validate_in_capture=False,
                 start_prefix=None, parallel_pilot=None, fast_initial=False, fast_minimum_ms=300,
                 us_pilot=False):
    parts = tuple(parts)
    if not parts or len(parts) > 32 or {p.language for p in parts} != {'fa', 'en-us'}:
        raise ValueError('expected a bounded mixed English/Persian clause')
    if any(p.text.rstrip().endswith(('.', '?', '!', ',', ';', ':', '،', '؟', '؛')) for p in parts[:-1]):
        raise ValueError('explicit punctuation ends this clause')
    if any(SAMPLE_RATES.get(voice._dll.eciGetParam(voice._handle, SAMPLE_RATE)) != rate for voice in (fa, en)):
        raise ValueError('source rates do not match the replay clock')
    capture_api = FrameCapture(fa._dll)
    native_pitch = capture_api.fit_pitch if capture_api.dense_curve is not None else None
    native_edges = capture_api.fit_pitch_edges if capture_api.dense_curve is not None else None
    def get(voice, planned, *, audio=True):
        if check:
            check()
        return CapturedRows.from_capture(capture_api.capture(voice, planned, maximum=maximum, audio=audio),
                                         native=capture_api.frame_ticks)
    all_bracketed = (not us_pilot and
                     all(_bracket_boundary(parts, edge) for edge in range(1, len(parts))))
    word_markers = _word_markers(parts)
    if us_pilot and validate_in_capture and not trusted_fa:
        # A US pilot cannot validate eSpeak-derived Persian phones. Validate
        # the complete Persian context before any English prefix can stream.
        validation = _context(parts, 'fa', en, fa, word_markers,
                              guarded_spans=guarded_spans, spelling_spans=spelling_spans)
        get(fa, validation, audio=False)
    pilot, pilot_parts, pilot_pitch, pilot_bounds = None, [], (), ()
    def contour(item, *, controls_only=False):
        nonlocal native_pitch
        if item.ticks[-1] > 60000:
            native_pitch = None
        pitch = _dense(item, 1, check, native=capture_api.dense_curve if native_pitch else None)
        end = (len(pitch) - 1 if controls_only else
               min(_last_audible(item.pcm, rate, native=capture_api.last_audible_sample), len(pitch) - 1))
        bounds = (0, *(round(item.indexes[i] * 1000 / rate) for i in range(1, len(parts))), end)
        if any(a >= b for a, b in zip(bounds, bounds[1:])):
            raise RuntimeError('invalid shared sentence boundaries')
        return pitch, bounds
    if all_bracketed:
        # No segment uses the shared pitch when every switch has a native
        # bracket boundary. Avoid synthesizing the entire sentence a third
        # time, but retain the usual native validation of Persian tags.
        for part in parts:
            if part.language == 'fa' and not trusted_fa and any(
                    not native_tag_accepted(m.group(), fa._phonemes_for_probe(m.group()))
                    for m in _TAG.finditer(part.text)):
                raise ValueError('Persian proxy was rejected')
    else:
        for index, part in enumerate(parts):
            if index:
                pilot_parts.extend((' ', EciIndex(index)))
            if us_pilot:
                if part.language == 'fa':
                    if not trusted_fa and any(
                            not native_tag_accepted(m.group(), fa._phonemes_for_probe(m.group()))
                            for m in _TAG.finditer(part.text)):
                        raise ValueError('Persian proxy was rejected')
                    marks = tuple(word_markers[index, word]
                                  for word in range(1, len(tuple(_TAG.finditer(part.text)))))
                    # Mixed jobs are split at a prose colon; keep the
                    # established full-stop contour for that boundary.
                    marked = re.sub(r':(?=\s*$)', '.', part.text)
                    pilot_parts.extend(indexed_us_proxy(marked, en, marks))
                else:
                    pilot_parts.append(_spelling_proxy(part.text, index, spelling_spans))
                continue
            proxy = (part.text if part.language == 'fa' else
                     english_proxy(en, _spelling_proxy(part.text, index, spelling_spans)))
            if part.language == 'fa':
                # The scoped capture validates the entire pilot before any
                # audio can start. Other callers still need the per-tag check.
                if not trusted_fa and not validate_in_capture and any(
                        not native_tag_accepted(m.group(), fa._phonemes_for_probe(m.group()))
                        for m in _TAG.finditer(proxy)):
                    raise ValueError('Persian proxy was rejected')
            elif any(not _GENERATED_FA_PROXY.fullmatch(m.group()) for m in _TAG.finditer(proxy)):
                # english_proxy constructs only m+vowel syllables from this
                # finite, tested native alphabet. Reject unexpected syntax.
                raise ValueError('invalid generated English proxy')
            pilot_parts.append(proxy)
        if parallel_pilot is None:
            pilot = get(en if us_pilot else fa, pilot_parts)
            pilot_pitch, pilot_bounds = contour(pilot)
    source_events = tuple(source_events)
    guarded_spans = tuple(guarded_spans)
    spelling_spans = tuple(spelling_spans)
    if any(type(segment) is not int or not 0 <= segment < len(parts) or
           type(start) is not int or type(end) is not int or not 0 <= start < end <= len(parts[segment].text)
           for segment, start, end in (*guarded_spans, *spelling_spans)):
        raise ValueError('invalid protected text span')
    next_mark = max((len(parts) - 1, *word_markers.values())) + 1
    source_marks = []
    for order, (segment, offset, value) in enumerate(source_events):
        if (type(segment) is not int or not 0 <= segment < len(parts) or
            type(offset) is not int or not 0 <= offset <= len(parts[segment].text) or
            type(value) is not int or not 0 <= value <= 0x7fffffff):
            raise ValueError('invalid source index')
        source_marks.append((segment, offset, order, next_mark + order, value))
    contexts, edges = {}, {}
    def fit_us_segments(item, bounds, donor, *, only=None):
        for segment, part in enumerate(parts):
            if part.language != 'fa' or only is not None and segment not in only:
                continue
            marks = tuple(word_markers[segment, word]
                          for word in range(1, len(tuple(_TAG.finditer(part.text)))))
            donor_start = 0 if segment == 0 else donor.indexes[segment]
            donor_end = (len(donor.pcm) // 2 if segment + 1 == len(parts)
                         else donor.indexes[segment + 1])
            fit_us_word_pitch(item, (bounds[segment], bounds[segment + 1]), donor,
                              (donor_start, donor_end), marks, marks, rate, capture_api, check,
                              syllable_counts=tuple(len(_SYLLABLE.findall(m.group(1)))
                                                    for m in _TAG.finditer(part.text)))
    voices = (('fa', fa), ('en-us', en))
    prepared = {}
    if start_prefix is not None:
        # An English-first replay keeps the English handle occupied, so both
        # contexts must be prepared before it starts. For Persian-first text,
        # the remaining English context probes only the separate English
        # handle: it can be prepared after the safe Persian prefix starts.
        # This avoids delaying the beginning of a long Persian phrase while
        # converting every word into an English context proxy.
        before_replay = ('fa', 'en-us') if parts[0].language == 'en-us' else ('fa',)
        prepared = {language: _context(parts, language, en, fa, word_markers, source_marks,
                                       guarded_spans, spelling_spans)
                    for language in before_replay}
        voices = tuple(sorted(voices, key=lambda pair: pair[0] != parts[0].language))
    pending, early = None, None
    fast_pilot = bool(fast_initial and start_prefix is not None and parallel_pilot is not None and
                      capture_api._controls_only is not None and capture_api._sample_count is not None and
                      not all_bracketed)
    def submit_pilot(*, audio):
        pilot_voice = en if us_pilot else fa
        return parallel_pilot.submit(parts=pilot_parts, prefix=pilot_voice.prefix,
                                     validate=False if us_pilot else fa.validate,
                                     parameters=pilot_voice.owner._engine_parameters[pilot_voice.key[0]],
                                     dictionary=fa.owner._dictionary_path,
                                     cancelled=fa.owner._cancelled, maximum=maximum, audio=audio)
    def collect_pilot(*, can_fallback):
        nonlocal pending
        future, pending = pending, None
        try:
            captured = future.result()
            return CapturedRows.from_capture(captured, native=capture_api.frame_ticks), False
        except InterruptedError:
            raise
        except (ValueError, RuntimeError, OSError):
            if not can_fallback:
                raise
            if check:
                check()
            return get(en if us_pilot else fa, pilot_parts), True
    if parallel_pilot is not None and not all_bracketed:
        if check:
            check()
        try:
            pending = submit_pilot(audio=not fast_pilot)
        except (RuntimeError, OSError):
            if check:
                check()
            pilot = get(en if us_pilot else fa, pilot_parts)
            pilot_pitch, pilot_bounds = contour(pilot)
            fast_pilot = False
    try:
        for language, voice in voices:
            planned = prepared[language] if language in prepared else _context(
                parts, language, en, fa, word_markers, source_marks, guarded_spans, spelling_spans)
            item = get(voice, planned, audio=False)
            bounds = _edges(item, len(parts))
            if pending is not None and fast_pilot:
                quick, fallback = collect_pilot(can_fallback=True)
                fast_pilot = False
                if fallback:
                    pilot = quick
                    pilot_pitch, pilot_bounds = contour(pilot)
                else:
                    # The first segment ends at an internal mark. Its pitch
                    # needs no audible-EOF scan, so the identical control
                    # capture can start that segment before rendering the
                    # full pilot. Keep the caller's required playable reserve.
                    quick_pitch, quick_bounds = contour(quick, controls_only=True)
                    _fit_context_pitch(item, bounds, quick_pitch, quick_bounds, parts, language,
                                       rate, check, native=native_pitch if item.ticks[-1] <= 60000 else None,
                                       native_edges=native_edges if item.ticks[-1] <= 60000 else None,
                                       only=(0,), retain_brackets=not (us_pilot and language == 'fa'))
                    if us_pilot and language == 'fa':
                        fit_us_segments(item, bounds, quick, only=(0,))
                    if start_prefix(item, bounds, source_marks, capture_api, minimum_ms=fast_minimum_ms):
                        early = (language, item, bounds)
                        contexts[language], edges[language] = item, bounds
                    try:
                        pending = submit_pilot(audio=True)
                    except (RuntimeError, OSError):
                        if early is not None:
                            raise
                        if check:
                            check()
                        pilot = get(en if us_pilot else fa, pilot_parts)
                        pilot_pitch, pilot_bounds = contour(pilot)
                    if early is not None:
                        continue
            if pending is not None:
                pilot, _fallback = collect_pilot(can_fallback=early is None)
                pilot_pitch, pilot_bounds = contour(pilot)
                if early is not None:
                    early_language, early_item, early_bounds = early
                    _fit_context_pitch(early_item, early_bounds, pilot_pitch, pilot_bounds, parts,
                                       early_language, rate, check,
                                       native=native_pitch if early_item.ticks[-1] <= 60000 else None,
                                       native_edges=native_edges if early_item.ticks[-1] <= 60000 else None,
                                       retain_brackets=not (us_pilot and early_language == 'fa'))
                    if us_pilot and early_language == 'fa':
                        fit_us_segments(early_item, early_bounds, pilot)
            _fit_context_pitch(item, bounds, pilot_pitch, pilot_bounds, parts, language, rate, check,
                               native=native_pitch if item.ticks[-1] <= 60000 else None,
                               native_edges=native_edges if item.ticks[-1] <= 60000 else None,
                               retain_brackets=not (us_pilot and language == 'fa'))
            if us_pilot and language == 'fa':
                fit_us_segments(item, bounds, pilot)
            contexts[language], edges[language] = item, bounds
            if start_prefix is not None and language == parts[0].language and early is None:
                start_prefix(item, bounds, source_marks, capture_api)
    finally:
        if pending is not None:
            parallel_pilot.cancel()
            try:
                pending.result()
            except BaseException:
                pass
    chunks, counts, firsts = [], [], []
    for i, part in enumerate(parts):
        item, bounds = contexts[part.language], edges[part.language]
        first = bisect_left(item.ticks, round(bounds[i] * 1000 / rate))
        last = bisect_left(item.ticks, round(bounds[i + 1] * 1000 / rate))
        if first >= last:
            raise RuntimeError('empty native context interval')
        chunks.append((item.frames, first, last))
        counts.append(last - first)
        firsts.append(first)
    frames = (NativeFrame * sum(counts))()
    offset = 0
    for source, first, last in chunks:
        length = (last - first) * ctypes.sizeof(NativeFrame)
        ctypes.memmove(ctypes.addressof(frames) + offset * ctypes.sizeof(NativeFrame),
                       ctypes.addressof(source) + first * ctypes.sizeof(NativeFrame), length)
        offset += last - first
    replay_marks = []
    for segment, offset, _order, mark, value in source_marks:
        item = contexts[parts[segment].language]
        source = bisect_left(item.ticks, round(item.indexes[mark] * 1000 / rate))
        local = (0 if offset == 0 else counts[segment] if offset == len(parts[segment].text) else
                 max(0, min(counts[segment], source - firsts[segment])))
        replay_marks.append((segment, local, value))
    if any(a[:2] > b[:2] for a, b in zip(replay_marks, replay_marks[1:])):
        raise RuntimeError('source indexes are not ordered')
    return SentencePlan(frames, tuple(counts), contexts, edges, word_markers, pilot,
                        tuple(pilot_bounds), tuple(replay_marks),
                        _native_timeline=capture_api.frame_timeline,
                        _native_frication=capture_api.recompute_frication,
                        _native_bridge_continuation=capture_api.bridge_continuation)


def replay_marks(plan, original_counts):
    """Map source marks through any removed Ach-onset frames."""
    starts = (0, *_edges_by_count(plan.counts))
    result = []
    for segment, local, value in plan.source_marks:
        removed = original_counts[segment] - plan.counts[segment]
        if removed < 0:
            raise RuntimeError('invalid source index mapping')
        result.append((starts[segment] + max(0, min(plan.counts[segment], local - removed)), value))
    if any(a[0] > b[0] for a, b in zip(result, result[1:])):
        raise RuntimeError('source indexes changed order')
    return tuple(result)


def _copy_frames(frames):
    result = (NativeFrame * len(frames))()
    if frames:
        ctypes.memmove(result, frames, ctypes.sizeof(NativeFrame) * len(frames))
    return result


def _starts(frames):
    result = [0.0]
    for frame in frames:
        result.append(result[-1] + float(frame.controls[0]))
    return result


def _edges_by_count(counts):
    result, total = [], 0
    for count in counts:
        total += count
        result.append(total)
    return result


def _ease(x):
    return x * x * (3 - 2 * x)


def bridge_sonorants(plan, parts, rate, half_window_ms=25.0):
    """Apply the accepted 25-ms filter bridge to safe voiced boundaries."""
    if not 0 < half_window_ms <= 50:
        raise ValueError('invalid sonorant bridge window')
    original = plan.frames
    output = None
    starts, frame_edges = plan.timeline(), _edges_by_count(plan.counts)
    bridged = []
    for i in range(1, len(parts)):
        if _bracket_boundary(parts, i):
            continue
        boundary = frame_edges[i - 1]
        at = starts[boundary]
        left = starts[0 if i == 1 else frame_edges[i - 2]]
        right = starts[frame_edges[i]]
        width = min(half_window_ms, (at - left) / 2, (right - at) / 2)
        if width < 2:
            continue
        first = bisect_left(starts, at - width, 0, len(original))
        last = bisect_right(starts, at + width, 0, len(original))
        indexes = range(first, last)
        if last - first < 3 or any(original[j].controls[2] <= 0 or original[j].controls[8] != 0 for j in indexes):
            continue
        relative = [starts[j] - at for j in indexes]
        sources = []
        for part in (parts[i - 1], parts[i]):
            item = plan.contexts[part.language]
            edge_ms = round(plan.edges[part.language][i] * 1000 / rate)
            at_index = bisect_left(item.ticks, edge_ms)
            if at_index >= len(item.ticks):
                break
            positions = [item.ticks[at_index] + x for x in relative]
            if positions[0] < item.ticks[0] or positions[-1] > item.ticks[-1]:
                break
            coordinates = _frame_coordinates(item, positions)
            if any(_frame_value(item, 2, c) <= 0 or _frame_value(item, 8, c) != 0
                   for c in coordinates):
                break
            sources.append((item, coordinates))
        if len(sources) != 2:
            continue
        if output is None:
            output = _copy_frames(original)
        for column in TRACT:
            a, pa = sources[0]
            b, pb = sources[1]
            for j, delta, ta, tb in zip(indexes, relative, pa, pb):
                weight = _ease((delta + width) / (2 * width))
                output[j].controls[column] = (_frame_value(a, column, ta) * (1 - weight) +
                                              _frame_value(b, column, tb) * weight)
        bridged.append(i)
    if output is not None:
        plan.frames = output
    return tuple(bridged)


def bridge_remaining_voiced(plan, parts, bridged, cuts, window_ms=15.0):
    """Smooth abrupt voiced joins when a contextual bridge was unavailable.

    Only already voiced frames are reshaped, so stop closures are preserved.
    Aspiration is tapered at a voiced join or bridged across a brief spurious
    voiced onset. The selected A-to-Ach path and its established sonorant
    bridges remain unchanged.
    """
    excluded = set(bridged) | {edge for edge, _ in cuts}
    original, output = plan.frames, None
    starts, bounds = plan.timeline(), _edges_by_count(plan.counts)
    changed = []
    for edge in range(1, len(parts)):
        if edge in excluded or _bracket_boundary(parts, edge):
            continue
        boundary = bounds[edge - 1]
        left_limit = 0 if edge == 1 else bounds[edge - 2]
        right_limit = bounds[edge]
        left, right = original[boundary - 1].controls, original[boundary].controls
        if left[2] <= 0 or right[2] <= 0:
            continue
        # Some contexts contribute only 1–4 ms of artificial voicing before
        # a voiceless /h/. Bridge the existing aspiration through those rows
        # instead of introducing an isolated voiced click or a noise hole.
        short = boundary
        while (short < right_limit and original[short].controls[2] > 0 and
               starts[short] - starts[boundary] < 5.0):
            short += 1
        if (short > boundary and short < right_limit and original[short].controls[2] == 0 and
                starts[short] - starts[boundary] <= 5.0 and left[2] <= 20 and
                left[7] > 0 and original[short].controls[7] > 0):
            destination = original[short].controls
            width = short - boundary
            if output is None:
                output = _copy_frames(original)
            for i in range(boundary, short):
                fraction = (i - boundary + 1) / (width + 1)
                output[i].controls[2] = round(left[2] * (1 - fraction))
                output[i].controls[7] = round(left[7] + (destination[7] - left[7]) * fraction)
                for column in (9, 13, 15):
                    output[i].controls[column] = left[column] + (destination[column] - left[column]) * fraction
            changed.append(edge)
            continue
        amplitude_jump = right[2] - left[2]
        aspiration_jump = right[7] - left[7]
        frequencies = (9, 13, 15)
        if (abs(amplitude_jump) < 12 and abs(aspiration_jump) < 12 and
                max(abs(right[c] - left[c]) for c in frequencies) < 100):
            continue
        first = max(left_limit, bisect_left(starts, starts[boundary] - window_ms))
        last = min(right_limit, bisect_right(starts, starts[boundary] + window_ms))
        while first < boundary and any(original[j].controls[2] <= 0 for j in range(first, boundary)):
            first += 1
        while last > boundary and any(original[j].controls[2] <= 0 for j in range(boundary, last)):
            last -= 1
        if boundary - first < 3 or last - boundary < 3 or starts[last - 1] <= starts[first]:
            continue
        span = starts[last - 1] - starts[first]
        if output is None:
            output = _copy_frames(original)
        for column in frequencies:
            a, b = original[first].controls[column], original[last - 1].controls[column]
            for i in range(first, last):
                weight = _ease((starts[i] - starts[first]) / span)
                output[i].controls[column] = a + (b - a) * weight
        if amplitude_jump >= 12 and starts[boundary - 1] > starts[first]:
            duration = starts[boundary - 1] - starts[first]
            for i in range(first, boundary):
                weight = _ease((starts[i] - starts[first]) / duration)
                output[i].controls[2] = min(80, round(original[i].controls[2] + amplitude_jump * weight))
        elif amplitude_jump <= -12 and starts[last - 1] > starts[boundary]:
            duration = starts[last - 1] - starts[boundary]
            for i in range(boundary, last):
                weight = 1 - _ease((starts[i] - starts[boundary]) / duration)
                output[i].controls[2] = min(80, round(original[i].controls[2] - amplitude_jump * weight))
        if left[7] > 0 and right[7] == 0 and starts[boundary - 1] > starts[first]:
            duration = starts[boundary - 1] - starts[first]
            for i in range(first, boundary):
                weight = _ease((starts[i] - starts[first]) / duration)
                output[i].controls[7] = round(original[i].controls[7] * (1 - weight))
        elif left[7] == 0 and right[7] > 0 and starts[last - 1] > starts[boundary]:
            duration = starts[last - 1] - starts[boundary]
            for i in range(boundary, last):
                weight = _ease((starts[i] - starts[boundary]) / duration)
                output[i].controls[7] = round(original[i].controls[7] * weight)
        changed.append(edge)
    if output is not None:
        plan.frames = output
    return tuple(changed)


def bridge_english_continuations(plan, parts, bridged, cuts, window_ms=30.0):
    """Carry US vowel resonances into an unhandled Persian onset.

    Native US word transitions retain the previous resonance at the boundary
    and progress into the next sound. Keep the preceding word and all source
    levels intact; decay only the initial resonance offset on the right.
    Accepted sonorant/Ach bridges, punctuation and stop closures are excluded.
    """
    if not 0 < window_ms <= 50:
        raise ValueError('invalid continuation window')
    excluded = set(bridged) | {edge for edge, _ in cuts}
    original, output = plan.frames, None
    starts, bounds = plan.timeline(), _edges_by_count(plan.counts)
    if plan._native_bridge_continuation is not None:
        skip = (ctypes.c_ubyte * len(parts))()
        for edge in range(1, len(parts)):
            skip[edge] = int(edge in excluded or parts[edge - 1].language != 'en-us' or
                parts[edge].language != 'fa' or
                bool(re.search(r'[()\[\]{}]\s*$', parts[edge - 1].text)) or
                bool(re.match(r'^\s*[()\[\]{}]', parts[edge].text)))
        if all(skip[1:]):
            return ()
        frame_times = ((ctypes.c_double * len(starts))(*starts)
                       if not isinstance(starts, ctypes.Array) else starts)
        frame_ends = (ctypes.c_size_t * len(bounds))(*bounds)
        changed = (ctypes.c_ubyte * len(parts))()
        output = (NativeFrame * len(original))()
        if plan._native_bridge_continuation(original, output, frame_times, frame_ends, skip,
                                             len(original), len(parts), window_ms, changed):
            raise RuntimeError('invalid native continuation frames')
        selected = tuple(edge for edge in range(1, len(parts)) if changed[edge])
        if selected:
            plan.frames = output
        return selected
    changed = []
    for edge in range(1, len(parts)):
        if (edge in excluded or parts[edge - 1].language != 'en-us' or
                parts[edge].language != 'fa' or
                re.search(r'[()\[\]{}]\s*$', parts[edge - 1].text) or
                re.match(r'^\s*[()\[\]{}]', parts[edge].text)):
            continue
        boundary, right_limit = bounds[edge - 1], bounds[edge]
        if boundary < 2 or boundary >= right_limit:
            continue
        left, first = original[boundary - 1].controls, original[boundary].controls
        if left[2] <= 0 or original[boundary - 2].controls[2] <= 0:
            continue
        if first[2] <= 0 and first[7] == 0 and first[8] == 0:
            continue
        columns = (9, 13, 15)
        offsets = tuple(left[c] - first[c] for c in columns)
        if max(map(abs, offsets)) < 30:
            continue
        last = boundary
        while last < right_limit and starts[last] - starts[boundary] <= window_ms:
            c = original[last].controls
            if c[2] <= 0 and c[7] == 0 and c[8] == 0:
                break
            last += 1
        if last - boundary < 3 or starts[last - 1] - starts[boundary] < 5:
            continue
        duration = starts[last - 1] - starts[boundary]
        if output is None:
            output = _copy_frames(original)
        for index in range(boundary, last):
            weight = 1 - (starts[index] - starts[boundary]) / duration
            for column, offset in zip(columns, offsets):
                output[index].controls[column] = max(0, original[index].controls[column] + offset * weight)
        changed.append(edge)
    if output is not None:
        plan.frames = output
    return tuple(changed)


def direct_ach(plan, parts):
    """Remove a voiced X lead-in only where the selected fricative follows."""
    frames, boundaries = plan.frames, _edges_by_count(plan.counts)
    cuts, ranges, counts = [], [], list(plan.counts)
    for i in range(1, len(parts)):
        if _bracket_boundary(parts, i):
            continue
        if (parts[i - 1].language != 'en-us' or parts[i].language != 'fa' or
                not re.match(r'^\s*`\[\.[012]G', parts[i].text)):
            continue
        boundary, end = boundaries[i - 1], boundaries[i]
        previous, first = frames[boundary - 1].controls, frames[boundary].controls
        if previous[2] <= 0 or previous[8] != 0 or first[2] <= 0:
            continue
        elapsed, cut = 0.0, None
        for j in range(boundary, end):
            c = frames[j].controls
            elapsed += c[0]
            if elapsed > 120:
                break
            if c[2] == 0 and c[8] >= 20 and c[36] >= 50 and c[38] >= 35 and c[36] > c[37] + 10:
                cut = j - boundary
                break
        if cut is None or not cut or any(frames[j].controls[2] <= 0 for j in range(boundary, boundary + cut)):
            continue
        removed = sum(float(frames[j].controls[0]) for j in range(boundary, boundary + cut))
        counts[i] -= cut
        ranges.append((boundary, boundary + cut))
        cuts.append((i, removed))
    if ranges:
        output = (NativeFrame * (len(frames) - sum(b - a for a, b in ranges)))()
        target, previous = 0, 0
        for a, b in (*ranges, (len(frames), len(frames))):
            count = a - previous
            if count:
                ctypes.memmove(ctypes.addressof(output) + target * ctypes.sizeof(NativeFrame),
                               ctypes.addressof(frames) + previous * ctypes.sizeof(NativeFrame),
                               count * ctypes.sizeof(NativeFrame))
                target += count
            previous = b
        plan.frames, plan.counts = output, tuple(counts)
        plan._timeline = None
    return tuple(cuts)


def direct_unvoiced_onsets(plan, parts, en):
    """Remove inherited voiced carrier before a confirmed voiceless onset.

    The two native context captures can carry the preceding language's vowel
    into the beginning of the next segment. Keep the preceding vowel intact
    and remove only a bounded all-voiced lead-in when the following native
    frames actually contain the expected frication, aspiration or closure.
    The separately selected Ach transition and voiced consonants are left to
    their existing paths.
    """
    frames, boundaries = plan.frames, _edges_by_count(plan.counts)
    ranges, counts, cuts = [], list(plan.counts), []
    for edge in range(1, len(parts)):
        if _bracket_boundary(parts, edge):
            continue
        part = parts[edge]
        if part.language == 'fa':
            source = part.text
        else:
            try:
                source = en._phonemes_for_probe(part.text)
            except (ValueError, RuntimeError, OSError, UnicodeError):
                continue
        first_tag = _TAG.search(source)
        onset = re.match(r'\.[012]([fstpkSh])', first_tag.group(1)) if first_tag else None
        if onset is None:
            continue
        phone = onset.group(1)
        boundary, end = boundaries[edge - 1], boundaries[edge]
        if boundary < 1 or boundary >= end or frames[boundary - 1].controls[2] <= 0:
            continue
        elapsed, cut = 0.0, None
        for pos in range(boundary, end):
            c = frames[pos].controls
            if c[2] <= 0:
                expected = (c[8] >= 20 if phone in 'fsS' else
                            c[7] > 0 if phone == 'h' else c[7] == 0 and c[8] == 0)
                if expected and 8.0 <= elapsed <= 120.0:
                    cut = pos
                break
            elapsed += float(c[0])
            if elapsed > 120.0:
                break
        if cut is None or cut == boundary:
            continue
        ranges.append((boundary, cut))
        counts[edge] -= cut - boundary
        cuts.append((edge, elapsed))
    if ranges:
        output = (NativeFrame * (len(frames) - sum(b - a for a, b in ranges)))()
        target, previous = 0, 0
        for a, b in (*ranges, (len(frames), len(frames))):
            count = a - previous
            if count:
                ctypes.memmove(ctypes.addressof(output) + target * ctypes.sizeof(NativeFrame),
                               ctypes.addressof(frames) + previous * ctypes.sizeof(NativeFrame),
                               count * ctypes.sizeof(NativeFrame))
                target += count
            previous = b
        plan.frames, plan.counts, plan._timeline = output, tuple(counts), None
    return tuple(cuts)


@dataclass(frozen=True)
class GermanAch:
    times: tuple[float, ...]
    rows: tuple[tuple[float, ...], ...]
    noise_times: tuple[float, ...]
    noise_f3: tuple[float, ...]


def german_ach_template(voice, *, maximum=60000):
    tag = "`[.1'aj'x.0u]"
    # This fixed internal tag is covered by the native transition tests; the
    # captured voiced/noise boundaries below verify the actual result.
    capture = FrameCapture(voice._dll)
    item = CapturedRows.from_capture(capture.capture(voice, (tag,), maximum=maximum, audio=False),
                                     native=capture.frame_ticks)
    frames = item.frames
    first = next((i for i, row in enumerate(frames)
                  if row.controls[2] > 0 and row.controls[8] > 0 and
                  row.controls[36] > 0 and row.controls[38] > 0), None)
    if first is None:
        raise RuntimeError('German X onset is unavailable')
    last = next((i for i in range(first, len(frames))
                 if frames[i].controls[2] == 0 and frames[i].controls[8] >= 20 and
                 frames[i].controls[36] >= 50 and frames[i].controls[38] >= 35), None)
    if last is None or last <= first or any(frames[i].controls[2] <= 0 for i in range(first, last)):
        raise RuntimeError('German X onset has an unexpected source')
    rows = tuple(tuple(frames[i].controls) for i in range(first, last + 1))
    times = [0.0]
    for row in rows[:-1]:
        times.append(times[-1] + row[0])
    if not 0 < times[-1] <= 100:
        raise RuntimeError('invalid German X onset duration')
    voiced = next((i for i in range(last, len(frames)) if frames[i].controls[2] > 0), None)
    if voiced is None or voiced - last < 2:
        raise RuntimeError('German X has no bounded friction')
    noise_times = [0.0]
    noise = tuple(frames[i].controls[15] for i in range(last, voiced))
    for i in range(last, voiced - 1):
        noise_times.append(noise_times[-1] + frames[i].controls[0])
    if noise_times[-1] <= 0:
        raise RuntimeError('invalid German X friction duration')
    return GermanAch(tuple(times), rows, tuple(noise_times), noise)


def _english_final_a(en, text):
    tags = _TAG.findall(en._phonemes_for_probe(text))
    return bool(tags and re.search(r'e$', tags[-1]))


def extend_english_a(plan, parts, cuts, en, factor=1.15):
    """Stretch only the native final A glide selected in the listening test."""
    if not 1.0 < factor <= 1.5:
        raise ValueError('invalid A extension')
    original, output = plan.frames, _copy_frames(plan.frames)
    starts, bounds = plan.timeline(), _edges_by_count(plan.counts)
    extended = []
    for edge, _ in cuts:
        if (parts[edge - 1].language != 'en-us' or
                not _english_final_a(en, parts[edge - 1].text)):
            continue
        boundary = bounds[edge - 1]
        left = 0 if edge == 1 else bounds[edge - 2]
        indexes = list(range(max(left, bisect_left(starts, starts[boundary] - 200)), boundary))
        bad = [j for j, i in enumerate(indexes) if original[i].controls[2] <= 0 or original[i].controls[8] != 0]
        if bad:
            indexes = indexes[bad[-1] + 1:]
        if len(indexes) < 3:
            continue
        peak = max(range(len(indexes)), key=lambda j: original[indexes[j]].controls[9])
        indexes = indexes[peak:]
        if len(indexes) < 3:
            continue
        first, last = original[indexes[0]].controls, original[indexes[-1]].controls
        if first[9] - last[9] < 25 or last[13] - first[13] < 40:
            continue
        duration = sum(float(original[i].controls[0]) for i in indexes)
        for i in indexes:
            output[i].controls[0] = original[i].controls[0] * factor
        extended.append((edge, duration * (factor - 1)))
    plan.frames = output
    if extended:
        plan._timeline = None
    return tuple(extended)


def apply_german_ach(plan, parts, cuts, template, en):
    """Apply the accepted German-guided noise and resonance at A-to-X."""
    original, output = plan.frames, _copy_frames(plan.frames)
    starts, bounds = plan.timeline(), _edges_by_count(plan.counts)
    chosen = []
    for edge, _ in cuts:
        if parts[edge - 1].language != 'en-us':
            continue
        boundary = bounds[edge - 1]
        left = 0 if edge == 1 else bounds[edge - 2]
        at = starts[boundary]
        width = min(template.times[-1], at - starts[left])
        indexes = list(range(bisect_left(starts, at - width, 0, len(original)), boundary))
        if not indexes or indexes[0] < left:
            continue
        bad = [j for j, i in enumerate(indexes) if original[i].controls[2] <= 0 or original[i].controls[8] != 0]
        if bad:
            indexes = indexes[bad[-1] + 1:]
        if not indexes:
            continue
        width = at - starts[indexes[0]]
        if width <= 0:
            continue
        times = [(starts[i] - starts[indexes[0]]) * template.times[-1] / width for i in indexes]
        target = original[boundary].controls
        for column in (8, *range(35, 43)):
            end = template.rows[-1][column]
            values = [row[column] for row in template.rows]
            for i, t in zip(indexes, times):
                if end <= 0:
                    output[i].controls[column] = 0
                else:
                    proportion = max(0.0, min(1.0, _interp(t, template.times, values) / end))
                    output[i].controls[column] = round(proportion * target[column])
        for column in (9, 13):
            first = template.rows[0][column]
            delta = template.rows[-1][column] - first
            if abs(delta) < 1:
                continue
            values = [row[column] for row in template.rows]
            target_delta = target[column] - original[boundary - 1].controls[column]
            for i, t in zip(indexes, times):
                progress = max(0.0, min(1.0, (_interp(t, template.times, values) - first) / delta))
                output[i].controls[column] = original[i].controls[column] + progress * target_delta
        chosen.append((edge, float(width)))
    plan.frames = output
    return tuple(chosen)


def smooth_ach_release(plan, cuts):
    original, output = plan.frames, _copy_frames(plan.frames)
    starts, bounds = plan.timeline(), _edges_by_count(plan.counts)
    changed = []
    for edge, _ in cuts:
        boundary, end = bounds[edge - 1], bounds[edge]
        before, first = original[boundary - 1].controls, original[boundary].controls
        if before[2] <= 0 or before[1] <= 0 or first[2] != 0 or first[8] < 20:
            continue
        duration = min(15.0, max(4.0, 10000.0 / before[1]))
        indexes = []
        for i in range(boundary, end):
            c = original[i].controls
            if c[2] != 0 or c[8] < 20 or starts[i] - starts[boundary] > duration:
                break
            indexes.append(i)
        if len(indexes) < 2 or starts[indexes[-1]] <= starts[boundary]:
            continue
        span = min(duration, starts[indexes[-1]] - starts[boundary])
        for i in indexes:
            weight = 1 - _ease((starts[i] - starts[boundary]) / span)
            for column in TRACT:
                delta = _f32(before[column] - first[column])
                output[i].controls[column] = original[i].controls[column] + weight * delta
        changed.append((edge, float(span)))
    plan.frames = output
    return tuple(changed)


def apply_german_f3(plan, edges, template):
    original, output = plan.frames, _copy_frames(plan.frames)
    starts, bounds = plan.timeline(), _edges_by_count(plan.counts)
    changed = []
    for edge, _ in edges:
        boundary, end = bounds[edge - 1], bounds[edge]
        indexes = []
        for i in range(boundary, end):
            c = original[i].controls
            t = starts[i] - starts[boundary]
            if c[2] != 0 or c[8] <= 0 or t > template.noise_times[-1]:
                break
            indexes.append(i)
        if len(indexes) < 2 or starts[indexes[-1]] <= starts[boundary]:
            continue
        duration = starts[indexes[-1]] - starts[boundary]
        for i in indexes:
            t = starts[i] - starts[boundary]
            weight = 1 - _ease(t / duration)
            german = (original[boundary - 1].controls[15] +
                      _interp(t, template.noise_times, template.noise_f3) - template.noise_f3[0])
            output[i].controls[15] = original[i].controls[15] * (1 - weight) + german * weight
        changed.append((edge, float(duration)))
    plan.frames = output
    return tuple(changed)


def fade_english_voice(plan, parts, edges, en, window_ms=24.0, attenuation_db=12.0):
    original, output = plan.frames, _copy_frames(plan.frames)
    starts, bounds = plan.timeline(), _edges_by_count(plan.counts)
    changed = []
    for edge, _ in edges:
        if parts[edge - 1].language != 'en-us':
            continue
        boundary = bounds[edge - 1]
        left = 0 if edge == 1 else bounds[edge - 2]
        indexes = list(range(max(left, bisect_left(starts, starts[boundary] - window_ms)), boundary))
        bad = [j for j, i in enumerate(indexes) if original[i].controls[2] <= 0]
        if bad:
            indexes = indexes[bad[-1] + 1:]
        if len(indexes) < 3 or starts[indexes[-1]] <= starts[indexes[0]]:
            continue
        span = starts[indexes[-1]] - starts[indexes[0]]
        for i in indexes:
            weight = _ease((starts[i] - starts[indexes[0]]) / span)
            output[i].controls[2] = max(1, round(original[i].controls[2] - attenuation_db * weight))
        changed.append((edge, float(starts[boundary] - starts[indexes[0]]),
                        float(original[indexes[-1]].controls[2] - output[indexes[-1]].controls[2])))
    plan.frames = output
    return tuple(changed)


def apply_persian_u_a(plan, parts, rate, original_counts):
    """Apply the selected U-to-A glide only to eligible word boundaries."""
    changes = []
    item = plan.contexts['fa']
    offset = 0
    for i, (part, count) in enumerate(zip(parts, plan.counts)):
        if part.language != 'fa':
            offset += count
            continue
        words = list(_TAG.finditer(part.text))
        if len(words) < 2:
            offset += count
            continue
        tags = tuple(match.group() for match in words)
        separators = tuple(part.text[a.end():b.start()] for a, b in zip(words, words[1:]))
        if not any(_FINAL_U.search(a) and _INITIAL_A.match(b) and 'w' not in a and 'w' not in b
                   and separator.isspace() for a, b, separator in zip(tags, tags[1:], separators)):
            offset += count
            continue
        section = [plan.frames[offset + j].controls for j in range(count)]
        ticks = [0.0]
        for row in section[:-1]:
            ticks.append(ticks[-1] + float(row[0]))
        start = round(plan.edges['fa'][i] * 1000 / rate)
        source_first = bisect_left(item.ticks, start)
        removed = original_counts[i] - count
        if removed < 0:
            raise RuntimeError('invalid Persian frame mapping')
        boundaries = {}
        missing = False
        for word in range(1, len(words)):
            marker = plan.word_markers.get((i, word))
            sample = item.indexes.get(marker)
            if sample is None:
                missing = True
                break
            source = bisect_left(item.ticks, round(sample * 1000 / rate))
            local = source - source_first - removed
            if not 0 <= local < len(ticks):
                missing = True
                break
            boundaries[word] = ticks[local]
        if missing:
            # Keep the connected sentence even when the optional glide's
            # word markers cannot be established for this text segment.
            offset += count
            continue
        glides = plan_u_a_glides(section, ticks, tags, separators, boundaries)
        apply_u_a_glides(section, ticks, glides)
        changes.extend((i, *glide.summary) for glide in glides)
        offset += count
    return tuple(changes)


def selected_sentence(plan, parts, en, german, rate, *, check=None):
    """Apply the user's selected sentence and A-to-Ach transition."""
    original_counts = plan.counts
    if check: check()
    direct_unvoiced_onsets(plan, parts, en)
    if check: check()
    bridged = bridge_sonorants(plan, parts, rate)
    if check: check()
    cuts = direct_ach(plan, parts)
    if check: check()
    continued = bridge_english_continuations(plan, parts, bridged, cuts)
    bridge_remaining_voiced(plan, parts, (*bridged, *continued), cuts)
    if check: check()
    template = german_ach_template(german() if callable(german) else german) if cuts else None
    if check: check()
    extended = extend_english_a(plan, parts, cuts, en) if cuts else ()
    ach = apply_german_ach(plan, parts, cuts, template, en) if template else ()
    if check: check()
    released = smooth_ach_release(plan, cuts) if cuts else ()
    f3 = apply_german_f3(plan, ach, template) if ach else ()
    faded = fade_english_voice(plan, parts, ach, en) if ach else ()
    glides = apply_persian_u_a(plan, parts, rate, original_counts)
    if check: check()
    # Source filters depend on the edited noise-band values, not on the
    # values from the unmodified context capture.
    if plan._native_frication is not None:
        plan._native_frication(plan.frames, len(plan.frames))
    else:
        for row in plan.frames:
            c = row.controls
            row.s_frication = int(c[8] > 0 and (c[38] > c[37] + 10 or c[39] > c[37] + 10))
    return bridged, cuts, extended, ach, released, f3, faded, glides
