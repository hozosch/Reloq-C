"""Compile the real Android update UI, background job and build identity."""
import argparse
import os
from pathlib import Path
import re
import subprocess
import tempfile

from codegen import generate

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--java-home', type=Path, required=True)
    parser.add_argument('--android-jar', type=Path, required=True)
    args = parser.parse_args()
    folder = ROOT / 'android/java/org/reloq/tts'
    sources = [folder / (name + '.java') for name in
               ('UpdateData', 'UpdateClient', 'UpdateActivity', 'UpdateJob', 'UpdateScheduler')]
    with tempfile.TemporaryDirectory(prefix='reloq-android-update-') as temp:
        root = Path(temp)
        generate(root / 'generated')
        target = root / 'generated/org/reloq/tts'
        text = '\n'.join(path.read_text() for path in sources)
        names = sorted(set(re.findall(r'(?<!android\.)\bR\.string\.(\w+)', text)))
        (target / 'R.java').write_text('package org.reloq.tts; final class R { static final class string {' +
            ''.join('static final int ' + name + '=1;' for name in names) + '} }\n')
        (target / 'BootPreferences.java').write_text(
            'package org.reloq.tts; final class BootPreferences {'
            'static boolean unlocked(android.content.Context c) {return true;} }\n')
        epoch = os.environ.get('RELOQ_BUILD_EPOCH', '0') or '0'
        assert 'BUILD_EPOCH=' + epoch + 'L' in (target / 'BuildData.java').read_text()
        sources += [target / name for name in ('R.java', 'BootPreferences.java', 'BuildData.java')]
        subprocess.run([str(args.java_home / 'bin/javac'), '-encoding', 'UTF-8',
            '-cp', str(args.android_jar), '-d', str(root), *map(str, sources)], check=True)
    print('Android update UI, background job and embedded build identity compiled successfully')


if __name__ == '__main__':
    main()
