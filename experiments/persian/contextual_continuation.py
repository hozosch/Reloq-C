"""Experimental cross-language continuation using full-context native frames.

Each core plans the whole clause, with silent syllable proxies for the other
language. Actual parts retain their own core's controls. They are combined
before rendering, in one continuous Klatt session with one shared F0 plan.
This prototype pre-renders, uses process-wide hooks, and is not an NVDA API.
"""
from __future__ import annotations

import ctypes
from dataclasses import dataclass
from pathlib import Path
import re
import sys
import tempfile

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import (
    BREATHINESS, EQ_FILTER_BASE, EQ_FILTER_COUNT, EQ_FILTER_STRIDE, EQ_HIGH,
    EQ_LOW, EQ_MID, GENDER, HEAD_SIZE, HIGH_RATE_PROFILE, MELLOW_MODE,
    NATIVE_INFLECTION, NATIVE_PITCH, NATIVE_SPEED, NATIVE_VOLUME, ROUGHNESS,
    SAMPLE_RATE, SAMPLE_RATES, DirectEciEngine, EciIndex,
)
from _reloq_persian_glide import apply_u_a_glides, plan_u_a_glides
from german_ach_transition import (
    apply_german_ach, apply_german_f3, apply_german_flow, capture_german_ach,
    extend_english_a, fade_english_a_voice,
)
from shared_sentence_contour import Captured, Comparison, _fit_pitch, capture, compare, english_proxy
from text_frontend import SpeechPart
from text_frontend import native_tag_accepted


# Approximate only the silent planning context. The real words are always
# synthesized by their own language core; unknown phones use the older proxy.
_EN_TO_FA = {**{c: c for c in 'aAeioubdfghjklmnoprstvzSZ'},
             'E': 'e', 'I': 'i', 'O': 'o', 'U': 'u', 'c': 'a',
             'H': 'A', 'x': 'A', 'X': 'e', 'Y': 'A', 'y': 'j',
             'w': 'v', 'G': 'n', 'D': 'd', 'T': 't', 'F': 't'}
_FA_TO_EN = {**{c: c for c in 'aAioubdfghklmnprstvzSZ'},
             'e': 'E', 'j': 'y', 'G': 'h', 'R': 'k', 'J': 'y', 'C': 'S'}


def _converted_tags(text: str, table: dict[str, str]) -> list[str] | None:
    tags = re.findall(r'`\[([^]]+)\]', text)
    converted = []
    for tag in tags:
        chars = []
        for ch in tag:
            if ch in '.012':
                chars.append(ch)
            elif ch in "':":
                continue  # the other core may not support this length mark
            elif ch in table:
                chars.append(table[ch])
            else:
                return None
        converted.append('`[' + ''.join(chars) + ']')
    return converted or None


def _punctuation(text: str) -> str:
    match = re.search(r'([.?!,;:])\s*$', text)
    return match.group(1) if match else ''


def english_phone_proxy(en: DirectEciEngine, fa: DirectEciEngine, text: str) -> str:
    """Represent English boundary consonants in the silent Persian plan."""
    tags = _converted_tags(en._phonemes_for_probe(text), _EN_TO_FA)
    if tags and all(native_tag_accepted(tag, fa._phonemes_for_probe(tag)) for tag in tags):
        return ' '.join(tags) + _punctuation(text)
    return english_proxy(en, text)


def persian_proxy_for_english(text: str, en: DirectEciEngine | None = None) -> str:
    """Approximate only Persian syllable count and stress for silent planning."""
    if en is not None:
        tags = _converted_tags(text, _FA_TO_EN)
        if tags:
            # English may normalize the stress digit for an isolated tag.
            # Verify its remaining phone sequence instead of trusting a
            # rejected tag that the engine would spell aloud as text.
            def sequence(value: str) -> str:
                return re.sub(r'\.[012]', '', value)
            accepted = True
            for tag in tags:
                displayed = re.findall(r'`\[([^]]+)\]', en._phonemes_for_probe(tag))
                if len(displayed) != 1 or sequence(tag[2:-1]) != sequence(displayed[0]):
                    accepted = False
                    break
            if accepted:
                return ' '.join(tags) + _punctuation(text)
    tags = re.findall(r'`\[([^]]+)\]', text)
    vowels = {'A': 'A', 'a': 'a', 'e': 'E', 'i': 'i', 'o': 'o', 'u': 'u'}
    result = []
    for tag in tags:
        syllables = re.findall(r'\.([012])([^.]+)', tag)
        if not syllables:
            raise ValueError(f'Persian planning tag has no syllables: {tag!r}')
        pieces = []
        for stress, body in syllables:
            vowel = next((vowels[ch] for ch in body if ch in vowels), 'A')
            pieces.append('.' + stress + 'm' + vowel)
        result.append('`[' + ''.join(pieces) + ']')
    if not result:
        raise ValueError(f'Persian planning part has no tags: {text!r}')
    punctuation = re.search(r'([.?!,;:])\s*$', text)
    return ' '.join(result) + (punctuation.group(1) if punctuation else '')


def _context(parts: tuple[SpeechPart, ...], language: str,
             en: DirectEciEngine, fa: DirectEciEngine, phone_context: bool,
             word_markers: dict[tuple[int, int], int] | None = None) -> list:
    planned = []
    for i, part in enumerate(parts):
        if i:
            planned.extend([' ', EciIndex(i)])
        if part.language == language:
            if language == 'fa' and word_markers:
                words = list(re.finditer(r'`\[[^]]+\]', part.text))
                cursor = 0
                for word, match in enumerate(words):
                    if word and (i, word) in word_markers:
                        planned.append(part.text[cursor:match.start()])
                        planned.append(EciIndex(word_markers[i, word]))
                        cursor = match.start()
                planned.append(part.text[cursor:])
            else:
                planned.append(part.text)
        elif language == 'fa':
            planned.append(english_phone_proxy(en, fa, part.text) if phone_context
                           else english_proxy(en, part.text))
        else:
            planned.append(persian_proxy_for_english(part.text, en if phone_context else None))
    return planned


def _persian_word_markers(parts: tuple[SpeechPart, ...]) -> dict[tuple[int, int], int]:
    """Allocate independent marks for word boundaries within Persian parts."""
    markers = {}
    next_index = len(parts)
    for i, part in enumerate(parts):
        if part.language == 'fa':
            words = list(re.finditer(r'`\[[^]]+\]', part.text))
            for word in range(1, len(words)):
                markers[i, word] = next_index
                next_index += 1
    return markers


def _sample_edges(item: Captured, count: int) -> list[int]:
    edges = [0]
    for i in range(1, count):
        if i not in item.indexes:
            raise RuntimeError(f'missing contextual word boundary {i}')
        edges.append(item.indexes[i])
    edges.append(len(item.pcm) // 2)
    if any(a >= b for a, b in zip(edges, edges[1:])):
        raise RuntimeError(f'invalid contextual boundaries: {edges!r}')
    return edges


def replay(library: Path, controls: np.ndarray, rate: int, gender: int = 0, head: int = 50) -> bytes:
    """Render native frames in one test session, retaining oscillator state."""
    values = np.ascontiguousarray(controls, dtype='<f4')
    if values.ndim != 2 or values.shape[1] != 62 or not len(values):
        raise ValueError('expected nonempty 62-control frames')
    dll = ctypes.CDLL(str(library.resolve()))
    callback_type = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.c_void_p,
                                    ctypes.POINTER(ctypes.c_int16), ctypes.c_size_t)
    output = bytearray()

    @callback_type
    def receive(_context, pcm, count):
        output.extend(ctypes.string_at(pcm, count * 2))
        return 0

    function = dll.reloq_fa_probe_replay
    function.argtypes = (ctypes.POINTER(ctypes.c_float), ctypes.c_size_t, ctypes.c_int,
                         ctypes.c_int, ctypes.c_int, callback_type, ctypes.c_void_p)
    function.restype = ctypes.c_int
    status = function(values.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), len(values),
                      rate, gender, head, receive, None)
    if status or not output:
        raise RuntimeError(f'continuous native-frame replay failed: {status}')
    return bytes(output)


@dataclass(frozen=True)
class ContextualComparison:
    prior: Comparison
    connected: bytes
    controls: np.ndarray
    context_before: dict[str, Captured]
    context_after: dict[str, Captured]
    edges: dict[str, tuple[int, ...]]
    frame_counts: tuple[int, ...]
    bridged_edges: tuple[int, ...] = ()
    ach_onset_edges: tuple[int, ...] = ()
    ach_frication_cuts: tuple[tuple[int, float], ...] = ()
    ach_release_edges: tuple[tuple[int, float], ...] = ()
    german_ach_edges: tuple[tuple[int, float], ...] = ()
    german_f3_edges: tuple[tuple[int, float], ...] = ()
    german_flow_edges: tuple[tuple[int, float, float], ...] = ()
    english_a_extensions: tuple[tuple[int, float], ...] = ()
    english_voice_releases: tuple[tuple[int, float, float], ...] = ()
    # Part index followed by word, first/end frame and F0 peak/end frame;
    # the frame indexes are local to the Persian part.
    persian_vowel_glides: tuple[tuple[int, int, int, int, int, int], ...] = ()


# Formant and nasal filter parameters. Excitation and frication controls are
# deliberately excluded. Entire windows with frication or silence are skipped.
_TRACT = (9, 10, *range(13, 35))


def _bridge_sonorant_edges(controls: np.ndarray, counts: tuple[int, ...],
                           parts: tuple[SpeechPart, ...],
                           after: dict[str, Captured], edges: dict[str, tuple[int, ...]],
                           rate: int, half_window_ms: float) -> tuple[np.ndarray, tuple[int, ...]]:
    """Mix nearby tract trajectories without adding time or changing sources.

    Both native plans include a phoneme-shaped proxy for the other language.
    Use those two local trajectories only when the whole window is voiced and
    has no frication. Otherwise keep each core's native controls unchanged.
    """
    if not 0 < half_window_ms <= 50:
        raise ValueError('unsupported transition window')
    output = controls.copy()
    starts = np.concatenate(([0.0], np.cumsum(controls[:, 0], dtype=np.float64)))
    frame_edges = np.cumsum(counts)
    bridged = []
    for i in range(1, len(parts)):
        boundary = int(frame_edges[i - 1])
        at = starts[boundary]
        left_start = starts[0 if i == 1 else int(frame_edges[i - 2])]
        right_end = starts[int(frame_edges[i])]
        width = min(half_window_ms, (at - left_start) / 2, (right_end - at) / 2)
        if width < 2:
            continue
        indexes = np.flatnonzero((starts[:-1] >= at - width) & (starts[:-1] <= at + width))
        if len(indexes) < 3:
            continue
        actual = controls[indexes]
        if np.any(actual[:, 2] <= 0) or np.any(actual[:, 8] != 0):
            continue
        relative = starts[indexes] - at
        sources = []
        for part, edge_index in ((parts[i - 1], i), (parts[i], i)):
            item = after[part.language]
            if item.frames is None:
                raise RuntimeError('missing context frames')
            frames = item.frames
            tick = frames['tick'].astype(np.float64)
            sample_edge_ms = round(edges[part.language][edge_index] * 1000 / rate)
            frame_index = int(np.searchsorted(tick, sample_edge_ms, side='left'))
            if frame_index >= len(tick):
                break
            positions = tick[frame_index] + relative
            if positions[0] < tick[0] or positions[-1] > tick[-1]:
                break
            raw = frames['controls']
            # A proxy with a stop or fricative cannot provide a trustworthy
            # voiced filter trajectory. Leave that entire edge untouched.
            for source in (2, 8):
                values = np.interp(positions, tick, raw[:, source])
                if (source == 2 and np.any(values <= 0)) or (source == 8 and np.any(values != 0)):
                    break
            else:
                sources.append(np.stack([np.interp(positions, tick, raw[:, c])
                                         for c in _TRACT], axis=1))
                continue
            break
        if len(sources) != 2:
            continue
        fraction = (relative + width) / (2 * width)
        weight = fraction * fraction * (3 - 2 * fraction)
        output[np.ix_(indexes, _TRACT)] = (sources[0] * (1 - weight[:, None]) +
                                          sources[1] * weight[:, None])
        bridged.append(i)
    return output, tuple(bridged)


def _prepare_ach_onsets(controls: np.ndarray, counts: tuple[int, ...],
                        parts: tuple[SpeechPart, ...], already_bridged: tuple[int, ...],
                        window_ms: float) -> tuple[np.ndarray, tuple[int, ...]]:
    """Anticipate a Persian X onset in the preceding English voiced tail.

    No frame in the X or its friction source is altered. This experiment is
    limited to an initial Persian G marker following a voiced, non-fricated
    English tail; stops and noisy endings are left to their native context.
    """
    if not 0 < window_ms <= 50:
        raise ValueError('unsupported Ach transition window')
    output = controls.copy()
    starts = np.concatenate(([0.0], np.cumsum(controls[:, 0], dtype=np.float64)))
    frame_edges = np.cumsum(counts)
    adjusted = []
    for i in range(1, len(parts)):
        if (i in already_bridged or parts[i - 1].language != 'en-us' or
                parts[i].language != 'fa' or
                not re.match(r'^\s*`\[\.[012]G', parts[i].text)):
            continue
        boundary = int(frame_edges[i - 1])
        previous_start = 0 if i == 1 else int(frame_edges[i - 2])
        at = starts[boundary]
        width = min(window_ms, (at - starts[previous_start]) / 2)
        indexes = np.flatnonzero((starts[:-1] >= at - width) & (starts[:-1] < at))
        if len(indexes) < 3 or indexes[0] < previous_start:
            continue
        tail = controls[indexes]
        first = controls[boundary]
        if first[2] <= 0 or first[8] != 0:
            continue
        # At fast speeds the preceding consonant can enter the 25 ms window.
        # Use only the continuous voiced tail after its last release.
        bad = np.flatnonzero((tail[:, 2] <= 0) | (tail[:, 8] != 0))
        if len(bad):
            indexes = indexes[int(bad[-1]) + 1:]
        if len(indexes) < 3:
            continue
        # Confirm an actual fricative onset follows, not just a G marker that
        # was deleted or reanalysed. Its original frames remain untouched.
        following_end = int(frame_edges[i])
        if not np.any(controls[boundary:min(boundary + 50, following_end), 8] > 0):
            continue
        time = starts[indexes]
        span = time[-1] - time[0]
        if span <= 0:
            continue
        phase = (time - time[0]) / span
        weight = phase * phase * (3 - 2 * phase)
        target = first[list(_TRACT)] - controls[boundary - 1, list(_TRACT)]
        output[np.ix_(indexes, _TRACT)] += weight[:, None] * target[None, :]
        adjusted.append(i)
    return output, tuple(adjusted)


def _direct_ach_frication(controls: np.ndarray, counts: tuple[int, ...],
                          parts: tuple[SpeechPart, ...]
                          ) -> tuple[np.ndarray, tuple[int, ...], tuple[tuple[int, float], ...]]:
    """Replace a voiced X lead-in with its native unvoiced friction onset.

    Only a Persian G marker after a voiced English tail is eligible. Preserve
    the preceding English frames and the selected native fricative frames;
    reject unexpected silence or a missing fricative rather than cut a word.
    The returned counts refer to the shortened, continuously replayed output.
    """
    frame_edges = np.cumsum(counts)
    keep = np.ones(len(controls), dtype=bool)
    result_counts = list(counts)
    cuts = []
    for i in range(1, len(parts)):
        if (parts[i - 1].language != 'en-us' or parts[i].language != 'fa' or
                not re.match(r'^\s*`\[\.[012]G', parts[i].text)):
            continue
        boundary = int(frame_edges[i - 1])
        end = int(frame_edges[i])
        if (controls[boundary - 1, 2] <= 0 or controls[boundary - 1, 8] != 0 or
                controls[boundary, 2] <= 0):
            continue
        section = controls[boundary:end]
        times = np.cumsum(section[:, 0], dtype=np.float64)
        # The selected Ach profile has prominent A2 and A4 noise bands.
        # Require that fingerprint as well as an unvoiced source, so a later
        # unrelated fricative cannot be mistaken for the marked onset.
        hits = np.flatnonzero((section[:, 2] == 0) & (section[:, 8] >= 20) &
                              (section[:, 36] >= 50) & (section[:, 38] >= 35) &
                              (section[:, 36] > section[:, 37] + 10) & (times <= 120))
        if not len(hits):
            continue
        cut = int(hits[0])
        if cut == 0 or np.any(section[:cut, 2] <= 0):
            continue
        keep[boundary:boundary + cut] = False
        result_counts[i] -= cut
        cuts.append((i, float(times[cut - 1])))
    return controls[keep], tuple(result_counts), tuple(cuts)


def _smooth_direct_ach_release(controls: np.ndarray, counts: tuple[int, ...],
                               cuts: tuple[tuple[int, float], ...]
                               ) -> tuple[np.ndarray, tuple[tuple[int, float], ...]]:
    """Keep the preceding vowel's tract briefly while X noise starts.

    The still-decaying last glottal period otherwise encounters an immediate
    jump from the English vowel's tract to X's very different filter. Blend
    for about one pitch period (bounded to 4–15 ms) while keeping both the
    unvoiced source and native frication levels from the first X frame.
    """
    output = controls.copy()
    starts = np.concatenate(([0.0], np.cumsum(controls[:, 0], dtype=np.float64)))
    frame_edges = np.cumsum(counts)
    adjusted = []
    for edge, _removed_ms in cuts:
        boundary = int(frame_edges[edge - 1])
        end = int(frame_edges[edge])
        before, first = controls[boundary - 1], controls[boundary]
        if before[2] <= 0 or before[1] <= 0 or first[2] != 0 or first[8] < 20:
            continue
        duration = min(15.0, max(4.0, 10000.0 / float(before[1])))
        frames = controls[boundary:end]
        eligible = (frames[:, 2] == 0) & (frames[:, 8] >= 20)
        invalid = np.flatnonzero(~eligible)
        usable = int(invalid[0]) if len(invalid) else len(frames)
        relative = starts[boundary:boundary + usable] - starts[boundary]
        relative = relative[relative <= duration]
        if len(relative) < 2 or relative[-1] <= 0:
            continue
        # If the noise portion is shorter than one period, finish the blend
        # before its last available frame; never alter the following vowel.
        span = min(duration, float(relative[-1]))
        fraction = relative / span
        weight = 1 - fraction * fraction * (3 - 2 * fraction)
        delta = before[list(_TRACT)] - first[list(_TRACT)]
        indexes = np.arange(boundary, boundary + len(relative))
        output[np.ix_(indexes, _TRACT)] += weight[:, None] * delta[None, :]
        adjusted.append((edge, span))
    return output, tuple(adjusted)


def _validate_replay_settings(voices: tuple[DirectEciEngine, ...], rate: int,
                              gender: int, head: int) -> None:
    """Reject settings that the private replay session does not implement."""
    expected = {
        GENDER: gender, HEAD_SIZE: head, ROUGHNESS: 0, BREATHINESS: 0,
        MELLOW_MODE: 0, NATIVE_VOLUME: 100, HIGH_RATE_PROFILE: 3,
        EQ_LOW: 12, EQ_MID: 12, EQ_HIGH: 12,
    }
    for voice in voices:
        voice._assert_owner()
        get = lambda param: voice._dll.eciGetParam(voice._handle, param)
        if SAMPLE_RATES.get(get(SAMPLE_RATE)) != rate:
            raise ValueError('replay rate does not match a source engine')
        if any(get(param) != value for param, value in expected.items()):
            raise ValueError('source voice settings unsupported by private replay')
        if any(get(EQ_FILTER_BASE + i * EQ_FILTER_STRIDE) != 0
               for i in range(EQ_FILTER_COUNT)):
            raise ValueError('custom equalizer filters unsupported by private replay')


def contextual_compare(parts: tuple[SpeechPart, ...], fa: DirectEciEngine,
                       en: DirectEciEngine, library: Path, rate: int,
                       gender: int = 0, head: int = 50, *, phone_context: bool = False,
                       bridge_ms: float = 0, ach_onset_ms: float = 0,
                       direct_ach: bool = False, ach_release: bool = False,
                       german: DirectEciEngine | None = None,
                       german_formants: bool = False, german_flow: int = 0,
                       german_overlap: float = 1.0,
                       english_a_stretch: float = 1.0,
                       voice_release_ms: float = 0.0,
                       voice_release_db: float = 12.0,
                       selected_u_a: bool = False) -> ContextualComparison:
    if bridge_ms and not phone_context:
        raise ValueError('the transition bridge needs phone-shaped context')
    if ach_onset_ms and (not phone_context or not bridge_ms):
        raise ValueError('the Ach onset test needs the accepted transition')
    if direct_ach and (not phone_context or not bridge_ms or ach_onset_ms):
        raise ValueError('direct Ach needs variant 2 without the rejected onset trial')
    if ach_release and not direct_ach:
        raise ValueError('the vowel-release test needs direct Ach friction')
    if (german is not None and not direct_ach) or (german_formants and german is None):
        raise ValueError('German transition needs direct Ach and a German reference')
    if german_flow not in (0, 1, 2) or (german_flow and not german_formants):
        raise ValueError('German flow needs the selected German resonance transition')
    if not 0.25 <= german_overlap <= 1.0 or not 1.0 <= english_a_stretch <= 1.5:
        raise ValueError('unsupported English A transition settings')
    if not 0 <= voice_release_ms <= 50 or not 0 < voice_release_db <= 24:
        raise ValueError('unsupported English voice release')
    if ((german_overlap != 1.0 or english_a_stretch != 1.0 or voice_release_ms) and
            (not german_formants or german is None or german_flow)):
        raise ValueError('English A trials need the original German resonance baseline')
    _validate_replay_settings((fa, en) if german is None else (fa, en, german), rate, gender, head)
    if german is not None:
        for param in (NATIVE_SPEED, NATIVE_PITCH, NATIVE_INFLECTION):
            values = [voice._dll.eciGetParam(voice._handle, param) for voice in (fa, en, german)]
            if len(set(values)) != 1:
                raise ValueError('German reference needs matching speed and voice settings')
    prior = compare(parts, fa, en, rate)
    voices = {'fa': fa, 'en-us': en}
    word_markers = _persian_word_markers(parts) if selected_u_a else {}
    contexts = {lang: _context(parts, lang, en, fa, phone_context, word_markers)
                for lang in voices}
    with tempfile.TemporaryDirectory(prefix='reloq-fa-context-') as name:
        folder = Path(name)
        before, after, edges = {}, {}, {}
        for language, voice in voices.items():
            item = capture(voice, contexts[language], folder / (language + '-before.f0'),
                           frames=folder / (language + '-before.frames'))
            bounds = _sample_edges(item, len(parts))
            edges[language] = tuple(bounds)
            whole_plan = item.f0.copy().astype('<f4')
            for i, part in enumerate(parts):
                if part.language != language:
                    continue
                start = round(bounds[i] * 1000 / rate)
                end = min(round(bounds[i + 1] * 1000 / rate), len(item.f0))
                if not 0 <= start < end <= len(item.f0):
                    raise RuntimeError('invalid contextual F0 interval')
                segment = Captured(item.pcm[bounds[i] * 2:bounds[i + 1] * 2],
                                   item.f0[start:end], item.av[start:end], {})
                target = _fit_pitch(prior.pilot, prior.pilot_boundaries_ms[i],
                                    prior.pilot_boundaries_ms[i + 1], segment)
                whole_plan[start:end] = target
            f0_file = folder / (language + '-plan')
            whole_plan.tofile(f0_file)
            adjusted = capture(voice, contexts[language], folder / (language + '-after.f0'),
                               f0_file, folder / (language + '-after.frames'))
            if _sample_edges(adjusted, len(parts)) != bounds or len(adjusted.f0) != len(item.f0):
                raise RuntimeError('shared F0 changed contextual timing')
            if language == 'fa' and any(item.indexes.get(mark) != adjusted.indexes.get(mark)
                                        or mark not in item.indexes for mark in word_markers.values()):
                raise RuntimeError('shared F0 changed Persian word timing')
            if not np.allclose(adjusted.f0, whole_plan, rtol=0, atol=.01):
                raise RuntimeError('contextual F0 plan did not reach native frames')
            if not np.array_equal(item.av, adjusted.av):
                raise RuntimeError('shared F0 changed native voicing targets')
            before[language], after[language] = item, adjusted
        chunks, counts = [], []
        for i, part in enumerate(parts):
            item = after[part.language]
            bounds = edges[part.language]
            if item.frames is None:
                raise RuntimeError('native control frames unavailable')
            start = round(bounds[i] * 1000 / rate)
            end = round(bounds[i + 1] * 1000 / rate)
            ticks = item.frames['tick']
            first = int(np.searchsorted(ticks, start, side='left'))
            last = int(np.searchsorted(ticks, end, side='left'))
            frame = item.frames['controls'][first:last]
            if not len(frame):
                raise RuntimeError('empty contextual audio section')
            chunks.append(frame)
            counts.append(len(frame))
        controls = np.concatenate(chunks)
        bridged = ()
        if bridge_ms:
            controls, bridged = _bridge_sonorant_edges(controls, tuple(counts), parts, after, edges,
                                                       rate, bridge_ms)
        ach_edges = ()
        if ach_onset_ms:
            controls, ach_edges = _prepare_ach_onsets(controls, tuple(counts), parts, bridged,
                                                       ach_onset_ms)
        ach_cuts = ()
        if direct_ach:
            controls, new_counts, ach_cuts = _direct_ach_frication(controls, tuple(counts), parts)
            counts = list(new_counts)
        english_a_extensions = ()
        if english_a_stretch > 1.0 and ach_cuts:
            controls, english_a_extensions = extend_english_a(
                controls, tuple(counts), parts, ach_cuts, en, english_a_stretch)
        german_edges = ()
        if german is not None and ach_cuts:
            template = capture_german_ach(german)
            controls, german_edges = apply_german_ach(controls, tuple(counts), parts, ach_cuts,
                                                      template, en, formants=german_formants,
                                                      overlap_scale=german_overlap)
        release_edges = ()
        if ach_release:
            controls, release_edges = _smooth_direct_ach_release(controls, tuple(counts), ach_cuts)
        german_f3_edges = ()
        if german_formants and german_edges:
            controls, german_f3_edges = apply_german_f3(controls, tuple(counts), german_edges, template)
        german_flow_edges = ()
        if german_flow and german_edges:
            controls, german_flow_edges = apply_german_flow(controls, tuple(counts), german_edges, template,
                                                            include_f2=(german_flow == 2))
        voice_releases = ()
        if voice_release_ms and german_edges:
            controls, voice_releases = fade_english_a_voice(
                controls, tuple(counts), parts, german_edges, en, voice_release_ms, voice_release_db)
        vowel_glides = []
        if selected_u_a:
            item = after['fa']
            for i, part in enumerate(parts):
                if part.language != 'fa':
                    continue
                words = list(re.finditer(r'`\[[^]]+\]', part.text))
                if len(words) < 2:
                    continue
                frame_first = sum(counts[:i])
                frame_last = frame_first + counts[i]
                section = controls[frame_first:frame_last]
                ticks = np.concatenate(([0.0], np.cumsum(section[:, 0], dtype=np.float64)))[:-1]
                source_ticks = item.frames['tick']
                start = round(edges['fa'][i] * 1000 / rate)
                source_first = int(np.searchsorted(source_ticks, start, side='left'))
                removed = len(chunks[i]) - counts[i]
                if removed < 0:
                    raise RuntimeError('invalid Persian frame mapping')
                boundaries = {}
                for word in range(1, len(words)):
                    sample = item.indexes[word_markers[i, word]]
                    source = int(np.searchsorted(source_ticks, round(sample * 1000 / rate), side='left'))
                    local = source - source_first - removed
                    if not 0 <= local < len(ticks):
                        raise RuntimeError('Persian word marker outside its native section')
                    boundaries[word] = float(ticks[local])
                tags = tuple(word.group() for word in words)
                separators = tuple(part.text[a.end():b.start()] for a, b in zip(words, words[1:]))
                plans = plan_u_a_glides(section, ticks, tags, separators, boundaries)
                apply_u_a_glides(section, ticks, plans)
                vowel_glides.extend((i, *plan.summary) for plan in plans)
        connected = replay(library, controls, rate, gender=gender, head=head)
        return ContextualComparison(prior, connected, controls, before, after, edges,
                                    tuple(counts), bridged, ach_edges, ach_cuts, release_edges,
                                    german_edges, german_f3_edges, german_flow_edges, english_a_extensions,
                                    voice_releases, tuple(vowel_glides))


def accepted_contextual_compare(parts: tuple[SpeechPart, ...], fa: DirectEciEngine,
                                en: DirectEciEngine, library: Path, rate: int,
                                gender: int = 0, head: int = 50, *,
                                ach_onset_ms: float = 0,
                                direct_ach: bool = False, ach_release: bool = False,
                                german: DirectEciEngine | None = None,
                                german_formants: bool = False, german_flow: int = 0,
                                german_overlap: float = 1.0,
                                english_a_stretch: float = 1.0,
                                voice_release_ms: float = 0.0,
                                voice_release_db: float = 12.0,
                                selected_u_a: bool = False) -> ContextualComparison:
    """Use the user's accepted second transition variant in the private probe."""
    return contextual_compare(parts, fa, en, library, rate, gender, head,
                              phone_context=True, bridge_ms=25, ach_onset_ms=ach_onset_ms,
                              direct_ach=direct_ach, ach_release=ach_release,
                              german=german, german_formants=german_formants, german_flow=german_flow,
                              german_overlap=german_overlap, english_a_stretch=english_a_stretch,
                              voice_release_ms=voice_release_ms, voice_release_db=voice_release_db,
                              selected_u_a=selected_u_a)


def accepted_german_ach_compare(parts: tuple[SpeechPart, ...], fa: DirectEciEngine,
                                en: DirectEciEngine, german: DirectEciEngine,
                                library: Path, rate: int, gender: int = 0, head: int = 50,
                                *, german_flow: int = 0, german_overlap: float = 1.0,
                                english_a_stretch: float = 1.0,
                                voice_release_ms: float = 0.0,
                                voice_release_db: float = 12.0,
                                selected_u_a: bool = False) -> ContextualComparison:
    """Use the user's preferred German resonance comparison as a baseline."""
    return accepted_contextual_compare(parts, fa, en, library, rate, gender, head,
                                       direct_ach=True, ach_release=True,
                                       german=german, german_formants=True,
                                       german_flow=german_flow, german_overlap=german_overlap,
                                       english_a_stretch=english_a_stretch,
                                       voice_release_ms=voice_release_ms, voice_release_db=voice_release_db,
                                       selected_u_a=selected_u_a)


def accepted_persian_ach_compare(parts: tuple[SpeechPart, ...], fa: DirectEciEngine,
                                 en: DirectEciEngine, german: DirectEciEngine,
                                 library: Path, rate: int, gender: int = 0,
                                 head: int = 50) -> ContextualComparison:
    """Use the user's selected second English A-to-Persian Ach candidate."""
    return accepted_german_ach_compare(parts, fa, en, german, library, rate,
                                       gender=gender, head=head,
                                       english_a_stretch=1.15, voice_release_ms=24)


def accepted_persian_sentence_compare(parts: tuple[SpeechPart, ...], fa: DirectEciEngine,
                                      en: DirectEciEngine, german: DirectEciEngine,
                                      library: Path, rate: int, gender: int = 0,
                                      head: int = 50) -> ContextualComparison:
    """Combine both selected transitions in the private sentence experiment."""
    return accepted_german_ach_compare(parts, fa, en, german, library, rate,
                                       gender=gender, head=head,
                                       english_a_stretch=1.15, voice_release_ms=24,
                                       selected_u_a=True)
