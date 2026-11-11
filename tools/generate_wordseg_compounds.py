#!/usr/bin/env python3
"""Find transparent English compounds with independently supported readings.

Usage: python3 tools/generate_wordseg_compounds.py /path/to/cmudict.dict /path/to/en_UK.txt

The input is CMUdict at 74790861f652b15e4ac49015a90074ad62a27690,
https://github.com/cmusphinx/cmudict, SHA-256 below. This is a maintenance
tool; neither the CMU dictionary nor a network connection is needed at runtime.
See nvda/wordseg-data/NOTICE for CMU's license.
The British reference is ipa-dict en_UK at
972cd67dd5f96bc5660e28070cde26e3928abcd7, based on ipacards under GPL-3.0.

Only accept an unambiguous split whose every whole-word reference reading is
exactly the concatenation of readings of its two OpenEVV pieces. The observed
whole reading must disagree with CMU, and the observed piece readings must
agree with CMU in sounds and main stress. At runtime both observed readings
are checked again, so a later engine or dialect cannot inherit a stale fix.
"""
import argparse
import hashlib
import importlib.util
import json
from collections import defaultdict
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
CMU_SHA256 = '81917843c7f44ce2b094ac63873c2c7a4cf802040792c455ba3ca406891c3d22'
UK_SHA256 = '221394caef0cf723b4f2df81a98ac33191293257b88aed5b1fb89466d3a0dc77'
OUTPUT = ROOT / 'src' / 'reloq_wordseg_compounds.inc'
LEXICONS = ('reloq_english_lexicon.inc', 'reloq_english_vocabulary82.inc',
            'reloq_english_vocabulary83.inc')

sys.path.insert(0, str(ROOT / 'vendor/openevv61/tools'))
spec = importlib.util.spec_from_file_location(
    'wordseg_referee', ROOT / 'vendor/openevv61/tools/measure/referee.py')
referee = importlib.util.module_from_spec(spec)
spec.loader.exec_module(referee)
sys.path.insert(0, str(ROOT / 'vendor/openevv61/tools/measure'))
import wordseg_uk_reference as uk_reference


def cmu_words(path):
    data = path.read_bytes()
    if hashlib.sha256(data).hexdigest() != CMU_SHA256:
        raise ValueError('CMUdict input is not the documented revision')
    result = defaultdict(set)
    for line in data.decode('utf8').splitlines():
        match = re.fullmatch(r'([a-z]+)(?:\(\d+\))? +([A-Z0-9 ]+)', line)
        if match:
            result[match.group(1)].add(tuple(match.group(2).split()))
    return result


def observed_words():
    result = {}
    for path in LEXICONS:
        for line in (ROOT / 'src' / path).read_text(encoding='utf8').splitlines():
            match = re.match(r'\s*\{"([a-z]+)","([^\"]*)",', line)
            if match and re.fullmatch(r'`\[[^\]\r\n]*\]', match.group(2)):
                result.setdefault(match.group(1), match.group(2))
    return result


def without_stress(reading):
    return tuple(re.sub(r'\d', '', phone) for phone in reading)


def candidates(words, pieces, observed):
    for word, readings in sorted(words.items()):
        if len(word) < 6 or len(word) > 32 or word not in observed:
            continue
        boundaries = []
        for at in range(3, len(word) - 2):
            a, b = word[:at], word[at:]
            if a not in pieces or b not in pieces or a not in words or b not in words:
                continue
            combinations = {without_stress(left + right)
                            for left in words[a] for right in words[b]}
            if all(without_stress(reading) in combinations for reading in readings):
                boundaries.append(at)
        if len(boundaries) != 1:
            continue
        at = boundaries[0]
        first, second = observed.get(word[:at]), observed.get(word[at:])
        if not first or not second:
            continue
        # Match wordseg_compound_spr: the first stress in a following part is
        # secondary, and its phones are inserted into the same SPR word.
        if second.startswith('`[.1'):
            second = '`[.2' + second[4:]
        combined = first[:-1] + second[2:]
        old_fits = referee.fits(referee.ours(observed[word]), readings)
        new_fits = referee.fits(referee.ours(combined), readings)
        if not old_fits[0] and all(new_fits):
            yield word, at, observed[word], combined


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('cmudict', type=Path)
    parser.add_argument('uk_ipa', type=Path)
    args = parser.parse_args()
    if hashlib.sha256(args.uk_ipa.read_bytes()).hexdigest() != UK_SHA256:
        raise ValueError('British IPA input is not the documented revision')
    pieces = {line for line in
              (ROOT / 'nvda/wordseg-data/enus-pieces').read_text(encoding='utf8').splitlines()
              if re.fullmatch('[a-z]+', line)}
    words = cmu_words(args.cmudict)
    original = observed_words()
    base = list(candidates(words, pieces, original))
    # Measure both dialects in the native core at the source revision recorded
    # in this fixture. Consider its independently selected candidates and their
    # regular inflections. Every measured word still passes the same CMU sound,
    # stress and unique-boundary checks, separately for each dialect.
    measured = json.loads((ROOT / 'tests/data/wordseg-english-observed.json').read_text(encoding='utf8'))
    if measured['source_commit'] != '8dd892cb1670295e16ba938257d435ed98d5df2f':
        raise ValueError('unexpected English observations')
    for dialect in ('enus', 'engb'):
        if any(not re.fullmatch(r'[a-z]+', word) or
               not re.fullmatch(r'`\[[^\]\"\\\r\n]*\]', reading)
               for word, reading in measured['readings'][dialect].items()):
            raise ValueError('invalid English observation')
    allowed = {word: at for word, at, _, _ in base}
    for word, at, _, _ in base:
        for suffix in ('s', 'es', 'ed', 'ing'):
            allowed.setdefault(word + suffix, at)
    american_observed = dict(original)
    american_observed.update(measured['readings']['enus'])
    american = [item for item in candidates(words, pieces, american_observed)
                if allowed.get(item[0]) == item[1]]
    british = {item[0]: item for item in candidates(words, pieces, measured['readings']['engb'])
               if allowed.get(item[0]) == item[1]}
    # The US phoneme reference cannot decide British variants such as LOT or
    # non-rhotic vowels. Where an independent British reference is available,
    # retain a correct native word and add a split only if its British parts
    # match the reference and the whole word does not.
    british_observed = measured['readings']['engb']
    references = uk_reference.read_references(args.uk_ipa, {item[0] for item in american})
    for word, at, _, _ in american:
        first, second = british_observed.get(word[:at]), british_observed.get(word[at:])
        whole, readings = british_observed.get(word), references.get(word)
        if not first or not second or not whole:
            continue
        if second.startswith('`[.1'):
            second = '`[.2' + second[4:]
        joined = first[:-1] + second[2:]
        old_fits = uk_reference.matches(whole, readings or ())
        new_fits = uk_reference.matches(joined, readings or ())
        if old_fits and not new_fits:
            british.pop(word, None)
        elif not new_fits and any(phone in whole + joined for phone in '@L'):
            # These British phones could not pass the former US-only
            # validation. Do not silently activate an unverified correction
            # merely because the parser now accepts them.
            british.pop(word, None)
        elif new_fits and not old_fits:
            british[word] = (word, at, whole, joined)
    british = sorted(british.values())
    lines = ['/* Generated by tools/generate_wordseg_compounds.py; do not edit. */\n']
    for name, values in (('enus', american), ('engb', british)):
        lines.append(f'static const struct wordseg_verified_compound wordseg_verified_{name}[] = {{\n')
        for word, at, original, combined in values:
            lines.append(f'    {{"{word}",{at},"{original}","{combined}"}},\n')
        lines.append('};\n')
    OUTPUT.write_text(''.join(lines), encoding='ascii')
    print(f'Wrote {len(american)} US and {len(british)} British reference-checked compounds to {OUTPUT}')


if __name__ == '__main__':
    main()
