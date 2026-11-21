"""Choose the newest Windows probe built from this acoustic source tree."""
from __future__ import annotations

import argparse
import json
from pathlib import Path
import re
import subprocess

PATHS = ('eci/Makefile', 'src', 'include', 'vendor/openevv61',
         'tools/prepare_persian_provider.py',
         'tools/reuse_persian_provider_cache.py',
         'experiments/aragonese/build_native_probe.py',
         'experiments/persian/build_native_probe.py',
         'experiments/persian/build_windows_probe.py',
         'experiments/persian/prepare_continuous_host.py',
         'experiments/persian/continuous_frames.c',
         'experiments/persian/continuous_frames.h',
         'experiments/persian/continuous_replay.inc',
         'experiments/persian/scoped_acoustics_probe.c',
         'experiments/persian/test_embedded_text_rules.py',
         'eci/test_persian_project.py', 'eci/test_persian_frames.py', 'eci/test_persian_acoustics.py',
         'eci/test_concurrent_startup.py',
         'tests/test_low_arena_concurrent.c',
         'tests/test_reloq_classic_noise.c',
         'experiments/persian/direct_fa_*.c',
         'experiments/persian/direct_fa_*.h',
         'experiments/persian/direct_fa_*.inc',
         '.github/workflows/persian-windows-acoustic-probe.yml')

PINGLISH_EXTRA = 'experiments/persian/direct_fa_pinglish_extra.inc'


def matching_pinglish_extra(sha):
    """Only the generated source-digest comment may differ across builds."""
    def read(ref):
        result = subprocess.run(['git', 'show', f'{ref}:{PINGLISH_EXTRA}'],
                                check=False, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
        return result.stdout if result.returncode == 0 else None
    old, current = read(sha), read('HEAD')
    if old == current:
        return True
    if old is None or current is None:
        return False
    pattern = rb'(?m)^ \* Sources SHA256: [0-9a-f]{64} [0-9a-f]{64} [0-9a-f]{64}\r?$'
    old, old_count = re.subn(pattern, b' * Sources SHA256: <digest>', old)
    current, current_count = re.subn(pattern, b' * Sources SHA256: <digest>', current)
    return old_count == current_count == 1 and old == current


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('runs', type=Path)
    parser.add_argument('--audio-only', action='store_true',
                        help='ignore embedded text helpers when the package builds its own text DLL')
    parser.add_argument('--include-cancelled', action='store_true',
                        help='allow a source-matching cancelled run for its early artifact only')
    parser.add_argument('--allow-branch-list-change', action='store_true',
                        help='allow only the workflow push branch list to differ')
    args = parser.parse_args()
    # The NVDA package uses the probe's own text exports, so it must use the
    # default full-source match. Audio-only remains useful for comparisons
    # that explicitly supply a separate text library.
    workflow = '.github/workflows/persian-windows-acoustic-probe.yml'
    paths = tuple(p for p in PATHS if not (args.audio_only and
                                           p.startswith('experiments/persian/direct_fa_')) and
                  not (args.allow_branch_list_change and p == workflow))
    def matching_workflow(sha):
        if not args.allow_branch_list_change:
            return True
        def read(ref):
            result = subprocess.run(['git', 'show', f'{ref}:{workflow}'], check=False,
                                    stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True)
            if result.returncode:
                return None
            # Branch filters change when a test branch is added. Require the
            # rest of the build and validation workflow to match byte-for-byte.
            value, count = re.subn(r'^[ \t]*branches:[ \t]*\[[^\r\n]*\][ \t]*\r?$', '    branches: []',
                                   result.stdout, flags=re.MULTILINE)
            return value if count == 1 else None
        old, current = read(sha), read('HEAD')
        return old is not None and old == current
    runs = json.loads(args.runs.read_text(encoding='utf8'))['workflow_runs']
    pending = None
    for run in runs:
        # Prefer an already verified compatible artifact. If none exists,
        # return an in-flight match so the caller can wait for its tests.
        if (run.get('status') == 'completed' and run.get('conclusion') != 'success' and
                not (args.include_cancelled and run.get('conclusion') == 'cancelled')):
            continue
        sha = run['head_sha']
        if len(sha) != 40 or any(c not in '0123456789abcdef' for c in sha):
            continue
        result = subprocess.run(['git', 'diff', '--quiet', sha, 'HEAD', '--', *paths,
                                 f':(exclude){PINGLISH_EXTRA}'],
                                check=False, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        if (result.returncode == 0 and (args.audio_only or matching_pinglish_extra(sha))
                and matching_workflow(sha)):
            if run.get('status') == 'completed':
                print(int(run['id']))
                return
            if pending is None:
                pending = int(run['id'])
        if result.returncode not in (0, 1):
            raise RuntimeError('cannot compare an acoustic workflow revision')
    if pending is not None:
        print(pending)


if __name__ == '__main__':
    main()
