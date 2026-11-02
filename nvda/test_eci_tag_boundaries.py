"""Causal word-boundary checks, independent of tag spelling equivalence.

Fresh engines speak the identical lexical sentence. A changed tag must leave
every sample and the public index before its word untouched, while changing
the following audio. Audio stays in RAM.
"""
import argparse
import ctypes as C
import json
from pathlib import Path

from test_eci_inline_queue import Engine, TEXTS

TAGS = ('vv50', 'vb80', 'vs120', 'vf70', 'vg1', 'vh30', 'vr40', 'vy40', 'v2')


def capture(library, language, prefix, tail):
    e = Engine(library, language)
    try:
        e.add(prefix)
        assert e.lib.eciInsertIndex(e.h, 17)
        e.add(tail)
        data = e.finish()
        assert len(e.indices) == 1 and e.indices[0][0] == 17, e.indices
        return data, e.indices[0][1]
    finally:
        e.close()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--library', type=Path, required=True)
    p.add_argument('--language', type=lambda s: int(s, 0), action='append')
    p.add_argument('--output', type=Path)
    p.add_argument('--report-only', action='store_true')
    p.add_argument('--all-languages', action='store_true', help='Include diagnostic German/CJK cases')
    a = p.parse_args()
    rows = []
    pitch_checks = []
    languages=list(TEXTS) if a.all_languages else [x for x in TEXTS if x not in (0x40000,0x60000,0x60001,0x80000,0xa0000)]
    for language in a.language or languages:
        prefix, word, suffix = TEXTS[language]
        if language in (0x10000, 0x10001):
            prefix, word, suffix = 'Please remember the ', 'message', ' with all the different things in this example.'
        elif language == 0x40000:
            prefix, word, suffix = 'Bitte lesen Sie diesen ', 'Text', ' mit allen verschiedenen Dingen in diesem Beispiel.'
        plain, at = capture(a.library, language, prefix, word + suffix)
        for tag in TAGS:
            changed, changed_at = capture(a.library, language, prefix, '`' + tag + ' ' + word + suffix)
            first = next((i for i, (x, y) in enumerate(zip(plain, changed)) if x != y), min(len(plain), len(changed)))
            rows.append(dict(language=hex(language), tag=tag, before=at, after=changed_at,
                first_difference=first, prefix_equal=plain[:at] == changed[:at],
                suffix_changed=plain[at:] != changed[changed_at:], length_ratio=len(changed)/len(plain)))
        def pitched(parts):
            e=Engine(a.library,language)
            try:
                e.lib.eciInsertPitch.argtypes=(C.c_void_p,C.c_int)
                e.lib.eciInsertPitch.restype=C.c_int
                for part in parts:
                    if isinstance(part,int):assert e.lib.eciInsertPitch(e.h,part)
                    else:e.add(part)
                return e.finish()
            finally:e.close()
        for label,actual,expected in (
            ('api-after-inline',[prefix,'`vb80 ',65,word+suffix],[prefix,65,word+suffix]),
            ('inline-after-api',[prefix,65,'`vb80 '+word+suffix],[prefix,'`vb80 '+word+suffix])):
            pitch_checks.append(dict(language=hex(language),case=label,equal=pitched(actual)==pitched(expected)))
    failures = [r for r in rows if r['before'] != r['after'] or not r['prefix_equal'] or not r['suffix_changed']]
    failures.extend(r for r in pitch_checks if not r['equal'])
    result = dict(checks=len(rows)+len(pitch_checks),causal_word_cases=len(rows),pitch_order_cases=len(pitch_checks),
        failures=len(failures), rows=rows,pitch_order=pitch_checks)
    if a.output:
        a.output.write_text(json.dumps(result, ensure_ascii=False, indent=2) + '\n')
    print(json.dumps(result, ensure_ascii=False))
    if failures and not a.report_only:
        raise SystemExit(1)


if __name__ == '__main__':
    main()
