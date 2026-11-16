"""Ensure NVDA can reuse a verified acoustic build after text-only changes."""
from __future__ import annotations

import json
from pathlib import Path
import subprocess
import sys
import tempfile

SCRIPT = Path(__file__).with_name('select_windows_acoustic_run.py').resolve()


def main():
    with tempfile.TemporaryDirectory(prefix='fa-acoustic-selection-') as temp:
        root = Path(temp)
        def git(*args):
            return subprocess.check_output(['git', *args], cwd=root, text=True, stderr=subprocess.DEVNULL).strip()
        git('init', '-q')
        git('config', 'user.name', 'Test')
        git('config', 'user.email', 'test@example.invalid')
        (root / 'src').mkdir()
        (root / 'src/acoustic.c').write_text('one\n')
        text = root / 'experiments/persian/direct_fa_text.c'
        text.parent.mkdir(parents=True)
        text.write_text('old\n')
        git('add', '.')
        git('commit', '-qm', 'base')
        base = git('rev-parse', 'HEAD')
        text.write_text('new\n')
        git('add', '.')
        git('commit', '-qm', 'text')
        current = git('rev-parse', 'HEAD')
        runs = root / 'runs.json'
        runs.write_text(json.dumps({'workflow_runs': [
            {'id': 2, 'head_sha': current, 'status': 'in_progress', 'conclusion': None},
            {'id': 1, 'head_sha': base, 'status': 'completed', 'conclusion': 'success'},
        ]}))
        def select(*options):
            return subprocess.check_output([sys.executable, '-B', str(SCRIPT), str(runs), *options],
                                           cwd=root, text=True).strip()
        assert select() == '2' and select('--audio-only') == '1'
        (root / 'src/acoustic.c').write_text('two\n')
        git('add', '.')
        git('commit', '-qm', 'acoustic')
        assert select('--audio-only') == '', 'an outdated acoustic build was accepted'
    print('Passed verified acoustic reuse for text-only changes and acoustic-change rejection.')


if __name__ == '__main__':
    main()
