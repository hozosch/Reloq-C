"""Capture and audition a selected native phoneme without changing speech."""
from __future__ import annotations

import array
import ctypes
from dataclasses import dataclass
from pathlib import Path
import sys

from ._reloq_eci_direct import (DirectEciEngine, GENDER, HEAD_SIZE, NATIVE_PITCH,
                                NATIVE_SPEED, ORIGINAL_ACOUSTIC_CLOCK,
                                PROCESS_TAGS, SAMPLE_RATE, SAMPLE_RATES)
from ._reloq_persian_frames import FrameCapture, FrameReplay, NativeFrame
from . import lab, phonemes, resonance


@dataclass(frozen=True)
class LoadedPhone:
    language: str
    key: str
    params: tuple
    frames: bytes
    start_ms: float
    end_ms: float
    frequencies: tuple
    widths: tuple


def defaults():
    return {'language':'Persisch', 'phone':'i', 'gender':0, 'head':50, 'rate':3,
            'speed':65, 'pitch':65, 'clock':0, 'context':False,
            'frequencies':[0]*5, 'gains_db':[0]*5}


def validate(settings):
    if not isinstance(settings, dict) or phonemes.find(settings.get('language'), settings.get('phone')) is None:
        raise ValueError('Unbekannter Laut oder unbekannte Sprache.')
    for key, low, high in (('gender',0,1),('head',0,100),('speed',0,100),('pitch',0,100),('clock',0,1)):
        if type(settings.get(key)) is not int or not low <= settings[key] <= high:
            raise ValueError('Ungültiger Wert: ' + key)
    if settings.get('rate') not in SAMPLE_RATES or type(settings.get('context')) is not bool:
        raise ValueError('Ungültige Abtastrate oder Höransicht.')
    for key, limit in (('frequencies',2000),('gains_db',18)):
        values=settings.get(key)
        if not isinstance(values,list) or len(values)!=5 or any(type(x) is not int or abs(x)>limit for x in values):
            raise ValueError('Ungültige Formantänderung: ' + key)
    return settings


def _parameters(p):
    return (p['gender'],p['head'],p['rate'],p['speed'],p['pitch'],p['clock'])


def import_frequencies(settings, loaded, spectrum_frame):
    """Map the selected eSpeak keyframe's F1–F5 to this loaded phone.

    The editor stores offsets from the native midpoint, retaining the native
    trajectory and transitions. Missing, inactive or out-of-range peaks leave
    the existing edit untouched. eSpeak peak zero is not Eloquence's F1.
    """
    p = validate(settings)
    if (p['language'],p['phone'],_parameters(p)) != (loaded.language,loaded.key,loaded.params):
        raise ValueError('Der Laut muss vor dem Import neu geladen werden.')
    if len(spectrum_frame.peaks) < 6:
        raise ValueError('Das Spektralbild enthält zu wenige Formanten.')
    frequencies = list(p['frequencies'])
    nyquist = SAMPLE_RATES[p['rate']] * .475
    imported, skipped = 0, 0
    for i, peak in enumerate(spectrum_frame.peaks[1:6]):
        base = loaded.frequencies[i]
        delta = peak.frequency - base
        if peak.height <= 0 or base <= 0 or not 40 <= peak.frequency <= nyquist or abs(delta) > 2000:
            skipped += 1
            continue
        frequencies[i] = delta
        imported += 1
    if not imported:
        raise ValueError('Dieses Spektralbild enthält keine übernehmbaren Formantfrequenzen.')
    return frequencies, imported, skipped


def _voice(dll, p):
    dialect=0x90000 if p['language'] in ('Persisch','Pinglish') else lab.LANGUAGES[p['language']]
    engine=DirectEciEngine(Path(dll).resolve(),dialect=dialect,encoding='utf8',native_voice=True)
    try:
        for key,value in ((GENDER,p['gender']),(HEAD_SIZE,p['head']),(SAMPLE_RATE,p['rate']),
                          (NATIVE_SPEED,p['speed']),(NATIVE_PITCH,p['pitch']),
                          (ORIGINAL_ACOUSTIC_CLOCK,p['clock']),(PROCESS_TAGS,1)):
            engine.set_parameter(key,value)
        scope=engine._dll.eciPersianAcousticScope
        scope.argtypes=(ctypes.c_int,);scope.restype=ctypes.c_int
        return engine,scope
    except BaseException:
        engine.close()
        raise


def load(dll, settings):
    p=validate(settings); phone=phonemes.find(p['language'],p['phone'])
    with lab._RENDER_LOCK:
        voice,scope=_voice(dll,p)
        old=scope(int(p['language'] in ('Persisch','Pinglish')))
        try:
            capture=FrameCapture(voice._dll).capture(voice,(phone.tag,),audio=False,phone_segments=True)
            if capture is None or not capture.frame_bytes:
                raise ValueError('Der Laut konnte nicht geladen werden.')
            # A one-vowel probe forms exactly one native syllable. If a
            # provider omits phone trace events, its syllable clock is an
            # independent exact boundary for this case. Never guess a
            # consonant boundary or use a mismatching second rendering.
            if (not capture.syllables and not phone.onset and phone.first==0 and
                    phone.count==1 and len(phone.tag)==6 and phone.tag.startswith('`[.1')):
                fallback=FrameCapture(voice._dll).capture(voice,(phone.tag,),audio=False)
                if (fallback is not None and fallback.frame_bytes==capture.frame_bytes and
                        len(fallback.syllables)==1):
                    capture=fallback
            start,end=phonemes.interval(phone,capture.syllables)
            rows=capture.frames()
            candidates=[r for r in rows if start<=r.tick<end and
                        (r.controls[2]>5 or r.controls[7]>5 or r.controls[8]>5) and
                        r.controls[9]>0 and r.controls[13]>0]
            if not candidates:
                candidates=[r for r in rows if start<=r.tick<end and r.controls[9]>0 and r.controls[13]>0]
            if not candidates:
                raise ValueError('Der Laut enthält keine messbaren Resonanzwerte.')
            middle=candidates[len(candidates)//2].controls
            centres=tuple(round(middle[i]) for i in resonance.CENTRES)
            widths=tuple(round(middle[i]) for i in resonance.WIDTHS)
            return LoadedPhone(p['language'],p['phone'],_parameters(p),capture.frame_bytes,start,end,centres,widths)
        finally:
            scope(old)
            voice.close()


def _fade_crop(pcm, rate, start, end):
    samples=array.array('h');samples.frombytes(pcm)
    if sys.byteorder!='little':samples.byteswap()
    first=max(0,min(len(samples),round(start*rate/1000)))
    last=max(first,min(len(samples),round(end*rate/1000)))
    if first==last:raise ValueError('Der ausgewählte Laut ist leer.')
    selected=samples[first:last]
    fade=min(len(selected)//2,max(1,round(rate*.003)))
    for i in range(fade):
        scale=(i+1)/(fade+1)
        selected[i]=round(selected[i]*scale)
        selected[-1-i]=round(selected[-1-i]*scale)
    # Small leading and trailing silences keep short stop releases audible.
    result=array.array('h',[0]*round(rate*.015));result.extend(selected)
    result.extend([0]*round(rate*.025))
    if sys.byteorder!='little':result.byteswap()
    return result.tobytes()


def render(dll, settings, loaded, modified=True):
    p=validate(settings)
    if (p['language'],p['phone'],_parameters(p)) != (loaded.language,loaded.key,loaded.params):
        raise ValueError('Der Laut muss nach einer Änderung der Stimme neu geladen werden.')
    size=ctypes.sizeof(NativeFrame)
    if not loaded.frames or len(loaded.frames)%size:
        raise ValueError('Die gespeicherte Lautaufnahme ist ungültig.')
    rows=(NativeFrame*(len(loaded.frames)//size)).from_buffer_copy(loaded.frames)
    shifts=p['frequencies'] if modified else [0]*5
    gains=p['gains_db'] if modified else [0]*5
    changed=0
    nyquist=SAMPLE_RATES[p['rate']]*.475
    for row in rows:
        t=float(row.tick)
        if not loaded.start_ms<=t<loaded.end_ms:continue
        weight=min(1.0,max(0.0,(t-loaded.start_ms)/3.0),max(0.0,(loaded.end_ms-t)/3.0))
        for col,delta in zip(resonance.CENTRES,shifts):
            if delta and row.controls[col]>0 and weight:
                row.controls[col]=max(40,min(nyquist,row.controls[col]+delta*weight));changed+=1
    if any(shifts) and not changed:
        raise ValueError('Diese Resonanz ist im ausgewählten Laut nicht vorhanden.')
    raw=bytearray()
    with lab._RENDER_LOCK:
        voice,scope=_voice(dll,p);old=scope(int(p['language'] in ('Persisch','Pinglish')))
        try:
            if not FrameReplay(voice._dll).replay(voice,rows,(),raw.extend,lambda _:None) or not raw:
                raise ValueError('Die Lautprobe konnte nicht abgespielt werden.')
        finally:
            scope(old);voice.close()
    pcm,applied,normalized=resonance.apply(raw,rows,gains,SAMPLE_RATES[p['rate']],loaded.start_ms,loaded.end_ms)
    if any(gains) and not applied:
        raise ValueError('Die gewählte Resonanz liegt außerhalb des hörbaren Bereichs.')
    if not p['context']:
        pcm=_fade_crop(pcm,SAMPLE_RATES[p['rate']],loaded.start_ms,loaded.end_ms)
    return pcm,normalized
