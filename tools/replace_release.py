#!/usr/bin/env python3
"""Replace a published release only after its new assets have been verified."""
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile


def gh(*args, input_text=None):
    return subprocess.check_output(['gh', *args], input=input_text, text=True)


def api(method, endpoint, body=None):
    args = ['api', '--method', method, endpoint]
    if body is not None:
        args += ['--input', '-']
    result = gh(*args, input_text=None if body is None else json.dumps(body))
    return json.loads(result) if result.strip() else None


def digest(path):
    with path.open('rb') as stream:
        return 'sha256:' + hashlib.file_digest(stream, 'sha256').hexdigest()


def metadata_matches(asset, metadata):
    return (asset['size'] == metadata['size'] and
            asset['digest'] == 'sha256:' + metadata['sha256'])


def replace(release_id, directory):
    repo, commit = os.environ['GITHUB_REPOSITORY'], os.environ['GITHUB_SHA']
    request = json.loads(Path('.github/release.json').read_text())
    tag = 'v' + request['release_version']
    release_endpoint = f'repos/{repo}/releases/{release_id}'
    ref_endpoint = f'repos/{repo}/git/ref/tags/{tag}'
    update_ref_endpoint = f'repos/{repo}/git/refs/tags/{tag}'
    asset_endpoint = f'repos/{repo}/releases/assets/'
    original = api('GET', release_endpoint)
    original_ref = api('GET', ref_endpoint)['object']
    if (original['draft'] or original['prerelease'] or original['tag_name'] != tag or
            original_ref['type'] != 'commit'):
        raise ValueError('expected a published stable release with a lightweight tag')
    subprocess.run(['git', 'merge-base', '--is-ancestor', original_ref['sha'], commit], check=True)
    wanted = request['assets']
    old = {asset['name']: asset for asset in original['assets']}
    if set(old) != set(wanted):
        raise ValueError('existing release does not contain exactly the expected assets')
    staged_names = {name: 'replacement-' + name for name in wanted}

    with tempfile.TemporaryDirectory(prefix='reloq-release-') as temp:
        backup, staging = Path(temp) / 'backup', Path(temp) / 'staging'
        backup.mkdir();staging.mkdir()
        gh('release', 'download', tag, '--repo', repo, '--dir', str(backup))
        for name, asset in old.items():
            path = backup / name
            if not path.is_file() or path.stat().st_size != asset['size'] or digest(path) != asset['digest']:
                raise ValueError(f'could not verify existing asset backup: {name}')
        for name, metadata in wanted.items():
            path = directory / name
            if path.stat().st_size != metadata['size'] or not metadata_matches(
                    {'size': path.stat().st_size, 'digest': digest(path)}, metadata):
                raise ValueError(f'new asset does not match its manifest: {name}')
            shutil.copyfile(path, staging / staged_names[name])

        try:
            # Staged names keep the original extensions and MIME types. The
            # old downloads remain available until all new uploads verify.
            gh('release', 'upload', tag, *[str(staging / staged_names[n]) for n in wanted],
               '--repo', repo)
            current = {asset['name']: asset for asset in api('GET', release_endpoint)['assets']}
            if set(current) != set(old) | set(staged_names.values()):
                raise ValueError('unexpected asset set after staging')
            for name, metadata in wanted.items():
                if not metadata_matches(current[staged_names[name]], metadata):
                    raise ValueError(f'staged asset digest differs: {name}')

            for name in wanted:
                api('DELETE', asset_endpoint + str(current[name]['id']))
                api('PATCH', asset_endpoint + str(current[staged_names[name]]['id']), {'name': name})
            current = {asset['name']: asset for asset in api('GET', release_endpoint)['assets']}
            if set(current) != set(wanted) or any(not metadata_matches(current[n], m) for n, m in wanted.items()):
                raise ValueError('published asset verification failed')

            ref = api('GET', ref_endpoint)['object']
            if ref != original_ref:
                raise ValueError('release tag changed during publication')
            api('PATCH', update_ref_endpoint, {'sha': commit, 'force': False})
            body = Path(f'docs/releases/{request["release_version"]}.md').read_text()
            api('PATCH', release_endpoint, {'body': body, 'target_commitish': commit, 'make_latest': 'true'})
            final = api('GET', release_endpoint)
            if final['draft'] or final['body'] != body or api('GET', ref_endpoint)['object']['sha'] != commit:
                raise ValueError('release metadata or tag verification failed')
        except Exception:
            # Restore downloaded originals if replacement is interrupted.
            # Leave other releases and any unrelated assets untouched.
            try:
                current = {asset['name']: asset for asset in api('GET', release_endpoint)['assets']}
                for name, asset in old.items():
                    present = current.get(name)
                    if present and present['size'] == asset['size'] and present['digest'] == asset['digest']:
                        continue
                    if present:
                        api('DELETE', asset_endpoint + str(present['id']))
                    gh('release', 'upload', tag, str(backup / name), '--repo', repo)
                current = {asset['name']: asset for asset in api('GET', release_endpoint)['assets']}
                for name in staged_names.values():
                    if name in current:
                        api('DELETE', asset_endpoint + str(current[name]['id']))
                ref = api('GET', ref_endpoint)['object']
                if ref['sha'] == commit and ref != original_ref:
                    api('PATCH', update_ref_endpoint, {'sha': original_ref['sha'], 'force': True})
                latest = api('GET', release_endpoint)
                if latest['body'] != original['body'] or latest['target_commitish'] != original['target_commitish']:
                    api('PATCH', release_endpoint, {'body': original['body'],
                        'target_commitish': original['target_commitish'], 'make_latest': 'true'})
            except Exception as rollback_error:
                print(f'Could not fully restore the original release: {rollback_error}', file=sys.stderr)
            raise
    print(f'Updated {tag} with three verified assets and matching source')


if __name__ == '__main__':
    replace(int(sys.argv[1]), Path(sys.argv[2]))
