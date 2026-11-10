"""Regression coverage for optional unknown-English-word segmentation."""
import tempfile
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_wordseg import WordSegmenter, dictionary_keys, split_camel, protected_native_spans


def main():
    for language in ('enus', 'engb'):
        segmenter = WordSegmenter.from_files(ROOT / 'nvda/wordseg-data', language)
        examples = {'usrdct': 'U S R D C T', 'cgroup': 'C group',
                    'tmpfiles': 'T M P files', 'lvmlockd': 'L V M lock D',
                    'gethostbyaddr': 'get host by addr', 'fdopen': 'F D open'}
        for source, expected in examples.items():
            mixed = ''.join(c.upper() if i % 2 else c.lower() for i, c in enumerate(source))
            for spelling in (source, source.upper(), source.capitalize(), mixed):
                assert segmenter.text(spelling) == expected, (language, spelling, segmenter.text(spelling))
        for word in ('algorithm', 'HELLO', 'éusrdct', 'usrdct3', "usrdct's"):
            assert segmenter.text(word) == word, (language, word)
        assert segmenter.text('cGroup') == 'C group'
        assert segmenter.text('cgroup', frozenset({'cgroup'})) == 'cgroup'
        assert segmenter.text('CGROUP', frozenset({'cgroup'})) == 'CGROUP'
        assert segmenter.text('cgroups', frozenset({'cgroup'})) == 'cgroups'
        assert segmenter.text('cgrouping', frozenset({'cgroup'})) == 'cgrouping'
        assert segmenter.text('eci_usrdct.c') == 'eci_U S R D C T.c'
    with tempfile.TemporaryDirectory() as folder:
        Path(folder, 'ENUmain.dic').write_bytes(b'cgroup\tcustom pronunciation\n')
        Path(folder, 'ENGRoot.dic').write_bytes(b'fdopen\tcustom pronunciation\n')
        Path(folder, 'DEUmain.dic').write_bytes('WLANTest\tMein Test\n'.encode('cp1252'))
        Path(folder, 'DEURoot.dic').write_bytes(b'haus\tcustom pronunciation\n')
        keys = dictionary_keys(folder)
        assert 'cgroup' in keys[0x10000] and 'fdopen' in keys[0x10001]
        assert 'fdopen' not in keys[0x10000] and 'cgroup' not in keys[0x10001]
        assert 'CGROUP' not in keys[0x10000], 'ECI main entries distinguish case'
        assert 'FdOpen' in keys[0x10001] and 'FDOPEN' not in keys[0x10001]
        assert 'WLANTest' in keys[0x40000] and 'wlantest' not in keys[0x40000]
        assert 'HAUS' in keys[0x40000] and 'haus' in keys[0x40000]
        assert segmenter.text('CGROUP', keys[0x10000]) == 'C group'
        assert split_camel('WLANTest und meinTest', keys[0x40000]) == 'WLANTest und mein Test'
        assert list(protected_native_spans('Vor WLANTest nach', keys[0x40000])) == [
            ('Vor ', False), ('WLANTest', True), (' nach', False)]
    samples = {'meinWort': 'mein Wort', 'HTTPServer': 'HTTP Server',
               'XMLHttpRequest': 'XML Http Request', 'StraßenName': 'Straßen Name',
               'δοκιμήΛέξη': 'δοκιμή Λέξη', 'приветМир': 'привет Мир',
               'a\u0308B': 'a\u0308 B', 'schon Richtig': 'schon Richtig'}
    for source, expected in samples.items():
        assert split_camel(source) == expected, (source, split_camel(source))
        assert split_camel(source, {source.casefold()}) == source
    print('Word segmentation: case independence, multilingual camel case, and dictionary precedence passed')


if __name__ == '__main__': main()
