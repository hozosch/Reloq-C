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
        newest = git('rev-parse', 'HEAD')
        runs.write_text(json.dumps({'workflow_runs': [
            {'id': 3, 'head_sha': newest, 'status': 'in_progress', 'conclusion': None},
            {'id': 1, 'head_sha': base, 'status': 'completed', 'conclusion': 'success'},
        ]}))
        assert select('--audio-only') == '3', 'a compatible in-flight build could not be watched'
        runs.write_text(json.dumps({'workflow_runs': [
            {'id': 4, 'head_sha': newest, 'status': 'completed', 'conclusion': 'failure'},
            {'id': 3, 'head_sha': newest, 'status': 'in_progress', 'conclusion': None},
        ]}))
        assert select() == '3', 'a failed embedded-text build was selected'
        runs.write_text(json.dumps({'workflow_runs': [
            {'id': 3, 'head_sha': newest, 'status': 'in_progress', 'conclusion': None},
            {'id': 5, 'head_sha': newest, 'status': 'completed', 'conclusion': 'cancelled'},
        ]}))
        assert select() == '3', 'a cancelled build was accepted as fully verified'
        assert select('--include-cancelled') == '5', 'a compatible early artifact was not considered'
        workflow = root / '.github/workflows/persian-windows-acoustic-probe.yml'
        workflow.parent.mkdir(parents=True)
        workflow.write_text('on:\n  push:\n    branches: [old]\nsteps:\n  - run: make\n')
        git('add', '.')
        git('commit', '-qm', 'workflow')
        checked = git('rev-parse', 'HEAD')
        workflow.write_text('on:\n  push:\n    branches: [old, new]\nsteps:\n  - run: make\n')
        git('add', '.')
        git('commit', '-qm', 'branch filter')
        runs.write_text(json.dumps({'workflow_runs': [
            {'id': 6, 'head_sha': checked, 'status': 'completed', 'conclusion': 'success'},
        ]}))
        assert select() == '', 'a changed workflow was accepted without an explicit policy'
        assert select('--allow-branch-list-change') == '6', 'a branch-only change prevented safe reuse'
        workflow.write_text('on:\n  push:\n    branches: [old, new]\nsteps:\n  - run: false\n')
        git('add', '.')
        git('commit', '-qm', 'changed build')
        assert select('--allow-branch-list-change') == '', 'a changed build recipe was accepted'
        generated = root / 'experiments/persian/direct_fa_pinglish_extra.inc'
        generated.write_text('/* Generated\n * Sources SHA256: ' + 'a' * 64 + ' ' +
                             'b' * 64 + ' ' + 'c' * 64 + '\n */\nint value = 1;\n')
        git('add', '.')
        git('commit', '-qm', 'generated')
        generated_base = git('rev-parse', 'HEAD')
        generated.write_text(generated.read_text().replace('b' * 64, 'd' * 64))
        git('add', '.')
        git('commit', '-qm', 'updated provenance')
        runs.write_text(json.dumps({'workflow_runs': [
            {'id': 7, 'head_sha': generated_base, 'status': 'completed', 'conclusion': 'success'},
        ]}))
        assert select() == '7', 'a digest-only source comment prevented safe reuse'
        generated.write_text(generated.read_text().replace('int value = 1;', 'int value = 2;'))
        git('add', '.')
        git('commit', '-qm', 'changed generated data')
        assert select() == '', 'a changed generated table was accepted'
        assert select('--audio-only') == '7', 'audio-only reuse should exclude text data'
    print('Passed verified acoustic reuse for text-only changes and acoustic-change rejection.')


if __name__ == '__main__':
    main()
