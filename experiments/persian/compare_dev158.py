"""Compare dev157 and dev158 through their respective real US-contour plans."""
from __future__ import annotations

import argparse
import ast
from pathlib import Path
import sys
from unittest.mock import patch
import zipfile

ROOT = Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT / 'experiments/persian'), str(ROOT / 'nvda')]
from _reloq_eci_direct import DirectEciEngine, GENDER, NATIVE_PITCH, NATIVE_SPEED, SAMPLE_RATE
from compare_fa_clarity import matched_levels, write_wav
from compare_persian_r_us_contour import actual_us_render
import continuous_driver
import continuous_plan


def old_proxy(addon):
    with zipfile.ZipFile(addon) as archive:
        code = archive.read('synthDrivers/reloqC/_persian/continuous_plan.py').decode('utf8')
    tree = ast.parse(code)
    function = next(node for node in tree.body if isinstance(node, ast.FunctionDef)
                    and node.name == 'indexed_us_proxy')
    module = ast.fix_missing_locations(ast.Module(body=[function], type_ignores=[]))
    namespace = dict(vars(continuous_plan))
    exec(compile(module, str(addon), 'exec'), namespace)
    return namespace['indexed_us_proxy']


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--previous', required=True, type=Path)
    parser.add_argument('--previous-addon', required=True, type=Path)
    parser.add_argument('--candidate', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    args = parser.parse_args()
    former_proxy = old_proxy(args.previous_addon)
    narrator = DirectEciEngine(args.candidate.resolve(), dialect=0x40000,
                               encoding='utf8', native_voice=True)
    for key, value in ((SAMPLE_RATE, 3), (NATIVE_SPEED, 65), (NATIVE_PITCH, 65), (GENDER, 0)):
        narrator.set_parameter(key, value)
    audio, silence = bytearray(), bytes(16000)
    def say(text):
        result = bytearray()
        if not narrator.speak((text,), result.extend, lambda _: None) or not result:
            raise RuntimeError('Cannot narrate the comparison')
        audio.extend(result)
        audio.extend(silence)
    try:
        say('Persische Klangprobe. Alle Beispiele verwenden die US Melodie. Das e bleibt unverändert.')
        for label, source in (('Zahlen drei, zwei, eins', 'سه، دو، یک.'),
                              ('Satz', 'سلام دنیا. این یک آزمایش است.'),
                              ('R nach f. Null im Satz', 'این عدد صفر است.'),
                              ('R nach d', 'قدر.'),
                              ('R nach i. Unterstrich und Bindestrich', 'زیرخط، خطِ تیره.')):
            with patch.object(continuous_driver, 'indexed_us_proxy', new=former_proxy), \
                 patch.object(continuous_plan, 'indexed_us_proxy', new=former_proxy):
                before = actual_us_render(args.previous, source)
            after = actual_us_render(args.candidate, source)
            if before == after:
                raise RuntimeError(f'No audible comparison change in {source!r}')
            (before, after), _ = matched_levels((before, after))
            say(label + '.')
            for name, sound in (('Bisher.', before), ('Neu.', after)):
                say(name)
                audio.extend(sound)
                audio.extend(silence)
        say('Ende des Vergleichs.')
    finally:
        narrator.close()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    write_wav(args.output, audio)
    print('Verified US-contour comparison written:', args.output)


if __name__ == '__main__':
    main()
