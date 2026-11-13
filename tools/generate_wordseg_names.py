#!/usr/bin/env python3
"""Derive a compact set of name spellings whose native reading must survive.

Run on a POSIX host with a shared ECI library built from the original 1.4.1
candidate e6779f6e8713616676a79d51be025265a52a72ae, and the pinned inputs:

  python3 tools/generate_wordseg_names.py --library /path/to/libreloq_eci.so \
      --cmu /path/to/cmudict.dict --surnames /path/to/Names_2010Census.csv \
      --british /path/to/en_UK.txt

The US Census data are public domain. CMUdict and the British IPA reference
are credited in nvda/wordseg-data/NOTICE. None of these input files is
distributed with the runtime. Both English dialects are measured. When an
independent reference prefers the split, the name is not added; neither are
already reference-verified compounds. The resulting plain source list is
incorporated into the existing compact native word table at build time.
"""
import argparse
from collections import defaultdict
import csv
import ctypes
import hashlib
from pathlib import Path
import re
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
BASELINE = 'e6779f6e8713616676a79d51be025265a52a72ae'
PINNED = {
    'cmu': '81917843c7f44ce2b094ac63873c2c7a4cf802040792c455ba3ca406891c3d22',
    'surnames': '6b4933178060b4e5ac8dd9a611ec15472d35eab98fe9027474bc5ccc5b7a8708',
    'british': '221394caef0cf723b4f2df81a98ac33191293257b88aed5b1fb89466d3a0dc77',
}

sys.path[:0] = [str(ROOT / 'nvda'), str(ROOT / 'vendor/openevv61/tools'),
                str(ROOT / 'vendor/openevv61/tools/measure'), str(ROOT / 'tools')]
from _reloq_eci_direct import DirectEciEngine, WORD_SEGMENTATION
from referee import fits, ours
from wordseg_uk_reference import matches, read_references


def checked(path, expected):
    data = path.read_bytes()
    if hashlib.sha256(data).hexdigest() != expected:
        raise ValueError(f'unexpected source revision: {path}')
    return data


def baseline_segmenter(directory):
    for name in ('src/reloq_wordseg.c', 'src/reloq_wordseg_data.inc',
                 'src/reloq_wordseg_compounds.inc', 'include/reloq_wordseg.h'):
        target = directory / name
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(subprocess.check_output(['git', '-C', str(ROOT), 'show',
                                                    f'{BASELINE}:{name}']))
    output = directory / 'libwordseg.so'
    subprocess.run(['cc', '-O2', '-std=c11', '-fPIC', '-shared',
                    '-I' + str(directory / 'include'),
                    str(directory / 'src/reloq_wordseg.c'), '-o', str(output)], check=True)
    function = ctypes.CDLL(str(output)).reloq_wordseg_english
    function.argtypes = [ctypes.c_char_p, ctypes.c_size_t, ctypes.c_int,
                         ctypes.c_char_p, ctypes.c_size_t]
    return function


def split(function, word, uk):
    text = word.encode('ascii')
    out = ctypes.create_string_buffer(256)
    result = function(text, len(text), uk, out, len(out))
    if result < 0:
        raise ValueError(f'could not segment {word}')
    return out.value.decode('ascii').split() if result else []


def transparent(word, parts, readings):
    """Keep a split supported by every complete CMU reading, stress included."""
    if len(parts) != 2 or any(part not in readings for part in parts):
        return False
    combinations = {left + tuple(re.sub(r'1$', '2', phone) for phone in right)
                    for left in readings[parts[0]] for right in readings[parts[1]]}
    return bool(readings[word]) and all(reading in combinations for reading in readings[word])


def phones(engine, word, enabled):
    engine.set_parameter(WORD_SEGMENTATION, enabled)
    reading = ''.join(re.findall(r'`\[[^]]+\]', engine._phonemes_for_probe(word)))
    if not reading:
        raise ValueError(f'no native pronunciation for {word}')
    return reading


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('library', 'cmu', 'surnames', 'british'):
        parser.add_argument('--' + name, type=Path, required=True)
    args = parser.parse_args()
    for name, digest in PINNED.items():
        checked(getattr(args, name), digest)
    readings = defaultdict(list)
    for line in args.cmu.read_text(encoding='utf8').splitlines():
        parts = line.split()
        if len(parts) > 1:
            readings[re.sub(r'\(\d+\)$', '', parts[0])].append(tuple(parts[1:]))

    with tempfile.TemporaryDirectory() as temporary:
        directory = Path(temporary)
        function = baseline_segmenter(directory)
        verified = set(re.findall(r'\{"([a-z]+)",',
            (directory / 'src/reloq_wordseg_compounds.inc').read_text(encoding='ascii')))
        candidates = []
        with args.surnames.open(encoding='utf8', newline='') as source:
            for row in csv.DictReader(source):
                word = row['name'].lower()
                rank = int(row['rank'])
                if (not word.isascii() or not word.isalpha() or len(word) > 32 or
                        word in verified or (word not in readings and rank > 10000)):
                    continue
                parts = split(function, word, 0) or split(function, word, 1)
                if parts:
                    candidates.append((word, parts))
        british = read_references(args.british, {word for word, _ in candidates})
        engines = [DirectEciEngine(args.library.resolve(), dialect=language,
                                   encoding='utf8', native_voice=True)
                   for language in (0x10000, 0x10001)]
        names = []
        try:
            for word, parts in candidates:
                us, uk = [(phones(engine, word, 0), phones(engine, word, 1)) for engine in engines]
                if us[0] == us[1] and uk[0] == uk[1]:
                    continue
                values = readings.get(word)
                if values:
                    old, divided = fits(ours(us[0]), values), fits(ours(us[1]), values)
                    if (divided[0] and not old[0]) or transparent(word, parts, readings):
                        continue
                if british.get(word) and matches(uk[1], british[word]) and not matches(uk[0], british[word]):
                    continue
                names.append(word)
        finally:
            for engine in engines:
                engine.close()
    target = ROOT / 'nvda/wordseg-data/en-protected-names'
    target.write_text('# Generated by tools/generate_wordseg_names.py; do not edit.\n' +
                      ''.join(word + '\n' for word in sorted(set(names))), encoding='ascii')
    print(f'Wrote {len(set(names))} source names to {target}')


if __name__ == '__main__':
    main()
