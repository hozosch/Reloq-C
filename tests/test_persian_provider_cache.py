"""The Windows provider cache must reject any unverified source change."""
from __future__ import annotations

import contextlib
import io
import os
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import reuse_persian_provider_cache as cache


class CacheReuseTests(unittest.TestCase):
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
            (cache.DEV156_KEY, '\n'.join(cache.DEV156_ALLOWED), True),
            (cache.DEV156_KEY, '\n'.join(cache.DEV156_ALLOWED | {'include/reloq_eci.h'}), False),
            (cache.DEV156_KEY, 'src/reloq_persian_acoustics.c', False),
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
