"""Choose the newest Windows probe built from this acoustic source tree."""
from __future__ import annotations

import argparse
import json
from pathlib import Path
import subprocess

PATHS = ('eci/Makefile', 'src', 'include', 'vendor/openevv61',
         'experiments/aragonese/build_native_probe.py',
         'experiments/persian/build_native_probe.py',
         'experiments/persian/build_windows_probe.py',
         'experiments/persian/prepare_continuous_host.py',
         'experiments/persian/continuous_frames.c',
         'experiments/persian/continuous_frames.h',
         'experiments/persian/continuous_replay.inc',
         'experiments/persian/scoped_acoustics_probe.c',
         'experiments/persian/direct_fa_*.c',
         'experiments/persian/direct_fa_*.h',
         'experiments/persian/direct_fa_*.inc',
         '.github/workflows/persian-windows-acoustic-probe.yml')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('runs', type=Path)
    parser.add_argument('--audio-only', action='store_true',
                        help='ignore embedded text helpers when the package builds its own text DLL')
    args = parser.parse_args()
    # The NVDA package builds and tests fa-direct.dll from the current text
    # sources separately; it uses this artifact only for its acoustic ECI
    # handle. Keep the stricter full-source match as the default for anyone
    # using the probe DLL's own experimental text exports.
    paths = tuple(p for p in PATHS if not (args.audio_only and
                                           p.startswith('experiments/persian/direct_fa_')))
    runs = json.loads(args.runs.read_text(encoding='utf8'))['workflow_runs']
    pending = None
    for run in runs:
        # Prefer an already verified acoustic artifact over a newer build
        # whose only changes concern the separately built text DLL. If no
        # compatible completed build exists, return an in-flight match so the
        # package workflow can watch it for as long as the build needs.
        if args.audio_only and run.get('status') == 'completed' and run.get('conclusion') != 'success':
            continue
        sha = run['head_sha']
        if len(sha) != 40 or any(c not in '0123456789abcdef' for c in sha):
            continue
        result = subprocess.run(['git', 'diff', '--quiet', sha, 'HEAD', '--', *paths],
                                check=False, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        if result.returncode == 0:
            if not args.audio_only or run.get('status') == 'completed':
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
