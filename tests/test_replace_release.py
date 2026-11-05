"""Check published-release replacement and rollback without contacting GitHub."""
import copy
import hashlib
import json
import os
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import replace_release


class FakeGitHub:
    def __init__(self, old_files, fail_at=None):
        self.release = {'draft': False, 'prerelease': False, 'tag_name': 'v1.1.2',
                        'body': 'old notes', 'target_commitish': 'old-commit'}
        self.ref = {'type': 'commit', 'sha': 'old-commit', 'url': 'old-url'}
        self.files = {}
        self.next_id = 1
        self.fail_at = fail_at
        for name, data in old_files.items():
            self.upload(name, data)

    def upload(self, name, data):
        assert all(asset['name'] != name for asset in self.files.values())
        self.files[self.next_id] = {'id': self.next_id, 'name': name, 'data': data,
            'size': len(data), 'digest': 'sha256:' + hashlib.sha256(data).hexdigest()}
        self.next_id += 1

    def api(self, method, endpoint, body=None):
        if self.fail_at == (method, endpoint.rsplit('/', 1)[-1]):
            self.fail_at = None
            raise RuntimeError('simulated publication failure')
        if '/git/ref/tags/' in endpoint:
            return {'object': copy.deepcopy(self.ref)}
        if '/git/refs/tags/' in endpoint:
            self.ref['sha'] = body['sha']
            return {}
        if '/releases/assets/' in endpoint:
            number = int(endpoint.rsplit('/', 1)[-1])
            if method == 'DELETE':
                del self.files[number]
            else:
                self.files[number]['name'] = body['name']
            return {}
        if method == 'PATCH':
            self.release.update(body)
        release = copy.deepcopy(self.release)
        release['assets'] = [{k: v for k, v in asset.items() if k != 'data'}
                             for asset in self.files.values()]
        return release

    def gh(self, *args, input_text=None):
        assert args[0] == 'release'
        if args[1] == 'download':
            target = Path(args[args.index('--dir') + 1])
            for asset in self.files.values():
                (target / asset['name']).write_bytes(asset['data'])
        else:
            assert args[1] == 'upload'
            for name in args[3:args.index('--repo')]:
                path = Path(name)
                self.upload(path.name, path.read_bytes())
        return ''

    def contents(self):
        return {asset['name']: asset['data'] for asset in self.files.values()}


class ReplaceReleaseTest(unittest.TestCase):
    def run_case(self, failure=None):
        old_files = {'a.exe': b'old exe', 'b.apk': b'old apk', 'c.nvda-addon': b'old addon'}
        new_files = {name: b'new ' + data for name, data in old_files.items()}
        fake = FakeGitHub(old_files, failure)
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            (root / '.github').mkdir()
            (root / 'docs/releases').mkdir(parents=True)
            (root / 'packages').mkdir()
            (root / 'docs/releases/1.1.2.md').write_text('new notes')
            manifest = {'release_version': '1.1.2', 'assets': {}}
            for name, data in new_files.items():
                (root / 'packages' / name).write_bytes(data)
                manifest['assets'][name] = {'size': len(data), 'sha256': hashlib.sha256(data).hexdigest()}
            (root / '.github/release.json').write_text(json.dumps(manifest))
            previous = Path.cwd()
            try:
                os.chdir(root)
                with patch.dict(os.environ, {'GITHUB_REPOSITORY': 'owner/repo', 'GITHUB_SHA': 'new-commit'}), \
                     patch.object(replace_release, 'api', fake.api), \
                     patch.object(replace_release, 'gh', fake.gh), \
                     patch.object(replace_release.subprocess, 'run'):
                    if failure:
                        with self.assertRaises(RuntimeError):
                            replace_release.replace(1, root / 'packages')
                    else:
                        replace_release.replace(1, root / 'packages')
            finally:
                os.chdir(previous)
        if failure:
            self.assertEqual(fake.contents(), old_files)
            self.assertEqual(fake.ref['sha'], 'old-commit')
            self.assertEqual(fake.release['body'], 'old notes')
        else:
            self.assertEqual(fake.contents(), new_files)
            self.assertEqual(fake.ref['sha'], 'new-commit')
            self.assertEqual(fake.release['body'], 'new notes')

    def test_success(self):
        self.run_case()

    def test_restore_after_asset_was_removed(self):
        self.run_case(('PATCH', '5'))

    def test_restore_after_tag_was_moved(self):
        self.run_case(('PATCH', '1'))


if __name__ == '__main__':
    unittest.main()
