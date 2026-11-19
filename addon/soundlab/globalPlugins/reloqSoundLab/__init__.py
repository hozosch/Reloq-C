"""Keyboard-accessible listening lab, independent of NVDA's active synth."""
from __future__ import annotations

import json
from pathlib import Path
import threading

import config
import globalPluginHandler
import globalVars
import gui
from logHandler import log
import nvwave
from scriptHandler import script
import ui
import wx

from . import lab

RATE_CODES = (3, 2, 4, 5, 6, 1, 0)
RATE_LABELS = ('16000 Hz', '22050 Hz', '32000 Hz', '44100 Hz', '48000 Hz', '11025 Hz', '8000 Hz')


class SoundLabDialog(wx.Dialog):
    def __init__(self, parent):
        super().__init__(parent, title='Eloquence-C-Lautlabor', style=wx.DEFAULT_DIALOG_STYLE | wx.RESIZE_BORDER)
        self._closed = False
        self._busy = False
        root = wx.BoxSizer(wx.VERTICAL)
        note = wx.StaticText(self, label=('Hörversuch: Die Änderungen gelten nur für diese Probe. '
                             'Ein einzelnes Wort eignet sich am besten.'))
        root.Add(note, 0, wx.ALL | wx.EXPAND, 10)
        book = wx.Notebook(self)
        self.pages = {}
        for key, title in (('base','Grundlagen'),('frequencies','Resonanzen'),
                           ('bandwidths','Bandbreiten'),('noise_widths','Rauschbandbreiten')):
            page = wx.Panel(book)
            book.AddPage(page, title)
            layout = wx.FlexGridSizer(cols=2, vgap=8, hgap=12)
            layout.AddGrowableCol(1, 1)
            page.SetSizer(layout)
            self.pages[key] = (page, layout)
        root.Add(book, 1, wx.ALL | wx.EXPAND, 8)
        def add(page_key, label, factory):
            page, layout = self.pages[page_key]
            # Windows derives an accessible name from the immediately
            # preceding label window. Create the label before its control;
            # adding an already-created control to a sizer is too late.
            caption = wx.StaticText(page, label=label)
            control = factory(page)
            layout.Add(caption, 0, wx.ALIGN_CENTER_VERTICAL | wx.LEFT, 8)
            control.SetName(label)
            layout.Add(control, 1, wx.EXPAND | wx.RIGHT, 8)
            return control
        self.text = add('base', 'Testtext', lambda page: wx.TextCtrl(page, value='ایران'))
        self.language = add('base', 'Sprache', lambda page: wx.Choice(page, choices=list(lab.LANGUAGES)))
        self.language.SetSelection(0)
        self.gender = add('base', 'Stimme', lambda page: wx.Choice(page, choices=['Männlich', 'Weiblich']))
        self.gender.SetSelection(0)
        self.rate = add('base', 'Abtastrate', lambda page: wx.Choice(page, choices=RATE_LABELS))
        self.rate.SetSelection(0)
        def spin(key, label, low, high, value):
            return add(key, label, lambda page: wx.SpinCtrl(page, min=low, max=high, initial=value))
        self.head = spin('base', 'Kopfgröße', 0, 100, 50)
        self.speed = spin('base', 'Geschwindigkeit', 0, 100, 65)
        self.pitch = spin('base', 'Tonhöhe', 0, 100, 65)
        self.start = spin('base', 'Beginn des Abschnitts in Millisekunden', 0, 60000, 0)
        self.end = spin('base', 'Ende in Millisekunden; 0 bedeutet bis zum Ende', 0, 60000, 0)
        self.voiced = add('base', 'Abschnitte', lambda page: wx.Choice(page, choices=['Stimmhafte', 'Alle']))
        self.voiced.SetSelection(0)
        self.controls = {}
        for key, prefix in (('frequencies', 'Frequenz F'), ('bandwidths', 'Bandbreite B'),
                            ('noise_widths', 'Rauschbandbreite B')):
            self.controls[key] = [spin(key, prefix + str(i) + ': Änderung in Hz', -2000, 2000, 0)
                                  for i in range(1, 6)]
        readout_label = wx.StaticText(self, label='Gemessene Resonanzen der letzten Probe')
        self.readout = wx.TextCtrl(self, style=wx.TE_MULTILINE | wx.TE_READONLY, size=(-1, 65))
        self.readout.SetName('Gemessene Resonanzen der letzten Probe')
        root.Add(readout_label, 0, wx.LEFT | wx.RIGHT, 10)
        root.Add(self.readout, 0, wx.ALL | wx.EXPAND, 8)
        buttons = wx.BoxSizer(wx.HORIZONTAL)
        self.buttons = []
        for title, handler in (('Original anhören', self.on_original), ('Änderung anhören', self.on_modified),
                               ('Speichern', self.on_save), ('Laden', self.on_load),
                               ('Zurücksetzen', self.on_reset), ('Schließen', self.on_close)):
            button = wx.Button(self, label=title)
            button.Bind(wx.EVT_BUTTON, handler)
            buttons.Add(button, 0, wx.ALL, 3)
            self.buttons.append(button)
        root.Add(buttons, 0, wx.ALL | wx.ALIGN_CENTER, 5)
        self.SetSizerAndFit(root)
        self.SetMinSize((750, 620))
        self.SetSize((max(750, self.GetSize().width), max(620, self.GetSize().height)))
        self.CentreOnScreen()
        self.Bind(wx.EVT_CLOSE, self.on_close)
        self.text.SetFocus()

    def values(self):
        return lab.validate({'text': self.text.GetValue(), 'language': self.language.GetStringSelection(),
            'gender': self.gender.GetSelection(), 'rate': RATE_CODES[self.rate.GetSelection()],
            'head': self.head.GetValue(), 'speed': self.speed.GetValue(), 'pitch': self.pitch.GetValue(),
            'start': self.start.GetValue(), 'end': self.end.GetValue(), 'voiced': self.voiced.GetSelection() == 0,
            **{key: [item.GetValue() for item in controls] for key, controls in self.controls.items()}})

    def set_values(self, p):
        lab.validate(p)
        self.text.SetValue(p['text'])
        self.language.SetStringSelection(p['language'])
        self.gender.SetSelection(p['gender'])
        self.rate.SetSelection(RATE_CODES.index(p['rate']))
        for key in ('head','speed','pitch','start','end'):
            getattr(self,key).SetValue(p[key])
        self.voiced.SetSelection(0 if p['voiced'] else 1)
        for key, controls in self.controls.items():
            for item, value in zip(controls, p[key]): item.SetValue(value)

    def error(self, text):
        gui.messageBox(str(text), 'Eloquence-C-Lautlabor', wx.OK | wx.ICON_ERROR, parent=self)

    def listen(self, modified):
        if self._busy: return
        try:
            values = self.values()
            from synthDrivers.reloqC import DLL
            if not DLL.is_file(): raise FileNotFoundError('Eloquence C ist nicht installiert.')
            parent = Path(globalVars.appDir) / 'synthDrivers'
            output_device = config.conf['audio']['outputDevice']
        except Exception as exc:
            self.error('Eloquence C konnte nicht vorbereitet werden: ' + str(exc)); return
        self._busy = True
        for button in self.buttons[:-1]: button.Disable()
        ui.message('Hörprobe wird erzeugt.')
        def work():
            try:
                pcm, info, changed = lab.render(DLL, parent / 'espeak.dll', parent, values, modified)
                if self._closed:
                    return
                player = nvwave.WavePlayer(channels=1, samplesPerSec=lab.SAMPLE_RATES[values['rate']],
                                           bitsPerSample=16, outputDevice=output_device)
                try:
                    player.feed(pcm)
                    player.idle()
                finally:
                    player.close()
                wx.CallAfter(self.completed, info, None)
            except Exception as exc:
                log.exception('Eloquence sound lab failed')
                wx.CallAfter(self.completed, (), str(exc))
        threading.Thread(target=work, daemon=True, name='ReloqSoundLab').start()

    def completed(self, info, error):
        if self._closed: return
        self._busy = False
        for button in self.buttons[:-1]: button.Enable()
        if error:
            self.error(error); return
        if info:
            time, freq, width = info[len(info)//2]
            self.readout.SetValue('Bei etwa %s ms: ' % time + ', '.join(
                'F%s %s Hz, B%s %s Hz' % (i, f, i, b)
                for i, (f,b) in enumerate(zip(freq,width),1)))
        else:
            self.readout.SetValue('Keine stimmhafte Resonanz im gewählten Bereich gefunden.')
        ui.message('Hörprobe beendet. Die gemessenen Resonanzen stehen im Ergebnisfeld.')

    def on_original(self, event): self.listen(False)
    def on_modified(self, event): self.listen(True)

    def on_save(self, event):
        try:
            values = self.values()
            with wx.FileDialog(self, 'Hörversuch speichern', wildcard='JSON-Dateien (*.json)|*.json',
                               defaultFile='eloquence-hoerversuch.json', style=wx.FD_SAVE | wx.FD_OVERWRITE_PROMPT) as dlg:
                if dlg.ShowModal() != wx.ID_OK: return
                Path(dlg.GetPath()).write_text(json.dumps(values, ensure_ascii=False, indent=2) + '\n', encoding='utf8')
            ui.message('Hörversuch gespeichert.')
        except Exception as exc: self.error(exc)

    def on_load(self, event):
        try:
            with wx.FileDialog(self, 'Hörversuch laden', wildcard='JSON-Dateien (*.json)|*.json',
                               style=wx.FD_OPEN | wx.FD_FILE_MUST_EXIST) as dlg:
                if dlg.ShowModal() != wx.ID_OK: return
                path = Path(dlg.GetPath())
                if path.stat().st_size > 65536: raise ValueError('Die Datei ist zu groß.')
                values = json.loads(path.read_text(encoding='utf8'))
            self.set_values(values)
            ui.message('Hörversuch geladen.')
        except Exception as exc: self.error(exc)

    def on_reset(self, event):
        for controls in self.controls.values():
            for item in controls: item.SetValue(0)
        self.start.SetValue(0); self.end.SetValue(0)
        ui.message('Resonanzänderungen zurückgesetzt.')

    def on_close(self, event):
        self._closed = True
        self.EndModal(wx.ID_CANCEL)


from .editor import PhonemeEditorDialog


class GlobalPlugin(globalPluginHandler.GlobalPlugin):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.menu = gui.mainFrame.sysTrayIcon.toolsMenu
        self.item = self.menu.Append(wx.ID_ANY, 'Eloquence-C-Lautlabor...')
        gui.mainFrame.sysTrayIcon.Bind(wx.EVT_MENU, self.open, self.item)
        self.dialog = None

    @script(description='Eloquence-C-Lautlabor öffnen', gesture='kb:NVDA+control+shift+f10')
    def script_open(self, gesture):
        wx.CallAfter(self.open, None)

    def open(self, event):
        if self.dialog is not None: return
        gui.mainFrame.prePopup()
        try:
            self.dialog = PhonemeEditorDialog(gui.mainFrame)
            self.dialog.ShowModal()
        finally:
            if self.dialog:
                self.dialog.Destroy(); self.dialog = None
            gui.mainFrame.postPopup()

    def terminate(self):
        if self.dialog:
            self.dialog._closed = True
            self.dialog.EndModal(wx.ID_CANCEL)
        try:
            gui.mainFrame.sysTrayIcon.Unbind(wx.EVT_MENU, source=self.item)
            self.menu.Remove(self.item)
        finally:
            super().terminate()
