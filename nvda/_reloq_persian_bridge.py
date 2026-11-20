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
    if not all(path.is_file() for path in (text_library, library)):
        return languages
    try:
        dll = ctypes.CDLL(str(library.resolve()))
        def require(handle, public, legacy):
            if getattr(handle, public, None) is None:
                getattr(handle, legacy)
        for public, legacy in (('eciPersianAcousticScope', 'reloq_fa_acoustic_scope'),
                               ('eciPersianNativeOnsets', 'reloq_fa_native_onsets'),
                               ('eciPersianNativeGeminates', 'reloq_fa_native_geminates'),
                               ('eciPersianCaptureBegin', 'reloq_fa_capture_begin'),
                               ('eciPersianCaptureEnd', 'reloq_fa_capture_end'),
                               ('eciPersianCaptureRelease', 'reloq_fa_capture_release'),
                               ('eciPersianReplayEpoch', 'reloq_fa_replay_epoch'),
                               ('eciPersianReplayFrames', 'reloq_fa_replay_frames')):
            require(dll, public, legacy)
        text_dll = dll if text_library.resolve() == library.resolve() else ctypes.CDLL(str(text_library.resolve()))
        if not reference.is_file() and getattr(text_dll, 'eciPersianProjectCompleteUtf8', None) is None:
            return languages
        for public, legacy in (('eciPersianTranslateUtf8', 'reloq_fa_direct_text'),
                               ('eciPersianNumber', 'reloq_fa_direct_number'),
                               ('eciPersianLetter', 'reloq_fa_direct_letter'),
                               ('eciPersianRelease', 'reloq_fa_direct_free'),
                               ('eciPersianPinglishWord', 'reloq_fa_pinglish_word')):
            require(text_dll, public, legacy)
    except (OSError, AttributeError):
        return languages
    return {**languages, PERSIAN_PROBE.id: PERSIAN_PROBE, PINGLISH_PROBE.id: PINGLISH_PROBE}


def make_engine(library: Path, *, languages, language, voice):
    text_library, worker, reference, parent = _paths(library)
    return make_language_engine(library, text_library, worker, reference, parent,
                                language=language, voice=voice, languages=languages)
