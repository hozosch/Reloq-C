"""Exercise the keyboard-first editor in actual running NVDA."""
import json
import os
from pathlib import Path
import time
import traceback
import ctypes

import globalPluginHandler
import wx

RESULT=Path(os.environ['RELOQ_SEGMENTATION_PROBE_RESULT'])


class Player:
    def __init__(self,*args,**kwargs):self.chunks=[]
    def feed(self,data):self.chunks.append(bytes(data))
    def idle(self):pass
    def close(self):pass


class Key:
    def __init__(self,code,shift=False):self.code=code;self.shift=shift;self.skipped=False
    def GetKeyCode(self):return self.code
    def ShiftDown(self):return self.shift
    def ControlDown(self):return False
    def AltDown(self):return False
    def Skip(self):self.skipped=True


class GlobalPlugin(globalPluginHandler.GlobalPlugin):
    def __init__(self):
        super().__init__();wx.CallLater(1500,self.setup)

    def fail(self):RESULT.write_text(json.dumps({'ok':False,'error':traceback.format_exc()}),encoding='utf8')

    def setup(self):
        try:
            import gui
            import globalPlugins.reloqSoundLab as soundlab
            from globalPlugins.reloqSoundLab.editor import PhonemeEditorDialog
            active=[p for p in globalPluginHandler.runningPlugins if isinstance(p,soundlab.GlobalPlugin)]
            assert active and active[0].item,'editor menu missing'
            self.players=[]
            def make(*a,**kw):
                player=Player(*a,**kw);self.players.append(player);return player
            soundlab.nvwave.WavePlayer=make
            self.dialog=PhonemeEditorDialog(gui.mainFrame);self.errors=[];self.dialog.error=self.errors.append
            self.dialog.Show();self.dialog.Raise();wx.YieldIfNeeded()
            from NVDAObjects.IAccessible import getNVDAObjectFromEvent
            send=ctypes.windll.user32.SendMessageW
            send.argtypes=(ctypes.c_void_p,ctypes.c_uint,ctypes.c_size_t,ctypes.c_ssize_t);send.restype=ctypes.c_void_p
            labels=[('Sprache',self.dialog.language),('Laut',self.dialog.phone),('Stimme',self.dialog.gender),
                    ('Abtastrate',self.dialog.rate),('Kopfgröße',self.dialog.head),
                    ('Geschwindigkeit',self.dialog.speed),('Tonhöhe',self.dialog.pitch),
                    ('Lauttakt',self.dialog.clock),('Wiedergabe',self.dialog.context),
                    ('Formantbearbeitung: Ziffern 1 bis 5, Pfeiltasten und F2',self.dialog.area)]
            self.names=[]
            for label,field in labels:
                hwnd=int(field.GetHandle());handles=[hwnd]
                if isinstance(field,wx.SpinCtrl):
                    buddy=send(hwnd,0x046A,0,0)
                    if buddy:handles.insert(0,int(buddy))
                names=[]
                for handle in handles:
                    obj=getNVDAObjectFromEvent(handle,-4,0)
                    if obj is not None:names.append(obj.name)
                self.names.append((label,names));assert label in names,(label,names)
            self.dialog.on_prepare(None);self.phase=0;self.deadline=time.monotonic()+60
            wx.CallLater(100,self.check)
        except Exception:self.fail()

    def check(self):
        try:
            if self.dialog._busy:
                assert time.monotonic()<self.deadline,'phoneme editor timed out'
                wx.CallLater(100,self.check);return
            assert not self.errors,self.errors
            if self.phase==0:
                assert self.dialog._loaded and self.dialog._loaded.key=='i'
                self.dialog.area.SetFocus();wx.YieldIfNeeded()
                assert wx.Window.FindFocus() is self.dialog.area,'editor keyboard focus missing'
                # Real wx key-event routing uses the dialog's char hook.
                for code in (ord('2'),wx.WXK_RIGHT,wx.WXK_UP):
                    event=wx.KeyEvent(wx.wxEVT_CHAR_HOOK);event.m_keyCode=code
                    assert self.dialog.GetEventHandler().ProcessEvent(event),'key not handled'
                assert self.dialog.selected_formant==1
                assert self.dialog.frequencies[1]==25 and self.dialog.gains_db[1]==1
                from globalPlugins.reloqSoundLab import phonemes
                options=phonemes.PHONES['Persisch']
                def choose(key):
                    self.dialog.phone.SetSelection(next(i for i,p in enumerate(options) if p.key==key))
                    self.dialog.on_phone(Key(0))
                choose('u');self.dialog.frequencies[2]=75;self.dialog.gains_db[2]=-2;self.dialog._remember()
                choose('i')
                assert self.dialog.frequencies[1]==25 and self.dialog.gains_db[1]==1
                project=self.dialog.project();self.dialog.on_reset(None)
                assert not any(self.dialog.frequencies) and not any(self.dialog.gains_db)
                self.dialog.restore_project(project)
                assert self.dialog.frequencies[1]==25 and self.dialog.gains_db[1]==1
                choose('u');assert self.dialog.frequencies[2]==75 and self.dialog.gains_db[2]==-2
                choose('i');assert self.dialog.frequencies[1]==25 and self.dialog.gains_db[1]==1
                self.dialog.area.SetFocus();wx.YieldIfNeeded()
                self.phase=1
                event=wx.KeyEvent(wx.wxEVT_CHAR_HOOK);event.m_keyCode=wx.WXK_F2
                assert self.dialog.GetEventHandler().ProcessEvent(event),'F2 not handled'
                wx.CallLater(100,self.check);return
            assert self.players and self.players[-1].chunks,'editor did not play audio'
            if self.phase==1:
                self.modified=b''.join(self.players[-1].chunks);self.phase=2
                event=wx.KeyEvent(wx.wxEVT_CHAR_HOOK);event.m_keyCode=wx.WXK_F3
                assert self.dialog.GetEventHandler().ProcessEvent(event),'F3 not handled'
                wx.CallLater(100,self.check);return
            original=b''.join(self.players[-1].chunks)
            assert self.modified!=original and len(self.modified)==len(original),'edited phone not audible'
            self.dialog.phone.SetFocus();wx.YieldIfNeeded()
            event=Key(wx.WXK_RIGHT);self.dialog.on_key(event)
            assert event.skipped,'arrow was swallowed outside editor'
            self.dialog._closed=True;self.dialog.Destroy()
            RESULT.write_text(json.dumps({'ok':True,'labels':self.names,'f2_f3_audio':True,
                                          'keyboard_formant_2':True,'project_roundtrip':True}),encoding='utf8')
        except Exception:self.fail()
