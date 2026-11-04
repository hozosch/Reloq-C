"""Accessible update feedback: counts, dialogs, failures and stale callbacks."""
import ast
from pathlib import Path
import sys
import threading
import types
import unittest
from unittest import mock
import weakref

SOURCE=Path(__file__).resolve().parents[1]/'addon/synth/globalPlugins/reloqCDictionaries/__init__.py'
DRIVER=Path(__file__).resolve().parents[1]/'addon/synth/synthDrivers/reloqC/__init__.py'


class Control:
    def __init__(self):self.label=None
    def SetLabel(self,value):self.label=value


class UpdateDialogTests(unittest.TestCase):
    def setUp(self):
        self.dialogs=[];self.messages=[]
        tree=ast.parse(SOURCE.read_text(encoding='utf8'))
        wanted={'dictionary_update_message','show_dictionary_update','EloquenceSettingsPanel'}
        nodes=[node for node in tree.body if isinstance(node,(ast.FunctionDef,ast.ClassDef)) and node.name in wanted]
        gui=types.SimpleNamespace(mainFrame=object(),messageBox=lambda message,title,style,parent=None:
                                  self.dialogs.append((message,title,style,parent)))
        wx=types.SimpleNamespace(OK=1,ICON_INFORMATION=2,ICON_ERROR=4,CallAfter=lambda fn:fn())
        env={'SettingsPanel':object,'_':lambda text:text,'weakref':weakref,'gui':gui,'wx':wx,
             'ui':types.SimpleNamespace(message=self.messages.append)}
        exec(compile(ast.Module(body=nodes,type_ignores=[]),str(SOURCE),'exec'),env)
        self.env=env;self.panel=env['EloquenceSettingsPanel']()
        self.panel.pending=False;self.panel.operation=0;self.panel.closing=False
        self.panel.status=Control();self.panel.enable_controls=lambda enabled:None
        self.panel.current_synth=lambda:True

    def test_manual_update_uses_dialog_and_reports_unique_additions(self):
        done=self.panel.begin(show_result=True)
        done(None,{'dictionary_update':{'compared':True,'added':3,'total':12,'added_files':1,'total_files':4},'remote_changed':True})
        self.assertEqual(len(self.dialogs),1)
        self.assertIn('New entries: 3',self.dialogs[0][0])
        self.assertIn('New dictionary files: 1',self.dialogs[0][0])
        self.assertFalse(self.messages);self.assertFalse(self.panel.pending)

    def test_zero_first_and_unknown_counts(self):
        message=self.env['dictionary_update_message']
        unchanged=message({'dictionary_update':{'compared':True,'added':0,'total':12,'added_files':0,'total_files':3}})
        self.assertIn('New entries: 0',unchanged);self.assertIn('New dictionary files: 0',unchanged)
        first=message({'dictionary_update':{'compared':False,'added':9,'total':9,'added_files':2,'total_files':2}})
        self.assertIn('first time',first);self.assertIn('Dictionary files in total: 2',first)
        self.assertIn('files could not be determined',message({'dictionary_update':None}))

    def test_remote_update_after_settings_close_and_manual_error(self):
        done=self.panel.begin();self.panel.closing=True
        done(None,{'dictionary_update':{'compared':True,'added':2,'total':5,'added_files':1,'total_files':2},'remote_changed':True})
        self.assertEqual(len(self.dialogs),1)
        self.assertIn('New entries: 2',self.dialogs[0][0])
        self.panel.closing=False
        done=self.panel.begin(show_result=True);done(ValueError('offline'),None)
        self.assertIn('offline',self.dialogs[-1][0]);self.assertEqual(self.dialogs[-1][2],5)

    def test_stale_request_does_not_show_dialog(self):
        old=self.panel.begin(show_result=True);self.panel.begin(show_result=True)
        old(None,{'dictionary_update':{'compared':True,'added':9,'total':9,'added_files':1,'total_files':1},'remote_changed':True})
        self.assertFalse(self.dialogs)

    def test_background_notice_runs_on_ui_thread_and_skips_stale_or_closed_driver(self):
        tree=ast.parse(DRIVER.read_text(encoding='utf8'))
        driver=next(node for node in tree.body if isinstance(node,ast.ClassDef) and node.name=='SynthDriver')
        method=next(node for node in driver.body if isinstance(node,ast.FunctionDef) and node.name=='_dictionary_update_notice')
        env={};exec(compile(ast.Module(body=[method],type_ignores=[]),str(DRIVER),'exec'),env)
        revision=[5];pending=[];shown=[]
        instance=types.SimpleNamespace(_submit_lock=threading.Lock(),_terminating=False,
            _dictionary_updater=types.SimpleNamespace(current=lambda value:value==revision[0]))
        wx=types.ModuleType('wx');wx.CallAfter=pending.append
        package=types.ModuleType('globalPlugins');package.__path__=[]
        plugin=types.ModuleType('globalPlugins.reloqCDictionaries');plugin.show_dictionary_update=shown.append
        details={'dictionary_update':{'compared':True,'added':2,'total':8,'added_files':1,'total_files':2}}
        with mock.patch.dict(sys.modules,{'wx':wx,'globalPlugins':package,
                                          'globalPlugins.reloqCDictionaries':plugin}):
            env['_dictionary_update_notice'](instance,5,details)
            self.assertFalse(shown);pending.pop(0)();self.assertEqual(shown,[details])
            env['_dictionary_update_notice'](instance,5,details);revision[0]=6;pending.pop(0)()
            env['_dictionary_update_notice'](instance,6,details);instance._terminating=True;pending.pop(0)()
        self.assertEqual(shown,[details])


if __name__=='__main__':unittest.main()
