"""Exercise optional segmentation through the real common ECI core."""
import argparse
from pathlib import Path
import re
import tempfile

from _reloq_eci_direct import DirectEciEngine, EciIndex, EciText, WORD_SEGMENTATION, CAMEL_CASE
from test_eci_german_clock_native import render


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    args = parser.parse_args()
    checks = 0
    with tempfile.TemporaryDirectory() as temporary:
        for dialect, prefix in ((0x10000, 'ENU'), (0x10001, 'ENG')):
            engine = DirectEciEngine(args.library.resolve(), dialect=dialect, encoding='utf8', native_voice=True)
            try:
                assert engine._dll.eciGetParam(engine._handle, WORD_SEGMENTATION) == 0
                assert engine._dll.eciGetParam(engine._handle, CAMEL_CASE) == 0
                normal = {text: engine._phonemes_for_probe(text) for text in ('swapfile', 'framebuffer', 'keyframe', 'CGROUP')}
                engine.set_parameter(WORD_SEGMENTATION, 1)
                for text in ('swapfile', 'framebuffer', 'keyframe'):
                    result = engine._phonemes_for_probe(text)
                    assert result != normal[text] and len(re.findall(r'`\[', result)) == 1, (dialect, text, result)
                    checks += 1
                assert engine._phonemes_for_probe('CGROUP') == normal['CGROUP'], 'native uppercase spelling was changed'
                checks += 1
                # A personal pronunciation must beat automatic word splitting.
                folder = Path(temporary) / prefix
                folder.mkdir()
                (folder / (prefix + 'main.dic')).write_bytes(b'framebuffer\t`[.1tEst]\n')
                engine.set_dictionary_path(folder)
                with_split = engine._phonemes_for_probe('framebuffer')
                engine.set_parameter(WORD_SEGMENTATION, 0)
                assert with_split == engine._phonemes_for_probe('framebuffer') and 'tEst' in with_split
                checks += 1
                engine.set_dictionary_path(None)
                # A host dictionary may protect only its replaced run; an
                # unrelated word in the same utterance remains segmentable.
                baseline = render(engine, ['swapfile framebuffer'])[0]
                engine.set_parameter(WORD_SEGMENTATION, 1)
                protected = render(engine, [EciText('swapfile framebuffer', False, None, True)])[0]
                partially = render(engine, [EciText('swapfile', False, None, True), ' framebuffer'])[0]
                assert protected == baseline and partially != baseline
                checks += 1
                # A public index inside an original word must not be moved
                # into a generated phoneme annotation.
                parts = ['swap', EciIndex(57), 'file']
                changed = render(engine, parts)
                engine.set_parameter(WORD_SEGMENTATION, 0)
                assert changed == render(engine, parts) and changed[1][0][0] == 57
                checks += 1
            finally:
                engine.close()
        for dialect, source, split in ((0x40000, 'meinTest', 'mein Test'),
                                        (0x10000, 'myWindow', 'my Window'),
                                        (0x120000, 'δοκιμήΛέξη', 'δοκιμή Λέξη')):
            engine = DirectEciEngine(args.library.resolve(), dialect=dialect, encoding='utf8', native_voice=True)
            try:
                expected = engine._phonemes_for_probe(split)
                engine.set_parameter(CAMEL_CASE, 1)
                assert engine._phonemes_for_probe(source) == expected, (dialect, source)
                checks += 1
            finally:
                engine.close()
    print(f'Common ECI segmentation: {checks} dictionary, word, case and source-boundary checks passed')


if __name__ == '__main__':
    main()
