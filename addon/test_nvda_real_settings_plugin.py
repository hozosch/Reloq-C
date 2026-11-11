"""Temporary NVDA global plugin for testing the real wx settings panel."""
import json
import os
from pathlib import Path
import traceback

import globalPluginHandler
import wx

RESULT = Path(os.environ['RELOQ_SETTINGS_PROBE_RESULT'])


class Player:
    def feed(self, data, size=None, onDone=None):
        if callable(size) and onDone is None:
            onDone=size
        if onDone:
            onDone()
    def idle(self): pass
    def sync(self): pass
    def stop(self): pass
    def pause(self, switch): pass
    def close(self): pass


def event(widget, event_type):
    evt=wx.CommandEvent(event_type, widget.GetId())
    evt.SetEventObject(widget)
    if event_type==wx.wxEVT_CHECKBOX:
        evt.SetInt(int(widget.GetValue()))
    elif event_type==wx.wxEVT_CHOICE:
        evt.SetInt(widget.GetSelection())
    widget.GetEventHandler().ProcessEvent(evt)


def check(condition, message):
    # NVDA's packaged Python runs with assertions disabled. A real probe must
    # therefore never use assert, especially for a call with side effects.
    if not condition:
        raise AssertionError(message)


class GlobalPlugin(globalPluginHandler.GlobalPlugin):
    def __init__(self):
        super().__init__()
        wx.CallLater(1500, self.run_probe)

    def run_probe(self):
        frame=None
        try:
            import config
            import nvwave
            import speech
            import synthDriverHandler
            import synthDrivers.reloqC as reloq
            from globalPlugins.reloqCDictionaries import EloquenceSettingsPanel, EqualizerFiltersDialog

            nvwave.WavePlayer=lambda *a,**k: Player()
            reloq.SynthDriver._make_player=staticmethod(lambda sample_rate: Player())
            speech.cancelSpeech()
            selected=synthDriverHandler.setSynth('reloqC')
            check(selected, 'could not select ReLoq C')
            synth=synthDriverHandler.getSynth()
            check(synth is not None and synth.name=='reloqC', 'the selected synth is not ReLoq C')
            # The panel's save also initiates an unrelated dictionary refresh;
            # isolate the settings lifecycle without changing saved entries.
            synth.configure_dictionaries=lambda sources, enabled, cb, repository: cb(None,{})
            frame=wx.Frame(None)
            from gui.settingsDialogs import VoiceSettingsPanel
            voice_panel=VoiceSettingsPanel(frame)
            clock=voice_panel.originalAcousticClockCheckbox
            controls=list(voice_panel.sizerDict)
            check(controls.index('originalAcousticClock')==controls.index('rateBoost')+1,
                  'clock checkbox is not immediately after rate boost')
            languages=voice_panel._languages
            initial_language=synth.language
            greek=next(i for i,item in enumerate(languages) if item.id=='el_GR')
            voice_panel.languageList.SetSelection(greek);event(voice_panel.languageList,wx.wxEVT_CHOICE)
            wx.Yield()
            check(synth.language=='el_GR' and synth.originalAcousticClock and clock.GetValue(),
                  'Greek default or checkbox refresh failed')
            clock.SetValue(False);event(clock,wx.wxEVT_CHECKBOX)
            check(not synth.originalAcousticClock, 'clock preview failed')
            voice_panel.onSave()
            section=config.conf['speech'][synth.name]
            saved_clocks=section.get('_acousticClockMask')
            clock.SetValue(True);event(clock,wx.wxEVT_CHECKBOX)
            check(synth.originalAcousticClock, 'clock live switch failed')
            check(section.get('_acousticClockMask')==saved_clocks, 'preview was saved before Apply')
            voice_panel.onDiscard()
            check(not synth.originalAcousticClock, 'Cancel did not restore clock setting')
            voice_panel.Destroy()
            wx.Yield()
            import globalVars
            ring=globalVars.settingsRing
            ring_clock=next(item for item in ring.settings if item.setting.id=='originalAcousticClock')
            ring_language=next(item for item in ring.settings if item.setting.id=='language')
            ring_clock.value=1
            check(synth.originalAcousticClock and section['_acousticClockMask']==synth._acousticClockMask,
                  'ring choice was not persisted per language')
            home=next(i for i,item in enumerate(ring_language._values) if item.id==initial_language)
            greek=next(i for i,item in enumerate(ring_language._values) if item.id=='el_GR')
            ring_language.value=home
            ring_language.value=greek
            synth.loadSettings()
            check(synth.language=='el_GR' and synth.originalAcousticClock, 'ring choice did not survive language switches and reload')
            ring_language.value=home
            synth.saveSettings()
            panel=EloquenceSettingsPanel(frame)
            # The combined list accepts each format through one accessible
            # control and chooses the correct parser before asking scope.
            import gui
            native = RESULT.parent / 'native-ui-test.dic'
            imported = RESULT.parent / 'nvda-ui-test.dic'
            ambiguous = RESULT.parent / 'ambiguous-native-ui-test.dic'
            native.write_text('nativeprobe\tNativ\n', encoding='utf8')
            imported.write_text('nvdaprobe\tErsatz 2035\t0\t2\n', encoding='utf-8-sig')
            ambiguous.write_text('first\tvalue\t0\t2\nsecond\tother\n', encoding='utf8')
            selected_file = [imported]
            choose_ambiguous = [False]
            choices_seen = []
            class FakeFile:
                def __init__(self,*args,**kwargs): pass
                def __enter__(self): return self
                def __exit__(self,*args): pass
                def ShowModal(self): return wx.ID_OK
                def GetPath(self): return str(selected_file[0])
            class FakeChoice:
                def __init__(self,*args,**kwargs): self.value=0;self.title=args[2];self.choices=args[3]
                def __enter__(self): return self
                def __exit__(self,*args): pass
                def ShowModal(self):
                    choices_seen.append((self.title,self.value,self.choices))
                    if (selected_file[0] == ambiguous and self.title == 'Dictionary format'
                            and choose_ambiguous[0]): self.value = 2
                    return wx.ID_OK
                def SetSelection(self,value): self.value=value
                def GetSelection(self): return self.value
            old_file,old_choice,old_message = wx.FileDialog,wx.SingleChoiceDialog,gui.messageBox
            with old_choice(panel, 'Select a format', 'Dictionary format',
                            ['Choose a format…', 'NVDA', 'Eloquence']) as unselected:
                unselected.SetSelection(0)
                check(unselected.GetSelection()==0, 'wx did not select the format prompt')
            try:
                wx.FileDialog,wx.SingleChoiceDialog,gui.messageBox = FakeFile,FakeChoice,lambda *args,**kwargs:wx.OK
                panel.on_add(None)
                check(choices_seen and choices_seen[0][1]==0, 'NVDA format was not recognized')
                selected_file[0]=native;choices_seen.clear();panel.on_add(None)
                check(choices_seen and choices_seen[0][1]==1, 'native format was not recognized')
                selected_file[0]=ambiguous;choices_seen.clear();panel.on_add(None)
                check(choices_seen and choices_seen[0][1]==0
                      and choices_seen[0][2][0]=='Choose a format…',
                      'ambiguous format was selected without a user choice')
                check(len(panel.sources)==1, 'ambiguous file was added without a format choice')
                choose_ambiguous[0]=True;panel.on_add(None)
            finally:
                wx.FileDialog,wx.SingleChoiceDialog,gui.messageBox = old_file,old_choice,old_message
            check(panel.files.GetName() and panel.files.GetCount()==3, 'combined dictionary list is not accessible')
            check('Eloquence' in panel.files.GetString(0) and 'Eloquence' in panel.files.GetString(1)
                  and 'NVDA' in panel.files.GetString(2),
                  'combined rows do not identify their formats')
            check(synth.imported_dictionary_manager().process('nvdaprobe', synth._selectedLanguage, synth._voice)=='Ersatz 2035',
                  'NVDA options leaked or literal replacement digits were removed')
            panel.files.SetSelection(0);panel.on_dictionary_selection(wx.CommandEvent())
            check(not panel.edit_dictionary.IsEnabled() and panel.remove_dictionary.IsEnabled(),
                  'native selection offers an invalid edit action')
            panel.files.SetSelection(2);panel.on_dictionary_selection(wx.CommandEvent())
            check(panel.edit_dictionary.IsEnabled() and not panel.move_up.IsEnabled(),
                  'NVDA selection has incorrect actions')
            section=config.conf['speech'][synth.name]
            # A fresh NVDA profile may have no explicitly stored entry yet.
            # Compare optional values until Apply creates the settings.
            before=tuple(section.get(key) for key in ('_pitchHertz','_rateWpm','_eqEnabled'))
            check(not panel.pitch_hz_value.IsEnabled() and not panel.wpm_value.IsEnabled(), 'initial pitch or rate control enabled')
            check(not panel.eq_preset.IsEnabled(), 'initial equalizer control enabled')
            check(panel.pitch_hz_value.GetName() and all(x.GetName() for x in panel.eq_bands), 'accessible control names missing')
            check(panel.auto_update.GetValue(), 'automatic update checks should be enabled by default')
            panel.auto_update.SetValue(False)

            panel.pitch_hz_mode.SetValue(True)
            event(panel.pitch_hz_mode,wx.wxEVT_CHECKBOX)
            panel.pitch_hz_value.SetValue(151)
            event(panel.pitch_hz_value,wx.wxEVT_SPINCTRL)
            panel.wpm_mode.SetValue(True)
            event(panel.wpm_mode,wx.wxEVT_CHECKBOX)
            panel.wpm_value.SetValue(50)
            event(panel.wpm_value,wx.wxEVT_SPINCTRL)
            panel.eq_mode.SetValue(True)
            event(panel.eq_mode,wx.wxEVT_CHECKBOX)
            panel.eq_preset.SetSelection(1)
            event(panel.eq_preset,wx.wxEVT_CHOICE)
            check((synth._pitchInHertz,synth._pitchHertz)==(True,151), 'pitch preview failed')
            check((synth._rateInWpm,synth._rateWpm)==(True,50), 'rate preview failed')
            check(synth._eqEnabled and synth._eqPreset==1, 'equalizer preset preview failed')
            check(tuple(x.GetValue() for x in panel.eq_bands)==(4,0,-1), 'equalizer preset controls incorrect')
            check(tuple(section.get(key) for key in ('_pitchHertz','_rateWpm','_eqEnabled'))==before,
                  'preview changed saved settings')

            dialog=EqualizerFiltersDialog(panel,synth)
            failures=[]
            def edit_filter():
                try:
                    check(dialog.selector.GetName() and all(c.GetName() for c in dialog.inputs),
                          'accessible filter control names missing')
                    dialog.enabled.SetValue(True);event(dialog.enabled,wx.wxEVT_CHECKBOX)
                    for control,value in zip(dialog.inputs,(777,6,50)):
                        control.SetValue(value);event(control,wx.wxEVT_SPINCTRL)
                    check(synth.equalizer_filters()[0]==(True,777,6,50), 'advanced filter preview failed')
                    dialog.on_ok(None)
                except BaseException:
                    failures.append(traceback.format_exc());dialog.EndModal(wx.ID_CANCEL)
            wx.CallAfter(edit_filter)
            result=dialog.ShowModal();dialog.Destroy()
            check(not failures, 'advanced filter dialog failed: '+''.join(failures))
            check(result==wx.ID_OK, 'advanced filter dialog did not accept')
            panel.eq_dirty=True

            panel.onSave()
            check(not config.conf['reloqCUpdates']['automatic'], 'automatic update setting did not save')
            stored=tuple(section.get(key) for key in ('_pitchHertz','_rateWpm','_eqEnabled'))
            check(stored==(151,50,True), f'saved values: {stored!r}; keys: {list(section)!r}')
            check(section.get('_eqFilter1Frequency')==777 and section.get('_eqFilter1Enabled') is True,
                  'advanced filter settings were not saved')
            panel.pitch_hz_value.SetValue(220)
            event(panel.pitch_hz_value,wx.wxEVT_SPINCTRL)
            panel.wpm_value.SetValue(90)
            event(panel.wpm_value,wx.wxEVT_SPINCTRL)
            panel.eq_bands[1].SetValue(3)
            event(panel.eq_bands[1],wx.wxEVT_SPINCTRL)
            modified=list(synth.equalizer_filters());modified[0]=(True,900,2,100)
            synth.configure_equalizer_filters(modified)
            check((synth._pitchHertz,synth._rateWpm,synth._eqPreset)==(220,90,4), 'custom preview failed')
            panel.onDiscard()
            check((synth._pitchHertz,synth._rateWpm,synth._eqPreset)==(151,50,1), 'Cancel did not restore Apply values')
            check(synth.equalizer_filters()[0]==(True,777,6,50), 'Cancel did not restore saved filters')
            synth.configure_pitch_hertz(False,111)
            synth.configure_rate_wpm(False,100)
            synth.configure_equalizer(False,0,0,0,0)
            synth.configure_equalizer_filters([(False,125,0,100)]*5)
            synth.loadSettings()
            check((synth._pitchInHertz,synth._pitchHertz,synth._rateInWpm,synth._rateWpm,
                   synth._eqEnabled,synth._eqPreset)==(True,151,True,50,True,1), 'saved settings did not reload')
            check(synth.equalizer_filters()[0]==(True,777,6,50), 'saved filter did not reload')
            RESULT.write_text(json.dumps({'ok':True,'details':'real NVDA wx controls, acoustic clock, advanced filters, live preview, Apply and Cancel'}),encoding='utf8')
        except BaseException:
            RESULT.write_text(json.dumps({'ok':False,'error':traceback.format_exc()}),encoding='utf8')
        finally:
            if frame:
                frame.Destroy()
