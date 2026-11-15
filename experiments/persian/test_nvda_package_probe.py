"""Check the private NVDA package adapter without an installed NVDA."""
from __future__ import annotations

import argparse
import importlib
import json
from pathlib import Path
import shutil
import sys
import tempfile
import types

from build_experimental_nvda import PREFIX, add_persian

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--native-library', type=Path, required=True)
    parser.add_argument('--text-library', type=Path)
    parser.add_argument('--worker', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    args = parser.parse_args()
    base = {'manifest.ini': (ROOT / 'addon/synth/manifest.ini').read_bytes(),
            PREFIX + '__init__.py': (ROOT / 'addon/synth/synthDrivers/reloqC/__init__.py').read_bytes(),
            'doc/source.json': b'{"build":"normal release"}',
            'locale/de/manifest.ini': b'summary = "Alt"\ndescription = "Alt"\n'}
    text_library = (args.text_library or args.native_library).resolve()
    entries = add_persian(base, args.worker.read_bytes(), text_library.read_bytes())
    assert 'Persian acoustic probe' in json.loads(entries['doc/source.json'])['build']
    assert b'1.5.0-dev91' in entries['manifest.ini']
    assert 'Persisch- und Pinglish-Test' in entries['locale/de/manifest.ini'].decode('utf8')
    assert b'add_profiles(dict(LANGUAGES), DLL)' in entries[PREFIX + '__init__.py']
    assert b'lambda: make_engine(' in entries[PREFIX + '__init__.py']
    with tempfile.TemporaryDirectory(prefix='reloq-fa-package-') as temp:
        folder = Path(temp)
        package = folder / 'probe_driver'
        package.mkdir()
        (package / '__init__.py').write_text('', encoding='utf8')
        for module in ('_reloq_eci_direct.py', '_reloq_languages.py', '_reloq_persian_projection.py'):
            shutil.copyfile(ROOT / 'nvda' / module, package / module)
        for name, content in entries.items():
            if (not name.startswith(PREFIX) or name == PREFIX + '__init__.py' or
                    name == PREFIX + 'fa-direct.dll'):
                continue
            target = package / name[len(PREFIX):]
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(content)
        # Keep the loaded native DLL outside the temporary package: Windows
        # cannot remove a DLL while Python still owns its loaded module.
        library = args.native_library.resolve()
        installed = folder / 'nvda' / 'synthDrivers'
        installed.mkdir(parents=True)
        worker = package / 'fa-espeak-worker.exe'
        worker.chmod(0o755)
        sys.modules['globalVars'] = types.SimpleNamespace(appDir=str(installed.parent))
        sys.path.insert(0, str(folder))
        try:
            bridge = importlib.import_module('probe_driver._reloq_persian_bridge')
            languages = importlib.import_module('probe_driver._reloq_languages')
            eci = importlib.import_module('probe_driver._reloq_eci_direct')
            assert bridge._paths(package / 'eci.dll') == (package / 'fa-direct.dll', worker,
                                                           installed / 'espeak.dll', installed)
            # Keep NVDA's eSpeak in its real directory alongside all of its
            # Windows DLL dependencies; copying espeak.dll alone is not enough.
            bridge._paths = lambda _library: (text_library, worker,
                                               args.espeak_library.resolve(), args.data_parent.resolve())
            worker.rename(worker.with_suffix('.missing'))
            assert bridge.add_profiles(dict(languages.LANGUAGES), library) == languages.LANGUAGES
            worker.with_suffix('.missing').rename(worker)
            profiles = bridge.add_profiles(dict(languages.LANGUAGES), library)
            fa, pinglish = bridge.PERSIAN_PROBE, bridge.PINGLISH_PROBE
            assert languages.language_for('fa-IR', languages.GERMAN.id, profiles) == fa.id
            assert languages.language_for('fa-Latn', languages.GERMAN.id, profiles) == pinglish.id
            engine = bridge.make_engine(library, languages=profiles, language=pinglish.id, voice='1')
            try:
                pcm, marks = bytearray(), []
                assert engine.speak((eci.EciIndex(1), 'salam ', eci.EciIndex(2), 'donya?',
                                     eci.EciIndex(3)), pcm.extend, marks.append)
                assert pcm and marks == [1, 2, 3]
                assert engine._engines[pinglish.id]._frontend._worker is None
                original = bytes(pcm)
                engine.select_voice(fa.id, '1')
                pcm.clear()
                assert engine.speak(('سلام دنیا؟',), pcm.extend, lambda _: None)
                assert bytes(pcm) == original
                frontend = engine._engines[fa.id]._frontend
                assert frontend.analyze_direct('بدبخت').parts[0].text == '`[.1bAd.0bAGt]'
                assert frontend.analyze_direct('سوال').parts[0].text == '`[.1so.0wal]'
                engine.select_voice(pinglish.id, '1')
                pcm.clear()
                assert engine.speak(('salam ', eci.EciIndex(4), 'khubtar?'), pcm.extend, lambda _: None)
                assert pcm
            finally:
                engine.close()
        finally:
            sys.path.remove(str(folder))
            sys.modules.pop('globalVars', None)
    print('Passed private NVDA package profiles, selection and native playback.')


if __name__ == '__main__':
    main()
