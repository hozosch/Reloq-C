"""Build and verify the independent Persian linguistic planner.

This deliberately links no language/acoustic provider. It is an analysis test,
not evidence of a complete Persian voice or of an audible result.
"""
from __future__ import annotations
import argparse
import ctypes
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
WORD_BEGIN, PHONE, WORD_END, PUNCTUATION, WEAK, GLOTTAL_ONSET, VARIANT = range(1, 8)


class Event(ctypes.Structure):
    _fields_ = [(name, ctypes.c_uint32) for name in ('kind', 'value', 'flags', 'syllable', 'stress')]


class LegacyPhone(ctypes.Structure):
    _fields_ = [(name, ctypes.c_uint32) for name in ('id', 'flags', 'syllable', 'stress')]


def build(directory: Path, compiler: str) -> Path:
    shim = directory / 'export.c'
    shim.write_text(r'''
#include <stddef.h>
#include <stdlib.h>
#include "reloq_persian_plan.h"
#include "reloq_persian_phones.h"
__attribute__((visibility("default")))
int fa_analyze(const char *t,size_t n,int m,const size_t *e,const unsigned char *k,size_t r,
 const size_t *s,const unsigned char *i,size_t c,struct reloq_fa_plan_event **o,size_t *z,size_t *p,size_t *x)
{return reloq_fa_analyze_runs(t,n,m,e,k,r,s,i,c,o,z,p,x);}
__attribute__((visibility("default")))
int fa_legacy(const char *t,size_t n,struct reloq_fa_phone *p,size_t cap,size_t *z)
{return reloq_fa_phones_from_legacy(t,n,p,cap,z,NULL);}
__attribute__((visibility("default"))) void fa_release(void *p){free(p);}
''', encoding='utf8')
    lib = directory / 'persian-standalone.so'
    sources = ['reloq_persian_plan.c', 'reloq_persian_mixed_project.c',
               'reloq_persian_pinglish_phrase.c', 'reloq_persian_phones.c',
               'reloq_utf8.c', 'reloq_unicode.c', 'reloq_unicode_names.c']
    files = [ROOT / 'src' / name for name in sources]
    files.extend(sorted((ROOT / 'src').glob('reloq_fa_direct_*.c')))
    subprocess.run([compiler, '-std=c11', '-O1', '-fPIC', '-fvisibility=hidden',
        '-ffunction-sections', '-fdata-sections', '-shared', '-Wl,--gc-sections,--no-undefined',
        f'-I{ROOT / "include"}', f'-I{ROOT / "src"}', str(shim), *map(str, files), '-o', str(lib)], check=True)
    symbols = subprocess.check_output(['nm', str(lib)], text=True).lower()
    assert not re.search(r'fifi|finnish|espeak|reloq61_worker|delta_lang', symbols)
    return lib


class Analyzer:
    def __init__(self, path: Path):
        self.lib = ctypes.CDLL(str(path))
        self.call = self.lib.fa_analyze
        self.call.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.c_int,
            ctypes.POINTER(ctypes.c_size_t), ctypes.POINTER(ctypes.c_ubyte), ctypes.c_size_t,
            ctypes.POINTER(ctypes.c_size_t), ctypes.POINTER(ctypes.c_ubyte), ctypes.c_size_t,
            ctypes.POINTER(ctypes.POINTER(Event)), ctypes.POINTER(ctypes.c_size_t),
            ctypes.POINTER(ctypes.c_size_t), ctypes.POINTER(ctypes.c_size_t))
        self.call.restype = ctypes.c_int
        self.lib.fa_release.argtypes = (ctypes.c_void_p,)
        self.legacy = self.lib.fa_legacy
        self.legacy.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.POINTER(LegacyPhone),
                               ctypes.c_size_t, ctypes.POINTER(ctypes.c_size_t))
        self.legacy.restype = ctypes.c_int

    def analyze(self, text, *, mode=0, runs=None, offsets=(), indexes=None):
        raw = text.encode('utf8')
        runs = runs if runs is not None else ([(len(raw), 0)] if raw else [])
        ends = (ctypes.c_size_t * len(runs))(*(end for end, _ in runs))
        kinds = (ctypes.c_ubyte * len(runs))(*(kind for _, kind in runs))
        positions = (ctypes.c_size_t * len(offsets))(*offsets)
        indexes = [1] * len(offsets) if indexes is None else indexes
        flags = (ctypes.c_ubyte * len(offsets))(*indexes)
        targets = (ctypes.c_size_t * len(offsets))(*([123456] * len(offsets)))
        output = ctypes.POINTER(Event)()
        count, error = ctypes.c_size_t(), ctypes.c_size_t()
        result = self.call(raw, len(raw), mode, ends, kinds, len(runs), positions, flags,
                           len(offsets), ctypes.byref(output), ctypes.byref(count), targets, ctypes.byref(error))
        values = [(e.kind, e.value, e.flags, e.syllable, e.stress) for e in output[:count.value]]
        self.lib.fa_release(output)
        if result:
            assert not values and list(targets) == [123456] * len(offsets)
        return result, values, list(targets), error.value


def words(events):
    return [event for event in events if event[0] != VARIANT]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--cc', default='cc')
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='reloq-fa-plan-') as temp:
        core = Analyzer(build(Path(temp), args.cc))
        analyze = core.analyze
        for fa, latin in [('سلام', 'salam'), ('دنیا', 'donya'), ('ایران', 'iran'),
                          ('کتاب', 'ketab'), ('سلام دنیا', 'salam donya')]:
            left, right = analyze(fa), analyze(latin, mode=1)
            assert left[0] == right[0] == 0 and left[1] == right[1], (fa, latin, left, right)
        for number in ['0', '12', '66', '123', '3.14']:
            persian = number.translate(str.maketrans('0123456789.', '۰۱۲۳۴۵۶۷۸۹٫'))
            left, right = analyze(number), analyze(persian)
            assert left[0] == right[0] == 0 and left[1] == right[1], (number, left, right)
        # Amir's reports: final H remains one phone, and the saved stress
        # patterns survive the transfer to independent Persian identities.
        solh = analyze('صلح')
        solh_phones = [e for e in solh[1] if e[0] == PHONE]
        assert solh[0] == 0 and solh_phones[-1][1] == 22 and {e[3] for e in solh_phones} == {0}
        for word, stressed_syllable in [('پیمانکاران', 3), ('صدای', 1)]:
            result, events, _, _ = analyze(word)
            primary = {e[3] for e in events if e[0] == PHONE and e[4] == 1}
            assert result == 0 and primary == {stressed_syllable}, (word, result, primary)
        switched = analyze('سلام `l20.1 salam `l20.0 دنیا')
        assert switched[0] == 0 and [v[1] for v in switched[1] if v[0] == VARIANT] == [1, 0], switched
        expected = analyze('سلام سلام دنیا')
        assert expected[0] == 0 and words(switched[1]) == expected[1], (switched, expected)
        assert analyze('`l20.2 salam')[0] == 1
        assert analyze('`l127.1 salam')[0] == 1
        assert analyze('`l9.0 salam')[0] == 1
        for text, ids in [('`[.1qa]', [13, 2]), ('`[.1xa]', [21, 2]),
                          ('`[.1?a]', [14, 2]), ('`[.1h?a]', [22, 14, 2]),
                          ('`[.1h?]', [22, 14]), ('`[s]', [17])]:
            result, events, _, _ = analyze(text, runs=[(len(text), 2)])
            assert result == 0 and [v[1] for v in events if v[0] == PHONE] == ids, (text, result, events)
        invalid = '`[.1B]'
        spelled = analyze(invalid, runs=[(len(invalid), 2)])
        explicit = analyze(invalid, runs=[(len(invalid), 1)])
        assert spelled[0] == explicit[0] == 0 and spelled[1] == explicit[1]
        assert sum(e[0] == WORD_BEGIN for e in spelled[1]) > 1
        inline = analyze('سلام `[' + '.1B] دنیا')
        separate = analyze('سلام `[' + '.1B] دنیا',
            runs=[(len('سلام '.encode()), 0), (len('سلام `[.1B]'.encode()), 2),
                  (len('سلام `[.1B] دنیا'.encode()), 0)])
        assert inline[0] == separate[0] == 0 and inline[1] == separate[1]
        inline_pinglish = analyze('salam `[.1qa] donya', mode=1)
        assert inline_pinglish[0] == 0 and any(e[0] == PHONE and e[1] == 13 for e in inline_pinglish[1])
        for symbol in ['+', '؟', 'Ж', '😀']:
            encoded = symbol.encode()
            result, events, _, _ = analyze(symbol, runs=[(len(encoded), 1)])
            assert result == 0 and any(e[0] == PHONE for e in events), (symbol, result, events)
        sentence = 'سلام `l20.1 salam ۱۲۳'
        encoded = sentence.encode()
        result, events, mapped, _ = analyze(sentence, offsets=range(len(encoded) + 1))
        assert result == 0 and mapped == sorted(mapped) and all(0 <= p <= len(events) for p in mapped)
        # Splitting adjacent normal text runs must preserve the analysis.
        split = len('سلام '.encode())
        split_result = analyze(sentence, runs=[(split, 0), (len(encoded), 0)], offsets=range(len(encoded) + 1))
        assert (result, events, mapped) == split_result[:3]
        # A non-index control may not be moved through the middle of a word.
        result, _, targets, _ = analyze('سلام', offsets=[2], indexes=[0])
        assert result == 3 and targets == [123456]
        # Unknown text is explicitly rejected; it cannot enter a Finnish parser.
        result, _, _, _ = analyze('𐀀')
        assert result == 1
        # Decode the complete set of stored word groups, not just selected examples.
        tags = set()
        for path in (ROOT / 'experiments/persian').glob('direct_fa_*.inc'):
            tags.update(re.findall(r'`\[[^\]]+\]', path.read_text(encoding='utf8')))
        failures = []
        compared = 0
        for tag in tags:
            result, events, _, _ = analyze(tag, runs=[(len(tag.encode()), 2)])
            if result or sum(e[0] == WORD_BEGIN for e in events) != 1:
                failures.append((tag, result, events[:4]))
                if len(failures) >= 10:
                    break
            raw = tag.encode()
            length = ctypes.c_size_t()
            if core.legacy(raw, len(raw), None, 0, ctypes.byref(length)) == 0:
                buffer = (LegacyPhone * length.value)()
                assert core.legacy(raw, len(raw), buffer, len(buffer), ctypes.byref(length)) == 0
                expected_phones = [(p.id, p.flags, p.syllable, p.stress) for p in buffer]
                actual_phones = [(e[1], e[2], e[3], e[4]) for e in events if e[0] == PHONE]
                # In stored internal data, final hw encodes H plus a silent
                # backend carrier. Explicit input otherwise preserves w.
                if tag.endswith('hw]') and actual_phones[-1][0] == 14:
                    actual_phones.pop()
                assert actual_phones == expected_phones, (tag, expected_phones, actual_phones)
                compared += 1
        assert not failures, failures
        assert compared > 45000, compared
        print(f'Passed isolated Persian/Pinglish analysis, {len(tags)} stored groups and {compared} exact phone comparisons; no language provider linked.')


if __name__ == '__main__':
    main()
