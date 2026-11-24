"""Private adapter for an explicitly built Persian NVDA test package."""
from __future__ import annotations

import ctypes
from pathlib import Path
import sys

from ._persian.nvda_driver_probe import PERSIAN_PROBE, PINGLISH_PROBE, make_language_engine
from ._persian import continuous_plan, nvda_driver_probe


NATIVE_MELODY_FILE = 'persian_native_melody.txt'


def select_native_melody(library: Path, folder: Path | None = None):
    """Use the US sentence melody that is fixed in the core, if this package asks.

    A test package can carry a one-line file containing 1 next to this
    module. The core then makes the melody with its own rules in the same
    pass as the speech, ordinary Persian text is spoken by one call into the
    core, and no melody is taken from a second voice. Without the file, or
    with an older library, nothing changes. Returns whether it is in use.
    """
    folder = Path(__file__).resolve().parent if folder is None else folder
    try:
        value = (folder / NATIVE_MELODY_FILE).read_text(encoding='ascii').strip()
    except (OSError, UnicodeError):
        return False
    if value != '1':
        return False
    try:
        loader = ctypes.WinDLL if sys.platform == 'win32' else ctypes.CDLL
        select = loader(str(library.resolve())).eciPersianNativeMelody
    except (OSError, AttributeError):
        return False
    select.argtypes, select.restype = (ctypes.c_int,), ctypes.c_int
    if select(1) < 0:
        return False
    continuous_plan.CORE_MELODY = nvda_driver_probe.CORE_MELODY = True
    return True


def _paths(library: Path):
    # The private Persian test voice uses the complete language-owned C
    # analyzer even when NVDA has its own eSpeak installation. Unknown
    # written words follow the core's orthographic inference instead of
    # invoking a separate synthesizer as a pronunciation provider.
    return (library, None, None, None)


SOUND_VARIANT_FILE = 'persian_sound_variant.txt'


def select_sound_variant(library: Path, folder: Path | None = None):
    """Apply the Persian listening variant named by this package, if any.

    A test package can carry a one-line file containing 0 (previous sound) or
    a candidate number from 1 to 4 next to this module. Without the file, or with an
    older library, the library's default is left alone. Returns the selected
    variant or None.
    """
    folder = Path(__file__).resolve().parent if folder is None else folder
    try:
        value = (folder / SOUND_VARIANT_FILE).read_text(encoding='ascii').strip()
    except (OSError, UnicodeError):
        return None
    if value not in ('0', '1', '2', '3', '4'):
        return None
    try:
        loader = ctypes.WinDLL if sys.platform == 'win32' else ctypes.CDLL
        select = loader(str(library.resolve())).eciPersianSoundVariant
    except (OSError, AttributeError):
        return None
    select.argtypes, select.restype = (ctypes.c_int,), ctypes.c_int
    return int(value) if select(int(value)) >= 0 else None


def add_profiles(languages, library: Path):
    """Do not expose test profiles if their private components are absent."""
    text_library, _, _, _ = _paths(library)
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
        if (getattr(text_dll, 'eciPersianProjectCompleteUtf8', None) is None or
                getattr(text_dll, 'eciPersianProjectTextCompleteUtf8', None) is None):
            return languages
        for public, legacy in (('eciPersianTranslateUtf8', 'reloq_fa_direct_text'),
                               ('eciPersianNumber', 'reloq_fa_direct_number'),
                               ('eciPersianLetter', 'reloq_fa_direct_letter'),
                               ('eciPersianRelease', 'reloq_fa_direct_free'),
                               ('eciPersianPinglishWord', 'reloq_fa_pinglish_word')):
            require(text_dll, public, legacy)
    except (OSError, AttributeError):
        return languages
    select_sound_variant(library)
    select_native_melody(library)
    return {**languages, PERSIAN_PROBE.id: PERSIAN_PROBE, PINGLISH_PROBE.id: PINGLISH_PROBE}


def make_engine(library: Path, *, languages, language, voice):
    text_library, worker, reference, parent = _paths(library)
    return make_language_engine(library, text_library, worker, reference, parent,
                                language=language, voice=voice, languages=languages,
                                native_sentences=True)
