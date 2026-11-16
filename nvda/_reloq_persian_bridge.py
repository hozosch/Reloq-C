"""Private adapter for an explicitly built Persian NVDA test package."""
from __future__ import annotations

import ctypes
from pathlib import Path

from ._persian.nvda_driver_probe import PERSIAN_PROBE, PINGLISH_PROBE, make_language_engine


def _paths(library: Path):
    import globalVars
    parent = Path(globalVars.appDir) / 'synthDrivers'
    return (library, None,
            parent / 'espeak.dll', parent)


def add_profiles(languages, library: Path):
    """Do not expose test profiles if their private components are absent."""
    text_library, worker, reference, _ = _paths(library)
    if not all(path.is_file() for path in (text_library, reference, library)):
        return languages
    try:
        dll = ctypes.CDLL(str(library.resolve()))
        getattr(dll, 'reloq_fa_acoustic_scope')
        getattr(dll, 'reloq_fa_native_onsets')
        getattr(dll, 'reloq_fa_native_geminates')
        for name in ('reloq_fa_capture_begin', 'reloq_fa_capture_end', 'reloq_fa_capture_release',
                     'reloq_fa_replay_epoch', 'reloq_fa_replay_frames'):
            getattr(dll, name)
        text_dll = dll if text_library.resolve() == library.resolve() else ctypes.CDLL(str(text_library.resolve()))
        for name in ('reloq_fa_direct_text', 'reloq_fa_direct_number',
                     'reloq_fa_direct_letter', 'reloq_fa_direct_free', 'reloq_fa_pinglish_word'):
            getattr(text_dll, name)
    except (OSError, AttributeError):
        return languages
    return {**languages, PERSIAN_PROBE.id: PERSIAN_PROBE, PINGLISH_PROBE.id: PINGLISH_PROBE}


def make_engine(library: Path, *, languages, language, voice):
    text_library, worker, reference, parent = _paths(library)
    return make_language_engine(library, text_library, worker, reference, parent,
                                language=language, voice=voice, languages=languages)
