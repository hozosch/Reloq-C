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
import zipfile
from unittest.mock import patch
from dataclasses import replace
from _reloq_updates import check, download, select, Update, version


class Updates(unittest.TestCase):
    def setUp(self):
        self.bytes = b'fixture package\0' * 5000
        self.asset = dict(name='reloq-c-synth-experimental-x64-0.2.197.nvda-addon',
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

    def test_metadata_rejects_wrong_source_or_missing_integrity(self):
        for values in [dict(digest=None), dict(digest='sha1:' + '0'*40), dict(size=0),
                       dict(size=130*1024*1024), dict(size=True),
                       dict(browser_download_url=self.asset['browser_download_url'].replace('github.com', 'example.net')),
                       dict(browser_download_url=self.asset['browser_download_url'].replace('/hozosch/', '/someone/'))]:
            with self.subTest(values=values), self.assertRaises(ValueError):
                select([{'assets': [{**self.asset, **values}]}], '0.2.196')

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
                    archive.writestr('manifest.ini', json.dumps(dict(name='other' if wrong_identity else 'reloqCExperimental', version='0.2.197')))
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
                panel.installed_name = 'reloqCExperimental'; panel.installed_version = '0.2.196'
                panel.package_button = Control(); panel.package_status = Control()
                with patch.dict(sys.modules, modules):
                    panel.on_package_update(None)
                    self.assertEqual(panel.package_update, candidate)
                    panel.closing = closed
                    panel.on_package_update(None)
                self.assertEqual(len(installed), 0 if wrong_identity or closed or corrupt else 1)
                self.assertTrue(all(not path.exists() for path in paths))


if __name__ == '__main__':
    unittest.main()
