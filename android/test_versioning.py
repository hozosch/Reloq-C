"""Check Android release version ordering."""
from pathlib import Path
import re
import xml.etree.ElementTree as ET

root = Path(__file__).resolve().parents[1]
manifest = ET.parse(root / 'android/AndroidManifest.xml').getroot()
android = '{http://schemas.android.com/apk/res/android}'
version = manifest.get(android + 'versionName')
code = int(manifest.get(android + 'versionCode'))
match = re.fullmatch(r'(\d+)\.(\d+)\.(\d+)', version or '')
development = re.fullmatch(r'(\d+)\.(\d+)\.(\d+)-(?:dev|rc)\d+', version or '')
if match:
    major, minor, patch = map(int, match.groups())
    expected = major * 1_000_000 + minor * 10_000 + patch
    if major > 1 or (major == 1 and minor >= 1):
        expected += 10_000_000
    assert code == expected, (version, code, expected)
    if (major, minor, patch) >= (1, 1, 0):
        assert code > 1_090_027, 'stable version code must retain release ordering'
elif development:
    major, minor, patch = map(int, development.groups())
    expected = major * 1_000_000 + minor * 10_000 + patch
    if major > 1 or (major == 1 and minor >= 1):
        expected += 10_000_000
    assert code == expected - 1, (version, code, expected)
print(f'Android version {version}: code {code}; release ordering checked')
