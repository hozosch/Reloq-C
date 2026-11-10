"""Test-only NVDA plugin: one real Remote endpoint with simulated audio output."""
import json
import os
from pathlib import Path
import threading
import traceback

import globalPluginHandler
import wx

ROOT = Path(os.environ['RELOQ_PROBE_DIR'])
ROLE = os.environ['RELOQ_PROBE_ROLE']
PORT = int(os.environ['RELOQ_PROBE_PORT'])
CHANNEL = 'reloq-loopback-test'
LOCK = threading.Lock()


def record(kind, **data):
    with LOCK, (ROOT / 'events.jsonl').open('a', encoding='utf-8') as out:
        out.write(json.dumps({'kind': kind, **data}, ensure_ascii=False) + '\n')


class Player:
    def __init__(self, kind):
        self.kind = kind
    def feed(self, data, size=None, onDone=None):
        if callable(size) and onDone is None:
            onDone = size
        record('audio', source=self.kind, bytes=size if isinstance(size, int) else len(data))
        if onDone:
            onDone()
    def idle(self): pass
    def sync(self): pass
    def stop(self): pass
    def pause(self, switch): pass
    def close(self): pass


def capture_tone(hz, length, left=50, right=50, **kwargs):
    record('beep', hz=hz, length=length)
    return True


def capture_wave(fileName, **kwargs):
    record('wave', file=Path(fileName).name)
    return True


class GlobalPlugin(globalPluginHandler.GlobalPlugin):
    def __init__(self):
        super().__init__()
        wx.CallLater(1500, self.start)

    def start(self):
        try:
            import globalVars
            import nvwave
            import tones
            import synthDriverHandler
            import config
            import _remoteClient
            from _remoteClient.server import LocalRelayServer
            from _remoteClient.connectionInfo import ConnectionInfo, ConnectionMode
            import threading
            # Only output-device playback is replaced. NVDA's speech manager,
            # ECI synthesis, Remote session and TLS transport stay real.
            nvwave.WavePlayer = lambda *args, **kwargs: Player('wave')
            tones.player = Player('tone')
            tones.decide_beep.register(capture_tone)
            nvwave.decide_playWaveFile.register(capture_wave)
            if not synthDriverHandler.setSynth('reloqC'):
                raise RuntimeError('Could not select Eloquence C')
            record('synth', name=synthDriverHandler.getSynth().name)
            client = _remoteClient._remoteClient
            if client is None:
                raise RuntimeError('NVDA Remote was not initialized')
            client.localMachine.isMuted = False
            config.conf['remote']['ui']['muteOnLocalControl'] = False
            server = LocalRelayServer(PORT, CHANNEL, bindHost='127.0.0.1', bindHost6='::1')
            client.localControlServer = server
            threading.Thread(target=server.run, daemon=True).start()
            fingerprint = server.certManager.getCurrentFingerprint()
            info = ConnectionInfo(hostname='127.0.0.1', port=PORT, key=CHANNEL,
                mode=ConnectionMode.FOLLOWER if ROLE=='follower' else ConnectionMode.LEADER,
                insecure=True, trustedFingerprint=fingerprint)
            client.connect(info)
            record('server', port=PORT, role=ROLE, wav=str(Path(globalVars.appDir)/'waves'/'start.wav'))
            if ROLE == 'follower':
                wx.CallLater(250, self.test_follower, 0)
        except Exception:
            record('error', detail=traceback.format_exc())

    def test_follower(self, attempts):
        try:
            import _remoteClient
            client = _remoteClient._remoteClient
            session = client.followerSession
            if session is None or not session.callbacksAdded:
                if attempts >= 200:
                    raise RuntimeError('Follower callback registration timed out')
                wx.CallLater(250, self.test_follower, attempts + 1)
                return
            import globalVars
            import nvwave
            import speech
            import tones
            from speech.commands import BeepCommand, WaveFileCommand
            wav = str(Path(globalVars.appDir)/'waves'/'start.wav')
            speech.speak(['Test vor dem Signal. ', BeepCommand(763, 50),
                          WaveFileCommand(wav), ' Test nach dem Signal.'])
            tones.beep(764, 50)
            record('follower_triggered')
        except Exception:
            record('error', detail=traceback.format_exc())
