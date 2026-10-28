"""Manual NVDA hearing preview of the independent C Eloquence port.

This deliberately does not register a synthesizer. The native ECI text voice
does not yet cover a language, so taking over NVDA's ordinary speech would
leave uncovered navigation text silent. The user's configured synthesizer
remains responsible for dialogs, status, and unsupported-text messages.
"""

from __future__ import annotations

import ctypes
from pathlib import Path

import addonHandler
import config
import gui
import globalPluginHandler
from logHandler import log
import nvwave
from scriptHandler import script
import ui
import wx

from ._reloq_eci_direct import DirectEciEngine, EciSynthesisError
from ._reloq_eci_transport import EciTextTransport


addonHandler.initTranslation()


def _is_64_bit_process() -> bool:
    """Reject 32-bit NVDA; let the Windows loader decide DLL compatibility.

    On Windows ARM, IsWow64Process2 may report the host's ARM64 machine for an
    ARM64EC process that *can* load an x64 DLL. Pointer size is only a quick
    precheck: a pure ARM64 process also has 64-bit pointers, but its attempt
    to load the bundled x64 DLL will fail safely in DirectEciEngine.
    """
    return ctypes.sizeof(ctypes.c_void_p) == 8


class _EciLoadError(OSError):
    """The engine factory could not load the packaged x64 DLL."""


def _load_engine(dll_path: Path) -> DirectEciEngine:
    try:
        return DirectEciEngine(dll_path)
    except OSError as error:
        raise _EciLoadError(f"eci.dll could not be loaded: {error}") from error


class GlobalPlugin(globalPluginHandler.GlobalPlugin):
    """A self-contained listening tool that never changes NVDA's synthesizer."""

    def __init__(self):
        super().__init__()
        self._transport = None
        self._load_failure = None
        self._terminating = False
        self._last_text = "Pappe. Packen. Tappen."

    def _on_worker_error(self, error: Exception) -> None:
        # ECI callbacks and WavePlayer callbacks may run outside the GUI
        # thread. Messages must be posted to the NVDA main thread.
        log.error("reloq preview failed",
                  exc_info=(type(error), error, error.__traceback__))
        if isinstance(error, EciSynthesisError) and error.status == -2:
            location = (" " + _("Ab Zeichen {position}.").format(
                position=error.character_position)
                if error.character_position is not None else "")
            if error.stage == 2:
                message = (_("Der C-Port kennt ein Wort oder Zeichen dieses Textes noch nicht.")
                           + location)
            elif error.stage == 3:
                message = (_("Der C-Port erkennt die Aussprache, hat für diesen Lautverlauf aber noch kein Klangmodell.")
                           + location)
            elif error.stage == 4:
                message = (_("Für eine Textmarke fehlt noch die passende Audioposition.")
                           + location)
            elif error.stage == 1:
                message = _("Für die gewählte Abtastrate fehlt noch ein Textstimmenprofil.")
            else:
                message = _("Der C-Port kann diesen Text noch nicht sprechen.")
            message += " " + _("Die NVDA-Stimme bleibt aktiv.")
        elif "eciSynthesize failed" in str(error):
            message = _("Der C-Port konnte den Text noch nicht sprechen: nicht abgedeckt oder Synthesefehler. Die NVDA-Stimme bleibt aktiv.")
        elif isinstance(error, _EciLoadError):
            self._load_failure = error
            cause = error.__cause__ or error
            detail = str(cause)
            winerror = getattr(cause, "winerror", None)
            if winerror is not None and f"{winerror}" not in detail:
                detail = f"WinError {winerror}: {detail}"
            message = (_("Die x64-DLL des Hörtests konnte in diesem NVDA-Prozess nicht geladen werden. Die Prozessarchitektur könnte inkompatibel sein oder eine benötigte DLL fehlen. Ihre NVDA-Stimme bleibt aktiv.")
                       + " " + detail)
        else:
            message = _("Der Eloquence-C-Hörtest konnte nicht abgespielt werden. Die NVDA-Stimme bleibt aktiv.")
        wx.CallAfter(self._announce, message)

    def _announce(self, message: str) -> None:
        if not self._terminating:
            ui.message(message)

    def _get_transport(self) -> EciTextTransport:
        if not _is_64_bit_process():
            raise OSError("C preview requires a 64-bit NVDA process")
        if self._load_failure is not None and self._transport is not None:
            # Startup failed on the worker. Retire its closed transport so a
            # later preview command can try again (for example, after a DLL
            # dependency is restored without restarting NVDA).
            failed, self._transport = self._transport, None
            self._load_failure = None
            failed.shutdown(timeout=1.5)
        if self._transport is None:
            player = nvwave.WavePlayer(
                channels=1,
                samplesPerSec=11025,
                bitsPerSample=16,
                outputDevice=config.conf["audio"]["outputDevice"],
            )
            dll_path = Path(__file__).with_name("eci.dll")
            self._transport = EciTextTransport(
                lambda: _load_engine(dll_path),
                player,
                lambda _index: None,
                lambda _job: None,
                self._on_worker_error,
            )
        return self._transport

    @script(
        description=_("Eloquence-C-Hörtest: Text eingeben"),
        gesture="kb:NVDA+control+shift+f11",
    )
    def script_previewText(self, gesture):
        if self._terminating:
            return
        # NVDA scripts need not be entered on the wx event loop. Create the
        # modal dialog there and return from the script immediately.
        wx.CallAfter(self._ask_for_text)

    def _ask_for_text(self) -> None:
        if self._terminating:
            return
        gui.mainFrame.prePopup()
        try:
            dialog = wx.TextEntryDialog(
                gui.mainFrame,
                _("Deutschen Testtext eingeben, z. B. Pappe. Packen. Tappen."),
                _("Eloquence-C-Hörtest"),
                self._last_text,
            )
            try:
                if dialog.ShowModal() != wx.ID_OK:
                    return
                text = dialog.GetValue()
            finally:
                dialog.Destroy()
        finally:
            gui.mainFrame.postPopup()
        if not text:
            ui.message(_("Kein Testtext eingegeben."))
            return
        self._last_text = text
        if not _is_64_bit_process():
            ui.message(_("Dieser Hörtest benötigt NVDA als 64-Bit-Prozess. Ihre bisherige Stimme bleibt aktiv."))
            return
        try:
            self._get_transport().submit((text,))
        except RuntimeError as error:
            if str(error) == "transport has shut down" and self._load_failure is not None:
                # The worker reported the actual loader error before submit
                # could run. Avoid speaking a second, generic failure.
                return
            self._on_worker_error(error)
        except Exception as error:
            self._on_worker_error(error)

    @script(
        description=_("Eloquence-C-Hörtest abbrechen"),
        gesture="kb:NVDA+control+shift+f10",
    )
    def script_cancelPreview(self, gesture):
        if self._transport is not None:
            try:
                self._transport.cancel()
            except Exception as error:
                self._on_worker_error(error)
                return
        ui.message(_("Eloquence-C-Hörtest abgebrochen."))

    def terminate(self):
        self._terminating = True
        transport, self._transport = self._transport, None
        if transport is not None and not transport.shutdown(timeout=1.5):
            # A worker that did not exit still owns the player. Closing it
            # underneath an ECI callback is unsafe; the daemon can finish its
            # cooperative cancel during NVDA shutdown.
            log.warning("reloq preview worker did not stop within 1.5 seconds")
        super().terminate()
