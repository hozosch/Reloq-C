"""Bounded, in-memory version of the accepted Persian sentence planner.

The native frame capture and replay own cancellation and voice settings. This
module has no NumPy, temporary files, global environment changes or helper
processes. It is still an isolated planner until the driver supplies verified
source positions and selected Persian text tags.
"""
from __future__ import annotations

from bisect import bisect_left, bisect_right
import ctypes
from dataclasses import dataclass
import re
import struct

if __package__:
    from .._reloq_eci_direct import EciIndex, SAMPLE_RATE, SAMPLE_RATES
    from .._reloq_persian_frames import FrameCapture, NativeFrame
    from .._reloq_persian_glide import TRACT, apply_u_a_glides, plan_u_a_glides
    from .text_frontend import native_tag_accepted
else:
    from _reloq_eci_direct import EciIndex, SAMPLE_RATE, SAMPLE_RATES
    from _reloq_persian_frames import FrameCapture, NativeFrame
    from _reloq_persian_glide import TRACT, apply_u_a_glides, plan_u_a_glides
    from text_frontend import native_tag_accepted

_TAG = re.compile(r'`\[([^]]+)\]')
_SYLLABLE = re.compile(r'\.([012])([^.]+)')
_EN_TO_FA = {**{c: c for c in 'aAeioubdfghjklmnoprstvzSZ'},
             'E': 'e', 'I': 'i', 'O': 'o', 'U': 'u', 'c': 'a',
             'H': 'A', 'x': 'A', 'X': 'e', 'Y': 'A', 'y': 'j',
             'w': 'v', 'G': 'n', 'D': 'd', 'T': 't', 'F': 't'}
_FA_TO_EN = {**{c: c for c in 'aAioubdfghklmnprstvzSZ'},
             'e': 'E', 'j': 'y', 'G': 'h', 'R': 'k', 'J': 'y', 'C': 'S'}
_EN_VOWELS = {'i': 'i', 'I': 'i', 'e': 'e', 'E': 'A', 'a': 'a', 'A': 'a',
              'o': 'o', 'O': 'o', 'u': 'u', 'U': 'u', 'x': 'A', 'X': 'A',
              'y': 'i', 'Y': 'a', '@': 'A', 'V': 'A'}


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


def english_proxy(en, text):
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
    shown = en._phonemes_for_probe(text)
    tags = _converted(shown, _EN_TO_FA)
    if tags and all(native_tag_accepted(tag, fa._phonemes_for_probe(tag)) for tag in tags):
        return (_display_punctuation(shown, tags) if re.search(r'[.?!,;:]\s*\S', text)
                else ' '.join(tags) + _punctuation(text))
    return english_proxy(en, text)


def persian_proxy_for_english(text, en):
    tags = _converted(text, _FA_TO_EN)
    if tags:
        def sequence(value):
            return re.sub(r'\.[012]', '', value)
        if all(len(shown := _TAG.findall(en._phonemes_for_probe(tag))) == 1 and
               sequence(tag[2:-1]) == sequence(shown[0]) for tag in tags):
            it = iter(tags)
            return _TAG.sub(lambda _match: next(it), text)
    vowels = {'A': 'A', 'a': 'a', 'e': 'E', 'i': 'i', 'o': 'o', 'u': 'u'}
    result = []
    for match in _TAG.finditer(text):
        syllables = _SYLLABLE.findall(match.group(1))
        if not syllables:
            raise ValueError('Persian proxy has no syllables')
        result.append('`[' + ''.join('.' + stress + 'm' +
                                     next((vowels[c] for c in body if c in vowels), 'A')
                                     for stress, body in syllables) + ']')
    if not result:
        raise ValueError('Persian proxy has no words')
    it = iter(result)
    return _TAG.sub(lambda _match: next(it), text)


def _context(parts, language, en, fa, word_markers, source_marks=()):
    result = []
    for i, part in enumerate(parts):
        if i:
            result.extend((' ', EciIndex(i)))
        if part.language == language:
            inserts = [(offset, order, mark) for segment, offset, order, mark, _ in source_marks
                       if segment == i]
            if language == 'fa':
                inserts.extend((match.start(), -1, word_markers[i, word])
                               for word, match in enumerate(_TAG.finditer(part.text))
                               if word and (i, word) in word_markers)
            cursor = 0
            for offset, _order, mark in sorted(inserts):
                if offset < cursor or offset > len(part.text):
                    raise ValueError('invalid source index offset')
                result.extend((part.text[cursor:offset], EciIndex(mark)))
                cursor = offset
            result.append(part.text[cursor:])
        elif language == 'fa':
            result.append(english_phone_proxy(en, fa, part.text))
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


def _last_audible(pcm, rate):
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


@dataclass
class CapturedRows:
    pcm: bytes
    indexes: dict[int, int]
    frames: object
    ticks: list[int]

    @classmethod
    def from_capture(cls, value):
        if value is None:
            raise RuntimeError('native planning was cancelled')
        frames = value.frames()
        ticks = [row.tick for row in frames]
        if not ticks or ticks[0] < 0 or any(a >= b for a, b in zip(ticks, ticks[1:])):
            raise RuntimeError('invalid native planning clock')
        indexes = {}
        for index, sample in value.indexes:
            if index in indexes:
                raise RuntimeError('duplicate planning mark')
            indexes[index] = sample
        return cls(value.pcm, indexes, frames, ticks)


def _edges(item, count):
    result = [0] + [item.indexes[i] for i in range(1, count)] + [len(item.pcm) // 2]
    if any(a >= b for a, b in zip(result, result[1:])):
        raise RuntimeError('invalid native context boundaries')
    return tuple(result)


def _dense(item, column):
    values = [row.controls[column] for row in item.frames]
    return [_f32(_interp(i, item.ticks, values)) for i in range(item.ticks[-1] + 1)]


def _fit_context_pitch(item, bounds, pilot_pitch, pilot_bounds, parts, language, rate):
    f0 = _dense(item, 1)
    av = _dense(item, 2)
    for i, part in enumerate(parts):
        if part.language != language:
            continue
        start = round(bounds[i] * 1000 / rate)
        end = min(round(bounds[i + 1] * 1000 / rate), len(f0))
        if not 0 <= start < end <= len(f0):
            raise RuntimeError('invalid native pitch interval')
        voiced = [j for j in range(start, end) if av[j] > 0]
        if len(voiced) < 2:
            raise RuntimeError('no voiced pitch anchor')
        first, last = voiced[0], voiced[-1]
        a, b = pilot_bounds[i], pilot_bounds[i + 1]
        if not 0 <= a < b < len(pilot_pitch):
            raise RuntimeError('invalid shared pitch interval')
        for row in item.frames:
            tick = row.tick
            if tick < start or tick >= end:
                continue
            t = a if tick < first else b if tick > last else a + (b - a) * (tick - first) / (last - first)
            lo = int(t)
            f0[tick] = _f32(pilot_pitch[lo] if lo >= len(pilot_pitch) - 1 else
                            pilot_pitch[lo] + (pilot_pitch[lo + 1] - pilot_pitch[lo]) * (t - lo))
    for row in item.frames:
        row.controls[1] = f0[row.tick]


@dataclass
class SentencePlan:
    frames: object
    counts: tuple[int, ...]
    contexts: dict[str, CapturedRows]
    edges: dict[str, tuple[int, ...]]
    word_markers: dict[tuple[int, int], int]
    pilot: CapturedRows
    pilot_bounds: tuple[int, ...]
    source_marks: tuple[tuple[int, int, int], ...] = ()


def context_plan(parts, fa, en, rate, *, maximum=60000, source_events=()):
    parts = tuple(parts)
    if not parts or len(parts) > 32 or {p.language for p in parts} != {'fa', 'en-us'}:
        raise ValueError('expected a bounded mixed English/Persian clause')
    if any(p.text.rstrip().endswith(('.', '?', '!', ',', ';', ':', '،', '؟', '؛')) for p in parts[:-1]):
        raise ValueError('explicit punctuation ends this clause')
    if any(SAMPLE_RATES.get(voice._dll.eciGetParam(voice._handle, SAMPLE_RATE)) != rate for voice in (fa, en)):
        raise ValueError('source rates do not match the replay clock')
    capture_api = FrameCapture(fa._dll)
    def get(voice, planned):
        return CapturedRows.from_capture(capture_api.capture(voice, planned, maximum=maximum))
    pilot_parts = []
    for index, part in enumerate(parts):
        if index:
            pilot_parts.extend((' ', EciIndex(index)))
        proxy = part.text if part.language == 'fa' else english_proxy(en, part.text)
        if any(not native_tag_accepted(m.group(), fa._phonemes_for_probe(m.group())) for m in _TAG.finditer(proxy)):
            raise ValueError('Persian proxy was rejected')
        pilot_parts.append(proxy)
    pilot = get(fa, pilot_parts)
    pilot_pitch = _dense(pilot, 1)
    pilot_bounds = (0, *(round(pilot.indexes[i] * 1000 / rate) for i in range(1, len(parts))),
                    min(_last_audible(pilot.pcm, rate), len(pilot_pitch) - 1))
    if any(a >= b for a, b in zip(pilot_bounds, pilot_bounds[1:])):
        raise RuntimeError('invalid shared sentence boundaries')
    word_markers = _word_markers(parts)
    source_events = tuple(source_events)
    next_mark = max((len(parts) - 1, *word_markers.values())) + 1
    source_marks = []
    for order, (segment, offset, value) in enumerate(source_events):
        if (type(segment) is not int or not 0 <= segment < len(parts) or
            type(offset) is not int or not 0 <= offset <= len(parts[segment].text) or
            type(value) is not int or not 0 <= value <= 0x7fffffff):
            raise ValueError('invalid source index')
        source_marks.append((segment, offset, order, next_mark + order, value))
    contexts, edges = {}, {}
    for language, voice in (('fa', fa), ('en-us', en)):
        item = get(voice, _context(parts, language, en, fa, word_markers, source_marks))
        bounds = _edges(item, len(parts))
        _fit_context_pitch(item, bounds, pilot_pitch, pilot_bounds, parts, language, rate)
        contexts[language], edges[language] = item, bounds
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
    for segment, _offset, _order, mark, value in source_marks:
        item = contexts[parts[segment].language]
        source = bisect_left(item.ticks, round(item.indexes[mark] * 1000 / rate))
        local = max(0, min(counts[segment], source - firsts[segment]))
        replay_marks.append((segment, local, value))
    if any(a[:2] > b[:2] for a, b in zip(replay_marks, replay_marks[1:])):
        raise RuntimeError('source indexes are not ordered')
    return SentencePlan(frames, tuple(counts), contexts, edges, word_markers, pilot,
                        tuple(pilot_bounds), tuple(replay_marks))


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
    output = _copy_frames(original)
    starts, frame_edges = _starts(original), _edges_by_count(plan.counts)
    bridged = []
    for i in range(1, len(parts)):
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
            source_av = [row.controls[2] for row in item.frames]
            source_af = [row.controls[8] for row in item.frames]
            if any(_interp(t, item.ticks, source_av) <= 0 or _interp(t, item.ticks, source_af) != 0
                   for t in positions):
                break
            sources.append((item, positions))
        if len(sources) != 2:
            continue
        for column in TRACT:
            a, pa = sources[0]
            b, pb = sources[1]
            for j, delta, ta, tb in zip(indexes, relative, pa, pb):
                weight = _ease((delta + width) / (2 * width))
                output[j].controls[column] = (_interp_frame(ta, a, column) * (1 - weight) +
                                              _interp_frame(tb, b, column) * weight)
        bridged.append(i)
    plan.frames = output
    return tuple(bridged)


def direct_ach(plan, parts):
    """Remove a voiced X lead-in only where the selected fricative follows."""
    frames, boundaries = plan.frames, _edges_by_count(plan.counts)
    cuts, ranges, counts = [], [], list(plan.counts)
    for i in range(1, len(parts)):
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
    return tuple(cuts)


@dataclass(frozen=True)
class GermanAch:
    times: tuple[float, ...]
    rows: tuple[tuple[float, ...], ...]
    noise_times: tuple[float, ...]
    noise_f3: tuple[float, ...]


def german_ach_template(voice, *, maximum=60000):
    tag = "`[.1'aj'x.0u]"
    if tag not in voice._phonemes_for_probe(tag):
        raise RuntimeError('German transition reference was rejected')
    item = CapturedRows.from_capture(FrameCapture(voice._dll).capture(voice, (tag,), maximum=maximum))
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
    starts, bounds = _starts(original), _edges_by_count(plan.counts)
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
    return tuple(extended)


def apply_german_ach(plan, parts, cuts, template, en):
    """Apply the accepted German-guided noise and resonance at A-to-X."""
    original, output = plan.frames, _copy_frames(plan.frames)
    starts, bounds = _starts(original), _edges_by_count(plan.counts)
    chosen = []
    for edge, _ in cuts:
        if (parts[edge - 1].language != 'en-us' or
                not _english_final_a(en, parts[edge - 1].text)):
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
    starts, bounds = _starts(original), _edges_by_count(plan.counts)
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
    starts, bounds = _starts(original), _edges_by_count(plan.counts)
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
    starts, bounds = _starts(original), _edges_by_count(plan.counts)
    changed = []
    for edge, _ in edges:
        if (parts[edge - 1].language != 'en-us' or
                not _english_final_a(en, parts[edge - 1].text)):
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
        for word in range(1, len(words)):
            sample = item.indexes[plan.word_markers[i, word]]
            source = bisect_left(item.ticks, round(sample * 1000 / rate))
            local = source - source_first - removed
            if not 0 <= local < len(ticks):
                raise RuntimeError('Persian word mark is outside its context')
            boundaries[word] = ticks[local]
        tags = tuple(match.group() for match in words)
        separators = tuple(part.text[a.end():b.start()] for a, b in zip(words, words[1:]))
        glides = plan_u_a_glides(section, ticks, tags, separators, boundaries)
        apply_u_a_glides(section, ticks, glides)
        changes.extend((i, *glide.summary) for glide in glides)
        offset += count
    return tuple(changes)


def selected_sentence(plan, parts, en, german, rate):
    """Apply the user's selected sentence and A-to-Ach transition."""
    original_counts = plan.counts
    bridged = bridge_sonorants(plan, parts, rate)
    cuts = direct_ach(plan, parts)
    template = german_ach_template(german) if cuts else None
    extended = extend_english_a(plan, parts, cuts, en) if cuts else ()
    ach = apply_german_ach(plan, parts, cuts, template, en) if template else ()
    released = smooth_ach_release(plan, cuts) if cuts else ()
    f3 = apply_german_f3(plan, ach, template) if ach else ()
    faded = fade_english_voice(plan, parts, ach, en) if ach else ()
    glides = apply_persian_u_a(plan, parts, rate, original_counts)
    # Source filters depend on the edited noise-band values, not on the
    # values from the unmodified context capture.
    for row in plan.frames:
        c = row.controls
        row.s_frication = int(c[8] > 0 and (c[38] > c[37] + 10 or c[39] > c[37] + 10))
    return bridged, cuts, extended, ach, released, f3, faded, glides
