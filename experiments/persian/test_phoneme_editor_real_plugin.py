"""Exercise the keyboard-first editor in actual running NVDA."""
import json
import os
from pathlib import Path
import struct
import time
import traceback
import ctypes

import globalPluginHandler
import wx

RESULT=Path(os.environ['RELOQ_SEGMENTATION_PROBE_RESULT'])


def require(condition, detail):
    # NVDA's frozen Python can run with assertions optimized out. Explicit
    # checks also ensure event dispatch is never skipped by optimization.
    if not condition:
        raise AssertionError(detail)


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
            require(active and active[0].item,'editor menu missing')
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
                self.names.append((label,names));require(label in names,(label,names))
            self.dialog.on_prepare(None);self.phase=0;self.deadline=time.monotonic()+60
            wx.CallLater(100,self.check)
        except Exception:self.fail()

    def check(self):
        try:
            if self.dialog._busy:
                require(time.monotonic()<self.deadline,'phoneme editor timed out')
                wx.CallLater(100,self.check);return
            require(not self.errors,self.errors)
            if self.phase==0:
                require(self.dialog._loaded and self.dialog._loaded.key=='i','Persian I was not loaded')
                self.dialog.area.SetFocus();wx.YieldIfNeeded()
                require(wx.Window.FindFocus() is self.dialog.area,'editor keyboard focus missing')
                # Send actual key presses to the focused window. Constructed
                # wx.KeyEvent objects don't carry a native event source and
                # aren't delivered to the dialog's char hook on Windows.
                self.keys=wx.UIActionSimulator()
                for code in (ord('2'),wx.WXK_RIGHT,wx.WXK_UP):
                    require(self.keys.Char(code),'could not send a key')
                    wx.YieldIfNeeded()
                require(self.dialog.selected_formant==1,'formant selection failed')
                require(self.dialog.frequencies[1]==25 and self.dialog.gains_db[1]==1,'arrow edits failed')
                # Exercise the real import handler and frame choice in NVDA,
                # including the normal Windows file-dialog path. Only the
                # modal selections are stubbed to keep this automated.
                bases=self.dialog._loaded.frequencies
                data=bytearray(b'SPECTSEQ'+struct.pack('<I',4)+b'test'+struct.pack('<hhhh',2,100,0,0))
                for frame in range(2):
                    data.extend(b'\0'*40+struct.pack('<Hhh',0,0,100))
                    for i in range(9):
                        frequency=(200 if i==0 else bases[i-1]+(frame+1)*25 if i<=5 else 6000+i*100)
                        data.extend(struct.pack('<hhhhhh',0,0,frequency,1000,400,400))
                sample=RESULT.with_name('test-espeak-spectrum')
                sample.write_bytes(data)
                class FileSelection:
                    def __init__(self,*a,**kw):pass
                    def __enter__(self):return self
                    def __exit__(self,*a):pass
                    def ShowModal(self):return wx.ID_OK
                    def GetPath(self):return str(sample)
                class FrameSelection(FileSelection):
                    def SetSelection(self,value):self.selection=value
                    def GetSelection(self):return 1
                file_dialog,choice_dialog=wx.FileDialog,wx.SingleChoiceDialog
                try:
                    wx.FileDialog,wx.SingleChoiceDialog=FileSelection,FrameSelection
                    self.dialog.on_import(None)
                finally:
                    wx.FileDialog,wx.SingleChoiceDialog=file_dialog,choice_dialog
                require(self.dialog.frequencies==[50]*5 and self.dialog.gains_db[1]==1,
                        ('eSpeak import',self.dialog.frequencies,self.dialog.gains_db))
                self.dialog.frequencies=[0,25,0,0,0];self.dialog.gains_db=[0,1,0,0,0]
                self.dialog._remember();self.dialog._refresh()
                from globalPlugins.reloqSoundLab import phonemes
                options=phonemes.PHONES['Persisch']
                def choose(key):
                    self.dialog.phone.SetSelection(next(i for i,p in enumerate(options) if p.key==key))
                    self.dialog.on_phone(Key(0))
                choose('u');self.dialog.frequencies[2]=75;self.dialog.gains_db[2]=-2;self.dialog._remember()
                choose('i')
                require(self.dialog.frequencies[1]==25 and self.dialog.gains_db[1]==1,'sound switch lost I')
                project=self.dialog.project();self.dialog.on_reset(None)
                require(not any(self.dialog.frequencies) and not any(self.dialog.gains_db),'reset failed')
                self.dialog.restore_project(project)
                require(self.dialog.frequencies[1]==25 and self.dialog.gains_db[1]==1,'project lost I')
                choose('u');require(self.dialog.frequencies[2]==75 and self.dialog.gains_db[2]==-2,'project lost U')
                choose('i');require(self.dialog.frequencies[1]==25 and self.dialog.gains_db[1]==1,'project lost I after U')
                # F2 on the sound list loads the probe, then hands focus to
                # the edit field so number/arrow controls are immediately usable.
                self.dialog.phone.SetFocus();wx.YieldIfNeeded()
                self.phase=1
                require(self.keys.Char(wx.WXK_F2),'could not send F2')
                wx.CallLater(250,self.check);return
            require(self.players and self.players[-1].chunks,'editor did not play audio')
            if self.phase==1:
                require(wx.Window.FindFocus() is self.dialog.area,'F2 did not focus the formant editor')
                self.modified=b''.join(self.players[-1].chunks);self.phase=2
                require(self.keys.Char(wx.WXK_F3),'could not send F3')
                wx.CallLater(250,self.check);return
            original=b''.join(self.players[-1].chunks)
            require(self.modified!=original and len(self.modified)==len(original),'edited phone not audible')
            self.dialog.phone.SetFocus();wx.YieldIfNeeded()
            event=Key(wx.WXK_RIGHT);self.dialog.on_key(event)
            require(event.skipped,'arrow was swallowed outside editor')
            self.dialog._closed=True;self.dialog.Destroy()
            RESULT.write_text(json.dumps({'ok':True,'labels':self.names,'f2_f3_audio':True,
                'keyboard_formant_2':True,'project_roundtrip':True,'espeak_import':True}),encoding='utf8')
        except Exception:self.fail()
