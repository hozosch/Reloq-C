"""Accessible runtime management of native pronunciation dictionary data."""
import weakref
import os

import addonHandler
import globalPluginHandler
import gui
from gui.settingsDialogs import NVDASettingsDialog, SettingsPanel
import synthDriverHandler
import ui
import wx

addonHandler.initTranslation()


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
        content.Add(wx.StaticText(self, label=_("Pronunciation dictionaries")), 0, wx.ALL, 10)
        self.community = wx.CheckBox(self, label=_("Use &community pronunciation dictionaries"))
        self.community.SetValue(self.synth.communityDictionaries if self.synth else False)
        content.Add(self.community, 0, wx.ALL, 10)
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
        self.Bind(wx.EVT_WINDOW_DESTROY, self.on_destroy)
        self.redraw()
        self.enable_controls(bool(self.synth))

    def enable_controls(self, enabled):
        for control in (*self.edits, self.files, self.update, self.community):
            control.Enable(enabled)

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

    def begin(self):
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
                else:
                    commit = details.get("community_commit")
                    message = (_("Dictionaries updated. Community revision: %s") % commit[:12]
                               if commit else _("Own dictionaries updated."))
                if instance is not None and not instance.closing and instance.operation == operation:
                    instance.pending = False
                    instance.enable_controls(instance.current_synth())
                    instance.status.SetLabel(message)
                ui.message(message)
            wx.CallAfter(report)
        return completed

    def on_update(self, event):
        # Refresh the already applied configuration. Unapplied list edits and
        # the community checkbox still obey NVDA's OK / Apply / Cancel flow.
        if not self.pending and self.current_synth():
            completed = self.begin()
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
                self.synth.configure_dictionaries(self.sources, self.community.GetValue(), completed)
                self.synth.saveSettings()
            except Exception as error:
                completed(error, None)

    def onDiscard(self):
        self.closing = True

    def on_destroy(self, event):
        if event.GetEventObject() is self:
            self.closing = True
        event.Skip()

    def onPanelActivated(self):
        synth = synthDriverHandler.getSynth()
        if synth is not None and synth.name == "reloqC" and synth is not self.synth:
            self.operation += 1
            self.pending = False
            self.synth = synth
            self.sources = list(synth.dictionary_manager().sources)
            self.community.SetValue(synth.communityDictionaries)
            self.redraw()
            self.status.SetLabel(_("Own entries override community entries. Original files are kept unchanged."))
        self.enable_controls(not self.pending and self.current_synth())
        super().onPanelActivated()


class GlobalPlugin(globalPluginHandler.GlobalPlugin):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        if EloquenceSettingsPanel not in NVDASettingsDialog.categoryClasses:
            NVDASettingsDialog.categoryClasses.append(EloquenceSettingsPanel)

    def terminate(self):
        if EloquenceSettingsPanel in NVDASettingsDialog.categoryClasses:
            NVDASettingsDialog.categoryClasses.remove(EloquenceSettingsPanel)
        super().terminate()
