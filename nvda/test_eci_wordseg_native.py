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
                compounds = {'swapfile': ('swap', 'file'), 'fileswap': ('file', 'swap'),
                             'filetable': ('file', 'table'),
                             'swaphost': ('swap', 'host'), 'swapbuffer': ('swap', 'buffer'),
                             'filecandle': ('file', 'candle'), 'candlefile': ('candle', 'file'),
                             'shopfile': ('shop', 'file'), 'filelock': ('file', 'lock'),
                             'framebuffer': ('frame', 'buffer'), 'lineshape': ('line', 'shape'),
                             'keyframe': ('key', 'frame'), 'blackboard': ('black', 'board'),
                             'newspaper': ('news', 'paper'), 'candlelight': ('candle', 'light'),
                             'homepage': ('home', 'page'), 'waterbed': ('water', 'bed'),
                             'redhead': ('red', 'head'), 'redheaded': ('red', 'headed'),
                             'redheads': ('red', 'heads')}
                if dialect == 0x10000:
                    compounds['piebald'] = ('pie', 'bald')
                # Both dialects must handle the same productive component
                # boundaries, using each dialect's native pronunciations.
                for first, second in (
                    ('file','cache'), ('cache','file'), ('file','type'), ('type','file'),
                    ('file','mode'), ('mode','file'), ('file','path'), ('path','file'),
                    ('file','slot'), ('slot','file'), ('file','index'), ('index','file'),
                    ('file','map'), ('map','file'), ('file','port'), ('port','file'),
                    ('lock','buffer'), ('buffer','lock'), ('disk','file'), ('file','disk'),
                    ('stock','file'), ('file','stock'), ('file','host'), ('host','file'),
                    ('screen','file'), ('file','screen'), ('swap','cache'), ('cache','swap'),
                    ('swap','data'), ('swap','name'), ('swap','frame'), ('lock','file'),
                    ('home','spot'), ('top','swap')):
                    compounds[first + second] = (first, second)
                # The complete set Amir reported in issue #34, rather than
                # just the two examples initially repeated in discussion.
                amir_words = ('available', 'landmark', 'selection', 'Paperback',
                              'organically', 'expression', 'Jackson',
                              'developmental', 'suggestion')
                established = ('cupboard', 'Cupboard', 'CUPBOARD', 'cupboards',
                               'breakfast', 'breakfasts', 'raspberry', 'raspberries',
                               'strawberry', 'waistcoat', 'butterfly', 'lighthouse',
                               *amir_words, 'switchable', 'Switchable',
                               'switchably', 'switchability', 'switchless', 'spellless',
                               'switchlessness', 'happiness', 'paintable', 'deletable',
                               'rollable', 'viewable', 'searchable', 'jumpable', 'parkable')
                normal = {text: engine._phonemes_for_probe(text) for text in (*compounds, 'CGROUP')}
                audio = {text: render(engine, [text])[0] for text in compounds}
                capital_audio = render(engine, ['CGROUP'])[0]
                known_phones = {text: re.findall(r'`\[([^]]+)\]', engine._phonemes_for_probe(text))
                                for text in established}
                known_audio = {text: render(engine, [text])[0] for text in established}
                derived_sentences = ('This setting is switchable.', 'A searchable list is open.')
                sentence_audio = {text: render(engine, [text])[0] for text in derived_sentences}
                amir_parts = [word + '  ' for word in amir_words]
                amir_audio = render(engine, amir_parts)[0]
                if dialect == 0x10001:
                    # A correct British word and uncertain British compound
                    # readings must not be replaced just because the US
                    # pronunciation can be split.
                    uk_protected = ('homophone', 'piebald', 'coverall', 'cation')
                    known_uk = {word: engine._phonemes_for_probe(word) for word in uk_protected}
                    known_uk_audio = {word: render(engine, [word])[0] for word in uk_protected}
                def phones(value):
                    return ''.join(re.sub(r'\.[0-3]', '', group) for group in re.findall(r'`\[([^]]+)\]', value))
                pieces = {text: ''.join(phones(engine._phonemes_for_probe(part)) for part in pair)
                          for text, pair in compounds.items()}
                engine.set_parameter(WORD_SEGMENTATION, 1)
                for text in established:
                    assert re.findall(r'`\[([^]]+)\]', engine._phonemes_for_probe(text)) == known_phones[text], (dialect, text)
                    assert render(engine, [text])[0] == known_audio[text], (dialect, text, 'established pronunciation changed')
                    checks += 1
                for text in derived_sentences:
                    assert render(engine, [text])[0] == sentence_audio[text], (dialect, text)
                    checks += 1
                for word_seg, camel in ((0, 1), (1, 0), (1, 1)):
                    engine.set_parameter(WORD_SEGMENTATION, word_seg)
                    engine.set_parameter(CAMEL_CASE, camel)
                    for word in amir_words:
                        assert re.findall(r'`\[([^]]+)\]', engine._phonemes_for_probe(word)) == known_phones[word], (dialect, word_seg, camel, word)
                        assert render(engine, [word])[0] == known_audio[word], (dialect, word_seg, camel, word)
                        checks += 1
                    assert render(engine, amir_parts)[0] == amir_audio, (dialect, word_seg, camel, 'Amir list in one utterance')
                    checks += 1
                engine.set_parameter(WORD_SEGMENTATION, 1)
                engine.set_parameter(CAMEL_CASE, 0)
                if dialect == 0x10001:
                    for word in uk_protected:
                        assert re.findall(r'`\[([^]]+)\]', engine._phonemes_for_probe(word)) == \
                            re.findall(r'`\[([^]]+)\]', known_uk[word]), (word, dialect)
                        assert render(engine, [word])[0] == known_uk_audio[word], (word, dialect)
                        checks += 1
                for text in compounds:
                    result = engine._phonemes_for_probe(text)
                    assert len(re.findall(r'`\[', result)) == 1 and phones(result) == pieces[text], (dialect, text, result)
                    if phones(normal[text]) == pieces[text]:
                        assert render(engine, [text])[0] == audio[text], (dialect, text, 'already correct speech changed')
                    checks += 1
                # ECI may omit a redundant leading volume annotation on the
                # second query; compare each spoken letter and actual audio.
                capital = engine._phonemes_for_probe('CGROUP')
                assert re.findall(r'`\[([^]]+)\]', capital) == re.findall(r'`\[([^]]+)\]', normal['CGROUP'])
                assert render(engine, ['CGROUP'])[0] == capital_audio, 'native uppercase spelling was changed'
                assert phones(engine._phonemes_for_probe('SWAPFILE')) == pieces['swapfile']
                assert phones(engine._phonemes_for_probe('FILESWAP')) == pieces['fileswap']
                assert phones(engine._phonemes_for_probe('Fileswap')) == pieces['fileswap']
                sentence = re.findall(r'`\[([^]]+)\]', engine._phonemes_for_probe('Open fileswap now'))
                assert len(sentence)==3 and re.sub(r'\.[0-3]', '', sentence[1])==pieces['fileswap'], (dialect, sentence)
                checks += 1
                # A personal pronunciation must beat automatic word splitting.
                folder = Path(temporary) / prefix
                folder.mkdir()
                (folder / (prefix + 'main.dic')).write_bytes(
                    b'fileswap\t`[.1mup]\nframebuffer\t`[.1tEst]\nredhead\t`[.1zibrx]\n')
                engine.set_dictionary_path(folder)
                with_split = engine._phonemes_for_probe('framebuffer')
                engine.set_parameter(WORD_SEGMENTATION, 0)
                assert with_split == engine._phonemes_for_probe('framebuffer') and 'tEst' in with_split
                checks += 1
                engine.set_parameter(WORD_SEGMENTATION, 1)
                with_split = engine._phonemes_for_probe('redhead')
                engine.set_parameter(WORD_SEGMENTATION, 0)
                assert phones(with_split) == phones(engine._phonemes_for_probe('redhead')) == 'zibrx'
                checks += 1
                engine.set_parameter(WORD_SEGMENTATION, 1)
                with_split = engine._phonemes_for_probe('fileswap')
                engine.set_parameter(WORD_SEGMENTATION, 0)
                assert phones(with_split) == phones(engine._phonemes_for_probe('fileswap')) == 'mup'
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
