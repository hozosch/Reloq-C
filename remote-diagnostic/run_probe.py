"""Connect a simulated peer to one real NVDA Remote instance on localhost."""
import argparse
import json
import os
from pathlib import Path
import shutil
import socket
import ssl
import subprocess
import tempfile
import time
import zipfile


def wait_for(predicate, deadline, message):
    while time.monotonic() < deadline:
        value = predicate()
        if value:
            return value
        time.sleep(.1)
    raise RuntimeError(message)


def read_events(root):
    path = root / 'events.jsonl'
    if not path.exists():
        return []
    return [json.loads(line) for line in path.read_text(encoding='utf-8').splitlines() if line]


def run(role, exe, addon, plugin, root):
    work = root / role
    if work.exists(): shutil.rmtree(work)
    work.mkdir(parents=True)
    profile = work / 'config'
    target = profile / 'addons' / 'reloqC'
    target.mkdir(parents=True)
    with zipfile.ZipFile(addon) as z:
        assert z.testzip() is None
        z.extractall(target)
    shutil.copy2(plugin, target / 'globalPlugins' / 'reloqRemoteProbe.py')
    (profile / 'nvda.ini').write_text('[general]\nplayStartAndExitSounds = False\n[speech]\nsynth = silence\n[remote]\nenabled = True\n', encoding='utf-8')
    with socket.socket() as probe:
        probe.bind(('127.0.0.1', 0))
        port = probe.getsockname()[1]
    env = dict(os.environ, RELOQ_PROBE_DIR=str(work), RELOQ_PROBE_ROLE=role, RELOQ_PROBE_PORT=str(port))
    process = subprocess.Popen([str(exe), '--minimal', '--no-sr-flag', '--debug-logging',
        '--config-path='+str(profile), '--log-file='+str(work/'nvda.log')], env=env)
    try:
        def initialized():
            if process.poll() is not None:
                raise RuntimeError(f'NVDA exited early: {process.returncode}')
            events = read_events(work)
            errors = [e for e in events if e['kind']=='error']
            if errors: raise RuntimeError(errors[-1]['detail'])
            return next((e for e in events if e['kind']=='server'), None)
        ready = wait_for(initialized, time.monotonic()+90, 'NVDA Remote startup timed out')
        context = ssl._create_unverified_context()
        with context.wrap_socket(socket.create_connection(('127.0.0.1', port), timeout=10), server_hostname='localhost') as peer:
            peer.settimeout(.5)
            def send(data): peer.sendall(json.dumps(data).encode()+b'\n')
            send({'type':'protocol_version','version':2})
            send({'type':'join','channel':'reloq-loopback-test',
                  'connection_type':'master' if role=='follower' else 'slave'})
            messages = []
            buffer = b''
            sent = False
            deadline = time.monotonic()+50
            while time.monotonic() < deadline:
                try:
                    data = peer.recv(65536)
                    if not data: break
                    buffer += data
                    while b'\n' in buffer:
                        line, buffer = buffer.split(b'\n',1)
                        if line: messages.append(json.loads(line))
                except socket.timeout:
                    pass
                events = read_events(work)
                errors = [e for e in events if e['kind']=='error']
                if errors: raise RuntimeError(errors[-1]['detail'])
                if role=='leader' and not sent and any(m.get('type')=='channel_joined' for m in messages):
                    # Give the NVDA client time to process channel_joined.
                    time.sleep(1)
                    send({'type':'speak','sequence':['Fernzugriff Test.'], 'priority':0})
                    send({'type':'tone','hz':763,'length':50,'left':50,'right':50})
                    send({'type':'wave','fileName':ready['wav']})
                    sent = True
                if role=='follower':
                    tone = any(m.get('type')=='tone' and m.get('hz')==763 for m in messages)
                    direct = any(m.get('type')=='tone' and m.get('hz')==764 for m in messages)
                    wave = any(m.get('type')=='wave' and Path(m.get('fileName','')).name=='start.wav' for m in messages)
                    speech = any(m.get('type')=='speak' for m in messages)
                    if tone and direct and wave and speech:
                        print('PASS real NVDA follower: speech, queued beep/WAV and direct progress beep sent over TLS', flush=True)
                        return
                else:
                    tone = any(e.get('kind')=='beep' and e.get('hz')==763 for e in events)
                    wave = any(e.get('kind')=='wave' and e.get('file')=='start.wav' for e in events)
                    pcm = any(e.get('kind')=='audio' and e.get('bytes',0)>0 for e in events)
                    if tone and wave and pcm:
                        print('PASS real NVDA leader: remote beep and WAV reached audio feed while ReLoq was active', flush=True)
                        return
            raise RuntimeError(f'{role} incomplete: messages={messages!r}, events={events!r}')
    finally:
        process.terminate()
        try: process.wait(timeout=10)
        except subprocess.TimeoutExpired: process.kill();process.wait()
        log = work / 'nvda.log'
        if log.exists() and os.environ.get('RELOQ_DUMP_NVDA_LOG'):
            print(log.read_text(encoding='utf-8', errors='replace')[-16000:], flush=True)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--nvda', type=Path, required=True)
    p.add_argument('--addon', type=Path, required=True)
    p.add_argument('--plugin', type=Path, required=True)
    p.add_argument('--work', type=Path, required=True)
    a = p.parse_args()
    for role in ('follower','leader'):
        run(role, a.nvda.resolve(), a.addon.resolve(), a.plugin.resolve(), a.work.resolve())


if __name__ == '__main__': main()
