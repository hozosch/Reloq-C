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
    def test_host_cache_tracks_embedded_persian_inputs(self):
        workflow = (Path(__file__).resolve().parents[1] /
                    '.github/workflows/persian-windows-acoustic-probe.yml').read_text(encoding='utf8')
        host_key = next(line for line in workflow.splitlines() if 'key: persian-windows-host-' in line)
        for extension in ('c', 'h', 'inc'):
            self.assertIn('experiments/persian/direct_fa_*.' + extension, host_key)

    def test_rebuilds_selected_objects_or_rejects_unverified_cache(self):
        for key, diff, reusable in (
            (cache.PREVIOUS_KEY, 'experiments/persian/build_native_probe.py', True),
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
                    self.assertFalse((build / 'persian-provider').exists())
                    self.assertFalse(any((build / 'lang' / 'fifi' /
                                          Path(name).with_suffix('.o')).exists() for name in names))
                else:
                    self.assertFalse(build.exists())


if __name__ == '__main__':
    unittest.main()
