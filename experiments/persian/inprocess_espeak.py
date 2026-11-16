"""Private, independently loaded eSpeak reference for the Persian frontend.

Each frontend loads a temporary copy of NVDA's library, so its voice and
callback state cannot overwrite NVDA's own eSpeak instance. Audio generated
for word/phone event evidence is discarded; Eloquence provides all speech.
"""
from __future__ import annotations

import ctypes
import os
from pathlib import Path
import shutil
import tempfile
import threading

if __package__:
    from .espeak_ng_probe import Clause, ReferenceEvent
else:
    from espeak_ng_probe import Clause, ReferenceEvent


_MAX_REQUEST = 1024 * 1024
_MAX_EVENTS = 262144
_VOICES = ('fa', 'fa-latn', 'en-us')


class _EventId(ctypes.Union):
    _fields_ = [('number', ctypes.c_int), ('name', ctypes.c_void_p), ('string', ctypes.c_char * 8)]


class _Event(ctypes.Structure):
    _fields_ = [('type', ctypes.c_int), ('identifier', ctypes.c_uint),
                ('text_position', ctypes.c_int), ('length', ctypes.c_int),
                ('audio_position', ctypes.c_int), ('sample', ctypes.c_int),
                ('user_data', ctypes.c_void_p), ('id', _EventId)]


_Callback = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.POINTER(ctypes.c_short), ctypes.c_int,
                            ctypes.POINTER(_Event))


class InProcessEspeak:
    """Serialize one private reference; cancellation never touches NVDA's copy."""
    def __init__(self, library: Path, data_parent: Path):
        self._library, self._data_parent = library.resolve(), data_parent.resolve()
        self._calls, self._guard = threading.Lock(), threading.Lock()
        self._generation, self._closed, self._dirty = 0, False, False
        self._temp = None
        self._dll = None
        self._initialized = False
        self._callback = None
        self._open()

    def _open(self):
        self._temp = tempfile.TemporaryDirectory(prefix='reloq-fa-espeak-')
        try:
            private = Path(self._temp.name) / ('reloq-fa-espeak.dll' if os.name == 'nt'
                                               else 'libreloq-fa-espeak.so')
            shutil.copyfile(self._library, private)
            if os.name == 'nt':
                # NVDA's eSpeak dependencies are in the original directory.
                with os.add_dll_directory(str(self._library.parent)):
                    self._dll = ctypes.CDLL(str(private))
            else:
                # Deep binding prevents another already-loaded eSpeak copy
                # from interposing this image's exported globals on ELF.
                deepbind = getattr(os, 'RTLD_DEEPBIND', None)
                if deepbind is None:
                    raise RuntimeError('independent eSpeak loading is unavailable on this platform')
                self._dll = ctypes.CDLL(str(private), mode=ctypes.RTLD_LOCAL | deepbind)
            self._configure()
            if self._dll.espeak_Initialize(2, 0, os.fsencode(self._data_parent), 0x8001) <= 0:
                raise RuntimeError('eSpeak data could not be initialized')
            self._initialized = True
        except BaseException:
            self._release()
            raise

    def _configure(self):
        dll = self._dll
        dll.espeak_Initialize.argtypes = (ctypes.c_int, ctypes.c_int, ctypes.c_char_p, ctypes.c_int)
        dll.espeak_Initialize.restype = ctypes.c_int
        dll.espeak_SetVoiceByName.argtypes = (ctypes.c_char_p,)
        dll.espeak_SetVoiceByName.restype = ctypes.c_int
        dll.espeak_TextToPhonemesWithTerminator.argtypes = (ctypes.POINTER(ctypes.c_void_p),
                                                           ctypes.c_int, ctypes.c_int,
                                                           ctypes.POINTER(ctypes.c_int))
        dll.espeak_TextToPhonemesWithTerminator.restype = ctypes.c_char_p
        dll.espeak_SetSynthCallback.argtypes = (_Callback,)
        dll.espeak_SetSynthCallback.restype = None
        dll.espeak_Synth.argtypes = (ctypes.c_void_p, ctypes.c_size_t, ctypes.c_uint, ctypes.c_int,
                                     ctypes.c_uint, ctypes.c_uint, ctypes.c_void_p, ctypes.c_void_p)
        dll.espeak_Synth.restype = ctypes.c_int
        dll.espeak_SetParameter.argtypes = (ctypes.c_int, ctypes.c_int, ctypes.c_int)
        dll.espeak_SetParameter.restype = ctypes.c_int
        dll.espeak_GetParameter.argtypes = (ctypes.c_int, ctypes.c_int)
        dll.espeak_GetParameter.restype = ctypes.c_int
        dll.espeak_Terminate.argtypes = ()
        dll.espeak_Terminate.restype = ctypes.c_int

    def _request(self, text, voice, operation):
        if not isinstance(text, str) or '\0' in text:
            raise ValueError('text must be a string without NUL')
        if voice not in _VOICES:
            raise ValueError('unsupported reference voice')
        encoded = text.encode('utf8')
        if len(encoded) > _MAX_REQUEST:
            raise ValueError('text exceeds eSpeak reference request limit')
        with self._guard:
            if self._closed:
                raise RuntimeError('eSpeak reference is closed')
            generation = self._generation
        if not encoded:
            return ()
        with self._calls:
            self._check(generation)
            if self._dirty:
                # A stopped synchronous synthesis can leave eSpeak's parser
                # state incomplete. Recreate only this private image before
                # analysing the next request; NVDA's copy is unaffected.
                self._release()
                self._open()
                self._dirty = False
            self._check(generation)
            try:
                if self._dll.espeak_SetVoiceByName(voice.encode('ascii')) != 0:
                    raise RuntimeError(f'eSpeak voice unavailable: {voice}')
                self._check(generation)
                result = operation(encoded, generation)
                self._check(generation)
                return result
            except BaseException:
                self._dirty = True
                raise

    def _check(self, generation):
        with self._guard:
            if self._closed or self._generation != generation:
                raise RuntimeError('eSpeak reference analysis was cancelled')

    def translate(self, text: str, *, voice: str = 'fa', separator: str = '') -> tuple[Clause, ...]:
        if separator and (len(separator) != 1 or not 0x21 <= ord(separator) <= 0x7e):
            raise ValueError('separator must be one printable ASCII character')
        def run(encoded, generation):
            buffer = ctypes.create_string_buffer(encoded + b'\0')
            base, cursor = ctypes.addressof(buffer), ctypes.c_void_p(ctypes.addressof(buffer))
            end, clauses = base + len(encoded), []
            while cursor.value:
                self._check(generation)
                start, terminator = cursor.value, ctypes.c_int()
                row = self._dll.espeak_TextToPhonemesWithTerminator(
                    ctypes.byref(cursor), 1, (ord(separator) << 8) if separator else 0,
                    ctypes.byref(terminator))
                self._check(generation)
                stop = end if cursor.value is None else cursor.value
                if row is None or not base <= start < stop <= end:
                    raise RuntimeError('eSpeak returned invalid clause analysis')
                clauses.append(Clause(row.decode('utf8'), terminator.value))
            return tuple(clauses)
        return self._request(text, voice, run)

    def events(self, text: str, *, voice: str = 'fa', normal_speed: bool = False) -> tuple[ReferenceEvent, ...]:
        def run(encoded, generation):
            result, errors = [], []
            @_Callback
            def collect(_audio, _samples, events):
                try:
                    self._check(generation)
                    if not events:
                        return 0
                    index = 0
                    while events[index].type:
                        event = events[index]
                        index += 1
                        if event.type not in (1, 7):
                            continue
                        if event.text_position < 0 or event.length < 0 or len(result) >= _MAX_EVENTS:
                            raise RuntimeError('invalid eSpeak reference event')
                        phone = (ctypes.string_at(ctypes.addressof(event.id), 8).split(b'\0', 1)[0]
                                 .decode('utf8') if event.type == 7 else '')
                        if event.type == 7 and not phone:
                            raise RuntimeError('empty eSpeak phone event')
                        result.append(ReferenceEvent(event.type, event.text_position, event.length, phone))
                    return 0
                except BaseException as error:
                    errors.append(error)
                    return 1
            self._callback = collect
            self._dll.espeak_SetSynthCallback(collect)
            old_rate, changed = self._dll.espeak_GetParameter(1, 1), False
            if not normal_speed and old_rate > 0:
                changed = self._dll.espeak_SetParameter(1, 450, 0) == 0
            buffer = ctypes.create_string_buffer(encoded + b'\0')
            try:
                status = self._dll.espeak_Synth(buffer, len(encoded) + 1, 0, 1, 0, 1, None, None)
            finally:
                restored = not changed or self._dll.espeak_SetParameter(1, old_rate, 0) == 0
            self._check(generation)
            if errors or status or not restored:
                raise RuntimeError('eSpeak failed to generate reference events') from (errors[0] if errors else None)
            return tuple(result)
        return self._request(text, voice, run)

    def cancel(self):
        with self._guard:
            self._generation += 1

    def _release(self):
        if self._dll is not None:
            if self._initialized:
                self._dll.espeak_Terminate()
                self._initialized = False
            # Unload before removing the temporary file on Windows. No caller
            # may still be in a DLL function at this point.
            import _ctypes
            handle = self._dll._handle
            if os.name == 'nt':
                _ctypes.FreeLibrary(handle)
            else:
                _ctypes.dlclose(handle)
            self._dll._handle = 0
            self._dll = None
        self._callback = None
        if self._temp is not None:
            self._temp.cleanup()
            self._temp = None

    def close(self):
        with self._guard:
            self._closed = True
            self._generation += 1
        with self._calls:
            self._release()

    def __enter__(self):
        return self

    def __exit__(self, *_args):
        self.close()
