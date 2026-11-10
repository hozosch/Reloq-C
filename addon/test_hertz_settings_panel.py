"""Exercise live Hertz/WPM preview and NVDA's Apply/Cancel lifecycle."""
import builtins
import importlib.util
from pathlib import Path
import sys
import types


class Widget:
    def __init__(self,*args,**kwargs):
        self.value=kwargs.get('initial',False);self.label=kwargs.get('label','')
        self.enabled=True;self.name='';self.selection=0;self.items=[]
    def Add(self,*a,**k):pass
    def Bind(self,*a,**k):pass
    def SetValue(self,value):self.value=value
    def GetValue(self):return self.value
    def Enable(self,value):self.enabled=value
    def SetName(self,value):self.name=value
    def GetLabelText(self):return self.label
    def SetLabel(self,value):self.label=value
    def SetSelection(self,value):self.selection=value
    def GetSelection(self):return self.selection
    def Set(self,values):self.items=values
    def Skip(self):pass


def module(name,**values):
    result=types.ModuleType(name);vars(result).update(values);sys.modules[name]=result;return result


def main():
    builtins._=lambda x:x
    module('addonHandler',initTranslation=lambda:None,
           getCodeAddon=lambda:types.SimpleNamespace(manifest={'name':'reloqC','version':'test'}))
    class Configuration(dict):
        spec = {}
    module('config',conf=Configuration(reloqCUpdates={'automatic':True}))
    module('globalPluginHandler',GlobalPlugin=type('GlobalPlugin',(),{}))
    module('gui',__path__=[],messageBox=lambda *a,**k:None)
    class SettingsPanel(Widget):
        def onPanelActivated(self):pass
    module('gui.settingsDialogs',SettingsPanel=SettingsPanel,
           NVDASettingsDialog=type('NVDASettingsDialog',(),{'categoryClasses':[]}))
    module('ui',message=lambda message:None)
    wx=module('wx',Dialog=Widget,StaticText=Widget,CheckBox=Widget,SpinCtrl=Widget,Choice=Widget,
              ListBox=Widget,BoxSizer=Widget,Button=Widget,CallAfter=lambda fn,*a:fn(*a))
    for name in ('ALL','LEFT','RIGHT','BOTTOM','EXPAND','HORIZONTAL','EVT_CHECKBOX',
                 'EVT_SPINCTRL','EVT_TEXT','EVT_BUTTON','EVT_WINDOW_DESTROY','EVT_CHOICE'):
        setattr(wx,name,1)
    langs=module('synthDrivers.reloqC._reloq_languages',LANGUAGES={},LANGUAGE_DISPLAY_ORDER=[])
    module('synthDrivers',__path__=[]);module('synthDrivers.reloqC',__path__=[],
        EQ_PRESETS=((0,0,0),(4,0,-1),(-1,0,4),(0,-1,-4)))
    module('synthDrivers.reloqC._reloq_dictionary_manager',DictionarySource=object)
    class Synth:
        name='reloqC';communityDictionaries=False;_pitchInHertz=False;_pitchHertz=150
        _rateInWpm=False;_rateWpm=150
        _eqEnabled=False;_eqPreset=0;_eqCustomLow=0;_eqCustomMid=0;_eqCustomHigh=0
        filters=tuple((False,f,0,100) for f in (125,500,1200,3000,6000))
        def dictionary_manager(self):return types.SimpleNamespace(sources=[],repository='community')
        def configure_pitch_hertz(self,enabled,value):self._pitchInHertz=enabled;self._pitchHertz=value
        def configure_rate_wpm(self,enabled,value):self._rateInWpm=enabled;self._rateWpm=value
        def configure_equalizer(self,enabled,preset,low,mid,high):
            self._eqEnabled,self._eqPreset=enabled,preset
            self._eqCustomLow,self._eqCustomMid,self._eqCustomHigh=low,mid,high
        def equalizer_filters(self):return self.filters
        def configure_equalizer_filters(self,filters):self.filters=tuple(tuple(row) for row in filters)
        def configure_dictionaries(self,sources,enabled,callback,repository):callback(None,{})
        def saveSettings(self):self.saved=True
    synth=Synth();module('synthDriverHandler',getSynth=lambda:synth)
    path=Path(__file__).parent/'synth/globalPlugins/reloqCDictionaries/__init__.py'
    spec=importlib.util.spec_from_file_location('reloq_panel_test',path)
    plugin=importlib.util.module_from_spec(spec);spec.loader.exec_module(plugin)
    panel=plugin.EloquenceSettingsPanel();panel.makeSettings(Widget())
    assert panel.pitch_hz_mode.enabled and not panel.pitch_hz_value.enabled
    assert panel.wpm_mode.enabled and not panel.wpm_value.enabled
    assert panel.pitch_hz_value.name=='Base pitch in hertz (1–24000):'
    assert panel.camel_case.enabled and not panel.camel_case.GetValue()
    panel.pitch_hz_mode.SetValue(True);panel.on_pitch_changed(Widget())
    assert panel.pitch_hz_value.enabled and panel.pitch_dirty
    assert synth._pitchInHertz and synth._pitchHertz==150 and not hasattr(synth, 'saved')
    panel.pitch_hz_value.SetValue(151);panel.on_pitch_changed(Widget())
    assert synth._pitchHertz==151 and not hasattr(synth, 'saved')
    panel.camel_case.SetValue(True);panel.on_camel_changed(Widget())
    panel.onSave()
    assert synth._pitchInHertz and synth._pitchHertz==151 and synth.saved
    assert synth._camelCase and not panel.camel_dirty
    assert not panel.pitch_dirty
    panel.wpm_mode.SetValue(True);panel.on_wpm_changed(Widget())
    assert panel.wpm_value.enabled and panel.wpm_dirty
    panel.wpm_value.SetValue(50);panel.on_wpm_changed(Widget())
    assert synth._rateInWpm and synth._rateWpm==50
    panel.onSave()
    assert synth._rateInWpm and synth._rateWpm==50 and not panel.wpm_dirty
    # Apply becomes the new baseline; cancelling subsequent live changes
    # restores it instead of the values from when the dialog was opened.
    panel.pitch_hz_value.SetValue(220);panel.on_pitch_changed(Widget())
    panel.wpm_mode.SetValue(False);panel.on_wpm_changed(Widget())
    assert synth._pitchHertz==220 and not synth._rateInWpm
    panel.onDiscard()
    assert (synth._pitchInHertz,synth._pitchHertz)==(True,151)
    assert (synth._rateInWpm,synth._rateWpm)==(True,50)
    assert not panel.pitch_dirty and not panel.wpm_dirty
    panel=plugin.EloquenceSettingsPanel();panel.makeSettings(Widget())
    panel.pitch_hz_mode.SetValue(False);panel.on_pitch_changed(Widget())
    panel.wpm_value.SetValue(90);panel.on_wpm_changed(Widget())
    assert not synth._pitchInHertz and synth._rateWpm==90
    panel.onDiscard()
    assert (synth._pitchInHertz,synth._pitchHertz)==(True,151)
    assert (synth._rateInWpm,synth._rateWpm)==(True,50)
    panel=plugin.EloquenceSettingsPanel();panel.makeSettings(Widget())
    assert not panel.eq_mode.GetValue() and not panel.eq_preset.enabled
    panel.eq_mode.SetValue(True);panel.on_equalizer_mode(Widget())
    panel.eq_preset.SetSelection(1);panel.on_equalizer_preset(Widget())
    assert synth._eqEnabled and synth._eqPreset==1
    assert tuple(b.GetValue() for b in panel.eq_bands)==(4,0,-1)
    panel.eq_bands[1].SetValue(3);panel.on_equalizer_band(Widget())
    assert (synth._eqPreset,synth._eqCustomLow,synth._eqCustomMid,synth._eqCustomHigh)==(4,4,3,-1)
    panel.onSave()
    changed_filters=list(synth.equalizer_filters());changed_filters[0]=(True,900,4,50)
    synth.configure_equalizer_filters(changed_filters);panel.eq_dirty=True
    panel.eq_preset.SetSelection(2);panel.on_equalizer_preset(Widget())
    assert synth._eqPreset==2 and (synth._eqCustomLow,synth._eqCustomMid,synth._eqCustomHigh)==(4,3,-1)
    panel.eq_preset.SetSelection(4);panel.on_equalizer_preset(Widget())
    assert tuple(b.GetValue() for b in panel.eq_bands)==(4,3,-1)
    panel.eq_mode.SetValue(False);panel.on_equalizer_mode(Widget())
    panel.onDiscard()
    assert (synth._eqEnabled,synth._eqPreset,synth._eqCustomLow,synth._eqCustomMid,synth._eqCustomHigh)==(True,4,4,3,-1)
    assert synth.equalizer_filters()[0]==(False,125,0,100)
    panel=plugin.EloquenceSettingsPanel();panel.makeSettings(Widget())
    # Incomplete typed input must not preview an invalid native value.
    panel.wpm_value.SetValue(0);panel.on_wpm_changed(Widget())
    assert synth._rateWpm==50 and not panel.wpm_dirty
    synth._rateInWpm=False
    panel.onPanelActivated()
    assert not panel.wpm_mode.GetValue() and not panel.wpm_value.enabled
    synth._pitchInHertz=False
    panel.onPanelActivated()
    assert not panel.pitch_hz_mode.GetValue() and not panel.pitch_hz_value.enabled
    synth._camelCase=False;panel.onPanelActivated()
    assert not panel.camel_case.GetValue()
    print('NVDA settings panel: live Hertz/WPM/EQ preview, apply, cancel and refresh passed')


if __name__=='__main__':main()
