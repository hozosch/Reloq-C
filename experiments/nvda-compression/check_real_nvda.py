"""Exercise NVDA's own AddonBundle extractor with an LZMA-compressed DLL."""
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import time
import zipfile

source = Path("candidate/reloq-c-synth-x64-1.4.1-dev4.nvda-addon")
work = Path("compression-probe").resolve()
work.mkdir(exist_ok=True)
bundle = work / "candidate-lzma.nvda-addon"
name = "synthDrivers/reloqC/eci.dll"
with zipfile.ZipFile(source) as old, zipfile.ZipFile(bundle, "w") as new:
    reference = hashlib.sha256(old.read(name)).hexdigest()
    assert reference == "ec416c58df88fd3f87a1b64429d67aa01ff438da16af99bd5d3e04e5ca4b89a8"
    for info in old.infolist():
        data = old.read(info)
        info.compress_type = zipfile.ZIP_LZMA if info.filename == name else zipfile.ZIP_DEFLATED
        new.writestr(info, data)
with zipfile.ZipFile(bundle) as check:
    assert check.testzip() is None
    assert check.getinfo(name).compress_type == zipfile.ZIP_LZMA
print("Package bytes:", source.stat().st_size, "->", bundle.stat().st_size, flush=True)

plugin = r'''
import ctypes
import hashlib
import json
import os
from pathlib import Path
import traceback
import addonHandler
import globalPluginHandler
import wx

class GlobalPlugin(globalPluginHandler.GlobalPlugin):
    def __init__(self):
        super().__init__()
        wx.CallLater(1500, self.check)

    def check(self):
        result = Path(os.environ["RELOQ_COMPRESSION_RESULT"])
        try:
            import _lzma
            package = addonHandler.AddonBundle(os.environ["RELOQ_COMPRESSION_PACKAGE"])
            target = result.parent / "extracted"
            package.extract(str(target))
            dll = target / "synthDrivers/reloqC/eci.dll"
            digest = hashlib.sha256(dll.read_bytes()).hexdigest()
            assert digest == os.environ["RELOQ_COMPRESSION_SHA256"]
            library = ctypes.CDLL(str(dll))
            assert library.eciNew
            payload = {"ok": True, "sha256": digest, "extractor": "addonHandler.AddonBundle.extract",
                       "dllLoaded": True}
        except Exception:
            payload = {"ok": False, "error": traceback.format_exc()}
        result.write_text(json.dumps(payload), encoding="utf8")
'''
for version in ("2026.1", "2026.3.1"):
    run = work / version
    profile = run / "config"
    target = profile / "addons" / "zipProbe"
    (target / "globalPlugins").mkdir(parents=True)
    (target / "globalPlugins" / "zipProbe.py").write_text(plugin, encoding="utf8")
    (target / "manifest.ini").write_text(
        'name = "zipProbe"\nsummary = "Compression probe"\nversion = "1.0"\n'
        'author = "Test"\nminimumNVDAVersion = "2026.1"\nlastTestedNVDAVersion = "2026.3.0"\n',
        encoding="utf8")
    (profile / "nvda.ini").write_text(
        "[general]\nplayStartAndExitSounds = False\n[speech]\nsynth = silence\n", encoding="utf8")
    result = run / "result.json"
    log = run / "nvda.log"
    env = dict(os.environ, RELOQ_COMPRESSION_RESULT=str(result),
               RELOQ_COMPRESSION_PACKAGE=str(bundle), RELOQ_COMPRESSION_SHA256=reference)
    exe = Path("portable-" + version, "nvda.exe").resolve()
    process = subprocess.Popen([str(exe), "--minimal", "--no-sr-flag",
        "--debug-logging", "--config-path=" + str(profile), "--log-file=" + str(log)], env=env)
    try:
        deadline = time.monotonic() + 120
        while time.monotonic() < deadline and not result.exists():
            if process.poll() is not None:
                raise RuntimeError(f"NVDA {version} exited early: {process.returncode}")
            time.sleep(.2)
        if not result.exists():
            raise RuntimeError(f"NVDA {version} timed out")
        data = json.loads(result.read_text(encoding="utf8"))
        print(version, json.dumps(data), flush=True)
        if not data.get("ok"):
            raise RuntimeError("NVDA extraction or DLL load failed")
    finally:
        process.terminate()
        try:
            process.wait(timeout=10)
        except subprocess.TimeoutExpired:
            process.kill()
            process.wait()
        if log.exists() and not result.exists():
            print(log.read_text(encoding="utf8", errors="replace")[-12000:], flush=True)
