"""Isolated eSpeak NG text-to-phoneme probe for Persian frontend research.

The public data types are also used by the private in-process adapter.
EspeakPhonemeProbe itself loads the original library directly and is for
standalone comparison only: its voice and decoder state are process-global.
"""
from __future__ import annotations

import ctypes
from dataclasses import dataclass
import os
from pathlib import Path
import threading


@dataclass(frozen=True)
class Clause:
    phonemes: str
    terminator: int


@dataclass(frozen=True)
class ReferenceEvent:
    kind: int
    position: int              # eSpeak's 1-based source character position
    length: int
    phone: str = ''


class EspeakPhonemeProbe:
    """Call a matched eSpeak NG library and data directory in this process."""

    _state_lock = threading.Lock()
    _active = False

    def __init__(self, library: Path, data_parent: Path):
        self._closed = True
        self._call_lock = threading.Lock()
        with self._state_lock:
            if type(self)._active:
                raise RuntimeError('eSpeak probe already active in this process')
            type(self)._active = True
        try:
            self._dll = ctypes.CDLL(os.fspath(library.resolve()))
            self._dll.espeak_Initialize.argtypes = (ctypes.c_int, ctypes.c_int, ctypes.c_char_p, ctypes.c_int)
            self._dll.espeak_Initialize.restype = ctypes.c_int
            self._dll.espeak_SetVoiceByName.argtypes = (ctypes.c_char_p,)
            self._dll.espeak_SetVoiceByName.restype = ctypes.c_int
            self._dll.espeak_TextToPhonemesWithTerminator.argtypes = (
                ctypes.POINTER(ctypes.c_void_p), ctypes.c_int, ctypes.c_int, ctypes.POINTER(ctypes.c_int))
            self._dll.espeak_TextToPhonemesWithTerminator.restype = ctypes.c_char_p
            self._dll.espeak_Info.argtypes = (ctypes.POINTER(ctypes.c_char_p),)
            self._dll.espeak_Info.restype = ctypes.c_char_p
            self._dll.espeak_Terminate.argtypes = ()
            self._dll.espeak_Terminate.restype = ctypes.c_int
            # The argument is the directory *containing* espeak-ng-data.
            # Bit 15 reports a missing data set instead of terminating Python.
            if self._dll.espeak_Initialize(2, 0, os.fsencode(data_parent.resolve()), 0x8000) <= 0:
                raise RuntimeError('eSpeak data could not be initialized')
            self._closed = False
        except BaseException:
            with self._state_lock:
                type(self)._active = False
            raise

    @property
    def version(self) -> str:
        with self._call_lock:
            self._check_open()
            return self._dll.espeak_Info(None).decode('ascii')

    def _check_open(self) -> None:
        if self._closed:
            raise RuntimeError('eSpeak probe is closed')

    def translate(self, text: str, *, voice: str = 'fa', separator: str = '') -> tuple[Clause, ...]:
        """Preserve eSpeak's stress, word separation and clause termination."""
        if not isinstance(text, str) or '\0' in text:
            raise ValueError('text must be a string without NUL')
        if voice not in ('fa', 'fa-latn', 'en-us'):
            raise ValueError('unsupported reference voice')
        if separator and (len(separator) != 1 or not 0x21 <= ord(separator) <= 0x7e):
            raise ValueError('separator must be one printable ASCII character')
        with self._call_lock:
            self._check_open()
            if not text:
                return ()
            if self._dll.espeak_SetVoiceByName(voice.encode('ascii')) != 0:
                raise RuntimeError(f'eSpeak voice unavailable: {voice}')
            encoded = text.encode('utf8')
            buffer = ctypes.create_string_buffer(encoded + b'\0')
            base = ctypes.addressof(buffer)
            end = base + len(encoded)
            cursor = ctypes.c_void_p(base)
            clauses = []
            while cursor.value:
                start = cursor.value
                term = ctypes.c_int()
                result = self._dll.espeak_TextToPhonemesWithTerminator(
                    ctypes.byref(cursor), 1, (ord(separator) << 8) if separator else 0,
                    ctypes.byref(term))
                if result is None:
                    raise RuntimeError('eSpeak failed to translate the clause')
                stop = end if cursor.value is None else cursor.value
                if not base <= start < stop <= end:
                    raise RuntimeError('eSpeak returned an invalid or stagnant text pointer')
                # eSpeak may consume a character of the next clause as
                # lookahead. The cursor is useful for continuing the API but
                # is not an exact source span for this clause.
                clauses.append(Clause(result.decode('utf8'), term.value))
            return tuple(clauses)

    def close(self) -> None:
        with self._call_lock:
            if not self._closed:
                self._dll.espeak_Terminate()
                self._closed = True
                with self._state_lock:
                    type(self)._active = False

    def __enter__(self) -> 'EspeakPhonemeProbe':
        self._check_open()
        return self

    def __exit__(self, *_args) -> None:
        self.close()
