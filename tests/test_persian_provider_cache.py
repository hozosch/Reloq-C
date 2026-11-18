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

    def test_rebuilds_selected_objects_or_rejects_unverified_cache(self):
        for key, diff, reusable in (
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
                names = [f'delta_rules_c{i:02}_fifi.c' for i in range(18)]
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
                (build / 'libreloq61.a').touch()
                (build / 'persian-provider').mkdir()
                with patch.object(cache, 'PROVIDER', provider), patch.object(cache, 'BUILD', build), \
                        patch.object(cache, 'git', return_value=diff), \
                        patch.dict(os.environ, {'PERSIAN_PROVIDER_CACHE_KEY': key}), \
                        contextlib.redirect_stdout(io.StringIO()):
                    cache.main()
                if reusable:
                    self.assertTrue(untouched.exists())
                    self.assertFalse((build / 'libreloq61.a').exists())
                    self.assertFalse(scope_object.exists())
                    self.assertFalse((build / 'persian-provider').exists())
                    self.assertFalse(any((build / 'lang' / 'fifi' /
                                          Path(name).with_suffix('.o')).exists() for name in names))
                else:
                    self.assertFalse(build.exists())


if __name__ == '__main__':
    unittest.main()
