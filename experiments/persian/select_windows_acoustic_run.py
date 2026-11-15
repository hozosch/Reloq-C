"""Choose the newest Windows probe built from this acoustic source tree."""
from __future__ import annotations

import json
from pathlib import Path
import subprocess
import sys

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
    runs = json.loads(Path(sys.argv[1]).read_text(encoding='utf8'))['workflow_runs']
    for run in runs:
        sha = run['head_sha']
        if len(sha) != 40 or any(c not in '0123456789abcdef' for c in sha):
            continue
        result = subprocess.run(['git', 'diff', '--quiet', sha, 'HEAD', '--', *PATHS],
                                check=False, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        if result.returncode == 0:
            print(int(run['id']))
            return
        if result.returncode != 1:
            raise RuntimeError('cannot compare an acoustic workflow revision')


if __name__ == '__main__':
    main()
