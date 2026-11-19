"""Keyboard-first native phoneme editor for the separate NVDA add-on."""
from __future__ import annotations

import json
from pathlib import Path
import threading

import config
import globalVars
import gui
from logHandler import log
import nvwave
import ui
import wx

from . import lab, phoneme_engine as engine, phonemes

RATES=(3,2,4,5,6,1,0)
RATE_NAMES=('16000 Hz','22050 Hz','32000 Hz','44100 Hz','48000 Hz','11025 Hz','8000 Hz')
FORMAT='reloq-phoneme-project-1'


class PhonemeEditorDialog(wx.Dialog):
    def __init__(self,parent):
        super().__init__(parent,title='Eloquence-C-Lauteditor',style=wx.DEFAULT_DIALOG_STYLE|wx.RESIZE_BORDER)
        self._closed=False;self._busy=False;self._loaded=None;self._changing=False
        self._edits={};self.selected_formant=0
        self._identity=('Persisch','i')
        self.frequencies=[0]*5;self.gains_db=[0]*5
        root=wx.BoxSizer(wx.VERTICAL)
        instructions=wx.StaticText(self,label=('Sprache und Laut auswählen und laden. Im Formantfeld wählen '
            '1 bis 5 den Formanten; links und rechts ändern die Frequenz, hoch und runter den Pegel. '
            'F2 spielt die Änderung, F3 das Original.'))
        root.Add(instructions,0,wx.ALL|wx.EXPAND,10)
        grid=wx.FlexGridSizer(cols=2,vgap=6,hgap=12);grid.AddGrowableCol(1,1)
        def add(label, factory):
            caption=wx.StaticText(self,label=label)
            control=factory();control.SetName(label)
            grid.Add(caption,0,wx.ALIGN_CENTER_VERTICAL|wx.LEFT,8)
            grid.Add(control,1,wx.EXPAND|wx.RIGHT,8)
            return control
        self.language=add('Sprache',lambda:wx.Choice(self,choices=list(phonemes.PHONES)))
        self.language.SetStringSelection('Persisch')
        self.phone=add('Laut',lambda:wx.Choice(self,choices=[]))
        self.gender=add('Stimme',lambda:wx.Choice(self,choices=['Männlich','Weiblich']))
        self.gender.SetSelection(0)
        self.rate=add('Abtastrate',lambda:wx.Choice(self,choices=RATE_NAMES));self.rate.SetSelection(0)
        self.head=add('Kopfgröße',lambda:wx.SpinCtrl(self,min=0,max=100,initial=50))
        self.speed=add('Geschwindigkeit',lambda:wx.SpinCtrl(self,min=0,max=100,initial=65))
        self.pitch=add('Tonhöhe',lambda:wx.SpinCtrl(self,min=0,max=100,initial=65))
        self.clock=add('Lauttakt',lambda:wx.Choice(self,choices=['Fein, 1 ms','Original, 5 ms']))
        self.clock.SetSelection(0)
        self.context=add('Wiedergabe',lambda:wx.Choice(self,choices=['Nur der ausgewählte Laut','Laut mit Vokalkontext']))
        self.context.SetSelection(0)
        root.Add(grid,0,wx.ALL|wx.EXPAND,8)
        area_label=wx.StaticText(self,label='Formantbearbeitung: Ziffern 1 bis 5, Pfeiltasten und F2')
        self.area=wx.TextCtrl(self,style=wx.TE_MULTILINE|wx.TE_READONLY|wx.WANTS_CHARS,size=(-1,165))
        self.area.SetName('Formantbearbeitung: Ziffern 1 bis 5, Pfeiltasten und F2')
        root.Add(area_label,0,wx.LEFT|wx.RIGHT,12);root.Add(self.area,1,wx.ALL|wx.EXPAND,8)
        note=wx.StaticText(self,label='Pegeländerungen sind zusätzliche Resonanzfilter für die Hörprobe.')
        root.Add(note,0,wx.LEFT|wx.RIGHT|wx.BOTTOM,12)
        bar=wx.BoxSizer(wx.HORIZONTAL);self.buttons=[]
        for label,handler in (('Laut laden',self.on_prepare),('Änderung hören (F2)',self.on_play),
             ('Original hören (F3)',self.on_original),('Speichern...',self.on_save),('Projekt laden...',self.on_load),
             ('Laut zurücksetzen',self.on_reset),('Textversuch...',self.on_text),('Schließen',self.on_close)):
            button=wx.Button(self,label=label);button.Bind(wx.EVT_BUTTON,handler)
            bar.Add(button,0,wx.ALL,3);self.buttons.append(button)
        root.Add(bar,0,wx.ALL|wx.ALIGN_CENTER,5)
        self.SetSizerAndFit(root);self.SetMinSize((850,620))
        self.SetSize((max(850,self.GetSize().width),max(620,self.GetSize().height)));self.CentreOnScreen()
        self._set_phone_choices('i');self._refresh()
        self.language.Bind(wx.EVT_CHOICE,self.on_language)
        self.phone.Bind(wx.EVT_CHOICE,self.on_phone)
        for choice in (self.gender,self.rate,self.clock):choice.Bind(wx.EVT_CHOICE,self.on_voice)
        for spin in (self.head,self.speed,self.pitch):
            spin.Bind(wx.EVT_SPINCTRL,self.on_voice)
            spin.Bind(wx.EVT_TEXT,self.on_voice)
        self.Bind(wx.EVT_CHAR_HOOK,self.on_key)
        self.Bind(wx.EVT_CLOSE,self.on_close)
        self.phone.SetFocus()

    def _set_phone_choices(self, selected=None):
        options=phonemes.PHONES[self.language.GetStringSelection()]
        self.phone.Set([item.label for item in options])
        selection=next((i for i,p in enumerate(options) if p.key==selected),0)
        self.phone.SetSelection(selection)
        self._identity=(self.language.GetStringSelection(),options[selection].key)

    def _remember(self):
        self._edits[self._identity]=(list(self.frequencies),list(self.gains_db))

    def _select_edit(self):
        f,g=self._edits.get(self._identity,([0]*5,[0]*5))
        self.frequencies,self.gains_db=list(f),list(g)
        self._loaded=None;self.selected_formant=0;self._refresh()

    def values(self):
        return engine.validate({'language':self._identity[0],'phone':self._identity[1],
            'gender':self.gender.GetSelection(),'rate':RATES[self.rate.GetSelection()],
            'head':self.head.GetValue(),'speed':self.speed.GetValue(),'pitch':self.pitch.GetValue(),
            'clock':self.clock.GetSelection(),'context':self.context.GetSelection()==1,
            'frequencies':list(self.frequencies),'gains_db':list(self.gains_db)})

    def _compatible(self,p,loaded):
        return loaded is not None and (p['language'],p['phone'],engine._parameters(p)) == (
            loaded.language,loaded.key,loaded.params)

    def _name(self):
        item=phonemes.find(*self._identity)
        return item.label if item else 'Kein Laut'

    def _status(self,index):
        index=max(0,min(4,index));delta=self.frequencies[index];gain=self.gains_db[index]
        if self._loaded is None:
            value='Frequenz nach dem Laden verfügbar'
        else:
            frequency=self._loaded.frequencies[index]
            limit=lab.SAMPLE_RATES[RATES[self.rate.GetSelection()]]*.475
            if frequency<=0:
                value='Keine messbare Resonanz in diesem Laut'
            else:
                value=f'Etwa {round(max(40,min(limit,frequency+delta)))} Hertz, Änderung {delta:+d} Hertz'
        return f'Formant {index+1}, {value}, Pegeländerung {gain:+d} Dezibel'

    def _refresh(self):
        label=self._name()
        if self._loaded:
            head=f'{label}, ausgewählter Abschnitt {self._loaded.start_ms:.0f} bis {self._loaded.end_ms:.0f} Millisekunden.'
        else:head=f'{label}. F2 lädt den Laut und spielt ihn ab.'
        entries=[self._status(i) for i in range(5)]
        self.area.SetValue(head+'\n'+'\n'.join(entries)+'\nPegeländerungen wirken als Resonanzfilter nur in der Hörprobe.')
        self.area.SetInsertionPoint(0)

    def on_language(self,event):
        if self._changing:return
        self._remember();self._set_phone_choices();self._select_edit()
        event.Skip()

    def on_phone(self,event):
        if self._changing:return
        self._remember();options=phonemes.PHONES[self.language.GetStringSelection()]
        selection=self.phone.GetSelection()
        if selection>=0:self._identity=(self.language.GetStringSelection(),options[selection].key)
        self._select_edit();event.Skip()

    def on_voice(self,event):
        if not self._changing:
            self._loaded=None;self._refresh()
        event.Skip()

    def on_key(self,event):
        code=event.GetKeyCode()
        if code==wx.WXK_F2:
            self._start('play',True);return
        if code==wx.WXK_F3:
            self._start('play',False);return
        if event.ControlDown() or event.AltDown():
            event.Skip();return
        focus=wx.Window.FindFocus()
        if focus != self.area:
            event.Skip();return
        if ord('1')<=code<=ord('5'):
            self.selected_formant=code-ord('1');ui.message(self._status(self.selected_formant));return
        numpad=getattr(wx,'WXK_NUMPAD1',None)
        if numpad is not None and numpad<=code<=getattr(wx,'WXK_NUMPAD5',numpad+4):
            self.selected_formant=code-numpad;ui.message(self._status(self.selected_formant));return
        if code not in (wx.WXK_LEFT,wx.WXK_RIGHT,wx.WXK_UP,wx.WXK_DOWN):
            event.Skip();return
        if self._busy:
            ui.message('Bitte warten, die Hörprobe wird vorbereitet.');return
        i=self.selected_formant
        if code in (wx.WXK_LEFT,wx.WXK_RIGHT):
            step=5 if event.ShiftDown() else 25
            delta=step if code==wx.WXK_RIGHT else -step
            self.frequencies[i]=max(-2000,min(2000,self.frequencies[i]+delta))
        else:
            delta=1 if code==wx.WXK_UP else -1
            self.gains_db[i]=max(-18,min(18,self.gains_db[i]+delta))
        self._remember();self._refresh();ui.message(self._status(i))

    def _start(self,kind,modified=True):
        if self._busy:
            ui.message('Bitte warten, die Hörprobe wird vorbereitet.');return
        try:
            p=self.values()
            from synthDrivers.reloqC import DLL
            if not DLL.is_file():raise FileNotFoundError('Eloquence C ist nicht installiert.')
            device=config.conf['audio']['outputDevice']
        except Exception as exc:
            self.error(exc);return
        loaded=self._loaded if self._compatible(p,self._loaded) else None
        self._busy=True
        if loaded is None:ui.message('Laut wird geladen.')
        def work():
            try:
                result=loaded or engine.load(DLL,p)
                normalized=False
                if kind=='play':
                    pcm,normalized=engine.render(DLL,p,result,modified=modified)
                    if not self._closed:
                        player=nvwave.WavePlayer(channels=1,samplesPerSec=lab.SAMPLE_RATES[p['rate']],
                                                 bitsPerSample=16,outputDevice=device)
                        try:player.feed(pcm);player.idle()
                        finally:player.close()
                wx.CallAfter(self._completed,kind,p,result,normalized,None)
            except Exception as exc:
                log.exception('Eloquence phoneme editor failed')
                wx.CallAfter(self._completed,kind,p,None,False,str(exc))
        threading.Thread(target=work,daemon=True,name='ReloqPhonemeEditor').start()

    def _completed(self,kind,p,result,normalized,error):
        if self._closed:return
        self._busy=False
        if error:self.error(error);return
        if self._compatible(self.values(),result):
            self._loaded=result;self._refresh()
            if kind=='load':
                self.area.SetFocus();ui.message(self._name()+' geladen. '+self._status(self.selected_formant))
        if normalized:ui.message('Die Hörprobe wurde gegen Übersteuerung abgesenkt.')

    def error(self,message):
        gui.messageBox(str(message),'Eloquence-C-Lauteditor',wx.OK|wx.ICON_ERROR,parent=self)

    def on_prepare(self,event):self._start('load')
    def on_play(self,event):self._start('play',True)
    def on_original(self,event):self._start('play',False)

    def on_reset(self,event):
        self.frequencies=[0]*5;self.gains_db=[0]*5;self._remember();self._refresh()
        ui.message('Änderungen für diesen Laut zurückgesetzt.')

    def project(self):
        self._remember()
        entries=[]
        for (language,key),(freq,gain) in sorted(self._edits.items()):
            if any(freq) or any(gain):
                entries.append({'language':language,'phone':key,'frequencies':list(freq),'gains_db':list(gain)})
        return {'format':FORMAT,'settings':self.values(),'edits':entries}

    def restore_project(self,project):
        if not isinstance(project,dict) or project.get('format')!=FORMAT or not isinstance(project.get('edits'),list):
            raise ValueError('Diese Datei ist kein Lauteditor-Projekt.')
        p=engine.validate(project.get('settings'))
        if len(project['edits'])>200:raise ValueError('Das Projekt enthält zu viele Laute.')
        edits={}
        for entry in project['edits']:
            if not isinstance(entry,dict):raise ValueError('Ungültiger Lauteintrag.')
            check=engine.defaults();check.update(entry);engine.validate(check)
            identity=(check['language'],check['phone'])
            if identity in edits:raise ValueError('Ein Laut ist mehrfach gespeichert.')
            edits[identity]=(list(check['frequencies']),list(check['gains_db']))
        self._changing=True
        try:
            self.language.SetStringSelection(p['language']);self._set_phone_choices(p['phone'])
            self.gender.SetSelection(p['gender']);self.rate.SetSelection(RATES.index(p['rate']))
            self.head.SetValue(p['head']);self.speed.SetValue(p['speed']);self.pitch.SetValue(p['pitch'])
            self.clock.SetSelection(p['clock']);self.context.SetSelection(int(p['context']))
            self._edits=edits
            self.frequencies=list(p['frequencies']);self.gains_db=list(p['gains_db'])
            self._remember();self._loaded=None;self.selected_formant=0;self._refresh()
        finally:self._changing=False

    def on_save(self,event):
        try:
            data=self.project()
            with wx.FileDialog(self,'Lautversuche speichern',wildcard='Lauteditor-Projekt (*.json)|*.json',
                               defaultFile='eloquence-laute.json',style=wx.FD_SAVE|wx.FD_OVERWRITE_PROMPT) as dlg:
                if dlg.ShowModal()!=wx.ID_OK:return
                Path(dlg.GetPath()).write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf8')
            ui.message('Lautversuche gespeichert.')
        except Exception as exc:self.error(exc)

    def on_load(self,event):
        try:
            with wx.FileDialog(self,'Lautversuche laden',wildcard='Lauteditor-Projekt (*.json)|*.json',
                               style=wx.FD_OPEN|wx.FD_FILE_MUST_EXIST) as dlg:
                if dlg.ShowModal()!=wx.ID_OK:return
                path=Path(dlg.GetPath())
                if path.stat().st_size>65536:raise ValueError('Die Datei ist zu groß.')
                data=json.loads(path.read_text(encoding='utf8'))
            self.restore_project(data);ui.message('Lautversuche geladen.')
        except Exception as exc:self.error(exc)

    def on_text(self,event):
        from . import SoundLabDialog
        dialog=SoundLabDialog(self)
        try:dialog.ShowModal()
        finally:dialog.Destroy()

    def on_close(self,event):
        self._closed=True;self.EndModal(wx.ID_CANCEL)
