"""Project Persian text and ECI events through a native source map.

This reusable component is not registered as an NVDA language. The analyzer
must provide an ASCII phoneme-tag string and a source-UTF-8-byte mapping.
"""
from __future__ import annotations

from dataclasses import dataclass
import re
from typing import Callable, Sequence

if __package__:
    from ._reloq_eci_direct import DirectEciEngine, EciIndex, EciPitch, EciPitchHz, EciProsody, EciText
else:
    from _reloq_eci_direct import DirectEciEngine, EciIndex, EciPitch, EciPitchHz, EciProsody, EciText

Command = EciIndex | EciPitch | EciPitchHz | EciProsody


class PersianProjectionError(ValueError):
    """A job cannot be projected accurately with the available source map."""


@dataclass(frozen=True)
class ProjectedJob:
    source: str
    tagged: str
    parts: tuple[EciText | Command, ...]
    needs_native_validation: bool = False


def project_persian(parts: Sequence[str | EciText | Command],
                    analyze: Callable[[str], tuple[str, tuple[int, ...]] | None]) -> ProjectedJob:
    """Project a single ECI job without starting a reference worker.

    An index inside a word maps to that word's start, in original event order.
    In-word changes of pitch/speed/volume are rejected because a word-level
    map cannot position them accurately. Character and literal-tag handling
    require separate integration and are not assumed here.
    """
    parts = tuple(parts)
    DirectEciEngine._prepare(parts, 'utf-8')
    source_parts = []
    events: list[tuple[int, Command]] = []
    offset = 0
    for part in parts:
        if isinstance(part, (str, EciText)):
            if isinstance(part, EciText) and (part.character_mode or part.speed_control):
                raise PersianProjectionError('character and inline-control modes need separate handling')
            text = part if isinstance(part, str) else part.text
            if '\0' in text:
                raise ValueError('ECI text cannot contain NUL bytes')
            source_parts.append(text)
            offset += len(text.encode('utf8'))
        elif isinstance(part, (EciIndex, EciPitch, EciPitchHz, EciProsody)):
            events.append((offset, part))
        else:
            raise TypeError('unsupported ECI job item')
    source = ''.join(source_parts)
    if source.strip() and not any(c.isalpha() or c.isdecimal() for c in source):
        raise PersianProjectionError('standalone symbol names require NVDA integration')
    result = analyze(source)
    if result is None:
        raise PersianProjectionError('text is not fully covered by direct Persian rules')
    tagged, mapping = result
    encoded = source.encode('utf8')
    if (not isinstance(tagged, str) or not tagged.isascii() or len(mapping) != len(encoded) + 1 or
        mapping[-1] != len(tagged) or any(type(value) is not int or not 0 <= value <= len(tagged) for value in mapping) or
        any(a > b for a, b in zip(mapping, mapping[1:]))):
        raise PersianProjectionError('invalid Persian source mapping')
    tags = tuple((m.start(), m.end()) for m in re.finditer(r'`\[[^]]+\]', tagged))
    output = []
    position = 0
    tag_cursor = 0
    for source_offset, command in events:
        target = mapping[source_offset]
        while tag_cursor < len(tags) and tags[tag_cursor][1] <= target:
            tag_cursor += 1
        if tag_cursor < len(tags) and tags[tag_cursor][0] < target < tags[tag_cursor][1]:
            raise PersianProjectionError('source mapping would split a phoneme tag')
        if not isinstance(command, EciIndex) and 0 < source_offset < len(encoded) and (
                mapping[source_offset] == mapping[source_offset - 1]):
            raise PersianProjectionError('in-word prosody changes need more precise mapping')
        if target > position:
            output.append(EciText(tagged[position:target], segmentation_guarded=True))
        output.append(command)
        position = target
    if position < len(tagged):
        output.append(EciText(tagged[position:], segmentation_guarded=True))
    return ProjectedJob(source, tagged, tuple(output))
