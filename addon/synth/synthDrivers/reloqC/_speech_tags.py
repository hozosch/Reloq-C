"""Carry ECI controls through NVDA's symbol processing as synth commands.

Only controls are protected. Normal text still passes through NVDA's speech
dictionaries, symbol pronunciation and Unicode normalization. In particular,
never hide a complete sentence from NVDA to preserve one backtick.
"""
import re

from speech.commands import CharacterModeCommand, SynthCommand


class EciTagCommand(SynthCommand):
    def __init__(self, text):
        self.text = text

    def __repr__(self):
        return f"EciTagCommand({self.text!r})"


class SegmentationGuard(SynthCommand):
    """Protect a text run matched by an NVDA speech dictionary."""
    def __init__(self, enabled):
        self.enabled = bool(enabled)

    def __repr__(self):
        return f"SegmentationGuard({self.enabled!r})"


class NativeDictionaryWord(SynthCommand):
    """Pass a native dictionary key through NVDA's builtin splitter intact."""
    def __init__(self, text):
        self.text = text

    def __repr__(self):
        return f"NativeDictionaryWord({self.text!r})"


class CapitalWordCommand(SynthCommand):
    """Carry both spellings to the worker that owns the native ECI voice."""
    def __init__(self, original, segmented):
        self.original, self.segmented = original, segmented

    def __repr__(self):
        return f"CapitalWordCommand({self.original!r}, {self.segmented!r})"


_NUMBER = re.compile(r"`(vbhz|vwpm|v[bsvfghry]|ts|da|pp|l|p|v(?=[1-9])|(?=[0-9]))([0-9]+)")
_LIMITS = {"vs": 250, "vg": 1, "ts": 2, "da": 1, "pp": 1,
           "l": 32767, "p": 60000, "v": 9, "": 9, "vbhz": 24000, "vwpm": 6000}
_WORD_CONTROLS = {"vb", "vbhz", "vs", "vwpm", "vv", "vf", "vg", "vh", "vr", "vy", "v"}


def _read(text, at):
    # Match the public bounded recognizer in reloq_inline_tags.h. A malformed
    # value must remain ordinary text, rather than accepting a valid prefix.
    if text.startswith("`[", at):
        end = text.find("]", at + 2)
        if end >= 0 and not any(c in text[at + 2:end] for c in "`[\0"):
            return end + 1, "spr"
        return None
    match = _NUMBER.match(text, at)
    if match is None:
        return None
    name, digits = match.groups()
    # Bound before int() as NVDA can be asked to speak arbitrarily long input.
    significant = digits.lstrip("0") or "0"
    limit = _LIMITS.get(name, 100)
    if len(significant) > 5 or int(significant) > limit:
        return None
    if name in ("vbhz", "vwpm") and int(significant) < 1:
        return None
    if name == "" and len(digits) > 1 and digits != "00":
        return None
    end = match.end()
    if name == "l":
        for _ in range(2):
            suffix = re.match(r"\.([0-9]+)", text[end:])
            if suffix is None:
                break
            value = suffix[1].lstrip("0") or "0"
            if len(value) > 5 or int(value) > 65535:
                return None
            end += suffix.end()
    return end, name


def _letter(char):
    # Match the native Western/Greek word scope. CJK keeps its own acoustic
    # segmentation; a run of Han characters must not become one Latin word.
    cp = ord(char)
    return ("a" <= char <= "z" or "A" <= char <= "Z" or
            0xc0 <= cp <= 0x2af and cp not in (0xd7, 0xf7) or
            0x300 <= cp <= 0x3ff or 0x1f00 <= cp <= 0x1fff)


def protect_text(text):
    if "`" not in text:
        return [text]
    output, pending, word, controls = [], [], [], []

    def flush_text():
        if pending:
            output.append("".join(pending))
            pending.clear()

    def flush_word():
        if controls:
            flush_text()
            output.extend(EciTagCommand(tag) for tag in controls)
            controls.clear()
        pending.extend(word)
        word.clear()

    at = 0
    while at < len(text):
        tag = _read(text, at) if text[at] == "`" else None
        if tag is not None:
            end, name = tag
            after = end
            if word and name in _WORD_CONTROLS:
                while after < len(text):
                    following = _read(text, after)
                    if following is None or following[1] not in _WORD_CONTROLS:
                        break
                    after = following[0]
                if after < len(text) and _letter(text[after]):
                    # NVDA appends separators to each string. Hoist interior
                    # voice controls so it sees the intact lexical word.
                    controls.append(text[at:end])
                    at = end
                    continue
            flush_word()
            flush_text()
            output.append(EciTagCommand(text[at:end]))
            at = end
            continue
        char = text[at]
        if _letter(char):
            word.append(char)
        else:
            flush_word()
            pending.append(char)
        at += 1
    flush_word()
    flush_text()
    return output


def protect_sequence(sequence):
    output = []
    spelling = False
    for item in sequence:
        if isinstance(item, CharacterModeCommand):
            spelling = item.state
        if isinstance(item, str) and not spelling:
            output.extend(protect_text(item))
        else:
            output.append(item)
    return output
