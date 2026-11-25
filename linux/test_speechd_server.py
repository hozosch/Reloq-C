#!/usr/bin/env python3
"""Exercise the module through an isolated Speech Dispatcher server."""
import os
from pathlib import Path
import subprocess
import tempfile
import time


ROOT = Path(__file__).resolve().parents[1]


def main():
    with tempfile.TemporaryDirectory(prefix='reloq-speechd-server-') as temp:
        root = Path(temp)
        config, runtime, logs = (root / name for name in ('config', 'runtime', 'logs'))
        for path in (config, runtime, logs):
            path.mkdir()
        socket = runtime / 'speechd.sock'
        (config / 'speechd.conf').write_text(
            'LogLevel 5\nAudioOutputMethod "alsa"\nAudioALSADevice "null"\n'
            f'AddModule "reloq" "{ROOT / "eci/sd_reloq"}" "{ROOT / "speechd/reloq.conf"}"\n'
            'DefaultModule reloq\n', encoding='utf-8')
        environment = {key: value for key, value in os.environ.items() if not key.startswith('XDG_')}
        environment.update(HOME=str(root), XDG_RUNTIME_DIR=str(runtime), LC_ALL='C',
                           SPEECHD_ADDRESS=f'unix_socket:{socket}')
        server_output = (root / 'server-output.log').open('wb')
        server = subprocess.Popen(['speech-dispatcher', '-s', '-c', 'unix_socket', '-S', str(socket),
                                   '-C', str(config), '-P', str(runtime / 'speechd.pid'),
                                   '-L', str(logs), '-t', '0'], env=environment,
                                  stdout=server_output, stderr=subprocess.STDOUT)
        try:
            deadline = time.monotonic() + 20
            while not socket.exists() and time.monotonic() < deadline:
                if server.poll() is not None:
                    raise RuntimeError('Speech Dispatcher exited during startup')
                time.sleep(0.05)
            if not socket.exists():
                raise RuntimeError('Speech Dispatcher did not create its socket')
            voices = subprocess.run(['spd-say', '-o', 'reloq', '-l', 'en-US', '-L'],
                                    env=environment, capture_output=True, text=True, timeout=30)
            if voices.returncode or 'Reed (American English)' not in voices.stdout:
                raise RuntimeError('ReLoq voices were not available: ' + voices.stdout + voices.stderr)
            result = subprocess.run(['spd-say', '-w', '-o', 'reloq', '-l', 'en-US',
                                     'The ReLoq voice is ready.'], env=environment,
                                    capture_output=True, text=True, timeout=60)
            if result.returncode or 'Invalid output module' in result.stdout:
                raise RuntimeError(result.stdout + result.stderr)
        except Exception:
            server_output.flush()
            print((root / 'server-output.log').read_text(errors='replace')[-12000:])
            for path in logs.rglob('*'):
                if path.is_file():
                    print(f'{path.name}:\n{path.read_text(errors="replace")[-12000:]}')
            raise
        finally:
            server.terminate()
            try:
                server.wait(timeout=10)
            except subprocess.TimeoutExpired:
                server.kill()
                server.wait()
            server_output.close()
    print('Speech Dispatcher server: module loaded and speech completed')


if __name__ == '__main__':
    main()
