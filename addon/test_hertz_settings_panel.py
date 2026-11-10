"""Exercise the accessible Hertz controls and NVDA settings-panel lifecycle."""
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
    module('globalPluginHandler',GlobalPlugin=type('GlobalPlugin',(),{}))
    module('gui',__path__=[],messageBox=lambda *a,**k:None)
    class SettingsPanel(Widget):
        def onPanelActivated(self):pass
    module('gui.settingsDialogs',SettingsPanel=SettingsPanel,
           NVDASettingsDialog=type('NVDASettingsDialog',(),{'categoryClasses':[]}))
    module('ui',message=lambda message:None)
    wx=module('wx',StaticText=Widget,CheckBox=Widget,SpinCtrl=Widget,Choice=Widget,
              ListBox=Widget,BoxSizer=Widget,Button=Widget,CallAfter=lambda fn,*a:fn(*a))
    for name in ('ALL','LEFT','RIGHT','BOTTOM','EXPAND','HORIZONTAL','EVT_CHECKBOX',
                 'EVT_SPINCTRL','EVT_TEXT','EVT_BUTTON','EVT_WINDOW_DESTROY'):
        setattr(wx,name,1)
    langs=module('synthDrivers.reloqC._reloq_languages',LANGUAGES={},LANGUAGE_DISPLAY_ORDER=[])
    module('synthDrivers',__path__=[]);module('synthDrivers.reloqC',__path__=[])
    module('synthDrivers.reloqC._reloq_dictionary_manager',DictionarySource=object)
    class Synth:
        name='reloqC';communityDictionaries=False;_pitchInHertz=False;_pitchHertz=150
        def dictionary_manager(self):return types.SimpleNamespace(sources=[],repository='community')
        def configure_pitch_hertz(self,enabled,value):self._pitchInHertz=enabled;self._pitchHertz=value
        def configure_dictionaries(self,sources,enabled,callback,repository):callback(None,{})
        def saveSettings(self):self.saved=True
    synth=Synth();module('synthDriverHandler',getSynth=lambda:synth)
    path=Path(__file__).parent/'synth/globalPlugins/reloqCDictionaries/__init__.py'
    spec=importlib.util.spec_from_file_location('reloq_panel_test',path)
    plugin=importlib.util.module_from_spec(spec);spec.loader.exec_module(plugin)
    panel=plugin.EloquenceSettingsPanel();panel.makeSettings(Widget())
    assert panel.pitch_hz_mode.enabled and not panel.pitch_hz_value.enabled
    assert panel.pitch_hz_value.name=='Base pitch in hertz (1–24000):'
    assert panel.camel_case.enabled and not panel.camel_case.GetValue()
    panel.pitch_hz_mode.SetValue(True);panel.on_pitch_changed(Widget())
    assert panel.pitch_hz_value.enabled and panel.pitch_dirty
    panel.camel_case.SetValue(True);panel.on_camel_changed(Widget())
    panel.pitch_hz_value.SetValue(151);panel.onSave()
    assert synth._pitchInHertz and synth._pitchHertz==151 and synth.saved
    assert synth._camelCase and not panel.camel_dirty
    assert not panel.pitch_dirty
    synth._pitchInHertz=False
    panel.onPanelActivated()
    assert not panel.pitch_hz_mode.GetValue() and not panel.pitch_hz_value.enabled
    synth._camelCase=False;panel.onPanelActivated()
    assert not panel.camel_case.GetValue()
    print('NVDA settings panel: Hertz and camel case enablement, save and refresh passed')


if __name__=='__main__':main()
