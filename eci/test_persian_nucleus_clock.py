"""Compare Persian vowel anchors with actual native syllable/phone clocks.

This exercises a preparatory C alignment helper, not a new voice. Captures use
separate diagnostic passes; a future incremental voice must supply these
clocks during normal synthesis instead of making additional passes.
"""
from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor
import ctypes as c
from pathlib import Path
import random
import sys

from test_persian_owned_capture import NativeFrame, NativeSyllable
from test_persian_syllable_plan_clock import Event


class Anchor(c.Structure):
    _fields_ = (('syllable_from', c.c_int32), ('syllable_to', c.c_int32),
                ('nucleus_from', c.c_int32), ('nucleus_to', c.c_int32),
                ('word', c.c_uint32), ('syllable', c.c_uint32), ('stress', c.c_uint32),
                ('weak', c.c_uint32), ('nucleus', c.c_uint32), ('bridged_j', c.c_uint32),
                ('syllable_extended', c.c_uint32))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--dump-identities', action='store_true')
    args = parser.parse_args()
    loader = c.WinDLL if sys.platform == 'win32' else c.CDLL
    dll = loader(str(args.library.resolve()))
    H, I, S, B = c.c_void_p, c.c_int, c.c_size_t, c.c_ubyte
    callback_type = (c.WINFUNCTYPE if sys.platform == 'win32' else c.CFUNCTYPE)(I, H, I, I, H)
    functions = (
        ('eciNewEx', H, (I,)), ('eciDelete', H, (H,)), ('eciSetParam', I, (H, I, I)),
        ('eciSetOutputBuffer', I, (H, I, c.POINTER(c.c_short))),
        ('eciPersianCaptureBegin', I, (S,)), ('eciPersianCaptureSetControlsOnly', I, (I,)),
        ('eciPersianCaptureSetPhoneTrace', I, (I,)),
        ('eciPersianSpeakNativeUtf8', I, (H, c.c_char_p, S, I, c.POINTER(S), c.POINTER(I), S, c.POINTER(S))),
        ('eciPersianAnalyzeRunsCompleteUtf8', I, (c.c_char_p, S, I, c.POINTER(S), c.POINTER(B), S,
            c.POINTER(S), c.POINTER(B), S, c.POINTER(c.POINTER(Event)), c.POINTER(S),
            c.POINTER(S), c.POINTER(S))),
        ('eciPersianRelease', None, (H,)),
        ('eciPersianCaptureEnd', c.POINTER(NativeFrame), (c.POINTER(S), c.POINTER(I))),
        ('eciPersianCaptureSyllables', c.POINTER(NativeSyllable), (c.POINTER(S), c.POINTER(I))),
        ('eciPersianCapturePhoneIdentities', c.POINTER(c.c_uint32), (c.POINTER(S), c.POINTER(I))),
        ('eciPersianCaptureRelease', None, ()),
    )
    for name, result, parameters in functions:
        function = getattr(dll, name)
        function.restype, function.argtypes = result, parameters
    dll.eciRegisterCallback.argtypes = (H, callback_type, H)
    align = dll.eciPersianAlignNucleusClocks
    align.restype = I
    align.argtypes = (c.POINTER(Event), S, c.POINTER(NativeSyllable), S,
                      c.POINTER(NativeSyllable), S, c.POINTER(c.c_uint32), S,
                      c.POINTER(Anchor), S, c.POINTER(S))

    @callback_type
    def callback(_handle, _message, _value, _context):
        return 1

    def analyze(raw, variant):
        end, mode, events, count, error = S(len(raw)), B(0), c.POINTER(Event)(), S(), S()
        status = dll.eciPersianAnalyzeRunsCompleteUtf8(
            raw, len(raw), variant, c.byref(end), c.byref(mode), 1, None, None, 0,
            c.byref(events), c.byref(count), None, c.byref(error))
        assert status == 0, (raw, variant, status, error.value)
        try:
            return (Event * count.value)(*(Event(e.kind, e.value, e.flags, e.syllable, e.stress)
                                          for e in events[:count.value]))
        finally:
            dll.eciPersianRelease(events)

    def capture(voice, raw, variant, phone_trace):
        assert dll.eciPersianCaptureBegin(30000) == 0
        try:
            assert dll.eciPersianCaptureSetControlsOnly(1) == 0
            assert dll.eciPersianCaptureSetPhoneTrace(phone_trace) == 0
            error = S()
            status = dll.eciPersianSpeakNativeUtf8(voice, raw, len(raw), variant, None, None, 0, c.byref(error))
            assert status == 0, (raw, variant, status, error.value)
            count, failed = S(), I()
            dll.eciPersianCaptureEnd(c.byref(count), c.byref(failed))
            assert not failed.value and count.value
            clocks = dll.eciPersianCaptureSyllables(c.byref(count), c.byref(failed))
            assert not failed.value and clocks and count.value
            result = (NativeSyllable * count.value)(*(NativeSyllable(clocks[i].from_, clocks[i].to)
                                                       for i in range(count.value)))
            identities = ()
            if phone_trace:
                ids = dll.eciPersianCapturePhoneIdentities(c.byref(count), c.byref(failed))
                assert not failed.value and ids and count.value == len(result), (raw, count.value, len(result))
                identities = tuple(ids[:count.value])
            return result, identities
        finally:
            dll.eciPersianCaptureRelease()
            count, failed = S(999), I(999)
            assert not dll.eciPersianCapturePhoneIdentities(c.byref(count), c.byref(failed))
            assert count.value == 0 and failed.value == 0

    def check(voice, text, variant, expect_bridged=False, expect_extended=False):
        raw = text.encode('utf8')
        events = analyze(raw, variant)
        syllables, _ = capture(voice, raw, variant, 0)
        phones, identities = capture(voice, raw, variant, 1)
        slots = (c.c_uint32 * len(identities))(*identities)
        n = S()
        status = align(events, len(events), syllables, len(syllables), phones, len(phones),
                       slots, len(slots), None, 0, c.byref(n))
        assert status == 0 and n.value == len(syllables), (text, status, n.value, len(syllables))
        anchors = (Anchor * n.value)()
        status = align(events, len(events), syllables, len(syllables), phones, len(phones),
                       slots, len(slots), anchors, len(anchors), c.byref(n))
        assert status == 0 and n.value == len(syllables), (text, status)
        # Independently map vowel events to their recorded provider intervals.
        # Phone and syllable clocks can disagree on doubled J, but never on
        # the vowel clock used as an intonation anchor.
        p, expected, weak, word, pending_j = 0, [], 0, -1, False
        for at, e in enumerate(events):
            if e.kind == 5:
                weak = 1
            elif e.kind == 1:
                word += 1
            elif e.kind == 2:
                if e.value == 22:  # H is a feature, not a separate traced phone.
                    continue
                identity_map.setdefault(e.value, set()).add(identities[p])
                if 1 <= e.value <= 6 or e.value in (30, 31):
                    expected.append((phones[p].from_, phones[p].to, word, e.syllable,
                                     e.stress, weak, e.value))
                if e.value == 23:  # CH has stop and fricative clocks.
                    assert identities[p:p+2] == (2, 11)
                    p += 2
                elif e.value == 24:
                    if pending_j:
                        assert identities[p] == 12
                        pending_j = False
                        p += 1
                    elif at+1 < len(events) and events[at+1].kind == 2 and events[at+1].value == 24:
                        assert identities[p] == 5
                        pending_j = True
                        p += 1
                    else:
                        assert identities[p:p+2] == (5, 12)
                        p += 2
                else:
                    p += 1
            elif e.kind == 3:
                weak = 0
            elif e.kind == 4 and e.value in (ord(','), 0x060c):
                p += 1
        actual = [(a.nucleus_from, a.nucleus_to, a.word, a.syllable, a.stress, a.weak, a.nucleus)
                  for a in anchors]
        assert actual == expected, (text, actual, expected)
        for i, anchor in enumerate(anchors):
            assert anchor.syllable_from == syllables[i].from_
            assert anchor.syllable_from <= anchor.nucleus_from < anchor.nucleus_to <= anchor.syllable_to
            if anchor.syllable_extended:
                assert anchor.syllable_to > syllables[i].to
            else:
                assert anchor.syllable_to == syllables[i].to
        assert bool(any(a.bridged_j for a in anchors)) == expect_bridged, text
        if expect_extended is not None:
            assert bool(any(a.syllable_extended for a in anchors)) == expect_extended, text
        return events, syllables, phones, slots

    words = (
        ('سلام', 'دنیا', 'کتاب', 'را', 'من', 'او', 'آمد', 'سوال', 'خوب',
         'برای', 'گزینه', 'است', 'خانه'),
        ('salaam', 'donyaa', 'ketaab', 'raa', 'man', 'u', 'aamad', 'soal', 'khoob',
         'baraaye', 'gozine', 'ast', 'khoone'),
    )
    rng = random.Random(745)
    corpus = [[rng.choice((' ', ', ', '، ')).join(rng.choice(words[variant])
               for _ in range(rng.randrange(1, 5))) + rng.choice(('', '.', '?'))
               for _ in range(100)] for variant in (0, 1)]
    checks = bridges = 0
    identity_map = {}
    for rate in (0, 1, 3, 6):
        for gender in (0, 1):
            voice = dll.eciNewEx(0x90000)
            assert voice
            output = (c.c_short * 2048)()
            try:
                assert dll.eciSetOutputBuffer(voice, len(output), output) == 1
                dll.eciRegisterCallback(voice, callback, None)
                for parameter, value in ((5, rate), (0x1010, gender),
                                         (0x1007, 140 if gender else 85), (0x1008, 75 if gender else 65),
                                         (0x100b, 0)):
                    assert dll.eciSetParam(voice, parameter, value) >= 0
                for variant, samples in enumerate(corpus):
                    for text in samples:
                        check(voice, text, variant)
                        checks += 1
                for variant, text in ((0, 'تعجب.'), (1, 'tajjub.')):
                    events, syllables, phones, slots = check(voice, text, variant, expect_bridged=True)
                    bridges += 1
                for variant, text in ((0, 'هوا'), (0, 'چرخ'), (0, 'جوجه'),
                                      (0, '۱,۹۰۰,۰۰۰ ریال'), (0, 'سه هزار'),
                                      (1, 'havaa'), (1, 'charkh'), (1, 'juje'),
                                      (1, '1,900,000 rial'), (1, 'se hezaar')):
                    check(voice, text, variant)
                    checks += 1
                check(voice, 'قشنگ', 0, expect_extended=True)
                check(voice, 'ghashang', 1, expect_extended=True)
                checks += 2
                for text in ('قابل', 'قانون', 'قدم', 'واقعیت'):
                    check(voice, text, 0, expect_extended=None)
                    checks += 1
                # An invalid vowel clock cannot produce a partially filled
                # anchor array or be mistaken for a normal geminate bridge.
                saved = (phones[1].from_, phones[1].to)
                phones[1].from_, phones[1].to = syllables[-1].from_, syllables[-1].to
                guard = (Anchor * len(syllables))()
                c.memset(guard, 0x5a, c.sizeof(guard)); before = bytes(guard)
                n = S(999)
                assert align(events, len(events), syllables, len(syllables), phones, len(phones),
                             slots, len(slots), guard, len(guard), c.byref(n)) == 1 and n.value == 0 and bytes(guard) == before
                phones[1].from_, phones[1].to = saved
                old_slot = slots[1]
                slots[1] = 999
                assert align(events, len(events), syllables, len(syllables), phones, len(phones),
                             slots, len(slots), guard, len(guard), c.byref(n)) == 1 and n.value == 0 and bytes(guard) == before
                slots[1] = old_slot
                assert align(events, len(events), syllables, len(syllables), phones, len(phones),
                             None, 0, guard, len(guard), c.byref(n)) == 1 and n.value == 0 and bytes(guard) == before
                assert align(events, len(events), syllables, len(syllables), phones, len(phones),
                             slots, len(slots), guard, len(guard)-1, c.byref(n)) == -2 and n.value == 0 and bytes(guard) == before
                # The old provider can place a vowel in a gap after its
                # recorded syllable. Reconcile only the unambiguous gap;
                # an overlapping vowel must be declined without output.
                raw = 'قشنگ'.encode('utf8')
                unclear = analyze(raw, 0)
                unclear_syllables, _ = capture(voice, raw, 0, 0)
                unclear_phones, unclear_ids = capture(voice, raw, 0, 1)
                unclear_slots = (c.c_uint32 * len(unclear_ids))(*unclear_ids)
                unclear_phones[1].to = unclear_syllables[1].from_ + 1
                buffer = (Anchor * len(unclear_syllables))()
                c.memset(buffer, 0x5a, c.sizeof(buffer)); unchanged = bytes(buffer)
                status = align(unclear, len(unclear), unclear_syllables, len(unclear_syllables),
                               unclear_phones, len(unclear_phones), unclear_slots, len(unclear_slots),
                               buffer, len(buffer), c.byref(n))
                assert status == 1 and n.value == 0 and bytes(buffer) == unchanged, (rate, gender, status)
            finally:
                dll.eciDelete(voice)
    def concurrent(worker):
        voice = dll.eciNewEx(0x90000)
        assert voice
        output = (c.c_short * 2048)()
        try:
            assert dll.eciSetOutputBuffer(voice, len(output), output) == 1
            dll.eciRegisterCallback(voice, callback, None)
            for parameter, value in ((5, 3), (0x1010, worker % 2), (0x1007, 85), (0x100b, 0)):
                assert dll.eciSetParam(voice, parameter, value) >= 0
            for text in corpus[worker % 2][:20]:
                check(voice, text, worker % 2)
            return 20
        finally:
            dll.eciDelete(voice)
    with ThreadPoolExecutor(max_workers=4) as pool:
        simultaneous = sum(pool.map(concurrent, range(4)))
    print(f'Passed {checks} native vowel-clock mappings, {bridges} doubled-affricate bridges '
          f'and {simultaneous} concurrent captures.')
    if args.dump_identities:
        print('Observed provider phone slots:', sorted((phone, sorted(slots)) for phone, slots in identity_map.items()))


if __name__ == '__main__':
    main()
