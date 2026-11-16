"""Compare context-dependent Persian vowel timing against eSpeak.

Requires a *diagnostic* Persian acoustic library whose scoped getenv forwards
RELOQ_FA_TRACE_NUC. This tool does not modify the shipped acoustic settings.
Native nucleus targets and eSpeak phoneme events have different boundaries;
only within-engine context ratios are compared, never their absolute times.
"""
from __future__ import annotations

import argparse
import ctypes
import hashlib
import json
import os
from pathlib import Path
import re
import statistics
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import NATIVE_SPEED
from nvda_driver_probe import PERSIAN_PROBE, make_language_engine
from test_vowel_timing import PATTERN
from trace_espeak_events import Event

PHONE_CODES = {'a': 31, 'A': 34, 'e': 29, 'i': 27, 'o': 33, 'u': 32}
_TAG = re.compile(r'`\[[^]]+\]')
_CALLBACK = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.POINTER(ctypes.c_short),
                             ctypes.c_int, ctypes.POINTER(Event))


class EspeakEvents:
    def __init__(self, library: Path, data_parent: Path):
        self.lib = lib = ctypes.CDLL(str(library.resolve()))
        lib.espeak_Initialize.argtypes = (ctypes.c_int, ctypes.c_int, ctypes.c_char_p, ctypes.c_int)
        lib.espeak_SetSynthCallback.argtypes = (_CALLBACK,)
        lib.espeak_SetVoiceByName.argtypes = (ctypes.c_char_p,)
        lib.espeak_SetParameter.argtypes = (ctypes.c_int, ctypes.c_int, ctypes.c_int)
        lib.espeak_Synth.argtypes = (ctypes.c_void_p, ctypes.c_size_t, ctypes.c_uint,
                                     ctypes.c_int, ctypes.c_uint, ctypes.c_uint,
                                     ctypes.c_void_p, ctypes.c_void_p)
        lib.espeak_Info.argtypes = (ctypes.POINTER(ctypes.c_char_p),)
        lib.espeak_Info.restype = ctypes.c_char_p
        if lib.espeak_Initialize(2, 0, os.fsencode(data_parent.resolve()), 0x8001) <= 0:
            raise RuntimeError('Cannot initialize eSpeak data')
        self.version = lib.espeak_Info(None).decode('ascii', errors='replace')
        self.events = []

        @_CALLBACK
        def callback(_pcm, _count, events):
            i = 0
            while events and events[i].type:
                event = events[i]
                if event.type in (1, 5, 7):
                    name = (bytes(event.id.string).decode('utf8', errors='replace')
                            if event.type == 7 else '')
                    self.events.append((event.type, event.text_position, event.audio_position, name))
                i += 1
            return 0

        self.callback = callback
        lib.espeak_SetSynthCallback(callback)
        if lib.espeak_SetVoiceByName(b'fa') != 0:
            raise RuntimeError('eSpeak Persian voice is unavailable')

    def vowels(self, text: str, wpm: int):
        self.events.clear()
        if self.lib.espeak_SetParameter(1, wpm, 0):
            raise RuntimeError('Cannot set eSpeak rate')
        raw = text.encode('utf8') + b'\0'
        buffer = ctypes.create_string_buffer(raw)
        if self.lib.espeak_Synth(buffer, len(raw), 0, 1, 0, 1, None, None) or self.lib.espeak_Synchronize():
            raise RuntimeError('eSpeak synthesis failed')
        result = []
        for i, (kind, position, at, name) in enumerate(self.events):
            phone = name.lstrip("',").rstrip(':')
            if kind != 7 or position != 1 or phone not in PHONE_CODES:
                continue
            if i + 1 >= len(self.events):
                raise RuntimeError('eSpeak vowel has no following event')
            duration = self.events[i + 1][2] - at
            if duration <= 0:
                raise RuntimeError('Invalid eSpeak event interval')
            result.append((phone, int(name.startswith("'")), duration))
        return result

    def close(self):
        self.lib.espeak_Terminate()


def native_trace(engine, text):
    old = os.environ.get('RELOQ_FA_TRACE_NUC')
    os.environ['RELOQ_FA_TRACE_NUC'] = '1'
    saved = os.dup(2)
    with tempfile.TemporaryFile() as trace:
        try:
            ctypes.CDLL(None).fflush(None)
            os.dup2(trace.fileno(), 2)
            pcm = bytearray()
            if not engine.speak((text,), pcm.extend, lambda _value: None):
                raise RuntimeError('Native timing probe was cancelled')
            ctypes.CDLL(None).fflush(None)
        finally:
            os.dup2(saved, 2)
            os.close(saved)
            if old is None:
                os.environ.pop('RELOQ_FA_TRACE_NUC', None)
            else:
                os.environ['RELOQ_FA_TRACE_NUC'] = old
        trace.seek(0)
        rows = [tuple(map(int, match.groups()))
                for line in trace.read().decode('utf8', errors='replace').splitlines()
                if (match := PATTERN.search(line))]
    if not rows:
        raise RuntimeError('No native timing trace: use the diagnostic scoped library')
    return rows


def corpus_words(path):
    seen = set()
    for line in path.read_text(encoding='utf8').splitlines():
        entry = json.loads(line)
        word = entry.get('text', '').strip()
        if entry.get('voice') != 'fa' or not word or any(c.isspace() for c in word) or word in seen:
            continue
        seen.add(word)
        yield word


def first_tag(analysis):
    if not analysis or len(analysis.parts) != 1 or analysis.parts[0].language != 'fa':
        return None
    tag = _TAG.match(analysis.parts[0].text)
    return tag.group() if tag else None


def paired_rows(word, native_a, native_b, ref_a, ref_b):
    """Reject rather than guess when pronunciation, stress, or count differs."""
    if len(native_a) != len(ref_a) or len(ref_a) > len(native_b) or len(ref_b) != len(ref_a):
        return []
    out = []
    for index, (a, b, x, y) in enumerate(zip(native_a, native_b, ref_a, ref_b)):
        if (a[:4] != b[:4] or x[:2] != y[:2] or a[0] != PHONE_CODES[x[0]] or
                a[2] != x[1] or a[5] <= 0 or b[5] <= 0):
            return []
        out.append({'word': word, 'vowel_index': index + 1, 'phone': x[0],
                    'stress': x[1], 'native_length_flag': a[3],
                    'native_alone_units': a[5], 'native_before_word_units': b[5],
                    'espeak_alone_ms': x[2], 'espeak_before_word_ms': y[2],
                    'native_ratio': round(b[5] / a[5], 4),
                    'espeak_ratio': round(y[2] / x[2], 4)})
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    for name in ('scope-probe', 'native-library', 'espeak-library', 'data-parent'):
        ap.add_argument('--' + name, type=Path, required=True)
    ap.add_argument('--corpus', type=Path, default=Path(__file__).with_name('reference_espeak_1_51.jsonl'))
    ap.add_argument('--output', type=Path, required=True)
    args = ap.parse_args()
    engine = make_language_engine(args.scope_probe, args.native_library, None,
                                  args.espeak_library, args.data_parent)
    reference = None
    try:
        frontend = engine._engines[PERSIAN_PROBE.id]._frontend
        words = []
        for word in corpus_words(args.corpus):
            a, b = first_tag(frontend.analyze_direct(word)), first_tag(frontend.analyze_direct(word + ' سلام'))
            if a and a == b:
                words.append(word)
        reference = EspeakEvents(args.espeak_library, args.data_parent)
        rows, rejected = [], []
        for native_speed, wpm in ((40, 150), (98, 175), (200, 220)):
            engine.set_parameter(NATIVE_SPEED, native_speed)
            for word in words:
                a, b = native_trace(engine, word), native_trace(engine, word + ' سلام')
                x, y = reference.vowels(word, wpm), reference.vowels(word + ' سلام', wpm)
                matched = paired_rows(word, a, b, x, y)
                if not matched:
                    rejected.append({'word': word, 'native_speed': native_speed,
                                     'reason': 'vowel count, identity, stress, or context differs'})
                    continue
                rows.extend({'native_speed': native_speed, 'espeak_wpm': wpm, **item} for item in matched)
        grouped = {}
        for row in rows:
            grouped.setdefault((row['word'], row['vowel_index'], row['phone']), []).append(row)
        candidates = []
        for (word, index, phone), group in grouped.items():
            if len(group) != 3:
                continue
            gaps = [row['native_ratio'] - row['espeak_ratio'] for row in group]
            # This is only a prioritization heuristic: distinct acoustic
            # measurement boundaries cannot establish a correction factor.
            if min(gaps) > 0.15 or max(gaps) < -0.15:
                candidates.append({'word': word, 'vowel_index': index, 'phone': phone,
                                   'median_ratio_gap': round(statistics.median(gaps), 4),
                                   'ratio_gaps': [round(value, 4) for value in gaps]})
        candidates.sort(key=lambda row: abs(row['median_ratio_gap']), reverse=True)
        output = {'method': 'Within-engine before-following-word / isolated-word ratio; '
                            'native nucleus targets and eSpeak event intervals have different boundaries. '
                            'Rate values are representative and are not claimed to be equivalent.',
                  'espeak_version': reference.version,
                  'corpus_sha256': hashlib.sha256(args.corpus.read_bytes()).hexdigest(),
                  'corpus_candidates': len(words), 'review_candidates': candidates,
                  'rows': rows, 'rejected': rejected}
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps(output, ensure_ascii=False, indent=2) + '\n', encoding='utf8')
        print(f"eSpeak {reference.version}: {len(words)} direct candidates, {len(rows)} matched vowel "
              f"observations, {len(rejected)} word/rate combinations excluded; {args.output}")
    finally:
        if reference:
            reference.close()
        engine.close()


if __name__ == '__main__':
    main()
