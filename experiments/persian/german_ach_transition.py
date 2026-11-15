"""Private German vowel-to-Ach timing reference for the multilingual probe."""
from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import re
import tempfile

import numpy as np

from _reloq_eci_direct import DirectEciEngine
from shared_sentence_contour import capture
from text_frontend import SpeechPart


# German /aɪ/ has a front glide like the English letter A. The artificial
# phonetic sequence puts that glide before German /x/; it is not a German
# spelling or a claimed linguistic equivalence between the two diphthongs.
GERMAN_TAG = "`[.1'aj'x.0u]"


@dataclass(frozen=True)
class GermanAchTemplate:
    times: np.ndarray
    controls: np.ndarray
    noise_times: np.ndarray
    noise_f3: np.ndarray
    flow_times: np.ndarray
    flow_f2: np.ndarray

    @property
    def duration(self) -> float:
        return float(self.times[-1])


def capture_german_ach(german: DirectEciEngine) -> GermanAchTemplate:
    """Capture German's source ramp and F1/F2 movement into /x/."""
    if GERMAN_TAG not in german._phonemes_for_probe(GERMAN_TAG):
        raise RuntimeError('German core did not accept the transition reference')
    with tempfile.TemporaryDirectory(prefix='reloq-de-ach-plan-') as folder:
        path = Path(folder)
        item = capture(german, [GERMAN_TAG], path / 'f0', frames=path / 'frames')
    if item.frames is None:
        raise RuntimeError('German transition frames unavailable')
    c = item.frames['controls']
    # Locate rising /x/ frication during the voiced diphthong, then its
    # first unvoiced steady-noise frame. Do not use a word/index timestamp.
    starts = np.flatnonzero((c[:, 2] > 0) & (c[:, 8] > 0) &
                            (c[:, 36] > 0) & (c[:, 38] > 0))
    if not len(starts):
        raise RuntimeError('German /x/ onset ramp unavailable')
    first = int(starts[0])
    ends = np.flatnonzero((c[first:, 2] == 0) & (c[first:, 8] >= 20) &
                          (c[first:, 36] >= 50) & (c[first:, 38] >= 35))
    if not len(ends):
        raise RuntimeError('German /x/ has no unvoiced noise target')
    last = first + int(ends[0])
    rows = c[first:last + 1].copy()
    if (last <= first or np.any(rows[:-1, 2] <= 0) or
            rows[-1, 8] <= 0 or rows[-1, 36] <= 0 or rows[-1, 38] <= 0):
        raise RuntimeError('unexpected German transition source sequence')
    times = np.concatenate(([0.0], np.cumsum(rows[:-1, 0], dtype=np.float64)))
    if not 0 < times[-1] <= 100 or np.any(np.diff(times) <= 0):
        raise RuntimeError('invalid German transition clock')
    # Include the unvoiced interval of the same X until the next vowel.
    # It supplies the German F3 direction and its speed-dependent time scale.
    following = c[last:]
    voice = np.flatnonzero(following[:, 2] > 0)
    if not len(voice) or voice[0] < 2:
        raise RuntimeError('German /x/ has no bounded unvoiced interval')
    noise = following[:int(voice[0])]
    noise_times = np.concatenate(([0.0], np.cumsum(noise[:-1, 0], dtype=np.float64)))
    if noise_times[-1] <= 0 or np.any(np.diff(noise_times) <= 0):
        raise RuntimeError('invalid German frication clock')
    voiced = int(voice[0])
    quiet = np.flatnonzero((following[voiced:, 2] > 0) & (following[voiced:, 8] == 0))
    if not len(quiet):
        raise RuntimeError('German /x/ has no following vowel target')
    end = voiced + int(quiet[0])
    if np.any(following[voiced:end + 1, 2] <= 0):
        raise RuntimeError('German /x/ vowel reference contains another boundary')
    flow = following[:end + 1]
    flow_times = np.concatenate(([0.0], np.cumsum(flow[:-1, 0], dtype=np.float64)))
    if flow_times[-1] <= 0 or np.any(np.diff(flow_times) <= 0):
        raise RuntimeError('invalid German X-to-vowel clock')
    return GermanAchTemplate(times, rows, noise_times, noise[:, 15].copy(),
                             flow_times, flow[:, 13].copy())


def _ends_in_english_a(en: DirectEciEngine, text: str) -> bool:
    tags = re.findall(r'`\[([^]]+)\]', en._phonemes_for_probe(text))
    # The native US symbol e is the diphthong in the spoken letter A.
    return bool(tags and re.search(r'e$', tags[-1]))


def apply_german_ach(controls: np.ndarray, counts: tuple[int, ...],
                     parts: tuple[SpeechPart, ...], cuts: tuple[tuple[int, float], ...],
                     template: GermanAchTemplate, en: DirectEciEngine, *,
                     formants: bool, overlap_scale: float = 1.0
                     ) -> tuple[np.ndarray, tuple[tuple[int, float], ...]]:
    """Overlay German's X-noise ramp on existing English A frames only.

    If requested, use German F1/F2 progression as a dimensionless guide for
    moving toward the existing Persian X targets. F3 and all voiced source,
    pitch and timing controls are left alone by this step.
    """
    if not 0.25 <= overlap_scale <= 1.0:
        raise ValueError('unsupported German onset overlap')
    output = controls.copy()
    starts = np.concatenate(([0.0], np.cumsum(controls[:, 0], dtype=np.float64)))
    edges = np.cumsum(counts)
    chosen = []
    for edge, _ in cuts:
        if parts[edge - 1].language != 'en-us' or not _ends_in_english_a(en, parts[edge - 1].text):
            continue
        boundary = int(edges[edge - 1])
        left = 0 if edge == 1 else int(edges[edge - 2])
        at = starts[boundary]
        width = min(template.duration * overlap_scale, at - starts[left])
        indexes = np.flatnonzero((starts[:-1] >= at - width) & (starts[:-1] < at))
        if not len(indexes) and overlap_scale != 1.0:
            # At extreme speed the shortened interval can be smaller than a
            # single frame. Retain the complete baseline treatment instead
            # of accidentally dropping its later German F3 trajectory.
            width = min(template.duration, at - starts[left])
            indexes = np.flatnonzero((starts[:-1] >= at - width) & (starts[:-1] < at))
        if not len(indexes) or indexes[0] < left:
            continue
        bad = np.flatnonzero((controls[indexes, 2] <= 0) | (controls[indexes, 8] != 0))
        if len(bad):
            indexes = indexes[int(bad[-1]) + 1:]
        if not len(indexes):
            continue
        width = at - starts[indexes[0]]
        if width <= 0:
            continue
        t = (starts[indexes] - starts[indexes[0]]) * (template.duration / width)
        reference = template.controls
        target = controls[boundary]
        # The source and band levels in native numeric frames are integers.
        for column in (8, *range(35, 43)):
            end = float(reference[-1, column])
            if end <= 0:
                output[indexes, column] = 0
                continue
            proportion = np.clip(np.interp(t, template.times, reference[:, column]) / end, 0, 1)
            output[indexes, column] = np.rint(proportion * target[column])
        if formants:
            for column in (9, 13):
                origin = float(reference[0, column])
                delta = float(reference[-1, column]) - origin
                if abs(delta) < 1:
                    continue
                progress = np.clip((np.interp(t, template.times, reference[:, column]) - origin) / delta, 0, 1)
                target_delta = float(target[column] - controls[boundary - 1, column])
                output[indexes, column] += progress * target_delta
        chosen.append((edge, float(width)))
    return output, tuple(chosen)


def extend_english_a(controls: np.ndarray, counts: tuple[int, ...],
                     parts: tuple[SpeechPart, ...], cuts: tuple[tuple[int, float], ...],
                     en: DirectEciEngine, factor: float
                     ) -> tuple[np.ndarray, tuple[tuple[int, float], ...]]:
    """Stretch the existing last English A glide without adding a phone.

    Identify the falling F1/rising F2 portion of the native final diphthong,
    and lengthen only its frame durations. Do not invent a static vowel tail
    or alter the following Ach, pitch, source levels or vowel targets.
    """
    if not 1.0 < factor <= 1.5:
        raise ValueError('unsupported English A glide extension')
    output = controls.copy()
    starts = np.concatenate(([0.0], np.cumsum(controls[:, 0], dtype=np.float64)))
    edges = np.cumsum(counts)
    extended = []
    for edge, _ in cuts:
        if parts[edge - 1].language != 'en-us' or not _ends_in_english_a(en, parts[edge - 1].text):
            continue
        boundary = int(edges[edge - 1])
        left = 0 if edge == 1 else int(edges[edge - 2])
        at = starts[boundary]
        indexes = np.flatnonzero((starts[:-1] >= at - 200) & (starts[:-1] < at))
        indexes = indexes[indexes >= left]
        if len(indexes) < 3:
            continue
        bad = np.flatnonzero((controls[indexes, 2] <= 0) | (controls[indexes, 8] != 0))
        if len(bad):
            indexes = indexes[int(bad[-1]) + 1:]
        if len(indexes) < 3:
            continue
        peak = int(np.argmax(controls[indexes, 9]))
        indexes = indexes[peak:]
        if len(indexes) < 3:
            continue
        start, last = controls[indexes[0]], controls[indexes[-1]]
        # Do not lengthen a different voiced phone that merely happens to
        # end a tagged English part. The actual A has an F1 fall and F2 rise.
        if start[9] - last[9] < 25 or last[13] - start[13] < 40:
            continue
        original_ms = float(np.sum(controls[indexes, 0], dtype=np.float64))
        output[indexes, 0] *= factor
        extended.append((edge, original_ms * (factor - 1)))
    return output, tuple(extended)


def fade_english_a_voice(controls: np.ndarray, counts: tuple[int, ...],
                         parts: tuple[SpeechPart, ...], edges: tuple[tuple[int, float], ...],
                         en: DirectEciEngine, window_ms: float, attenuation_db: float
                         ) -> tuple[np.ndarray, tuple[tuple[int, float, float], ...]]:
    """Reduce AV on the existing English vowel while Ach noise rises.

    No voiced X frame is added, and AH, AF, pitch and the tract are untouched.
    The final oscillator period still drains according to the native engine;
    AV updates take effect only at a period boundary, not every millisecond.
    """
    if not 0 < window_ms <= 50 or not 0 < attenuation_db <= 24:
        raise ValueError('unsupported English voice release')
    output = controls.copy()
    starts = np.concatenate(([0.0], np.cumsum(controls[:, 0], dtype=np.float64)))
    bounds = np.cumsum(counts)
    changes = []
    for edge, _ in edges:
        if parts[edge - 1].language != 'en-us' or not _ends_in_english_a(en, parts[edge - 1].text):
            continue
        boundary = int(bounds[edge - 1])
        left = 0 if edge == 1 else int(bounds[edge - 2])
        t = starts[boundary]
        indexes = np.flatnonzero((starts[:-1] >= t - window_ms) & (starts[:-1] < t))
        indexes = indexes[indexes >= left]
        if not len(indexes):
            continue
        invalid = np.flatnonzero(controls[indexes, 2] <= 0)
        if len(invalid):
            indexes = indexes[int(invalid[-1]) + 1:]
        if len(indexes) < 3:
            continue
        span = starts[indexes[-1]] - starts[indexes[0]]
        if span <= 0:
            continue
        phase = (starts[indexes] - starts[indexes[0]]) / span
        weight = phase * phase * (3 - 2 * phase)
        original = controls[indexes, 2]
        output[indexes, 2] = np.maximum(1, np.rint(original - attenuation_db * weight))
        changes.append((edge, float(t - starts[indexes[0]]),
                        float(original[-1] - output[indexes[-1], 2])))
    return output, tuple(changes)


def apply_german_f3(controls: np.ndarray, counts: tuple[int, ...],
                    edges: tuple[tuple[int, float], ...], template: GermanAchTemplate
                    ) -> tuple[np.ndarray, tuple[tuple[int, float], ...]]:
    """Follow German F3 through X, then smoothly return to the Persian target.

    Anchor the trajectory to the actual English vowel. The fade confines the
    experiment to native unvoiced X and preserves its next voiced vowel.
    """
    output = controls.copy()
    starts = np.concatenate(([0.0], np.cumsum(controls[:, 0], dtype=np.float64)))
    frame_edges = np.cumsum(counts)
    changed = []
    for edge, _ in edges:
        boundary = int(frame_edges[edge - 1])
        end = int(frame_edges[edge])
        section = controls[boundary:end]
        eligible = (section[:, 2] == 0) & (section[:, 8] > 0)
        invalid = np.flatnonzero(~eligible)
        usable = int(invalid[0]) if len(invalid) else len(section)
        times = starts[boundary:boundary + usable] - starts[boundary]
        times = times[times <= template.noise_times[-1]]
        if len(times) < 2 or times[-1] <= 0:
            continue
        duration = float(times[-1])
        proportion = times / duration
        weight = 1 - proportion * proportion * (3 - 2 * proportion)
        german = (controls[boundary - 1, 15] +
                  np.interp(times, template.noise_times, template.noise_f3) - template.noise_f3[0])
        indexes = np.arange(boundary, boundary + len(times))
        output[indexes, 15] = controls[indexes, 15] * (1 - weight) + german * weight
        changed.append((edge, duration))
    return output, tuple(changed)


def _mapped_progress(times: np.ndarray, reference_times: np.ndarray,
                     reference_values: np.ndarray) -> np.ndarray:
    """Map the German trajectory's monotone progress to another duration."""
    if len(times) < 2 or times[-1] <= 0 or reference_times[-1] <= 0:
        raise ValueError('trajectory needs a positive duration')
    first, last = float(reference_values[0]), float(reference_values[-1])
    if abs(last - first) < 1:
        progress = reference_times / reference_times[-1]
    else:
        progress = np.clip((reference_values - first) / (last - first), 0, 1)
        progress = np.maximum.accumulate(progress)
        progress[0], progress[-1] = 0, 1
    return np.interp(times / times[-1], reference_times / reference_times[-1], progress)


def apply_german_flow(controls: np.ndarray, counts: tuple[int, ...],
                      edges: tuple[tuple[int, float], ...], template: GermanAchTemplate,
                      *, include_f2: bool
                      ) -> tuple[np.ndarray, tuple[tuple[int, float, float], ...]]:
    """Project German X motion to the actual adjacent English/Persian targets.

    F3 runs from the final English vowel to the first Persian voiced frame.
    Optionally F2 runs from the X onset to the first settled following vowel.
    Source, pitch and timing are unchanged, and the following vowel's own
    target and all later frames are preserved. This is a listening trial.
    """
    output = controls.copy()
    starts = np.concatenate(([0.0], np.cumsum(controls[:, 0], dtype=np.float64)))
    frame_edges = np.cumsum(counts)
    adjusted = []
    for edge, _ in edges:
        boundary = int(frame_edges[edge - 1])
        end = int(frame_edges[edge])
        section = controls[boundary:end]
        voiced = np.flatnonzero(section[:, 2] > 0)
        if not len(voiced) or voiced[0] < 2:
            continue
        first_voice = int(voiced[0])
        if np.any(section[:first_voice, 8] <= 0):
            continue
        f3_times = starts[boundary:boundary + first_voice + 1] - starts[boundary]
        if f3_times[-1] <= 0:
            continue
        f3_progress = _mapped_progress(f3_times, template.noise_times, template.noise_f3)
        begin_f3 = float(controls[boundary - 1, 15])
        end_f3 = float(section[first_voice, 15])
        indexes = np.arange(boundary, boundary + first_voice + 1)
        output[indexes, 15] = begin_f3 + (end_f3 - begin_f3) * f3_progress
        f2_duration = 0.0
        if include_f2:
            quiet = np.flatnonzero((section[first_voice:, 2] > 0) &
                                   (section[first_voice:, 8] == 0))
            if len(quiet):
                stable = first_voice + int(quiet[0])
                # Do not pass a second, unvoiced phone while searching for
                # the next vowel's existing target.
                if stable > first_voice and np.all(section[first_voice:stable + 1, 2] > 0):
                    f2_times = starts[boundary:boundary + stable + 1] - starts[boundary]
                    f2_progress = _mapped_progress(f2_times, template.flow_times, template.flow_f2)
                    begin_f2 = float(section[0, 13])
                    end_f2 = float(section[stable, 13])
                    indexes = np.arange(boundary, boundary + stable + 1)
                    output[indexes, 13] = begin_f2 + (end_f2 - begin_f2) * f2_progress
                    f2_duration = float(f2_times[-1])
        adjusted.append((edge, float(f3_times[-1]), f2_duration))
    return output, tuple(adjusted)
