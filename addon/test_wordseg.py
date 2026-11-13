"""Regression coverage for optional unknown-English-word segmentation."""
import shutil
import tempfile
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_wordseg import (WordSegmenter, dictionary_keys, split_camel, builtin_acronym,
                            protected_native_spans, native_compound_spans, uppercase_candidates)


def main():
    for language in ('enus', 'engb'):
        segmenter = WordSegmenter.from_files(ROOT / 'nvda/wordseg-data', language)
        examples = {'usrdct': 'U S R D C T', 'cgroup': 'C group',
                    'tmpfiles': 'T M P files', 'lvmlockd': 'L V M lock D',
                    'gethostbyaddr': 'get host by addr', 'fdopen': 'F D open',
                    'swapfile': 'swap file', 'framebuffer': 'frame buffer',
                    'keyframe': 'key frame', 'blackboard': 'black board',
                    'webbutton': 'web button', 'buttonclick': 'button click',
                    'halfton': 'half ton', 'longton': 'long ton', 'metricton': 'metric ton'}
        for source, expected in examples.items():
            mixed = ''.join(c.upper() if i % 2 else c.lower() for i, c in enumerate(source))
            for spelling in (source, source.upper(), source.capitalize(), mixed):
                assert segmenter.text(spelling) == expected, (language, spelling, segmenter.text(spelling))
        for word in ('algorithm', 'inside', 'another', 'example', 'HELLO', 'éusrdct', 'usrdct3', "usrdct's"):
            assert segmenter.text(word) == word, (language, word)
        for name in ('Jayson', 'peterson', 'Petersons', 'Tilton', 'Petersen', 'Jayton', 'Carlson'):
            assert segmenter.text(name) == name, (language, name, segmenter.text(name))
        assert segmenter.text('cGroup') == 'C group'
        assert segmenter.text('CGROUP TMPFILES', preserve_upper=True) == 'CGROUP TMPFILES'
        assert uppercase_candidates('CGROUP TMPFILES', segmenter, frozenset()) == (
            (0, 6, 'C group'), (7, 15, 'T M P files'))
        assert uppercase_candidates('CGROUP TMPFILES', segmenter, frozenset(), ((0, 6),)) == (
            (7, 15, 'T M P files'),)
        assert segmenter.text('cgroup', frozenset({'cgroup'})) == 'cgroup'
        assert segmenter.text('framebuffer', frozenset({'framebuffer'})) == 'framebuffer'
        assert segmenter.text('CGROUP', frozenset({'cgroup'})) == 'CGROUP'
        assert segmenter.text('cgroups', frozenset({'cgroup'})) == 'cgroups'
        assert segmenter.text('cgrouping', frozenset({'cgroup'})) == 'cgrouping'
        assert segmenter.text('eci_usrdct.c') == 'eci_U S R D C T.c'
    with tempfile.TemporaryDirectory() as folder:
        source = ROOT / 'nvda/wordseg-data'
        root = Path(folder)
        shutil.copyfile(source / 'engb-sayable', root / 'engb-sayable')
        shutil.copyfile(source / 'enus-pieces', root / 'enus-pieces')
        shared = WordSegmenter.from_files(root, 'engb')
        reference = WordSegmenter.from_files(source, 'engb')
        assert shared.pieces == reference.pieces
        assert shared.abbreviations == reference.abbreviations
        for word in ('fileswap', 'swapfile', 'framebuffer', 'cgroup'):
            assert shared.text(word) == reference.text(word), word
        # A later British-specific table must not silently use the US data.
        (root / 'engb-pieces').write_text('britishonly\n', encoding='utf-8')
        distinct = WordSegmenter.from_files(root, 'engb')
        assert distinct.pieces == frozenset({'britishonly'})
        assert distinct.pieces != shared.pieces
    with tempfile.TemporaryDirectory() as folder:
        Path(folder, 'ENUmain.dic').write_bytes(b'cgroup\tcustom pronunciation\nfoo-BarBaz\tcustom pronunciation\n')
        Path(folder, 'ENGRoot.dic').write_bytes(b'fdopen\tcustom pronunciation\n')
        Path(folder, 'DEUmain.dic').write_bytes('WLANTest\tMein Test\n'.encode('cp1252'))
        Path(folder, 'DEURoot.dic').write_bytes(b'haus\tcustom pronunciation\n')
        keys = dictionary_keys(folder)
        assert 'cgroup' in keys[0x10000] and 'fdopen' in keys[0x10001]
        assert 'fdopen' not in keys[0x10000] and 'cgroup' not in keys[0x10001]
        assert 'CGROUP' not in keys[0x10000], 'ECI main entries distinguish case'
        assert 'cgroups' not in keys[0x10000] and 'CGROUPS' not in keys[0x10000]
        assert 'FdOpen' in keys[0x10001] and 'FDOPEN' not in keys[0x10001]
        assert 'WLANTest' in keys[0x40000] and 'wlantest' not in keys[0x40000]
        assert 'HAUS' in keys[0x40000] and 'haus' in keys[0x40000]
        assert segmenter.text('CGROUP', keys[0x10000]) == 'C group'
        assert segmenter.text('cgroups', keys[0x10000]) == segmenter.text('CGROUPS', keys[0x10000])
        assert native_compound_spans('vor foo-BarBaz danach', keys[0x10000]) == ((4, 14),)
        assert list(protected_native_spans('vor foo-BarBaz danach', keys[0x10000])) == [
            ('vor ', False), ('foo-BarBaz', True), (' danach', False)]
        assert not native_compound_spans('foo-BarBazooka', keys[0x10000])
        assert split_camel('WLANTest und meinTest', keys[0x40000]) == 'WLANTest und mein Test'
        assert list(protected_native_spans('Vor WLANTest nach', keys[0x40000])) == [
            ('Vor ', False), ('WLANTest', True), (' nach', False)]
        Path(folder, 'ENURoot.dic').write_bytes(b'cgroup\tcustom pronunciation\n')
        roots = dictionary_keys(folder)[0x10000]
        assert 'cgroups' in roots and 'CGROUPS' in roots and 'cGroups' in roots
        assert split_camel('cGroups und meinTest', roots) == 'cGroups und mein Test'
    samples = {'meinWort': 'mein Wort', 'HTTPServer': 'HTTP Server',
               'BlindRadio': 'Blind Radio', 'BlindRSS': 'Blind RSS',
               'CDs': 'CDs', 'DVDs': 'DVDs', 'ROMs': 'ROMs', 'IDs': 'IDs',
               'PDFs': 'PDFs', 'URLs': 'URLs', 'PACs': 'PACs', 'FAQs': 'FAQs',
               'SMSs': 'SMSs', 'AGBs': 'AGBs', 'LKWs': 'LKWs', 'USBs': 'USBs',
               'SOSes': 'SOSes', 'HTTPS': 'HTTPS', 'BlindCDs': 'Blind CDs',
               'myURLs': 'my URLs', 'allePDFs': 'alle PDFs', 'IDServer': 'ID Server',
               'XMLHttpRequest': 'XML Http Request', 'StraßenName': 'Straßen Name',
               'δοκιμήΛέξη': 'δοκιμή Λέξη', 'приветМир': 'привет Мир',
               'a\u0308B': 'a\u0308 B', 'schon Richtig': 'schon Richtig'}
    for source, expected in samples.items():
        assert split_camel(source) == expected, (source, split_camel(source))
        assert split_camel(source, {source.casefold()}) == source
    for word in ('CDs', 'SMSs', 'PACs', 'MP3', 'R2D2', 'CDs3'):
        assert builtin_acronym(word), word
    for word in ('CD', 'BlindRadio', 'iPhone', 'Version2', 'Wort'):
        assert not builtin_acronym(word), word
    print('Word segmentation: case independence, multilingual camel case, and dictionary precedence passed')


if __name__ == '__main__': main()
