"""Accessible runtime management of native pronunciation dictionary data."""
import weakref
import os
import tempfile
import threading

import addonHandler
import globalPluginHandler
import gui
from gui.settingsDialogs import NVDASettingsDialog, SettingsPanel
import synthDriverHandler
import ui
import wx

addonHandler.initTranslation()


def dictionary_update_message(details):
    counts = details.get("dictionary_update") if details else None
    if counts is None:
        return (_("Dictionary update complete. The number of new entries could not be determined.") +
                "\n" + _("The number of new dictionary files could not be determined."))
    if not counts["compared"] and (counts["total"] or counts["total_files"]):
        return (_("Dictionary loaded for the first time. Entries: %s") % counts["total"] +
                "\n" + _("Dictionary files in total: %s") % counts["total_files"])
    return (_("Dictionary update complete. New entries: %s") % counts["added"] +
            "\n" + _("New dictionary files: %s") % counts["added_files"])


def show_dictionary_update(details, parent=None):
    gui.messageBox(dictionary_update_message(details), _("Pronunciation dictionaries"),
                   wx.OK | wx.ICON_INFORMATION,
                   parent=parent if parent is not None else getattr(gui, "mainFrame", None))


class EloquenceSettingsPanel(SettingsPanel):
    title = _("Eloquence C")

    def makeSettings(self, content):
        from synthDrivers.reloqC._reloq_languages import LANGUAGES, LANGUAGE_DISPLAY_ORDER
        from synthDrivers.reloqC._reloq_dictionary_manager import DictionarySource
        synth = synthDriverHandler.getSynth()
        self.synth = synth if synth is not None and synth.name == "reloqC" else None
        self.source_class = DictionarySource
        self.languages = [LANGUAGES[key] for key in LANGUAGE_DISPLAY_ORDER]
        self.volumes = (_("Main words"), _("Word roots"), _("Abbreviations"))
        self.sources = list(self.synth.dictionary_manager().sources) if self.synth else []
        self.pending, self.closing = False, False
        self.operation = 0
        self.pitch_dirty = False
        self.wpm_dirty = False
        self.eq_dirty = False
        self._updating_controls = False
        self._saved_pitch = ((self.synth._pitchInHertz, self.synth._pitchHertz)
                             if self.synth else None)
        self._saved_wpm = ((self.synth._rateInWpm, self.synth._rateWpm)
                           if self.synth else None)
        self._saved_eq = self._equalizer_state(self.synth)
        self.segmentation_dirty = False
        self.camel_dirty = False
        content.Add(wx.StaticText(self, label=_("Exact base pitch")), 0, wx.ALL, 10)
        self.pitch_hz_mode = wx.CheckBox(self, label=_("Use base pitch in &hertz"))
        self.pitch_hz_mode.SetValue(self.synth._pitchInHertz if self.synth else False)
        content.Add(self.pitch_hz_mode, 0, wx.ALL, 10)
        pitch_label = wx.StaticText(self, label=_("Base pitch in hertz (1–24000):"))
        content.Add(pitch_label, 0, wx.LEFT | wx.RIGHT, 10)
        self.pitch_hz_value = wx.SpinCtrl(self, min=1, max=24000,
            initial=self.synth._pitchHertz if self.synth else 150)
        self.pitch_hz_value.SetName(pitch_label.GetLabelText())
        content.Add(self.pitch_hz_value, 0, wx.ALL, 10)
        content.Add(wx.StaticText(self, label=_(
            "Changing NVDA's normal pitch slider switches back to the ECI scale.")),
            0, wx.EXPAND | wx.LEFT | wx.RIGHT | wx.BOTTOM, 10)
        self.pitch_hz_mode.Bind(wx.EVT_CHECKBOX, self.on_pitch_changed)
        self.pitch_hz_value.Bind(wx.EVT_SPINCTRL, self.on_pitch_changed)
        self.pitch_hz_value.Bind(wx.EVT_TEXT, self.on_pitch_changed)
        content.Add(wx.StaticText(self, label=_("Exact speech rate")), 0, wx.ALL, 10)
        self.wpm_mode = wx.CheckBox(self, label=_("Use speech rate in &words per minute"))
        self.wpm_mode.SetValue(self.synth._rateInWpm if self.synth else False)
        content.Add(self.wpm_mode, 0, wx.ALL, 10)
        wpm_label = wx.StaticText(self, label=_("Speech rate in words per minute (1–6000):"))
        content.Add(wpm_label, 0, wx.LEFT | wx.RIGHT, 10)
        self.wpm_value = wx.SpinCtrl(self, min=1, max=6000,
            initial=self.synth._rateWpm if self.synth else 150)
        self.wpm_value.SetName(wpm_label.GetLabelText())
        content.Add(self.wpm_value, 0, wx.ALL, 10)
        content.Add(wx.StaticText(self, label=_(
            "Changing NVDA's normal rate or rate boost switches back to the normal rate scale.")),
            0, wx.EXPAND | wx.LEFT | wx.RIGHT | wx.BOTTOM, 10)
        self.wpm_mode.Bind(wx.EVT_CHECKBOX, self.on_wpm_changed)
        self.wpm_value.Bind(wx.EVT_SPINCTRL, self.on_wpm_changed)
        self.wpm_value.Bind(wx.EVT_TEXT, self.on_wpm_changed)
        content.Add(wx.StaticText(self, label=_("Equalizer")), 0, wx.ALL, 10)
        self.eq_mode = wx.CheckBox(self, label=_("Enable &equalizer"))
        self.eq_mode.SetValue(self.synth._eqEnabled if self.synth else False)
        content.Add(self.eq_mode, 0, wx.ALL, 10)
        eq_label = wx.StaticText(self, label=_("Equalizer &preset:"))
        content.Add(eq_label, 0, wx.LEFT | wx.RIGHT, 10)
        self.eq_preset = wx.Choice(self, choices=[_("Balanced"), _("Warm"),
            _("Bright"), _("Smooth"), _("Custom")])
        self.eq_preset.SetName(eq_label.GetLabelText())
        self.eq_preset.SetSelection(self.synth._eqPreset if self.synth else 0)
        content.Add(self.eq_preset, 0, wx.ALL, 10)
        eq_values = self._equalizer_values(self.synth)
        self.eq_bands = []
        for label_text, value in zip((_("Bass (250 Hz, -12 to +12 dB):"),
                                      _("Midrange (1200 Hz, -12 to +12 dB):"),
                                      _("Treble (3000 Hz, -12 to +12 dB):")), eq_values):
            label = wx.StaticText(self, label=label_text)
            content.Add(label, 0, wx.LEFT | wx.RIGHT, 10)
            control = wx.SpinCtrl(self, min=-12, max=12, initial=value)
            control.SetName(label.GetLabelText())
            control.Bind(wx.EVT_SPINCTRL, self.on_equalizer_band)
            control.Bind(wx.EVT_TEXT, self.on_equalizer_band)
            content.Add(control, 0, wx.ALL, 10)
            self.eq_bands.append(control)
        self.eq_mode.Bind(wx.EVT_CHECKBOX, self.on_equalizer_mode)
        self.eq_preset.Bind(wx.EVT_CHOICE, self.on_equalizer_preset)
        self.segmentation = wx.CheckBox(self, label=_("Split unfamiliar &English words"))
        self.segmentation.SetValue(getattr(self.synth, "_wordSegmentation", False) if self.synth else False)
        self.segmentation.Bind(wx.EVT_CHECKBOX, self.on_segmentation_changed)
        content.Add(self.segmentation, 0, wx.ALL, 10)
        self.camel_case = wx.CheckBox(self, label=_("Split &camel-case words (all languages)"))
        self.camel_case.SetValue(getattr(self.synth, "_camelCase", False) if self.synth else False)
        self.camel_case.Bind(wx.EVT_CHECKBOX, self.on_camel_changed)
        content.Add(self.camel_case, 0, wx.ALL, 10)
        content.Add(wx.StaticText(self, label=_("Pronunciation dictionaries")), 0, wx.ALL, 10)
        self.community = wx.CheckBox(self, label=_("Use &community pronunciation dictionaries"))
        self.community.SetValue(self.synth.communityDictionaries if self.synth else False)
        content.Add(self.community, 0, wx.ALL, 10)
        label = wx.StaticText(self, label=_("Dictionary &source:"))
        content.Add(label, 0, wx.LEFT | wx.RIGHT, 10)
        self.repository = wx.Choice(self, choices=[_("IBM TTS Dictionaries"), _("Alternative IBM TTS Dictionaries")])
        self.repository.SetName(label.GetLabelText())
        self.repository.SetSelection(1 if self.synth and self.synth.dictionary_manager().repository == "alternative" else 0)
        content.Add(self.repository, 0, wx.EXPAND | wx.ALL, 10)
        label = wx.StaticText(self, label=_("&Own dictionary files (later files take priority):"))
        content.Add(label, 0, wx.LEFT | wx.RIGHT, 10)
        self.files = wx.ListBox(self, size=(-1, 180))
        self.files.SetName(label.GetLabelText())
        content.Add(self.files, 1, wx.EXPAND | wx.ALL, 10)
        self.edits = []
        row = wx.BoxSizer(wx.HORIZONTAL)
        for text, callback in ((_("&Add file…"), self.on_add), (_("&Remove"), self.on_remove),
                               (_("Move &up"), lambda event: self.move(-1)),
                               (_("Move &down"), lambda event: self.move(1))):
            button = wx.Button(self, label=text)
            button.Bind(wx.EVT_BUTTON, callback)
            self.edits.append(button)
            row.Add(button, 0, wx.RIGHT, 8)
        content.Add(row, 0, wx.LEFT | wx.RIGHT | wx.BOTTOM, 10)
        content.Add(wx.StaticText(self, label=_(
            "Use OK or Apply to save. Changes to selected .dic files are loaded automatically.")),
            0, wx.EXPAND | wx.LEFT | wx.RIGHT | wx.BOTTOM, 10)
        self.status = wx.StaticText(self, label=_(
            "Own entries override community entries. Original files are kept unchanged.")
            if self.synth else _("Select Eloquence C as the synthesizer first."))
        content.Add(self.status, 0, wx.EXPAND | wx.LEFT | wx.RIGHT | wx.BOTTOM, 10)
        row = wx.BoxSizer(wx.HORIZONTAL)
        self.update = wx.Button(self, label=_("&Update dictionaries now"))
        self.update.Bind(wx.EVT_BUTTON, self.on_update)
        row.Add(self.update, 0, wx.RIGHT, 8)
        folder = wx.Button(self, label=_("Open dictionary &folder"))
        folder.Bind(wx.EVT_BUTTON, self.on_folder)
        row.Add(folder, 0, wx.RIGHT, 8)
        self.edits.append(folder)
        content.Add(row, 0, wx.ALL, 10)
        label = wx.StaticText(self, label=_("Imported NVDA speech dictionaries:"))
        content.Add(label, 0, wx.LEFT | wx.RIGHT, 10)
        self.imported = wx.ListBox(self, size=(-1, 100))
        self.imported.SetName(label.GetLabelText())
        content.Add(self.imported, 0, wx.EXPAND | wx.ALL, 10)
        self.imported_records = ()
        row = wx.BoxSizer(wx.HORIZONTAL)
        for text, callback in ((_("Import NVDA dictionary…"), self.on_import_nvda),
                               (_("Edit imported dictionary…"), self.on_edit_nvda),
                               (_("Remove imported dictionary"), self.on_remove_nvda)):
            button = wx.Button(self, label=text)
            button.Bind(wx.EVT_BUTTON, callback)
            self.edits.append(button)
            row.Add(button, 0, wx.RIGHT, 8)
        content.Add(row, 0, wx.LEFT | wx.RIGHT | wx.BOTTOM, 10)
        content.Add(wx.StaticText(self, label=_(
            "Imported NVDA dictionaries apply only to their chosen language and voices. Original files are unchanged.")),
            0, wx.EXPAND | wx.LEFT | wx.RIGHT | wx.BOTTOM, 10)
        self.Bind(wx.EVT_WINDOW_DESTROY, self.on_destroy)
        self.redraw()
        self.enable_controls(bool(self.synth))
        self.package_update = None
        self.package_pending = False
        package_manifest = addonHandler.getCodeAddon().manifest
        self.installed_version = package_manifest['version']
        self.installed_name = package_manifest['name']
        content.Add(wx.StaticText(self, label=_("Add-on updates")), 0, wx.ALL, 10)
        self.package_status = wx.StaticText(self, label=_("Installed version: %s") % self.installed_version)
        content.Add(self.package_status, 0, wx.LEFT | wx.RIGHT, 10)
        self.package_button = wx.Button(self, label=_("Check for add-on updates"))
        self.package_button.Bind(wx.EVT_BUTTON, self.on_package_update)
        content.Add(self.package_button, 0, wx.ALL, 10)

    def on_package_update(self, event):
        if self.package_pending:
            return
        from synthDrivers.reloqC import _reloq_updates
        candidate, installed = self.package_update, self.installed_version
        self.package_pending = True
        self.package_button.Enable(False)
        self.package_status.SetLabel(_("Downloading update…") if candidate else _("Checking for updates…"))
        panel_ref = weakref.ref(self)
        def report(error, update, temporary, path):
            panel = panel_ref()
            try:
                if panel is None or panel.closing:
                    return
                panel.package_pending = False
                panel.package_button.Enable(True)
                if error:
                    message = _("Update failed: %s") % str(error)
                elif path:
                    # Use NVDA's ordinary compatibility, confirmation and
                    # replacement flow; do not overwrite a running add-on.
                    try:
                        manifest = addonHandler.AddonBundle(str(path)).manifest
                        if manifest['name'] != panel.installed_name or manifest['version'] != update.version:
                            raise ValueError('Update package name or version mismatch')
                        from gui.addonGui import installAddon
                        installed_ok = installAddon(panel, str(path))
                        message = _("Update handed to NVDA.") if installed_ok else _("Update installation cancelled.")
                        if installed_ok and not panel.closing:
                            panel.package_button.Enable(False)
                    except Exception as install_error:
                        message = _("Update failed: %s") % str(install_error)
                elif update:
                    panel.package_update = update
                    panel.package_button.SetLabel(_("Download and install update"))
                    message = _("Version %s is available.") % update.version
                else:
                    message = _("No newer published add-on package is available.")
                if not panel.closing:
                    panel.package_status.SetLabel(message)
                    ui.message(message)
            finally:
                if temporary is not None:
                    temporary.cleanup()
        def work():
            temporary = None
            try:
                if candidate:
                    temporary = tempfile.TemporaryDirectory(prefix='reloq-update-')
                    path = _reloq_updates.download(candidate, temporary.name)
                    wx.CallAfter(report, None, candidate, temporary, path)
                else:
                    wx.CallAfter(report, None, _reloq_updates.check(installed), None, None)
            except Exception as error:
                wx.CallAfter(report, error, None, temporary, None)
        threading.Thread(target=work, name='ReLoq package update', daemon=True).start()

    def enable_controls(self, enabled):
        for control in (*self.edits, self.files, self.imported, self.update, self.community, self.repository, self.segmentation, self.camel_case):
            control.Enable(enabled)
        self.pitch_hz_mode.Enable(enabled)
        self.pitch_hz_value.Enable(enabled and self.pitch_hz_mode.GetValue())
        self.wpm_mode.Enable(enabled)
        self.wpm_value.Enable(enabled and self.wpm_mode.GetValue())
        self.eq_mode.Enable(enabled)
        self.eq_preset.Enable(enabled and self.eq_mode.GetValue())
        for control in self.eq_bands:
            control.Enable(enabled and self.eq_mode.GetValue())

    @staticmethod
    def _equalizer_state(synth):
        return ((synth._eqEnabled, synth._eqPreset, synth._eqCustomLow,
                 synth._eqCustomMid, synth._eqCustomHigh) if synth else None)

    @staticmethod
    def _equalizer_values(synth):
        if synth is None:
            return (0, 0, 0)
        from synthDrivers.reloqC import EQ_PRESETS
        return ((synth._eqCustomLow, synth._eqCustomMid, synth._eqCustomHigh)
                if synth._eqPreset == 4 else EQ_PRESETS[synth._eqPreset])

    def _preview_equalizer(self, preset, custom):
        if self.current_synth():
            self.synth.configure_equalizer(bool(self.eq_mode.GetValue()), preset, *custom)
            self.eq_dirty = True

    def on_equalizer_mode(self, event):
        if not self._updating_controls and self.current_synth():
            state = self._equalizer_state(self.synth)
            self._preview_equalizer(self.eq_preset.GetSelection(), state[2:])
        self.enable_controls(not self.pending and self.current_synth())
        event.Skip()

    def on_equalizer_preset(self, event):
        if not self._updating_controls and self.current_synth():
            from synthDrivers.reloqC import EQ_PRESETS
            preset = self.eq_preset.GetSelection()
            state = self._equalizer_state(self.synth)
            values = state[2:] if preset == 4 else EQ_PRESETS[preset]
            self._updating_controls = True
            try:
                for control, value in zip(self.eq_bands, values):
                    control.SetValue(value)
            finally:
                self._updating_controls = False
            self._preview_equalizer(preset, state[2:])
        event.Skip()

    def on_equalizer_band(self, event):
        if not self._updating_controls and self.current_synth():
            values = tuple(control.GetValue() for control in self.eq_bands)
            if all(isinstance(value, int) and -12 <= value <= 12 for value in values):
                self.eq_preset.SetSelection(4)
                self._preview_equalizer(4, values)
        event.Skip()

    def on_pitch_changed(self, event):
        synth = self.synth if self.current_synth() else None
        self.pitch_hz_value.Enable(bool(synth) and self.pitch_hz_mode.GetValue())
        if not self._updating_controls and synth:
            value = self.pitch_hz_value.GetValue()
            if isinstance(value, int) and 1 <= value <= 24000:
                synth.configure_pitch_hertz(bool(self.pitch_hz_mode.GetValue()), value)
                self.pitch_dirty = True
        event.Skip()

    def on_wpm_changed(self, event):
        synth = self.synth if self.current_synth() else None
        self.wpm_value.Enable(bool(synth) and self.wpm_mode.GetValue())
        if not self._updating_controls and synth:
            value = self.wpm_value.GetValue()
            if isinstance(value, int) and 1 <= value <= 6000:
                synth.configure_rate_wpm(bool(self.wpm_mode.GetValue()), value)
                self.wpm_dirty = True
        event.Skip()

    def on_segmentation_changed(self, event):
        self.segmentation_dirty = True
        event.Skip()

    def on_camel_changed(self, event):
        self.camel_dirty = True
        event.Skip()

    def redraw_imported(self, selected=None):
        self.imported_records = self.synth.imported_dictionary_manager().records if self.synth and hasattr(self.synth, "imported_dictionary_manager") else ()
        languages = {lang.id: _(lang.name) for lang in self.languages}
        voices = {voice.id: voice.name for lang in self.languages for voice in lang.voices}
        labels = []
        for path, language, voice, _dictionary in self.imported_records:
            source = path.stem.split("-", 3)[-1]
            scope = _("All voices") if voice == "all" else voices.get(voice, voice)
            labels.append(f"{languages.get(language, str(language))} — {scope} — {source}")
        self.imported.Set(labels)
        if selected is not None and self.imported_records:
            self.imported.SetSelection(min(selected, len(self.imported_records) - 1))

    def on_import_nvda(self, event):
        if not self.current_synth(): return
        try:
            from NVDAState import WritePaths, shouldWriteToDisk
            if not shouldWriteToDisk(): raise PermissionError(_("NVDA cannot save dictionaries in this mode."))
            folder = str(WritePaths.voiceDictsDir)
            with wx.FileDialog(self, _("Choose an NVDA speech dictionary"), defaultDir=folder,
                               wildcard=_("NVDA dictionary files (*.dic)|*.dic|All files (*.*)|*.*"),
                               style=wx.FD_OPEN | wx.FD_FILE_MUST_EXIST) as dialog:
                if dialog.ShowModal() != wx.ID_OK: return
                path = dialog.GetPath()
            with wx.SingleChoiceDialog(self, _("Language for this dictionary:"),
                                       _("NVDA dictionary language"), [_(x.name) for x in self.languages]) as dialog:
                current = next((i for i, language in enumerate(self.languages)
                                if language.id == self.synth._selectedLanguage), 0)
                dialog.SetSelection(current)
                if dialog.ShowModal() != wx.ID_OK: return
                language = self.languages[dialog.GetSelection()]
            choices = [_('All voices')] + [voice.name for voice in language.voices]
            with wx.SingleChoiceDialog(self, _("Apply to which voices?"),
                                       _("NVDA dictionary voice"), choices) as dialog:
                dialog.SetSelection(0)
                if dialog.ShowModal() != wx.ID_OK: return
                selected = dialog.GetSelection()
            voice = "all" if selected == 0 else language.voices[selected - 1].id
            _path, count, created = self.synth.imported_dictionary_manager().import_file(path, language.id, voice)
            self.redraw_imported()
            message = (_("Imported NVDA dictionary entries: %s") if created else _("This NVDA dictionary was already imported. Entries: %s")) % count
            gui.messageBox(message, _("NVDA speech dictionaries"), wx.OK | wx.ICON_INFORMATION, parent=self)
        except (OSError, ValueError, UnicodeError) as error:
            gui.messageBox(_("NVDA dictionary import failed: %s") % str(error),
                           _("NVDA speech dictionaries"), wx.OK | wx.ICON_ERROR, parent=self)

    def on_edit_nvda(self, event):
        selected = self.imported.GetSelection()
        if not self.current_synth() or selected == wx.NOT_FOUND: return
        try:
            from NVDAState import shouldWriteToDisk
            if not shouldWriteToDisk(): raise PermissionError(_("NVDA cannot save dictionaries in this mode."))
            from gui.speechDict import DictionaryDialog
            path, _language, _voice, original = self.imported_records[selected]
            dictionary = type(original)()
            dictionary.load(str(path))
            class ImportedDictionaryDialog(DictionaryDialog):
                def __init__(dialog, parent):
                    super().__init__(parent, _("Imported NVDA dictionary"), dictionary)
            with ImportedDictionaryDialog(self) as dialog:
                dialog.ShowModal()
            self.synth.imported_dictionary_manager().reload()
            self.redraw_imported(selected)
        except (OSError, ValueError, UnicodeError) as error:
            gui.messageBox(_("NVDA dictionary edit failed: %s") % str(error), self.title,
                           wx.OK | wx.ICON_ERROR, parent=self)

    def on_remove_nvda(self, event):
        selected = self.imported.GetSelection()
        if not self.current_synth() or selected == wx.NOT_FOUND: return
        from NVDAState import shouldWriteToDisk
        if not shouldWriteToDisk():
            gui.messageBox(_("NVDA cannot save dictionaries in this mode."), self.title,
                           wx.OK | wx.ICON_ERROR, parent=self)
            return
        if gui.messageBox(_("Remove this imported copy? The original dictionary is kept."),
                          _("NVDA speech dictionaries"), wx.YES_NO | wx.NO_DEFAULT | wx.ICON_QUESTION,
                          parent=self) != wx.YES: return
        try:
            self.synth.imported_dictionary_manager().remove(self.imported_records[selected][0])
            self.redraw_imported(selected)
        except (OSError, ValueError) as error:
            gui.messageBox(_("NVDA dictionary removal failed: %s") % str(error), self.title,
                           wx.OK | wx.ICON_ERROR, parent=self)

    def on_folder(self, event):
        if self.current_synth():
            os.startfile(str(self.synth.dictionary_manager().bundled))

    def current_synth(self):
        return self.synth is not None and synthDriverHandler.getSynth() is self.synth

    def redraw(self, selected=None):
        labels = {language.id: _(language.name) for language in self.languages}
        self.files.Set([f"{index + 1}. {source.path} — {labels[source.language]}, {self.volumes[source.volume]}"
                        for index, source in enumerate(self.sources)])
        if selected is not None and self.sources:
            self.files.SetSelection(min(selected, len(self.sources) - 1))
        self.redraw_imported()

    def on_add(self, event):
        with wx.FileDialog(self, _("Choose a pronunciation dictionary"),
                           wildcard=_("Dictionary files (*.dic)|*.dic|All files (*.*)|*.*"),
                           style=wx.FD_OPEN | wx.FD_FILE_MUST_EXIST) as dialog:
            if dialog.ShowModal() != wx.ID_OK:
                return
            path = dialog.GetPath()
        language_names = [_(language.name) for language in self.languages]
        with wx.SingleChoiceDialog(self, _("Language of the dictionary:"),
                                   _("Dictionary language"), language_names) as dialog:
            if dialog.ShowModal() != wx.ID_OK:
                return
            language = self.languages[dialog.GetSelection()].id
        with wx.SingleChoiceDialog(self, _("Dictionary type:"),
                                   _("Dictionary type"), self.volumes) as dialog:
            if dialog.ShowModal() != wx.ID_OK:
                return
            volume = dialog.GetSelection()
        self.sources.append(self.source_class(path, language, volume))
        self.redraw(len(self.sources) - 1)

    def on_remove(self, event):
        selected = self.files.GetSelection()
        if selected != wx.NOT_FOUND:
            del self.sources[selected]
            self.redraw(selected)

    def move(self, delta):
        selected = self.files.GetSelection()
        target = selected + delta
        if selected != wx.NOT_FOUND and 0 <= target < len(self.sources):
            self.sources[selected], self.sources[target] = self.sources[target], self.sources[selected]
            self.redraw(target)

    def begin(self, show_result=False):
        self.pending = True
        self.operation += 1
        operation = self.operation
        self.enable_controls(False)
        self.status.SetLabel(_("Updating pronunciation dictionaries…"))
        self_ref = weakref.ref(self)
        def completed(error, details):
            def report():
                instance = self_ref()
                if error:
                    message = _("Dictionary update failed: %s") % str(error)
                elif details and "dictionary_update" in details:
                    message = dictionary_update_message(details)
                else:
                    commit = details.get("community_commit")
                    message = (_("Dictionaries updated. Community revision: %s") % commit[:12]
                               if commit else _("Own dictionaries updated."))
                active = instance is not None and not instance.closing and instance.operation == operation
                if active:
                    instance.pending = False
                    instance.enable_controls(instance.current_synth())
                    instance.status.SetLabel(message)
                if instance is not None and instance.operation != operation:
                    return
                if error and show_result:
                    gui.messageBox(message, _("Pronunciation dictionaries"), wx.OK | wx.ICON_ERROR,
                                   parent=instance if active else getattr(gui, "mainFrame", None))
                elif not error and (show_result or details and details.get("remote_changed")):
                    show_dictionary_update(details, parent=instance if active else None)
                elif active:
                    ui.message(message)
            wx.CallAfter(report)
        return completed

    def on_update(self, event):
        # Refresh the already applied configuration. Unapplied list edits and
        # the community checkbox still obey NVDA's OK / Apply / Cancel flow.
        if not self.pending and self.current_synth():
            completed = self.begin(show_result=True)
            try:
                self.synth.refresh_dictionaries(completed)
            except Exception as error:
                completed(error, None)

    def isValid(self):
        if not self.current_synth():
            return True
        if self.pending:
            gui.messageBox(_("Wait for the dictionary update to finish."), self.title, parent=self)
            return False
        from synthDrivers.reloqC._reloq_dictionary_manager import validate_sources
        try:
            validate_sources(self.sources)
        except (OSError, ValueError) as error:
            gui.messageBox(_("Dictionary update failed: %s") % str(error), self.title, parent=self)
            return False
        return True

    def onSave(self):
        if self.current_synth():
            completed = self.begin()
            try:
                if self.pitch_dirty:
                    self.synth.configure_pitch_hertz(self.pitch_hz_mode.GetValue(), self.pitch_hz_value.GetValue())
                if self.wpm_dirty:
                    self.synth.configure_rate_wpm(self.wpm_mode.GetValue(), self.wpm_value.GetValue())
                if self.eq_dirty:
                    state = self._equalizer_state(self.synth)
                    self.synth.configure_equalizer(bool(self.eq_mode.GetValue()),
                                                   self.eq_preset.GetSelection(), *state[2:])
                if self.segmentation_dirty:
                    self.synth._wordSegmentation = self.segmentation.GetValue()
                    self.segmentation_dirty = False
                if self.camel_dirty:
                    self.synth._camelCase = self.camel_case.GetValue()
                    self.camel_dirty = False
                self.synth.configure_dictionaries(self.sources, self.community.GetValue(), completed,
                    "alternative" if self.repository.GetSelection() == 1 else "community")
                self.synth.saveSettings()
                self._saved_pitch = (self.synth._pitchInHertz, self.synth._pitchHertz)
                self._saved_wpm = (self.synth._rateInWpm, self.synth._rateWpm)
                self._saved_eq = self._equalizer_state(self.synth)
                self.pitch_dirty = self.wpm_dirty = self.eq_dirty = False
            except Exception as error:
                completed(error, None)

    def _restore_preview(self):
        synth = self.synth if self.current_synth() else None
        if synth:
            if self.pitch_dirty and self._saved_pitch is not None:
                synth.configure_pitch_hertz(*self._saved_pitch)
            if self.wpm_dirty and self._saved_wpm is not None:
                synth.configure_rate_wpm(*self._saved_wpm)
            if self.eq_dirty and self._saved_eq is not None:
                synth.configure_equalizer(*self._saved_eq)
        self.pitch_dirty = self.wpm_dirty = self.eq_dirty = False

    def onDiscard(self):
        self._restore_preview()
        self.closing = True

    def on_destroy(self, event):
        if event.GetEventObject() is self:
            if not self.closing:
                self._restore_preview()
            self.closing = True
        event.Skip()

    def onPanelActivated(self):
        synth = synthDriverHandler.getSynth()
        self._updating_controls = True
        try:
            self._refresh_settings(synth)
        finally:
            self._updating_controls = False
        self.enable_controls(not self.pending and self.current_synth())
        super().onPanelActivated()

    def _refresh_settings(self, synth):
        if synth is not None and synth.name == "reloqC" and synth is not self.synth:
            self.operation += 1
            self.pending = False
            self.synth = synth
            self.sources = list(synth.dictionary_manager().sources)
            self.pitch_hz_mode.SetValue(synth._pitchInHertz)
            self.pitch_hz_value.SetValue(synth._pitchHertz)
            self.pitch_dirty = False
            self.wpm_mode.SetValue(synth._rateInWpm)
            self.wpm_value.SetValue(synth._rateWpm)
            self.wpm_dirty = False
            self.eq_dirty = False
            self.segmentation.SetValue(getattr(synth, "_wordSegmentation", False))
            self.segmentation_dirty = False
            self.camel_case.SetValue(getattr(synth, "_camelCase", False))
            self.camel_dirty = False
            self.community.SetValue(synth.communityDictionaries)
            self.repository.SetSelection(1 if synth.dictionary_manager().repository == "alternative" else 0)
            self.redraw()
            self.status.SetLabel(_("Own entries override community entries. Original files are kept unchanged."))
        if self.current_synth() and not self.pitch_dirty:
            self.pitch_hz_mode.SetValue(self.synth._pitchInHertz)
            self.pitch_hz_value.SetValue(self.synth._pitchHertz)
            self._saved_pitch = (self.synth._pitchInHertz, self.synth._pitchHertz)
            self.pitch_dirty = False
        if self.current_synth() and not self.wpm_dirty:
            self.wpm_mode.SetValue(self.synth._rateInWpm)
            self.wpm_value.SetValue(self.synth._rateWpm)
            self._saved_wpm = (self.synth._rateInWpm, self.synth._rateWpm)
        if self.current_synth() and not self.eq_dirty:
            self.eq_mode.SetValue(self.synth._eqEnabled)
            self.eq_preset.SetSelection(self.synth._eqPreset)
            for control, value in zip(self.eq_bands, self._equalizer_values(self.synth)):
                control.SetValue(value)
            self._saved_eq = self._equalizer_state(self.synth)
        if self.current_synth() and not self.segmentation_dirty:
            self.segmentation.SetValue(getattr(self.synth, "_wordSegmentation", False))
        if self.current_synth() and not self.camel_dirty:
            self.camel_case.SetValue(getattr(self.synth, "_camelCase", False))


class GlobalPlugin(globalPluginHandler.GlobalPlugin):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        if EloquenceSettingsPanel not in NVDASettingsDialog.categoryClasses:
            NVDASettingsDialog.categoryClasses.append(EloquenceSettingsPanel)

    def terminate(self):
        if EloquenceSettingsPanel in NVDASettingsDialog.categoryClasses:
            NVDASettingsDialog.categoryClasses.remove(EloquenceSettingsPanel)
        super().terminate()
