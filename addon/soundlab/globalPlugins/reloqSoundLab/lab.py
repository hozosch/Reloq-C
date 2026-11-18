"""Independent, temporary frame-level listening experiments for NVDA."""
from __future__ import annotations

import ctypes
from pathlib import Path
import threading

from ._reloq_eci_direct import (DirectEciEngine, GENDER, HEAD_SIZE, NATIVE_PITCH,
                                 NATIVE_SPEED, SAMPLE_RATE, SAMPLE_RATES)
from ._reloq_persian_frames import FrameCapture, FrameReplay
from ._persian.nvda_driver_probe import make_language_engine, PERSIAN_PROBE, PINGLISH_PROBE

LANGUAGES = {'Persisch': None, 'Pinglish': None, 'Deutsch': 0x40000,
             'Englisch (USA)': 0x10000, 'Japanisch': 0x80000}
CENTRES = (9, 13, 15, 17, 19)
WIDTHS = (10, 14, 16, 18, 20)
NOISE_WIDTHS = (44, 45, 46, 47, 48)
_RENDER_LOCK = threading.Lock()


def defaults():
    return {'text': 'ایران', 'language': 'Persisch', 'gender': 0, 'head': 50,
            'rate': 3, 'speed': 65, 'pitch': 65, 'start': 0, 'end': 0,
            'voiced': True, 'frequencies': [0] * 5, 'bandwidths': [0] * 5,
            'noise_widths': [0] * 5}


def validate(p):
    if not isinstance(p, dict) or not isinstance(p.get('text'), str) or not p['text'].strip() or len(p['text']) > 400:
        raise ValueError('Bitte einen Testtext mit höchstens 400 Zeichen eingeben.')
    if p.get('language') not in LANGUAGES:
        raise ValueError('Unbekannte Sprache.')
    for key, lo, hi in (('gender',0,1),('head',0,100),('speed',0,100),('pitch',0,100),
                        ('start',0,60000),('end',0,60000)):
        if type(p.get(key)) is not int or not lo <= p[key] <= hi:
            raise ValueError('Ungültiger Wert: ' + key)
    if p.get('rate') not in SAMPLE_RATES:
        raise ValueError('Ungültige Abtastrate.')
    if p['end'] and p['end'] <= p['start']:
        raise ValueError('Das Ende muss nach dem Beginn liegen, oder null für das Ende der Probe sein.')
    if type(p.get('voiced')) is not bool:
        raise ValueError('Ungültiger Abschnittsfilter.')
    for key in ('frequencies', 'bandwidths', 'noise_widths'):
        if not isinstance(p.get(key), list) or len(p[key]) != 5 or any(type(x) is not int or abs(x) > 2000 for x in p[key]):
            raise ValueError('Resonanzänderungen müssen zwischen -2000 und 2000 Hz liegen.')
    return p


def adjust(rows, p, modified, diagnostics):
    if not rows:
        return 0
    selected, changed = [], 0
    origin = rows[0].tick
    nyquist = SAMPLE_RATES[p['rate']] * .475
    for row in rows:
        c = row.controls
        t = row.tick - origin
        if t < p['start'] or (p['end'] and t >= p['end']):
            continue
        if p['voiced'] and (c[2] < 25 or c[8] > 25):
            continue
        if c[9] > 0 and c[13] > 0:
            selected.append((t, [round(c[i]) for i in CENTRES], [round(c[i]) for i in WIDTHS]))
        if not modified:
            continue
        # Smooth the explicitly selected cut edges, avoiding abrupt jumps.
        weight = 1.0
        if p['start']:
            weight = min(weight, max(0.0, (t - p['start']) / 20.0))
        if p['end']:
            weight = min(weight, max(0.0, (p['end'] - t) / 20.0))
        for i, delta in zip(CENTRES, p['frequencies']):
            if delta and c[i] > 0:
                c[i] = max(40, min(nyquist, c[i] + delta * weight))
                changed += 1
        for i, delta in zip(WIDTHS, p['bandwidths']):
            if delta and c[i] > 0:
                c[i] = max(20, min(6000, c[i] + delta * weight))
                changed += 1
        for i, delta in zip(NOISE_WIDTHS, p['noise_widths']):
            if delta and c[i] > 0:
                c[i] = max(20, min(6000, c[i] + delta * weight))
                changed += 1
    if selected:
        diagnostics.append(selected[len(selected) // 2])
    return changed


def render(dll, espeak, data_parent, settings, modified):
    # A dialog may be closed and reopened while its previous worker exits.
    # Serialize private replay interception between those workers.
    with _RENDER_LOCK:
        return _render(dll, espeak, data_parent, settings, modified)


def _render(dll, espeak, data_parent, settings, modified):
    p = validate(settings)
    dll, espeak, data_parent = map(Path, (dll, espeak, data_parent))
    raw, diagnostics, changes, replays = bytearray(), [], [], []
    if p['language'] in ('Persisch', 'Pinglish'):
        language = PERSIAN_PROBE.id if p['language'] == 'Persisch' else PINGLISH_PROBE.id
        owner = make_language_engine(dll, dll, None, espeak, data_parent, language=language)
        original = FrameReplay.replay
        def replay(self, voice, rows, marks, *args, **kwargs):
            changes.append(adjust(rows, p, modified, diagnostics))
            replays.append(True)
            return original(self, voice, rows, marks, *args, **kwargs)
        try:
            for param, value in ((GENDER,p['gender']), (HEAD_SIZE,p['head']),
                                 (SAMPLE_RATE,p['rate']), (NATIVE_SPEED,p['speed']), (NATIVE_PITCH,p['pitch'])):
                owner.set_parameter(param, value)
            # This module contains its own private copy of FrameReplay; NVDA's
            # active synth and other add-ons never see this temporary patch.
            FrameReplay.replay = replay
            try:
                ok = owner.speak((p['text'],), raw.extend, lambda _: None)
            finally:
                FrameReplay.replay = original
        finally:
            owner.close()
        if not replays:
            raise ValueError('Für diesen Text konnte keine bearbeitbare persische Lautfolge erzeugt werden. Bitte ein einzelnes Wort versuchen.')
    else:
        voice = DirectEciEngine(dll.resolve(), dialect=LANGUAGES[p['language']], encoding='utf8', native_voice=True)
        try:
            for param, value in ((GENDER,p['gender']), (HEAD_SIZE,p['head']),
                                 (SAMPLE_RATE,p['rate']), (NATIVE_SPEED,p['speed']), (NATIVE_PITCH,p['pitch'])):
                voice.set_parameter(param, value)
            captured = FrameCapture(voice._dll).capture(voice, (p['text'],), audio=False)
            if captured is None:
                raise ValueError('Die Aufnahme wurde unterbrochen.')
            rows = captured.frames()
            changes.append(adjust(rows, p, modified, diagnostics))
            ok = FrameReplay(voice._dll).replay(voice, rows, (), raw.extend, lambda _: None)
        finally:
            voice.close()
    if not ok or not raw:
        raise ValueError('Die Probe konnte nicht erzeugt werden.')
    if modified and any(p['frequencies'] + p['bandwidths'] + p['noise_widths']) and not sum(changes):
        raise ValueError('Im ausgewählten Abschnitt wurden keine passenden Resonanzen gefunden.')
    return bytes(raw), tuple(diagnostics), sum(changes)
