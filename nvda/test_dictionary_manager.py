"""Runtime updates, independent source merging, rollback and selection races."""
from pathlib import Path
import json
import shutil
import tempfile
import threading
import time
import unittest
from urllib.error import URLError
from _reloq_dictionary_manager import DictionaryManager, DictionarySource, merge_sources, validate_merged, load_sources
from test_dictionaries import R, Server


class ManagerTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.cache = self.root / 'cache'
        self.deliveries, self.errors = [], []
        self.server = Server()
        self.manager = DictionaryManager(R / 'data/community', self.cache,
            lambda revision, path: self.deliveries.append((revision, path)), self.errors.append,
            fetch=self.server, watch_interval=0.02)
        self.addCleanup(self.manager.close)

    def wait(self, start):
        ready = threading.Event()
        result = []
        start(lambda error, details: (result.append((error, details)), ready.set()))
        self.assertTrue(ready.wait(10), 'dictionary completion was not delivered')
        return result[0]

    def source(self, name, data, language=0x40000, volume=0):
        path = self.root / name
        path.write_bytes(data)
        return DictionarySource(str(path), language, volume)

    def test_merge_native_volumes_precedence_and_unchanged_inputs(self):
        a = self.source('my-unrelated-name.dic', b'Armin\tErster\nOwn\t`[.1a]\n')
        b = self.source('other.dic', b'Armin\tZweiter\nOwn\t`[.1b]')
        root = self.source('roots.dic', b'ownroot\t`[.1r]\n', volume=1)
        english = self.source('english.dic', b'Other\tanother\n', 0x10000)
        before = {s.path: Path(s.path).read_bytes() for s in [a,b,root,english]}
        directory, details = merge_sources(R/'data/community', [a,b,root,english], self.cache)
        rows = dict(line.split(b'\t',1) for line in (directory/'DEUmain.dic').read_bytes().splitlines())
        self.assertEqual(rows[b'Armin'], b'Zweiter')
        self.assertEqual(rows[b'Own'], b'`[.1b]')
        self.assertIn(b'ownroot\t`[.1r]', (directory/'DEURoot.dic').read_bytes())
        self.assertIn(b'Other\tanother', (directory/'ENUmain.dic').read_bytes())
        self.assertEqual(before, {path: Path(path).read_bytes() for path in before})
        self.assertEqual(details['own_sources'],4)
        self.assertGreaterEqual(details['conflicts'],3)
        self.assertEqual(validate_merged(directory)['files'],details['files'])
        self.assertEqual(merge_sources(R/'data/community',[a,b,root,english],self.cache)[0],directory)

    def test_main_preserves_case_and_root_uses_native_case_identity(self):
        own=self.source('own.dic',b'word\town value\nARMIN\town armin\n')
        directory,_=merge_sources(R/'data/community',[own],self.cache)
        rows=dict(line.split(b'\t',1)for line in (directory/'DEUmain.dic').read_bytes().splitlines())
        self.assertEqual(rows[b'word'],b'own value');self.assertIn(b'Word',rows)
        self.assertEqual(rows[b'ARMIN'],b'own armin');self.assertIn(b'Armin',rows)
        root=self.source('root.dic',b'ELOQUENCE\town root\n',volume=1)
        directory,_=merge_sources(R/'data/community',[root],self.cache)
        rows=dict(line.split(b'\t',1)for line in (directory/'DEURoot.dic').read_bytes().splitlines())
        self.assertEqual(rows[b'ELOQUENCE'],b'own root');self.assertNotIn(b'eloquence',rows)
        en=self.source('english.dic',b'IFA\tletters\nifa\tname\n',0x10000)
        directory,_=merge_sources(None,[en],self.cache)
        rows=dict(line.split(b'\t',1)for line in (directory/'ENUmain.dic').read_bytes().splitlines())
        self.assertEqual(rows,{b'IFA':b'letters',b'ifa':b'name'})

    def test_own_files_work_without_community_and_restore(self):
        own = self.source('unrelated.dic', b'custom\tvalue\n', 0x90000)
        error, details = self.wait(lambda done:self.manager.set_sources([own],done))
        self.assertIsNone(error)
        self.assertFalse(self.server.calls)
        self.assertEqual(load_sources(self.cache),(own,))
        directory=self.deliveries[-1][1]
        self.assertEqual((directory/'FINmain.dic').read_bytes(), b'custom\tvalue\n')
        self.assertFalse((directory/'DEUmain.dic').exists())
        restored=DictionaryManager(R/'data/community',self.cache,lambda r,p:None,self.errors.append)
        self.assertEqual(restored.sources,(own,));restored.close()

    def test_manual_update_resolves_master_again_and_preserves_own(self):
        own=self.source('user.dic',b'Own\tvalue\n')
        self.wait(lambda done:self.manager.set_sources([own],done))
        self.manager.set_enabled(True)
        error,details=self.wait(self.manager.refresh)
        self.assertIsNone(error);self.assertEqual(details['community_commit'],'1'*40)
        self.server.commit='2'*40
        self.server.data['DEUmain.dic']+=b'NewWord\tnew value\n'
        error,details=self.wait(self.manager.refresh)
        self.assertIsNone(error);self.assertEqual(details['community_commit'],'2'*40)
        directory=self.deliveries[-1][1]
        self.assertIn(b'NewWord\tnew value',(directory/'DEUmain.dic').read_bytes())
        self.assertIn(b'Own\tvalue',(directory/'DEUmain.dic').read_bytes())
        self.assertEqual(json.loads((self.cache/'current.json').read_text())['commit'],'2'*40)

    def test_reload_changed_local_file_without_network(self):
        own=self.source('user.dic',b'Own\tone\n')
        self.wait(lambda done:self.manager.set_sources([own],done))
        old=self.deliveries[-1][1]
        Path(own.path).write_bytes(b'Own\ttwo\n')
        error,_=self.wait(self.manager.refresh)
        self.assertIsNone(error);self.assertNotEqual(self.deliveries[-1][1],old)
        self.assertEqual((old/'DEUmain.dic').read_bytes(),b'Own\tone\n')
        self.assertEqual((self.deliveries[-1][1]/'DEUmain.dic').read_bytes(),b'Own\ttwo\n')

    def test_discover_edit_replace_and_remove_folder_files(self):
        self.wait(lambda done: self.manager.configure([], False, done))
        self.assertIsNone(self.deliveries[-1][1])
        path = self.manager.local_directory / 'FINmain.dic'
        path.write_bytes(b'Own\tone\n')
        self.until(lambda: self.deliveries[-1][1] is not None)
        first = self.deliveries[-1][1]
        self.assertEqual((first / path.name).read_bytes(), path.read_bytes())
        replacement = path.with_suffix('.new')
        replacement.write_bytes(b'Own\ttwo\n')
        replacement.replace(path)
        self.until(lambda: self.deliveries[-1][1] != first)
        self.assertEqual((self.deliveries[-1][1] / path.name).read_bytes(), b'Own\ttwo\n')
        path.unlink()
        self.until(lambda: self.deliveries[-1][1] is None)
        self.assertFalse(self.server.calls)
        self.assertFalse(self.errors)

    def test_edit_bundled_volume_with_and_without_verified_cached_update(self):
        bundle = self.root / 'bundled'
        shutil.copytree(R / 'data/community', bundle)
        self.manager.bundled = bundle
        self.wait(lambda done: self.manager.configure([], True, done))
        original_root = (bundle / 'DEURoot.dic').read_bytes()
        (bundle / 'DEUmain.dic').write_bytes(b'Own\tlocal version\n')
        self.until(lambda: self.deliveries[-1][1] != bundle)
        current = self.deliveries[-1][1]
        self.assertEqual((current / 'DEUmain.dic').read_bytes(), b'Own\tlocal version\n')
        self.assertEqual(sorted((current / 'DEURoot.dic').read_bytes().splitlines()),
                         sorted(original_root.splitlines()))
        self.wait(self.manager.refresh)
        current = self.deliveries[-1][1]
        self.assertEqual((current / 'DEUmain.dic').read_bytes(), b'Own\tlocal version\n')
        self.assertTrue((current / 'ENURoot.dic').is_file())
        second = bundle / 'DEUmain.dic'
        second.write_bytes(b'Own\tnext local version\n')
        self.until(lambda: self.deliveries[-1][1] != current)
        self.assertEqual((self.deliveries[-1][1] / second.name).read_bytes(), b'Own\tnext local version\n')
        self.assertFalse(self.errors)

    def until(self, predicate):
        deadline = time.monotonic() + 5
        while not predicate() and time.monotonic() < deadline:
            threading.Event().wait(0.005)
        self.assertTrue(predicate(), 'automatic dictionary reload did not finish')

    def test_live_edits_and_atomic_editor_replacements_without_network(self):
        own = self.source('user.dic', b'Own\tone\n')
        error, _ = self.wait(lambda done: self.manager.configure([own], True, done))
        self.assertIsNone(error)
        before = self.deliveries[-1][1]
        Path(own.path).write_bytes(b'Own\ttwo\n')
        self.until(lambda: self.deliveries[-1][1] != before)
        second = self.deliveries[-1][1]
        self.assertIn(b'Own\ttwo\n', (second/'DEUmain.dic').read_bytes())
        replacement = self.root/'editor-temporary'
        replacement.write_bytes(b'Own\tnew\n')
        replacement.replace(own.path)
        self.until(lambda: self.deliveries[-1][1] != second)
        self.assertIn(b'Own\tnew\n', (self.deliveries[-1][1]/'DEUmain.dic').read_bytes())
        self.assertIn(b'Own\tone\n', (before/'DEUmain.dic').read_bytes())
        self.assertFalse(self.server.calls)
        self.assertFalse(self.errors)

    def test_live_invalid_or_missing_file_keeps_last_valid_data_and_recovers(self):
        own = self.source('user.dic', b'Own\tone\n')
        self.wait(lambda done: self.manager.set_sources([own], done))
        before = self.deliveries[-1][1]
        Path(own.path).write_bytes(b'Own\tbroken\0\n')
        self.until(lambda: len(self.errors) == 1)
        self.assertEqual(self.deliveries[-1][1], before)
        Path(own.path).unlink()
        self.until(lambda: len(self.errors) == 2)
        self.assertEqual(self.deliveries[-1][1], before)
        Path(own.path).write_bytes(b'Own\tvalid again\n')
        self.until(lambda: self.deliveries[-1][1] != before)
        self.assertEqual((self.deliveries[-1][1]/'DEUmain.dic').read_bytes(), b'Own\tvalid again\n')
        self.assertEqual(self.manager.sources, (own,))
        self.assertEqual(load_sources(self.cache), (own,))

    def test_live_monitor_stops_when_closed_and_waits_for_foreground_refresh(self):
        own = self.source('user.dic', b'Own\tone\n')
        self.wait(lambda done: self.manager.configure([own], True, done))
        self.server.block = threading.Event()
        self.manager.refresh()
        revision = self.manager._revision
        self.until(lambda: bool(self.server.calls))
        Path(own.path).write_bytes(b'Own\ttwo\n')
        self.assertEqual(self.manager._revision, revision)
        self.server.block.set()
        self.until(lambda: self.manager._working_revision is None)
        self.until(lambda: b'Own\ttwo\n' in (self.deliveries[-1][1]/'DEUmain.dic').read_bytes())
        thread = self.manager._watch_thread
        self.manager.close()
        thread.join(2)
        self.assertFalse(thread.is_alive())

    def test_combined_configuration_has_one_revision_and_validates_atomically(self):
        own = self.source('user.dic', b'Own\tone\n')
        revision = self.manager._revision
        error, _ = self.wait(lambda done: self.manager.configure([own], True, done))
        self.assertIsNone(error)
        self.assertEqual(self.manager._revision, revision+1)
        self.assertEqual(load_sources(self.cache), (own,))
        self.assertTrue((self.deliveries[-1][1]/'ENURoot.dic').is_file())
        from _reloq_dictionary_manager import validate_sources
        validate_sources([own])
        bad = self.source('bad.dic', b'Own\tbad\0\n')
        with self.assertRaises(ValueError):
            validate_sources([bad])
        self.assertEqual(self.manager.sources, (own,))

    def test_bad_source_rolls_back_preferences_and_active_data(self):
        own=self.source('user.dic',b'Own\tone\n')
        self.wait(lambda done:self.manager.set_sources([own],done))
        old=self.deliveries[-1][1];settings=(self.cache/'sources.json').read_bytes()
        bad=self.source('bad.dic',b'bad\tzero\0\n')
        error,_=self.wait(lambda done:self.manager.set_sources([bad],done))
        self.assertIsInstance(error,ValueError)
        self.assertEqual(self.manager.sources,(own,));self.assertEqual(self.deliveries[-1][1],old)
        self.assertEqual((self.cache/'sources.json').read_bytes(),settings)

    def test_offline_update_keeps_complete_current_set(self):
        self.manager.set_enabled(True)
        self.wait(self.manager.refresh)
        pointer=(self.cache/'current.json').read_bytes()
        def offline(*args,**kwargs):raise URLError('offline')
        self.manager.fetch=offline
        error,_=self.wait(self.manager.refresh)
        self.assertIsInstance(error,URLError)
        self.assertEqual((self.cache/'current.json').read_bytes(),pointer)
        self.assertTrue(self.deliveries[-1][1].is_dir())

    def test_off_and_close_discard_late_publication(self):
        gate=threading.Event();self.server.block=gate
        self.manager.set_enabled(True)
        self.manager.set_enabled(False)
        self.manager.close();gate.set()
        for thread in threading.enumerate():
            if thread.name=='Eloquence dictionary update':thread.join(10)
        self.assertIsNone(self.deliveries[-1][1])
        self.assertFalse((self.cache/'current.json').exists())

    def test_validation_and_corrupt_preferences(self):
        with self.assertRaises(ValueError):DictionarySource('relative.dic',0x40000,0)
        with self.assertRaises(ValueError):DictionarySource(str(self.root/'x'),0,0)
        with self.assertRaises(ValueError):DictionarySource(str(self.root/'x'),0x40000,3)
        self.cache.mkdir(exist_ok=True);(self.cache/'sources.json').write_text('{bad')
        manager=DictionaryManager(R/'data/community',self.cache,lambda r,p:None,self.errors.append)
        self.assertEqual(manager.sources,());self.assertTrue(self.errors)
        manager.close();self.assertEqual((self.cache/'sources.json').read_text(),'{bad')

if __name__=='__main__':unittest.main()
