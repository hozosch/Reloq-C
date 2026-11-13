"""Check automatic speech and manual dialogs in an actual NVDA process."""
import json
import os
from pathlib import Path
import traceback

import globalPluginHandler
import wx

RESULT = Path(os.environ['RELOQ_SEGMENTATION_PROBE_RESULT'])


class Status:
    def SetLabel(self, value):
        self.value = value


class GlobalPlugin(globalPluginHandler.GlobalPlugin):
    def __init__(self):
        super().__init__()
        wx.CallLater(1500, self.run)

    def restore(self):
        if hasattr(self, 'old_message'):
            self.ui.message = self.old_message
            self.gui.messageBox = self.old_dialog
            del self.old_message

    def fail(self):
        error = traceback.format_exc()
        self.restore()
        RESULT.write_text(json.dumps({'ok': False, 'error': error}, ensure_ascii=False), encoding='utf8')

    def run(self):
        try:
            import gui
            import ui
            import synthDriverHandler
            from globalPlugins import reloqCDictionaries as dictionaries
            if not synthDriverHandler.setSynth('reloqC'):
                raise RuntimeError('could not select ReLoq C')
            synth = synthDriverHandler.getSynth()
            updater = synth.dictionary_manager()
            revision = updater._revision
            details = {'dictionary_update': {'compared': True, 'added': 3, 'total': 12,
                                             'added_files': 1, 'total_files': 4},
                       'remote_changed': True}
            expected = dictionaries.dictionary_update_message(details)
            self.ui, self.gui, self.expected = ui, gui, expected
            self.messages, self.dialogs = [], []
            self.old_message, self.old_dialog = ui.message, gui.messageBox
            ui.message = self.messages.append
            gui.messageBox = lambda message, *args, **kwargs: self.dialogs.append(message)

            class Panel:
                begin = dictionaries.EloquenceSettingsPanel.begin
                def __init__(self):
                    self.pending = False; self.operation = 0; self.closing = False
                    self.status = Status()
                def enable_controls(self, enabled): pass
                def current_synth(self): return True

            self.manual, self.automatic = Panel(), Panel()
            self.manual.begin(show_result=True)(None, details)
            self.automatic.closing = True
            self.automatic.begin()(None, details)
            synth._dictionary_update_notice(revision, details)
            wx.CallAfter(self.check)
        except BaseException:
            self.fail()

    def check(self):
        try:
            if self.dialogs != [self.expected]:
                raise AssertionError(f'manual dialog or automatic popup mismatch: {self.dialogs!r}')
            if self.messages != [self.expected, self.expected]:
                raise AssertionError(f'automatic messages mismatch: {self.messages!r}')
            self.restore()
            RESULT.write_text(json.dumps({'ok': True, 'manual_dialogs': 1,
                                         'automatic_announcements': 2}), encoding='utf8')
        except BaseException:
            self.fail()
