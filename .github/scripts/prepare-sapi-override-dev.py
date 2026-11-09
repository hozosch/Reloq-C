from pathlib import Path
import sys

old, new = '1.2.2', '1.2.3-dev1'
if sys.argv[1] == 'native':
    path = Path('sapi/build.py')
    content = path.read_text(encoding='utf-8')
    assert content.count(old) == 2
    path.write_text(content.replace(old, new), encoding='utf-8')
elif sys.argv[1] == 'installer':
    path = Path('sapi/build_installer.py')
    content = path.read_text(encoding='utf-8')
    assert content.count(old) == 1
    path.write_text(content.replace(old, new), encoding='utf-8')
    path = Path('sapi/installer/reloq-sapi.iss')
    content = path.read_text(encoding='utf-8')
    line = 'AppVersion={#AppVersion}'
    assert content.count(line) == 1
    path.write_text(content.replace(line, line + '\nVersionInfoVersion=1.2.3.1'), encoding='utf-8')
elif sys.argv[1] == 'android':
    path = Path('android/AndroidManifest.xml')
    content = path.read_text(encoding='utf-8')
    assert content.count('android:versionName="1.2.2"') == 1
    path.write_text(content.replace('android:versionName="1.2.2"',
                                    'android:versionName="1.2.3-dev1"'), encoding='utf-8')
else:
    raise ValueError(sys.argv[1])
