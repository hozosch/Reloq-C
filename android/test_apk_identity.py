"""Exercise APK identity parsing and the distinction between upgrades and migration."""
from types import SimpleNamespace
from unittest.mock import patch

from verify_apk import apk_identity, verify_previous_package, verify_upgrade

certificate = "abcd" * 16
packages = {
    "previous.apk": ("org.reloq.tts." + "pre" + "view", 11010000, certificate),
    "current.apk": ("org.reloq.tts", 11010001, certificate),
    "released-112.apk": ("org.reloq.tts", 11010002, certificate),
    "candidate-113.apk": ("org.reloq.tts", 11010003, certificate),
    "same-identity-110.apk": ("org.reloq.tts", 11010000, certificate),
    "older-current.apk": ("org.reloq.tts", 11000000, certificate),
    "other-signer.apk": ("org.reloq.tts", 11010000, "dead" * 16),
}

def run(command, **_kwargs):
    package, code, digest = packages[str(command[-1])]
    if command[1:3] == ["dump", "badging"]:
        return SimpleNamespace(stdout=f"package: name='{package}' versionCode='{code}' versionName='1.1.1'\n")
    if command[1:3] == ["verify", "--print-certs"]:
        return SimpleNamespace(stdout=f"Signer #1 certificate SHA-256 digest: {digest}\n")
    raise AssertionError(command)

with patch("verify_apk.subprocess.run", side_effect=run):
    info = apk_identity("current.apk", "aapt2", "apksigner")
    assert info == {"package": "org.reloq.tts", "version_code": 11010001, "certificates": [certificate]}
    assert verify_previous_package("current.apk", "previous.apk", "aapt2", "apksigner")["separate"]
    assert not verify_previous_package("current.apk", "older-current.apk", "aapt2", "apksigner")["separate"]
    assert not verify_previous_package("current.apk", "current.apk", "aapt2", "apksigner")["separate"]
    verify_upgrade("current.apk", "older-current.apk", "aapt2", "apksigner")
    verify_upgrade("current.apk", "same-identity-110.apk", "aapt2", "apksigner")
    verify_upgrade("candidate-113.apk", "released-112.apk", "aapt2", "apksigner")
    for old in ("previous.apk", "current.apk", "other-signer.apk"):
        try:
            verify_upgrade("current.apk", old, "aapt2", "apksigner")
        except ValueError:
            pass
        else:
            raise AssertionError(f"invalid upgrade accepted: {old}")
    try:
        verify_previous_package("current.apk", "other-signer.apk", "aapt2", "apksigner")
    except ValueError:
        pass
    else:
        raise AssertionError("different signer accepted")

print("APK identities, signer continuity, and separate-app migration checked")
