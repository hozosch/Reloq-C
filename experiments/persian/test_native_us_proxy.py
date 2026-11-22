"""Compare the C US-contour planner with the established Persian planner."""
from __future__ import annotations
import argparse
import ctypes as c
from pathlib import Path
import re
import random
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT / 'experiments/persian'), str(ROOT / 'nvda')]
from _reloq_eci_direct import EciIndex, PROCESS_TAGS
from continuous_driver import _PlanningVoice
from continuous_plan import _indexed_us_proxy_python, indexed_us_proxy
from direct_fa_sequence import project
from nvda_driver_probe import make_language_engine, PERSIAN_PROBE, US_ENGLISH

SINK = c.CFUNCTYPE(c.c_int, c.c_void_p, c.c_void_p, c.c_size_t)
PROBE = c.CFUNCTYPE(c.c_int, c.c_void_p, c.c_void_p, c.c_size_t, SINK, c.c_void_p)

class Result(c.Structure):
    _fields_ = [('text', c.c_void_p), ('length', c.c_size_t),
                ('offsets', c.POINTER(c.c_size_t)), ('words', c.c_size_t)]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--proxy-library', type=Path)
    args = parser.parse_args()
    lib = c.CDLL(str(args.proxy_library.resolve())) if args.proxy_library else None
    if lib:
        lib.reloq_fa_us_proxy_make.argtypes = [c.c_char_p, c.c_size_t, PROBE, c.c_void_p, c.POINTER(Result)]
        lib.reloq_fa_us_proxy_make.restype = c.c_int
        lib.reloq_fa_us_proxy_make_reusing.argtypes = lib.reloq_fa_us_proxy_make.argtypes
        lib.reloq_fa_us_proxy_make_reusing.restype = c.c_int
        lib.reloq_fa_us_proxy_clear.argtypes = [c.POINTER(Result)]
    owner = make_language_engine(args.library, args.library, None,
                                 Path('/unavailable-espeak'), Path('/unavailable'))
    key = owner._key(US_ENGLISH.id, '1')
    en = _PlanningVoice(owner, key, owner._activate(key))
    en.raw.set_parameter(PROCESS_TAGS, 1)
    fa_key = owner._key(PERSIAN_PROBE.id, '1')
    fa = owner._activate(fa_key)
    errors, probes = [], []

    @PROBE
    def probe(_ctx, src, length, sink, sink_ctx):
        try:
            text = c.string_at(src, length).decode('utf-8')
            probes.append(text)
            result = en._phonemes_for_probe(text).encode('cp1252')
            # A probe may deliver multiple buffers; no C allocator crosses
            # the DLL boundary, including on Windows.
            middle = len(result) // 2
            for piece in (result[:middle], result[middle:]):
                if piece and sink(sink_ctx, piece, len(piece)):
                    return -2
            return 0
        except BaseException as exc:
            errors.append(exc)
            return -1

    def native(text, *, reuse=False):
        if not lib:
            count = len(re.findall(r'`\[[^]]+\]', text))
            return collect(indexed_us_proxy(text, en, tuple(range(count-1))))
        data = text.encode('utf-8')
        output = Result()
        call = lib.reloq_fa_us_proxy_make_reusing if reuse else lib.reloq_fa_us_proxy_make
        status = call(data, len(data), probe, None, c.byref(output))
        if errors:
            raise errors.pop()
        assert status == 0, (text, status)
        try:
            return (c.string_at(output.text, output.length),
                    tuple(output.offsets[i] for i in range(output.words-1)))
        finally:
            lib.reloq_fa_us_proxy_clear(c.byref(output))

    def collect(parts):
        content, offsets = bytearray(), []
        for part in parts:
            if isinstance(part, EciIndex):
                offsets.append(len(content))
            else:
                content.extend(part.encode('utf-8'))
        return bytes(content), tuple(offsets)

    def python(text):
        count = len(re.findall(r'`\[[^]]+\]', text))
        return collect(_indexed_us_proxy_python(text, en, tuple(range(count-1))))

    if not lib:
        assert getattr(en._dll, 'eciPersianUsProxyUtf8', None), 'native proxy API unavailable'

    manual = (
        '`[.1sefr]', '`[.1sAdr]', '`[.1fra]', '`[.0ket.1Ab] `00 `[.0ra].',
        '`[.1A]~`[.1u]', '`[.0bA.1ra.0je] (`[.1in]), `[.1bud]?',
        "`[.0'j:'u:' .1'A:'] `[.1J]',`[.1A]",
        '`[.2ABC.0Go] `[.0q.0x]', '`[.0a] `[.0u]',
        '`[.1ba]\u00a0`[.1di]\u2009`[.1go].',
        '`[.1ba] `00 `[.0di] `[.1go], `[.1u].',
        # Repeated words used to cause the same isolated pronunciation probe
        # once per occurrence, even within one uncached sentence.
        ' '.join(('`[.0sA.1lam]', '`[.1ir]', '`[.0sA.1lam]', '`[.1ir]') * 5),
    )
    texts = (
        'برای راه‌اندازی رایانه باید این گزینه انتخاب شود.',
        'سلام دنیا.', 'کتاب و خانه.', 'کتاب را.', 'خوب است.',
        'صفر، یک، دو.', 'این عدد صفر است.', 'قدر.', 'دو؟', '66',
        'نمایشِ اِتِّفاقاتِ ثبت‌شده', 'خواجه‌ئیان',
        'استفاده از رایانه هنگامِ ورود به ویندوز',
        'این (یک) مثال است؛ درست؟',
    )
    try:
        tests = list(manual)
        for text in texts:
            tagged = project((text,), fa._frontend, allow_fallback=False).tagged
            if re.search(r'`\[[^]]+\]', tagged):
                tests.append(tagged)
        rng = random.Random(140019)
        # Vary phonetic identity, stress, unsupported phones and boundaries.
        # The actual English parser, not a mock, judges each candidate.
        for _ in range(400):
            words = []
            for _word in range(rng.randint(1, 5)):
                syllables = []
                count = rng.randint(1, 3)
                for i in range(count):
                    stress = rng.choice('012') if i == count-1 else '0'
                    syllables.append('.' + stress + rng.choice(('', 'b', 's', 'R', 'J', 'x', 'q')) +
                                     rng.choice(('a', 'A', 'e', 'i', 'o', 'u')) +
                                     rng.choice(('', 'r', 'm', 't', 'fr')))
                words.append('`[' + ''.join(syllables) + ']')
            value = words[0]
            for word in words[1:]:
                value += rng.choice((' ', '\t', '\u00a0', '~', ', ', ') (',
                                     ' `00 ', ' `00\u00a0')) + word
            tests.append(value + rng.choice(('', '.', '?')))
        for text in tests:
            actual, expected = native(text), python(text)
            assert actual == expected, (text, actual, expected)
        if lib:
            # A deterministic worker can reuse the isolated check of an
            # identical weak tag only during this one request. The general
            # callback API retains its original per-occurrence contract.
            repeated = ' '.join(('`[.0ne.0mi]',) * 12)
            probes.clear(); ordinary = native(repeated); ordinary_calls = len(probes)
            probes.clear(); optimized = native(repeated, reuse=True); optimized_calls = len(probes)
            assert optimized == ordinary and ordinary_calls > optimized_calls and optimized_calls == 2, (
                ordinary_calls, optimized_calls)
        print(f'Passed {len(tests)} native US-proxy comparisons against the established planner.')
    finally:
        owner.close()

if __name__ == '__main__':
    main()
