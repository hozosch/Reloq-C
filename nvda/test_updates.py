"""Published update selection, streamed integrity and failed-transfer recovery."""
import hashlib
import io
import json
from pathlib import Path
import tempfile
import unittest
import ast
import types
import sys
import weakref
import threading
import zipfile
from unittest.mock import patch
from dataclasses import replace
from _reloq_updates import AutomaticUpdates, check, download, select, Update, version
import _reloq_updates


class Updates(unittest.TestCase):
    def setUp(self):
        self.bytes = b'fixture package\0' * 5000
        self.asset = dict(name='reloq-c-synth-x64-0.2.197.nvda-addon',
            size=len(self.bytes), digest='sha256:' + hashlib.sha256(self.bytes).hexdigest())
        self.asset['browser_download_url'] = 'https://github.com/hozosch/Reloq-C/releases/download/v0.2.197/' + self.asset['name']
        self.feed = [{'assets': [self.asset], 'draft': False}]

    def test_published_versions_and_platform(self):
        data = [{'draft': True, 'assets': [self.asset]}, {'assets': [{**self.asset, 'name': 'other.apk'}]}]
        self.assertIsNone(select(data, '0.2.196'))
        self.assertIsNone(select([], '0.2.196'))
        self.assertIsNone(select(self.feed, '0.2.197'))
        self.assertIsNone(select(self.feed, '0.2.198'))
        self.assertEqual(select(self.feed, '0.2.196').version, '0.2.197')
        self.assertGreater(version('0.2.1000'), version('0.2.999'))
        self.assertEqual(check('0.2.196', fetch=lambda *a, **kw: io.BytesIO(json.dumps(self.feed).encode())),
                         select(self.feed, '0.2.196'))

    def test_development_build_waits_for_matching_stable_release(self):
        def feed(number):
            name = f'reloq-c-synth-x64-{number}.nvda-addon'
            return [{'assets': [{**self.asset, 'name': name, 'browser_download_url':
                f'https://github.com/hozosch/Reloq-C/releases/download/v{number}/{name}'}]}]
        self.assertIsNone(select(feed('1.0.1'), '1.0.2-dev1'))
        self.assertIsNone(select(feed('1.0.2-dev2'), '1.0.2-dev1'))
        self.assertEqual(select(feed('1.0.2'), '1.0.2-dev1').version, '1.0.2')
        self.assertIsNone(select(feed('1.0.2'), '1.0.2'))
        self.assertIsNone(select(feed('1.0.2'), '1.1.0-rc1'))
        self.assertEqual(select(feed('1.1.0'), '1.1.0-rc1').version, '1.1.0')

    def test_metadata_rejects_wrong_source_or_missing_integrity(self):
        for values in [dict(digest=None), dict(digest='sha1:' + '0'*40), dict(size=0),
                       dict(size=130*1024*1024), dict(size=True),
                       dict(browser_download_url=self.asset['browser_download_url'].replace('github.com', 'example.net')),
                       dict(browser_download_url=self.asset['browser_download_url'].replace('/hozosch/', '/someone/'))]:
            with self.subTest(values=values), self.assertRaises(ValueError):
                select([{'assets': [{**self.asset, **values}]}], '0.2.196')

    def test_older_version_numbers_with_stable_filename(self):
        name = 'reloq-c-synth-x64-1.0.0.nvda-addon'
        asset = {**self.asset, 'name': name, 'browser_download_url':
                 'https://github.com/hozosch/Reloq-C/releases/download/v1.0/' + name}
        for installed in ('0.2.192', '0.2.204', '0.2.205'):
            self.assertEqual(select([{'assets': [asset]}], installed).version, '1.0.0')
        self.assertIsNone(select([{'assets': [asset]}], '1.0.0'))

    def test_version_ordering_for_stable_filename(self):
        name = 'reloq-c-synth-x64-1.0.1.nvda-addon'
        asset = {**self.asset, 'name': name, 'browser_download_url':
                 'https://github.com/hozosch/Reloq-C/releases/download/v1.0.1/' + name}
        self.assertEqual(select([{'assets': [asset]}], '1.0.0').version, '1.0.1')
        self.assertIsNone(select([{'assets': [asset]}], '1.0.1'))

    def test_same_version_requires_a_later_build_marker(self):
        asset = self.asset
        old = {'body': '<!-- ReLoq-C build:1700000000 -->', 'assets': [asset]}
        new = {'body': 'Release notes\n\n<!-- ReLoq-C build:1700000600 -->', 'assets': [asset]}
        self.assertIsNone(select([old], '0.2.197', 1700000000))
        self.assertEqual(select([new], '0.2.197', 1700000000).build_epoch, 1700000600)
        self.assertEqual(select([new, old], '0.2.197', 1699999999).build_epoch, 1700000600)
        self.assertEqual(select([{**new, 'body': new['body'].replace('\n', '\r\n')}],
                                '0.2.197', 1700000000).build_epoch, 1700000600)
        self.assertIsNone(select([new], '0.2.197', 0))
        self.assertIsNone(select([{'body': '', 'assets': [asset]}], '0.2.197', 1700000000))
        duplicate = new['body'] + '\n<!-- ReLoq-C build:1700000700 -->'
        self.assertIsNone(select([{'body': duplicate, 'assets': [asset]}], '0.2.197', 1700000000))
        self.assertIsNone(select([{'body': '<!-- ReLoq-C build:999999999999 -->', 'assets': [asset]}], '0.2.197', 1700000000))
        candidate = select([new], '0.2.197', 1700000000)
        self.assertEqual(candidate.identity, '0.2.197@1700000600')
        with tempfile.TemporaryDirectory() as folder:
            self.assertEqual(download(candidate, folder, fetch=lambda *a, **kw: io.BytesIO(self.bytes)).read_bytes(), self.bytes)

    def test_installed_build_is_read_from_addon_metadata(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            module = root / 'synthDrivers/reloqC/_reloq_updates.py'
            module.parent.mkdir(parents=True)
            (root / 'doc').mkdir()
            metadata = root / 'doc/source.json'
            with patch.object(_reloq_updates, '__file__', str(module)):
                self.assertEqual(_reloq_updates.installed_build_epoch(), 0)
                metadata.write_text(json.dumps({'build_epoch': 1700000000}))
                self.assertEqual(_reloq_updates.installed_build_epoch(), 1700000000)
                metadata.write_text(json.dumps({'build_epoch': True}))
                self.assertEqual(_reloq_updates.installed_build_epoch(), 0)

    def test_streamed_download_then_reject_corrupt_truncated_and_oversized(self):
        update = select(self.feed, '0.2.196')
        with tempfile.TemporaryDirectory() as folder:
            target = download(update, folder, fetch=lambda *a, **kw: io.BytesIO(self.bytes))
            self.assertEqual(target.read_bytes(), self.bytes)
            for data in (self.bytes[:-1], self.bytes+b'x', self.bytes.replace(b'fixture', b'corrupt')):
                with self.subTest(length=len(data)), self.assertRaises(ValueError):
                    download(update, folder, fetch=lambda *a, **kw: io.BytesIO(data))
                self.assertEqual(target.read_bytes(), self.bytes)
                self.assertFalse(Path(str(target)+'.part').exists())
            with self.assertRaises(ValueError):
                download(replace(update, name='../escape.nvda-addon'), folder)

    def test_automatic_check_is_background_stoppable_and_obeys_setting(self):
        arrived = threading.Event()
        calls = []
        enabled = [False]
        candidate = select(self.feed, '0.2.196')
        def fetch(installed):
            calls.append(installed)
            return candidate
        service = AutomaticUpdates('0.2.196', lambda: enabled[0],
                                   lambda value: arrived.set(), fetch=fetch,
                                   startup_delay=0, interval=.02)
        service.start()
        self.assertFalse(arrived.wait(.06))
        self.assertEqual(calls, [])
        enabled[0] = True
        self.assertTrue(arrived.wait(1))
        service.stop()
        service.thread.join(1)
        self.assertFalse(service.thread.is_alive())
        self.assertTrue(calls and all(value == '0.2.196' for value in calls))
        self.assertIsNone(select([{'prerelease': True, 'assets': [self.asset]}], '0.2.196'))

    def test_settings_check_download_and_native_installer_handoff(self):
        source = Path(__file__).resolve().parents[1] / 'addon/synth/globalPlugins/reloqCDictionaries/__init__.py'
        tree = ast.parse(source.read_text())
        panel_code = next(node for node in tree.body if isinstance(node, ast.ClassDef) and node.name == 'EloquenceSettingsPanel')
        class Control:
            def Enable(self, value): self.enabled = value
            def SetLabel(self, value): self.label = value
        class Thread:
            def __init__(self, *, target, **kw): self.target = target
            def start(self): self.target()
        for wrong_identity, closed, corrupt in ((False, False, False), (True, False, False),
                                                (False, True, False), (False, False, True)):
            with self.subTest(wrong_identity=wrong_identity, closed=closed, corrupt=corrupt):
                output = io.BytesIO()
                with zipfile.ZipFile(output, 'w') as archive:
                    archive.writestr('manifest.ini', json.dumps(dict(name='other' if wrong_identity else 'reloqC', version='0.2.197')))
                data = output.getvalue()
                asset = {**self.asset, 'size': len(data), 'digest': 'sha256:' + hashlib.sha256(data).hexdigest()}
                candidate = select([{'assets': [asset]}], '0.2.196')
                paths, installed = [], []
                def transfer(update, folder):
                    path = download(update, folder, fetch=lambda *a, **kw: io.BytesIO(data + b'x' if corrupt else data))
                    paths.append(path)
                    return path
                def bundle(path):
                    with zipfile.ZipFile(path) as archive:
                        return types.SimpleNamespace(manifest=json.loads(archive.read('manifest.ini')))
                adapter = types.ModuleType('synthDrivers.reloqC')
                adapter._reloq_updates = types.SimpleNamespace(check=lambda v: candidate, download=transfer)
                install_module = types.ModuleType('gui.addonGui')
                def native_install(parent, path):
                    self.assertTrue(Path(path).is_file())
                    installed.append(path)
                    return True
                install_module.installAddon = native_install
                modules = {'synthDrivers': types.ModuleType('synthDrivers'), 'synthDrivers.reloqC': adapter,
                           'gui': types.ModuleType('gui'), 'gui.addonGui': install_module}
                environment = dict(SettingsPanel=object, _=lambda s: s, weakref=weakref, tempfile=tempfile,
                    threading=types.SimpleNamespace(Thread=Thread), addonHandler=types.SimpleNamespace(AddonBundle=bundle),
                    wx=types.SimpleNamespace(CallAfter=lambda fn, *args: fn(*args)), ui=types.SimpleNamespace(message=lambda s: None))
                exec(compile(ast.Module(body=[panel_code], type_ignores=[]), str(source), 'exec'), environment)
                panel = environment['EloquenceSettingsPanel']()
                panel.package_update = None; panel.package_pending = False; panel.closing = False
                panel.installed_name = 'reloqC'; panel.installed_version = '0.2.196'
                panel.package_button = Control(); panel.package_status = Control()
                with patch.dict(sys.modules, modules):
                    panel.on_package_update(None)
                    self.assertEqual(panel.package_update, candidate)
                    panel.closing = closed
                    panel.on_package_update(None)
                self.assertEqual(len(installed), 0 if wrong_identity or closed or corrupt else 1)
                self.assertTrue(all(not path.exists() for path in paths))

    def test_automatic_offer_checks_consent_and_package_identity(self):
        source = Path(__file__).resolve().parents[1] / 'addon/synth/globalPlugins/reloqCDictionaries/__init__.py'
        tree = ast.parse(source.read_text())
        code = next(node for node in tree.body if isinstance(node, ast.ClassDef) and node.name == 'GlobalPlugin')
        class Thread:
            def __init__(self, *, target, **kwargs): self.target = target
            def start(self): self.target()
        for consent, wrong_identity, corrupt, enabled in ((True,False,False,True),
                (False,False,False,True),(True,True,False,True),(True,False,True,True),
                (True,False,False,False)):
            with self.subTest(consent=consent, identity=wrong_identity, corrupt=corrupt, enabled=enabled):
                output=io.BytesIO()
                with zipfile.ZipFile(output,'w') as archive:
                    archive.writestr('manifest.ini',json.dumps(dict(
                        name='wrong' if wrong_identity else 'reloqC',version='0.2.197')))
                data=output.getvalue()
                asset={**self.asset,'size':len(data),'digest':'sha256:'+hashlib.sha256(data).hexdigest()}
                candidate=select([{'assets':[asset]}],'0.2.196')
                installed=[];messages=[];paths=[]
                def transfer(update,folder):
                    path=download(update,folder,fetch=lambda *a,**kw:io.BytesIO(data+b'x' if corrupt else data))
                    paths.append(path);return path
                def bundle(path):
                    with zipfile.ZipFile(path) as archive:
                        return types.SimpleNamespace(manifest=json.loads(archive.read('manifest.ini')))
                adapter=types.ModuleType('synthDrivers.reloqC')
                adapter._reloq_updates=types.SimpleNamespace(download=transfer)
                installer=types.ModuleType('gui.addonGui')
                installer.installAddon=lambda parent,path:installed.append(path) or True
                gui=types.ModuleType('gui');gui.mainFrame=object()
                gui.messageBox=lambda message,*a,**kw:messages.append(message) or (1 if consent else 0)
                wx=types.SimpleNamespace(CallAfter=lambda f,*a:f(*a),YES=1,YES_NO=2,ICON_QUESTION=4,OK=8,ICON_ERROR=16)
                environment=dict(globalPluginHandler=types.SimpleNamespace(GlobalPlugin=object),_=lambda s:s,
                    wx=wx,gui=gui,config=types.SimpleNamespace(conf={'reloqCUpdates':{'automatic':enabled,'lastOffered':''}}),
                    threading=types.SimpleNamespace(Thread=Thread),tempfile=tempfile,
                    addonHandler=types.SimpleNamespace(AddonBundle=bundle),ui=types.SimpleNamespace(message=lambda s:None))
                modules={'synthDrivers':types.ModuleType('synthDrivers'),'synthDrivers.reloqC':adapter,
                         'gui':gui,'gui.addonGui':installer}
                exec(compile(ast.Module(body=[code],type_ignores=[]),str(source),'exec'),environment)
                plugin=environment['GlobalPlugin'].__new__(environment['GlobalPlugin'])
                plugin._closed=False;plugin._offered=set();plugin._addon_name='reloqC'
                with patch.dict(sys.modules,modules):
                    plugin._offer_update(candidate)
                    plugin._offer_update(candidate)
                self.assertEqual(len(installed),1 if consent and not wrong_identity and not corrupt and enabled else 0)
                self.assertTrue(all(not path.exists() for path in paths))
                if enabled:self.assertEqual(sum('available.' in x for x in messages),1)
                else:self.assertEqual(messages,[])


if __name__ == '__main__':
    unittest.main()
