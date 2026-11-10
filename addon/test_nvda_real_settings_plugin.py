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
    widget.GetEventHandler().ProcessEvent(evt)


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
            from globalPlugins.reloqCDictionaries import EloquenceSettingsPanel

            nvwave.WavePlayer=lambda *a,**k: Player()
            reloq.SynthDriver._make_player=staticmethod(lambda sample_rate: Player())
            speech.cancelSpeech()
            assert synthDriverHandler.setSynth('reloqC')
            synth=synthDriverHandler.getSynth()
            # The panel's save also initiates an unrelated dictionary refresh;
            # isolate the settings lifecycle without changing saved entries.
            synth.configure_dictionaries=lambda sources, enabled, cb, repository: cb(None,{})
            frame=wx.Frame(None)
            panel=EloquenceSettingsPanel(frame)
            section=config.conf['speech'][synth.name]
            before=(section['_pitchHertz'],section['_rateWpm'],section['_eqEnabled'])
            assert not panel.pitch_hz_value.IsEnabled() and not panel.wpm_value.IsEnabled()
            assert not panel.eq_preset.IsEnabled()
            assert panel.pitch_hz_value.GetName() and all(x.GetName() for x in panel.eq_bands)

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
            assert (synth._pitchInHertz,synth._pitchHertz)==(True,151)
            assert (synth._rateInWpm,synth._rateWpm)==(True,50)
            assert synth._eqEnabled and synth._eqPreset==1
            assert tuple(x.GetValue() for x in panel.eq_bands)==(4,0,-1)
            assert (section['_pitchHertz'],section['_rateWpm'],section['_eqEnabled'])==before

            panel.onSave()
            assert (section['_pitchHertz'],section['_rateWpm'],section['_eqEnabled'])==(151,50,True)
            panel.pitch_hz_value.SetValue(220)
            event(panel.pitch_hz_value,wx.wxEVT_SPINCTRL)
            panel.wpm_value.SetValue(90)
            event(panel.wpm_value,wx.wxEVT_SPINCTRL)
            panel.eq_bands[1].SetValue(3)
            event(panel.eq_bands[1],wx.wxEVT_SPINCTRL)
            assert (synth._pitchHertz,synth._rateWpm,synth._eqPreset)==(220,90,4)
            panel.onDiscard()
            assert (synth._pitchHertz,synth._rateWpm,synth._eqPreset)==(151,50,1)
            RESULT.write_text(json.dumps({'ok':True,'details':'real NVDA wx controls, live preview, Apply and Cancel'}),encoding='utf8')
        except BaseException:
            RESULT.write_text(json.dumps({'ok':False,'error':traceback.format_exc()}),encoding='utf8')
        finally:
            if frame:
                frame.Destroy()
