"""Exercise the packaged sound lab against real ECI frames and US planning."""
from __future__ import annotations
import argparse
import ctypes
import importlib
import sys
import tempfile
import types
import zipfile
from pathlib import Path
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'experiments/persian'))
from build_soundlab_nvda import entries


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--dll', type=Path, required=True)
    parser.add_argument('--espeak', type=Path, default=Path('/unavailable-espeak'))
    parser.add_argument('--data', type=Path, default=Path('/unavailable'))
    args = parser.parse_args()
    with tempfile.TemporaryDirectory() as temp:
        folder = Path(temp)
        for name, data in entries().items():
            target = folder / name
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(data)
        path = folder / 'globalPlugins/reloqSoundLab'
        # Import the private engine without loading NVDA's wx-dependent GUI.
        package = types.ModuleType('soundlab_test')
        package.__path__ = [str(path)]
        sys.modules['soundlab_test'] = package
        lab = importlib.import_module('soundlab_test.lab')
        planner = importlib.import_module('soundlab_test._persian.continuous_driver')
        checks = 0
        for language, text in (('Persisch','ایران.'), ('Pinglish','iran.'),
                               ('Deutsch','Sonne.'), ('Englisch (USA)','sun.'),
                               ('Japanisch','すし。')):
            p = lab.defaults(); p.update(language=language, text=text)
            calls = []
            original_fit = planner.fit_us_word_pitch
            def fitted(*a, **kw):
                value = original_fit(*a, **kw)
                calls.append(value)
                return value
            with patch.object(planner, 'fit_us_word_pitch', side_effect=fitted):
                original, info, changed = lab.render(args.dll, args.espeak, args.data, p, False)
            assert original and info and changed == 0, language
            if language in ('Persisch', 'Pinglish'):
                assert any(calls), (language, 'US contour was not fitted')
            unchanged, _, _ = lab.render(args.dll, args.espeak, args.data, p, True)
            assert original == unchanged, (language, 'zero adjustment')
            p['frequencies'][1] = 250
            modified, _, changed = lab.render(args.dll, args.espeak, args.data, p, True)
            assert original != modified and changed > 0, (language, 'F2')
            assert len(original) == len(modified), (language, 'duration')
            checks += 1
        # Mutate synthetic native frames so we can verify the exact scope,
        # time window and unchanged controls, including noise bandwidth.
        frames = importlib.import_module('soundlab_test._reloq_persian_frames')
        rows = (frames.NativeFrame * 3)()
        for i, row in enumerate(rows):
            row.tick = i * 30; row.controls[2] = 55; row.controls[9] = 500
            row.controls[13] = 1000; row.controls[14] = 100; row.controls[46] = 800
        p = lab.defaults(); p['start'] = 20; p['end'] = 60
        p['frequencies'][1] = 200; p['bandwidths'][1] = 40; p['noise_widths'][2] = 100
        before = bytes(rows); assert lab.adjust(rows, p, True, []) == 3
        assert rows[0].controls[13] == 1000 and rows[2].controls[13] == 1000
        assert rows[1].controls[13] == 1100 and rows[1].controls[14] == 120 and rows[1].controls[46] == 850
        old = (frames.NativeFrame * 3).from_buffer_copy(before)
        for a, b in zip(old, rows):
            for i in (13,14,46): a.controls[i] = b.controls[i]
        assert bytes(old) == bytes(rows), 'unrelated controls changed'
        print(f'Passed {checks} sound-lab languages, US-contour checks and frame-scope checks.')

if __name__ == '__main__': main()
