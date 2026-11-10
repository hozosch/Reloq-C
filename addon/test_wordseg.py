"""Regression coverage for optional unknown-English-word segmentation."""
import tempfile
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_wordseg import WordSegmenter, dictionary_keys


def main():
    for language in ('enus', 'engb'):
        segmenter = WordSegmenter.from_files(ROOT / 'nvda/wordseg-data', language)
        examples = {'usrdct': 'U S R D C T', 'cgroup': 'C group',
                    'tmpfiles': 'T M P files', 'lvmlockd': 'L V M lock D',
                    'gethostbyaddr': 'get host by addr', 'fdopen': 'F D open'}
        for source, expected in examples.items():
            assert segmenter.text(source) == expected, (language, source, segmenter.text(source))
        for word in ('algorithm', 'hello', 'cGroup', 'éusrdct', 'usrdct3', "usrdct's"):
            assert segmenter.text(word) == word, (language, word)
        assert segmenter.text('cgroup', frozenset({'cgroup'})) == 'cgroup'
        assert segmenter.text('cgroups', frozenset({'cgroup'})) == 'cgroups'
        assert segmenter.text('cgrouping', frozenset({'cgroup'})) == 'cgrouping'
        assert segmenter.text('eci_usrdct.c') == 'eci_U S R D C T.c'
    with tempfile.TemporaryDirectory() as folder:
        Path(folder, 'ENUmain.dic').write_bytes(b'cgroup\tcustom pronunciation\n')
        Path(folder, 'ENGRoot.dic').write_bytes(b'fdopen\tcustom pronunciation\n')
        keys = dictionary_keys(folder)
        assert 'cgroup' in keys['enus'] and 'fdopen' in keys['engb']
        assert 'fdopen' not in keys['enus'] and 'cgroup' not in keys['engb']
    print('Word segmentation: issue examples, safe boundaries and dictionary precedence passed')


if __name__ == '__main__': main()
