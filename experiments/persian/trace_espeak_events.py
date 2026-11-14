"""Observe eSpeak NG phoneme event positions; they are not vowel alignments."""
from __future__ import annotations
import ctypes
import ctypes.util


class EventId(ctypes.Union):
    _fields_ = [('number', ctypes.c_int), ('name', ctypes.c_char_p),
                ('string', ctypes.c_char * 8)]


class Event(ctypes.Structure):
    _fields_ = [('type', ctypes.c_int), ('unique_identifier', ctypes.c_uint),
                ('text_position', ctypes.c_int), ('length', ctypes.c_int),
                ('audio_position', ctypes.c_int), ('sample', ctypes.c_int),
                ('user_data', ctypes.c_void_p), ('id', EventId)]


def main() -> None:
    path = ctypes.util.find_library('espeak-ng')
    if not path:
        raise RuntimeError('Install libespeak-ng and its Persian voice data')
    lib = ctypes.CDLL(path)
    lib.espeak_Initialize.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.c_char_p, ctypes.c_int]
    lib.espeak_Initialize.restype = ctypes.c_int
    callback_type = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.POINTER(ctypes.c_short),
                                     ctypes.c_int, ctypes.POINTER(Event))
    lib.espeak_SetSynthCallback.argtypes = [callback_type]
    lib.espeak_SetVoiceByName.argtypes = [ctypes.c_char_p]
    lib.espeak_SetParameter.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.c_int]
    lib.espeak_Synth.argtypes = [ctypes.c_void_p, ctypes.c_size_t, ctypes.c_uint,
                                ctypes.c_int, ctypes.c_uint, ctypes.c_uint,
                                ctypes.c_void_p, ctypes.c_void_p]
    lib.espeak_Synchronize.argtypes = []
    rate = lib.espeak_Initialize(2, 0, None, 3)
    if rate <= 0:
        raise RuntimeError('eSpeak initialization failed')
    observed: list[tuple[int, int, str]] = []

    @callback_type
    def callback(_audio, _count, events):
        i = 0
        while events and events[i].type:
            e = events[i]
            if e.type in (5, 7):
                name = bytes(e.id.string).decode('utf8', errors='replace') if e.type == 7 else '<end>'
                observed.append((e.type, e.audio_position, name))
            i += 1
        return 0

    lib.espeak_SetSynthCallback(callback)
    if lib.espeak_SetVoiceByName(b'fa') != 0:
        raise RuntimeError('eSpeak Persian voice unavailable')
    for wpm in (150, 175, 220):
        if lib.espeak_SetParameter(1, wpm, 0) != 0:
            raise RuntimeError('Cannot set eSpeak speed')
        for text in ('سلام', 'دنیا', 'سلام دنیا.', 'ایران'):
            observed.clear()
            raw = text.encode('utf8') + b'\0'
            buf = ctypes.create_string_buffer(raw)
            result = lib.espeak_Synth(buf, len(raw), 0, 1, 0, 1, None, None)
            if result != 0 or lib.espeak_Synchronize() != 0:
                raise RuntimeError('eSpeak synthesis failed')
            print(f'\neSpeak {wpm} WPM {text}:', flush=True)
            for i, (kind, pos, name) in enumerate(observed):
                next_pos = observed[i + 1][1] if i + 1 < len(observed) else None
                delta = next_pos - pos if next_pos is not None else None
                print(f'  event {name!r} at {pos} ms; to next event {delta} ms')
    lib.espeak_Terminate()


if __name__ == '__main__':
    main()
