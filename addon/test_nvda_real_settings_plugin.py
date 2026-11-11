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
            check(clock.GetLabel() and not clock.GetValue(), 'clock checkbox missing or not off by default')
            clock.SetValue(True);event(clock,wx.wxEVT_CHECKBOX)
            check(synth.originalAcousticClock, 'clock preview failed')
            voice_panel.onSave()
            section=config.conf['speech'][synth.name]
            check(section.get('originalAcousticClock') is True, 'clock setting did not save')
            clock.SetValue(False);event(clock,wx.wxEVT_CHECKBOX)
            check(not synth.originalAcousticClock, 'clock live switch failed')
            voice_panel.onDiscard()
            check(synth.originalAcousticClock, 'Cancel did not restore clock setting')
            synth.originalAcousticClock=False
            synth.saveSettings()
            voice_panel.Destroy()
            panel=EloquenceSettingsPanel(frame)
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
