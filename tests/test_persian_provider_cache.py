"""The Windows provider cache must reject any unverified source change."""
from __future__ import annotations

import contextlib
import io
import os
from pathlib import Path
import shutil
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import reuse_persian_provider_cache as cache


class CacheReuseTests(unittest.TestCase):
    def test_dev161_rebuilds_phone_tracing_and_header_dependants(self):
        for changed, accepted in ((cache.DEV161_ALLOWED,True),
                                  (cache.DEV161_ALLOWED|{'include/reloq_persian_frames.h'},False),
                                  (set(),False)):
            with self.subTest(changed=changed), tempfile.TemporaryDirectory() as tmp:
                build=Path(tmp)/'windows-release';build.mkdir()
                names=('src/reloq61_bridge.o','lang/dede61/delta_rules_c24_dede61.o',
                       'lang/enus61/delta_rules_c31_enus61.o','lang/jajp61/delta_rules_c25_jajp61.o',
                       'lang/enus61/dependent.o')
                for name in names:
                    target=build/name;target.parent.mkdir(parents=True,exist_ok=True);target.touch()
                (build/'lang/enus61/dependent.d').write_text('obj: include/reloq_syllable_trace.h')
                with patch.object(cache,'BUILD',build),patch.object(cache,'git',return_value='\n'.join(changed)), \
                        patch.object(cache,'reuse_dev156_generated') as reuse, \
                        patch.dict(os.environ,{'PERSIAN_PROVIDER_CACHE_KEY':cache.DEV161_KEY}), \
                        contextlib.redirect_stdout(io.StringIO()):
                    cache.main()
                self.assertEqual(reuse.called,accepted)
                self.assertEqual(build.exists(),accepted)
                if accepted:
                    reuse.assert_called_once_with(frozenset({'delta_rules_c13_fifi.c'}))
                    self.assertTrue(all(not (build/name).exists() for name in names))

    def test_dev160_cache_requires_verified_liaison_rebuild(self):
        for changed, accepted in ((cache.DEV160_ALLOWED, True),
                                  (cache.DEV160_ALLOWED | {'include/reloq_eci.h'}, False),
                                  (set(), False)):
            with self.subTest(changed=changed), tempfile.TemporaryDirectory() as tmp:
                build = Path(tmp) / 'windows-release'
                build.mkdir()
                with patch.object(cache, 'BUILD', build), patch.object(cache, 'git', return_value='\n'.join(changed)), \
                        patch.object(cache, 'reuse_dev156_generated') as reuse, \
                        patch.dict(os.environ, {'PERSIAN_PROVIDER_CACHE_KEY': cache.DEV160_KEY}), \
                        contextlib.redirect_stdout(io.StringIO()):
                    cache.main()
                self.assertEqual(reuse.called, accepted)
                self.assertEqual(build.exists(), accepted)
                if accepted:
                    reuse.assert_called_once_with(frozenset({'delta_rules_c02_fifi.c', 'delta_rules_c08_fifi.c'}))

    def test_dev159_cache_requires_verified_final_r_rebuild(self):
        for changed, accepted in ((cache.DEV159_ALLOWED, True),
                                  (cache.DEV159_ALLOWED | {'include/reloq_eci.h'}, False),
                                  (set(), False)):
            with self.subTest(changed=changed), tempfile.TemporaryDirectory() as tmp:
                build = Path(tmp) / 'windows-release'
                build.mkdir()
                with patch.object(cache, 'BUILD', build), patch.object(cache, 'git', return_value='\n'.join(changed)), \
                        patch.object(cache, 'reuse_dev156_generated') as reuse, \
                        patch.dict(os.environ, {'PERSIAN_PROVIDER_CACHE_KEY': cache.DEV159_KEY}), \
                        contextlib.redirect_stdout(io.StringIO()):
                    cache.main()
                self.assertEqual(reuse.called, accepted)
                self.assertEqual(build.exists(), accepted)
                if accepted:
                    reuse.assert_called_once_with(frozenset({'delta_rules_c02_fifi.c'}))

    def test_dev158_cache_requires_verified_vowel_rebuild(self):
        for changed, accepted in ((cache.DEV158_ALLOWED, True),
                                  (cache.DEV158_ALLOWED | {'include/reloq_eci.h'}, False),
                                  ({'src/reloq_persian_acoustics.c'}, False)):
            with self.subTest(changed=changed), tempfile.TemporaryDirectory() as tmp:
                build = Path(tmp) / 'windows-release'
                build.mkdir()
                with patch.object(cache, 'BUILD', build), patch.object(cache, 'git', return_value='\n'.join(changed)), \
                        patch.object(cache, 'reuse_dev156_generated') as reuse, \
                        patch.dict(os.environ, {'PERSIAN_PROVIDER_CACHE_KEY': cache.DEV158_KEY}), \
                        contextlib.redirect_stdout(io.StringIO()):
                    cache.main()
                self.assertEqual(reuse.called, accepted)
                self.assertEqual(build.exists(), accepted)
                if accepted:
                    reuse.assert_called_once_with(frozenset({'delta_rules_c02_fifi.c', 'delta_rules_c08_fifi.c'}))

    def test_dev157_cache_requires_checked_two_source_rebuild(self):
        for changed, accepted in ((cache.DEV157_ALLOWED, True),
                                  (cache.DEV157_ALLOWED | {'include/reloq_eci.h'}, False),
                                  ({'src/reloq_persian_acoustics.c'}, False)):
            with self.subTest(changed=changed), tempfile.TemporaryDirectory() as tmp:
                build = Path(tmp) / 'windows-release'
                build.mkdir()
                with patch.object(cache, 'BUILD', build), patch.object(cache, 'git', return_value='\n'.join(changed)), \
                        patch.object(cache, 'reuse_dev156_generated') as reuse, \
                        patch.dict(os.environ, {'PERSIAN_PROVIDER_CACHE_KEY': cache.DEV157_KEY}), \
                        contextlib.redirect_stdout(io.StringIO()):
                    cache.main()
                self.assertEqual(reuse.called, accepted)
                self.assertEqual(build.exists(), accepted)
                if accepted:
                    reuse.assert_called_once_with(frozenset({'delta_rules_c02_fifi.c', 'delta_rules_c08_fifi.c'}))

    def test_incremental_generator_reuse_checks_each_source(self):
        for altered in (False, True):
            with self.subTest(altered=altered), tempfile.TemporaryDirectory() as tmp:
                build = Path(tmp) / 'windows-release'
                generated = build / 'persian-provider'
                generated.mkdir(parents=True)
                names = [f'delta_rules_c{i:02}_fifi.c' for i in range(21)]
                for name in names:
                    (generated / name).write_text('original:' + name, encoding='utf8')
                (generated / '.stamp').touch()
                objs = build / 'lang/fifi'
                objs.mkdir(parents=True)
                unchanged, changed = objs / 'delta_rules_c01_fifi.o', objs / 'delta_rules_c02_fifi.o'
                unchanged.touch(); changed.touch()
                (build / 'reloq_persian_acoustics.o').touch()
                (build / 'libreloq61.a').touch()
                def generate(command, check):
                    target = Path(command[-1])
                    shutil.copytree(generated, target)
                    (target / 'delta_rules_c02_fifi.c').write_text('new R', encoding='utf8')
                    if altered:
                        (target / 'delta_rules_c01_fifi.c').write_text('unexpected', encoding='utf8')
                with patch.object(cache, 'BUILD', build), patch.object(cache.subprocess, 'run', side_effect=generate), \
                        contextlib.redirect_stdout(io.StringIO()):
                    if altered:
                        with self.assertRaises(ValueError):
                            cache.reuse_dev156_generated()
                    else:
                        cache.reuse_dev156_generated()
                        self.assertTrue(unchanged.exists())
                        self.assertFalse(changed.exists())
                        self.assertFalse((build / 'libreloq61.a').exists())
                        self.assertFalse((build / 'reloq_persian_acoustics.o').exists())
                        self.assertEqual((generated / 'delta_rules_c02_fifi.c').read_text(), 'new R')
                        self.assertGreaterEqual(unchanged.stat().st_mtime_ns,
                                                (generated / 'delta_rules_c01_fifi.c').stat().st_mtime_ns)

    def test_dev156_reuse_requires_exact_inputs(self):
        for changed, accepted in ((cache.DEV156_ALLOWED, True),
                                  (cache.DEV156_ALLOWED | {'include/reloq_eci.h'}, False),
                                  ({'src/reloq_persian_acoustics.c'}, False)):
            with self.subTest(changed=changed), tempfile.TemporaryDirectory() as tmp:
                build = Path(tmp) / 'windows-release'
                build.mkdir()
                with patch.object(cache, 'BUILD', build), patch.object(cache, 'git', return_value='\n'.join(changed)), \
                        patch.object(cache, 'reuse_dev156_generated') as reuse, \
                        patch.dict(os.environ, {'PERSIAN_PROVIDER_CACHE_KEY': cache.DEV156_KEY}), \
                        contextlib.redirect_stdout(io.StringIO()):
                    cache.main()
                self.assertEqual(reuse.called, accepted)
                self.assertEqual(build.exists(), accepted)

    def test_dev153_cache_rejects_modified_build_recipe(self):
        with tempfile.TemporaryDirectory() as tmp:
            build = Path(tmp) / 'windows-release'
            build.mkdir()
            def git_result(*args):
                if args[0] == 'rev-parse':
                    return 'unverified-blob'
                return '\n'.join(cache.DEV153_ALLOWED)
            with patch.object(cache, 'BUILD', build), patch.object(cache, 'git', side_effect=git_result), \
                    patch.dict(os.environ, {'PERSIAN_PROVIDER_CACHE_KEY': cache.DEV153_KEY}), \
                    contextlib.redirect_stdout(io.StringIO()):
                cache.main()
            self.assertFalse(build.exists())

    def test_recent_cache_requires_exactly_checked_headers(self):
        self.assertTrue(cache.eligible_recent(cache.RECENT_KEY, cache.RECENT_HEADERS,
                                              cache.RECENT_HEADERS))
        self.assertFalse(cache.eligible_recent(cache.RECENT_KEY, (), cache.RECENT_HEADERS))
        self.assertFalse(cache.eligible_recent(cache.RECENT_KEY,
                         set(cache.RECENT_HEADERS) | {'vendor/openevv61/Makefile.native'},
                         cache.RECENT_HEADERS))
        self.assertFalse(cache.eligible_recent(cache.RECENT_KEY, cache.RECENT_HEADERS,
                         {**cache.RECENT_HEADERS, 'include/reloq_eci.h': 'different'}))

    def test_host_cache_tracks_embedded_persian_inputs(self):
        workflow = (Path(__file__).resolve().parents[1] /
                    '.github/workflows/persian-windows-acoustic-probe.yml').read_text(encoding='utf8')
        host_key = next(line for line in workflow.splitlines() if 'key: persian-windows-host-' in line)
        for extension in ('c', 'h', 'inc'):
            self.assertIn('experiments/persian/direct_fa_*.' + extension, host_key)
        for source in ('tools/prepare_persian_provider.py',
                       'experiments/persian/build_native_probe.py',
                       'experiments/aragonese/build_native_probe.py'):
            self.assertIn(source, host_key)

    def test_restore_key_count_fits_actions_cache_limit(self):
        workflow = (Path(__file__).resolve().parents[1] /
                    '.github/workflows/persian-windows-acoustic-probe.yml').read_text(encoding='utf8')
        for kind in ('provider', 'host'):
            keys = [line for line in workflow.splitlines()
                    if line.strip().startswith('persian-windows-' + kind + '-')]
            # The primary key also counts toward the service limit of ten.
            self.assertLessEqual(len(keys), 9, kind)

    def test_rebuilds_selected_objects_or_rejects_unverified_cache(self):
        for key, diff, reusable in (
            (cache.DEV155_KEY, '\n'.join(cache.DEV155_ALLOWED), True),
            (cache.DEV155_KEY, '\n'.join(cache.DEV155_ALLOWED | {'include/reloq_eci.h'}), False),
            (cache.DEV155_KEY, '', False),
            (cache.DEV154_KEY, '\n'.join(cache.DEV154_ALLOWED), True),
            (cache.DEV154_KEY, '\n'.join(cache.DEV154_ALLOWED | {'include/reloq_eci.h'}), False),
            (cache.DEV154_KEY, 'src/reloq_persian_acoustics.c', False),
            (cache.DEV153_KEY, '\n'.join(cache.DEV153_ALLOWED), True),
            (cache.DEV153_KEY, '\n'.join(cache.DEV153_ALLOWED | {'include/reloq_eci.h'}), False),
            (cache.DEV153_KEY, 'src/reloq_persian_acoustics.c', False),
            (cache.DEV152_KEY, '\n'.join(cache.DEV152_ALLOWED), True),
            (cache.DEV152_KEY, '\n'.join(cache.DEV152_ALLOWED | {'vendor/openevv61/Makefile.native'}), False),
            (cache.DEV152_KEY, 'src/reloq_persian_acoustics.c', False),
            (cache.DEV151_KEY, '\n'.join(cache.DEV151_ALLOWED), True),
            (cache.DEV151_KEY, '\n'.join(cache.DEV151_ALLOWED | {'vendor/openevv61/Makefile.native'}), False),
            (cache.DEV151_KEY, 'src/reloq_persian_acoustics.c', False),
            (cache.DEV150_KEY, '\n'.join(cache.DEV150_ALLOWED), True),
            (cache.DEV150_KEY, '\n'.join(cache.DEV150_ALLOWED | {'vendor/openevv61/Makefile.native'}), False),
            (cache.DEV150_KEY, 'src/reloq_persian_acoustics.c', False),
            (cache.DEV149_KEY, '\n'.join(cache.DEV149_ALLOWED), True),
            (cache.DEV149_KEY, '\n'.join(cache.DEV149_ALLOWED | {'vendor/openevv61/Makefile.native'}), False),
            (cache.DEV149_KEY, 'include/reloq_syllable_trace.h', False),
            (cache.TRACE_KEY, '\n'.join(cache.TRACE_ALLOWED), True),
            (cache.TRACE_KEY, '\n'.join(cache.TRACE_ALLOWED | {'vendor/openevv61/Makefile.native'}), False),
            (cache.PREVIOUS_KEY, 'experiments/persian/build_native_probe.py', True),
            (cache.PREVIOUS_KEY, 'experiments/persian/build_native_probe.py\nsrc/reloq_persian_acoustics.c', True),
            (cache.PREVIOUS_KEY, 'src/reloq_persian_acoustics.h', False),
            (cache.PREVIOUS_KEY, 'include/reloq_eci.h', False),
            ('unverified-key', 'experiments/persian/build_native_probe.py', False),
        ):
            with self.subTest(key=key, diff=diff), tempfile.TemporaryDirectory() as tmp:
                root = Path(tmp)
                provider = root / 'vendor'
                build = provider / 'build' / 'windows-release'
                names = [f'delta_rules_c{i:02}_fifi.c' for i in range(21)]
                (provider / 'lang' / 'fifi').mkdir(parents=True)
                (provider / 'Makefile.native').write_text(
                    'PERSIAN_NAMES := ' + ' '.join(names) + '\n', encoding='utf8')
                (build / 'lang' / 'fifi').mkdir(parents=True)
                for name in names:
                    (provider / 'lang' / 'fifi' / name).touch()
                    (build / 'lang' / 'fifi' / Path(name).with_suffix('.o')).touch()
                untouched = build / 'lang' / 'fifi' / 'unrelated.o'
                untouched.touch()
                scope_object = build / 'reloq_persian_acoustics.o'
                scope_object.touch()
                stream_object = build / 'src' / 'eci' / 'bridge' / 'eci_stmarray.o'
                stream_object.parent.mkdir(parents=True)
                stream_object.touch()
                japanese_objects = [build / 'lang' / 'jajp61' / f'delta_rules_c{part}_jajp61.o'
                                    for part in ('17', '30')]
                japanese_objects[0].parent.mkdir(parents=True)
                for obj in japanese_objects:
                    obj.touch()
                (build / 'libreloq61.a').touch()
                (build / 'persian-provider').mkdir()
                def git_result(*args):
                    if args[0] == 'rev-parse':
                        return cache.DEV153_CHECKED[args[1].removeprefix('HEAD:')]
                    return diff
                with patch.object(cache, 'PROVIDER', provider), patch.object(cache, 'BUILD', build), \
                        patch.object(cache, 'git', side_effect=git_result), \
                        patch.dict(os.environ, {'PERSIAN_PROVIDER_CACHE_KEY': key}), \
                        contextlib.redirect_stdout(io.StringIO()):
                    cache.main()
                if reusable:
                    self.assertTrue(untouched.exists())
                    self.assertFalse((build / 'libreloq61.a').exists())
                    self.assertFalse(scope_object.exists())
                    self.assertEqual(stream_object.exists(), key != cache.DEV154_KEY)
                    self.assertFalse((build / 'persian-provider').exists())
                    self.assertEqual([obj.exists() for obj in japanese_objects],
                                     [key != cache.DEV152_KEY] * 2)
                    self.assertFalse(any((build / 'lang' / 'fifi' /
                                          Path(name).with_suffix('.o')).exists() for name in names))
                else:
                    self.assertFalse(build.exists())


if __name__ == '__main__':
    unittest.main()
