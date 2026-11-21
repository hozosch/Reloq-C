"""Exercise release metadata and candidate filenames without building installers."""
from pathlib import Path
from tempfile import TemporaryDirectory

from check_release_versions import check
from build_synth import addon_output
from build import android_output


with TemporaryDirectory() as directory:
    root = Path(directory)
    nvda, android, sapi = root / 'manifest.ini', root / 'AndroidManifest.xml', root / 'version.py'

    def fixtures(version, code):
        nvda.write_text(f'version = "{version}"\n', encoding='utf-8')
        android.write_text('<manifest xmlns:android="http://schemas.android.com/apk/res/android" '
                           f'android:versionName="{version}" android:versionCode="{code}"/>', encoding='utf-8')
        sapi.write_text(f'VERSION = {version!r}\n', encoding='utf-8')

    for version, code in (('1.4.1', 11040001), ('2.0.0', 12000000), ('2.0.0-dev12', 11999999)):
        fixtures(version, code)
        assert check(nvda, android, sapi) == version
        assert addon_output(version).name == f'reloq-c-synth-x64-{version}.nvda-addon'
        for abi in ('arm64', 'x86_64'):
            assert android_output(abi, version).name == f'reloq-c-android-{abi}-{version}.apk'
    fixtures('2.0.0', 12000000)
    sapi.write_text("VERSION = '1.4.1'\n", encoding='utf-8')
    try:
        check(nvda, android, sapi)
        raise AssertionError('mismatched SAPI version was accepted')
    except ValueError:
        pass
    fixtures('2.0.0', 11040001)
    try:
        check(nvda, android, sapi)
        raise AssertionError('stale Android versionCode was accepted')
    except ValueError:
        pass
print('Release version checks passed for 1.4.1, 2.0.0, a development build and mismatched metadata.')
