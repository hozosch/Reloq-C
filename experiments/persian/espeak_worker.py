"""Separate-process eSpeak text analysis for the private Persian prototype."""
from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import os
import struct
import subprocess
import threading

if __package__:
    from .espeak_ng_probe import Clause
else:
    from espeak_ng_probe import Clause

_U32 = struct.Struct('<I')
_HEADER = struct.Struct('<IBB')
_REPLY = struct.Struct('<II')
_EVENT = struct.Struct('<IIII')
_MAX_REQUEST = 1024 * 1024
_MAX_RESPONSE = 8 * 1024 * 1024
_VOICES = {'fa': 1, 'fa-latn': 2, 'en-us': 3}


@dataclass(frozen=True)
class ReferenceEvent:
    kind: int
    position: int              # eSpeak's 1-based source character position
    length: int
    phone: str = ''


def _read_exact(stream, count):
    result = bytearray()
    while len(result) < count:
        chunk = stream.read(count - len(result))
        if not chunk:
            raise RuntimeError('eSpeak text worker stopped unexpectedly')
        result.extend(chunk)
    return bytes(result)


def _write_all(stream, data):
    remaining = memoryview(data)
    while remaining:
        count = stream.write(remaining)
        if not count:
            raise RuntimeError('eSpeak text worker stopped while receiving text')
        remaining = remaining[count:]


def _decode_clauses(body: bytes) -> tuple[Clause, ...]:
    count = _U32.unpack_from(body)[0]
    cursor = 4
    clauses = []
    for _ in range(count):
        if cursor + 8 > len(body):
            raise RuntimeError('truncated eSpeak worker response')
        terminator, length = _REPLY.unpack_from(body, cursor)
        cursor += 8
        if cursor + length > len(body):
            raise RuntimeError('truncated eSpeak worker phonemes')
        clauses.append(Clause(body[cursor:cursor + length].decode('utf8'), terminator))
        cursor += length
    if cursor != len(body):
        raise RuntimeError('unexpected eSpeak worker response data')
    return tuple(clauses)


def _decode_events(body: bytes) -> tuple[ReferenceEvent, ...]:
    count = _U32.unpack_from(body)[0]
    if count > 262144:
        raise RuntimeError('invalid eSpeak event count')
    cursor = 4
    result = []
    for _ in range(count):
        if cursor + _EVENT.size > len(body):
            raise RuntimeError('truncated eSpeak event response')
        kind, position, length, name_length = _EVENT.unpack_from(body, cursor)
        cursor += _EVENT.size
        if kind not in (1, 7) or name_length > 8 or cursor + name_length > len(body):
            raise RuntimeError('invalid eSpeak event')
        name = body[cursor:cursor + name_length].decode('utf8')
        cursor += name_length
        if (kind == 1 and name) or (kind == 7 and not name):
            raise RuntimeError('invalid eSpeak word or phoneme event')
        result.append(ReferenceEvent(kind, position, length, name))
    if cursor != len(body):
        raise RuntimeError('unexpected eSpeak event data')
    return tuple(result)


class EspeakWorker:
    """Provide the probe's translate API without sharing eSpeak global state.

    One persistent helper serves serialized requests. ``cancel`` can stop an
    active translation from another thread without waiting for it to finish;
    an idle helper stays ready for the next request.
    """
    def __init__(self, executable: Path, library: Path, data_parent: Path):
        self._paths = tuple(str(p.resolve()) for p in (executable, library, data_parent))
        self._calls = threading.Lock()
        self._guard = threading.Lock()
        self._process = None
        self._process_generation = -1
        self._closed = False
        self._generation = 0
        self._inflight = False

    def _start(self, generation):
        with self._guard:
            if self._closed:
                raise RuntimeError('eSpeak worker is closed')
            if self._generation != generation:
                raise RuntimeError('eSpeak translation was cancelled')
            if (self._process is None or self._process.poll() is not None or
                    self._process_generation != generation):
                previous = self._process
                if previous is not None:
                    if previous.poll() is None:
                        previous.terminate()
                    try:
                        previous.wait(timeout=2)
                    except subprocess.TimeoutExpired:
                        previous.kill()
                        previous.wait()
                    if previous.stdin:
                        previous.stdin.close()
                    if previous.stdout:
                        previous.stdout.close()
                self._process = subprocess.Popen(
                    self._paths, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                    stderr=subprocess.DEVNULL, bufsize=0,
                    creationflags=subprocess.CREATE_NO_WINDOW if os.name == 'nt' else 0,
                )
                self._process_generation = generation
            return self._process

    def _request(self, text: str, *, voice: str, decode, separator: str = '', events: bool = False,
                 normal_speed: bool = False):
        if not isinstance(text, str) or '\0' in text:
            raise ValueError('text must be a string without NUL')
        if voice not in _VOICES:
            raise ValueError('unsupported reference voice')
        if separator and (len(separator) != 1 or not 0x21 <= ord(separator) <= 0x7e):
            raise ValueError('separator must be one printable ASCII character')
        with self._guard:
            if self._closed:
                raise RuntimeError('eSpeak worker is closed')
            generation = self._generation
        if not text:
            return decode(_U32.pack(0))
        encoded = text.encode('utf8')
        if len(encoded) > _MAX_REQUEST:
            raise ValueError('text exceeds eSpeak worker request limit')
        with self._calls:
            with self._guard:
                if self._closed or self._generation != generation:
                    raise RuntimeError('eSpeak translation was cancelled')
                self._inflight = True
            try:
                proc = self._start(generation)
                assert proc.stdin is not None and proc.stdout is not None
                code = _VOICES[voice] | (0x80 if events else 0) | (0x40 if events and normal_speed else 0)
                _write_all(proc.stdin, _HEADER.pack(len(encoded), code, ord(separator) if separator else 0))
                _write_all(proc.stdin, encoded)
                status, length = _REPLY.unpack(_read_exact(proc.stdout, _REPLY.size))
                if length > _MAX_RESPONSE:
                    raise RuntimeError('invalid eSpeak worker response size')
                body = _read_exact(proc.stdout, length)
                if status:
                    raise RuntimeError('eSpeak worker failed to translate the text')
                if len(body) < 4:
                    raise RuntimeError('invalid eSpeak worker response')
                return decode(body)
            except (BrokenPipeError, OSError, UnicodeError, RuntimeError, struct.error):
                # A concurrent stop may already have started the next
                # generation. A late failure from this request must not
                # cancel a newer request waiting for the call lock.
                self._cancel(generation)
                raise
            finally:
                with self._guard:
                    self._inflight = False

    def translate(self, text: str, *, voice: str = 'fa', separator: str = '') -> tuple[Clause, ...]:
        return self._request(text, voice=voice, separator=separator, decode=_decode_clauses)

    def events(self, text: str, *, voice: str = 'fa', normal_speed: bool = False) -> tuple[ReferenceEvent, ...]:
        """Return word/phoneme evidence; eSpeak's audio is never played."""
        return self._request(text, voice=voice, events=True, normal_speed=normal_speed, decode=_decode_events)

    def cancel(self):
        self._cancel()

    def _cancel(self, expected_generation=None):
        with self._guard:
            if expected_generation is not None and self._generation != expected_generation:
                return
            self._generation += 1
            proc = self._process
            if proc is not None and proc.poll() is None:
                if self._inflight:
                    try:
                        proc.terminate()
                    except ProcessLookupError:
                        pass
                else:
                    # NVDA commonly stops speech before a new navigation
                    # request. An idle worker has no stale request to abort;
                    # keep its loaded language tables for the next new text.
                    self._process_generation = self._generation

    def close(self):
        with self._guard:
            self._closed = True
            self._generation += 1
            proc = self._process
            if proc is not None and proc.poll() is None:
                try:
                    proc.terminate()
                except ProcessLookupError:
                    pass
        with self._calls:
            with self._guard:
                proc, self._process = self._process, None
            if proc is not None:
                try:
                    proc.wait(timeout=2)
                except subprocess.TimeoutExpired:
                    proc.kill()
                    proc.wait()
                if proc.stdin:
                    proc.stdin.close()
                if proc.stdout:
                    proc.stdout.close()

    def __enter__(self):
        return self

    def __exit__(self, *_args):
        self.close()
