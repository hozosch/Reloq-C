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
import re
import sys

from test_persian_owned_capture import NativeFrame, NativeSyllable
from test_persian_syllable_plan_clock import Event


class Anchor(c.Structure):
    _fields_ = (('syllable_from', c.c_int32), ('syllable_to', c.c_int32),
                ('nucleus_from', c.c_int32), ('nucleus_to', c.c_int32),
                ('word', c.c_uint32), ('syllable', c.c_uint32), ('stress', c.c_uint32),
                ('weak', c.c_uint32), ('nucleus', c.c_uint32), ('bridged_j', c.c_uint32),
                ('syllable_extended', c.c_uint32))


class EarlyNucleus(c.Structure):
    _fields_ = (('from_', c.c_int32), ('to', c.c_int32), ('word', c.c_uint32),
                ('syllable', c.c_uint32), ('stress', c.c_uint32), ('weak', c.c_uint32),
                ('nucleus', c.c_uint32))


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
        ('eciAddTextUtf8', I, (H, c.c_char_p, S, I)), ('eciSynthesize', I, (H,)),
        ('eciPersianCaptureBegin', I, (S,)), ('eciPersianCaptureSetControlsOnly', I, (I,)),
        ('eciPersianCaptureSetPhoneTrace', I, (I,)),
        ('eciPersianCaptureSetNucleusTrace', I, (I,)),
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
    early_align = dll.eciPersianAlignEarlyNuclei
    early_align.restype = I
    early_align.argtypes = (c.POINTER(Event), S, c.POINTER(NativeSyllable), S,
                            c.POINTER(c.c_uint32), S, c.POINTER(EarlyNucleus), S,
                            c.POINTER(S))
    cursor_create = dll.eciPersianEarlyCursorCreate
    cursor_create.restype, cursor_create.argtypes = I, (c.POINTER(Event), S, c.POINTER(H))
    cursor_next = dll.eciPersianEarlyCursorNext
    cursor_next.restype, cursor_next.argtypes = I, (H, c.POINTER(NativeSyllable), c.c_uint32,
                                                   c.POINTER(EarlyNucleus))
    cursor_finish = dll.eciPersianEarlyCursorFinish
    cursor_finish.restype, cursor_finish.argtypes = I, (H,)
    cursor_release = dll.eciPersianEarlyCursorRelease
    cursor_release.restype, cursor_release.argtypes = None, (H,)

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

    def capture(voice, raw, variant, phone_trace, include_frames=False):
        assert dll.eciPersianCaptureBegin(30000) == 0
        try:
            assert dll.eciPersianCaptureSetControlsOnly(1) == 0
            if phone_trace == 2:
                assert dll.eciPersianCaptureSetNucleusTrace(1) == 0
            else:
                assert dll.eciPersianCaptureSetPhoneTrace(phone_trace) == 0
            error = S()
            status = dll.eciPersianSpeakNativeUtf8(voice, raw, len(raw), variant, None, None, 0, c.byref(error))
            assert status == 0, (raw, variant, status, error.value)
            count, failed = S(), I()
            frames = dll.eciPersianCaptureEnd(c.byref(count), c.byref(failed))
            assert not failed.value and count.value
            frame_data = (c.string_at(frames, count.value * c.sizeof(NativeFrame))
                          if include_frames else None)
            clocks = dll.eciPersianCaptureSyllables(c.byref(count), c.byref(failed))
            assert not failed.value and clocks and count.value
            result = (NativeSyllable * count.value)(*(NativeSyllable(clocks[i].from_, clocks[i].to)
                                                       for i in range(count.value)))
            identities = ()
            if phone_trace:
                ids = dll.eciPersianCapturePhoneIdentities(c.byref(count), c.byref(failed))
                assert not failed.value and ids and count.value == len(result), (raw, count.value, len(result))
                identities = tuple(ids[:count.value])
            return (result, identities, frame_data) if include_frames else (result, identities)
        finally:
            dll.eciPersianCaptureRelease()
            count, failed = S(999), I(999)
            assert not dll.eciPersianCapturePhoneIdentities(c.byref(count), c.byref(failed))
            assert count.value == 0 and failed.value == 0

    def check(voice, text, variant, expect_bridged=False, expect_extended=False,
              allow_unsupported=False):
        raw = text.encode('utf8')
        events = analyze(raw, variant)
        syllables, _ = capture(voice, raw, variant, 0)
        phones, identities = capture(voice, raw, variant, 1)
        slots = (c.c_uint32 * len(identities))(*identities)
        n = S()
        status = align(events, len(events), syllables, len(syllables), phones, len(phones),
                       slots, len(slots), None, 0, c.byref(n))
        if status == 1 and allow_unsupported:
            assert n.value == 0
            guard = (Anchor * len(syllables))()
            c.memset(guard, 0x5a, c.sizeof(guard)); original = bytes(guard)
            assert align(events, len(events), syllables, len(syllables), phones, len(phones),
                         slots, len(slots), guard, len(guard), c.byref(n)) == 1
            assert n.value == 0 and bytes(guard) == original
            return None
        assert status == 0 and n.value == len(syllables), (text, status, n.value, len(syllables))
        anchors = (Anchor * n.value)()
        status = align(events, len(events), syllables, len(syllables), phones, len(phones),
                       slots, len(slots), anchors, len(anchors), c.byref(n))
        assert status == 0 and n.value == len(syllables), (text, status)
        # Independently map vowel events to their recorded provider intervals.
        # Phone and syllable clocks can disagree on doubled J, but never on
        # the vowel clock used as an intonation anchor.
        p, expected, vowel_slots, q_counts, weak, word, pending_j = 0, [], [], [], 0, -1, False
        syllable_q, current_syllable = 0, None
        for at, e in enumerate(events):
            if e.kind == 5:
                weak = 1
            elif e.kind == 1:
                word += 1
                syllable_q = 0
                current_syllable = None
            elif e.kind == 2:
                if e.syllable != current_syllable:
                    current_syllable = e.syllable
                    syllable_q = 0
                if e.value == 22:  # H is a feature, with a final carrier at word end.
                    if at + 1 < len(events) and events[at + 1].kind == 3:
                        assert identities[p] == 3
                        p += 1
                    continue
                identity_map.setdefault(e.value, set()).add(identities[p])
                if e.value == 13:  # The current provider's marked uvular stop.
                    syllable_q += 1
                if 1 <= e.value <= 6 or e.value in (30, 31):
                    expected.append((phones[p].from_, phones[p].to, word, e.syllable,
                                     e.stress, weak, e.value))
                    vowel_slots.append(identities[p])
                    q_counts.append(syllable_q)
                    syllable_q = 0
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
        # This trace is emitted before the provider selects intonation. Its
        # vowel slot and any marked q release are read independently from the
        # provider's phone records, then compared to the later acoustic clock.
        early, early_ids = capture(voice, raw, variant, 2)
        assert len(early) == len(anchors) == len(early_ids), text
        for a, clock, identity, slot, q_count in zip(anchors, early, early_ids, vowel_slots, q_counts):
            assert (clock.from_, clock.to) == (a.nucleus_from, a.nucleus_to), (text, a.word, a.syllable)
            assert (identity & 255, identity >> 8) == (slot, q_count), (text, a.word, a.syllable, identity)
        early_slots = (c.c_uint32 * len(early_ids))(*early_ids)
        count = S()
        assert early_align(events, len(events), early, len(early), early_slots, len(early_slots),
                           None, 0, c.byref(count)) == 0 and count.value == len(anchors), text
        projected = (EarlyNucleus * count.value)()
        assert early_align(events, len(events), early, len(early), early_slots, len(early_slots),
                           projected, len(projected), c.byref(count)) == 0
        assert [(a.from_, a.to, a.word, a.syllable, a.stress, a.weak, a.nucleus)
                for a in projected] == actual, text
        # The same plan can consume one vowel at a time, before intonation;
        # a rejected identity must not advance the cursor or modify output.
        cursor = H()
        assert cursor_create(events, len(events), c.byref(cursor)) == 0 and cursor.value
        try:
            assert cursor_finish(cursor) == 1
            one = EarlyNucleus()
            c.memset(c.byref(one), 0x5a, c.sizeof(one)); untouched = bytes(one)
            assert cursor_next(cursor, c.byref(early[0]), early_ids[0] ^ 1, c.byref(one)) == 1
            assert bytes(one) == untouched and cursor_finish(cursor) == 1
            streamed = []
            for index, (clock, identity) in enumerate(zip(early, early_ids)):
                assert cursor_next(cursor, c.byref(clock), identity, c.byref(one)) == 0
                streamed.append((one.from_, one.to, one.word, one.syllable, one.stress, one.weak, one.nucleus))
                if index == 0 and len(early) > 1:
                    unchanged = bytes(one)
                    assert cursor_next(cursor, c.byref(clock), early_ids[1], c.byref(one)) == 1
                    assert bytes(one) == unchanged
            assert streamed == actual and cursor_finish(cursor) == 0
            assert cursor_next(cursor, c.byref(early[-1]), early_ids[-1], c.byref(one)) == 1
        finally:
            cursor_release(cursor)
        for i, anchor in enumerate(anchors):
            assert anchor.syllable_from <= anchor.nucleus_from < anchor.nucleus_to <= anchor.syllable_to
            if anchor.syllable_extended:
                assert anchor.syllable_from < syllables[i].from_ or anchor.syllable_to > syllables[i].to
            else:
                assert anchor.syllable_from == syllables[i].from_
                assert anchor.syllable_to == syllables[i].to
        if expect_bridged is not None:
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
    checks = bridges = final_q = 0
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
                                      (0, 'یاه'), (0, 'درمونگاه'), (0, 'راه خانه'),
                                      (0, 'گاه به گاه'), (0, 'ماهِ من'), (0, 'راه، خانه'),
                                      (0, 'راه.'),
                                      (1, 'havaa'), (1, 'charkh'), (1, 'juje'),
                                      (1, 'raah'), (1, 'shah'), (1, 'goh'),
                                      (1, 'raah khoone'), (1, 'raah, khoone'), (1, 'raah?'),
                                      (1, '1,900,000 rial'), (1, 'se hezaar')):
                    check(voice, text, variant)
                    checks += 1
                check(voice, 'قشنگ', 0, expect_extended=True)
                check(voice, 'ghashang', 1, expect_extended=True)
                checks += 2
                for text in ('قابل', 'قانون', 'قدم', 'واقعیت', 'قدرت', 'قیمت', 'قلب', 'قهوه', 'حقیقت'):
                    check(voice, text, 0, expect_extended=None)
                    checks += 1
                # The early adapter needs only the vowel interval: unlike the
                # older full-phone adapter it can handle a final q whose
                # release overlaps the subsequent provider pause. Compare its
                # vowels independently with the later phone clocks.
                for variant, text in ((0, 'دماغ'), (0, 'طلاق'), (0, 'اتاق'),
                                      (0, 'داغ'), (0, 'باغ'), (0, 'اتاق خانه'),
                                      (1, 'damaagh'), (1, 'taalaagh')):
                    raw = text.encode('utf8')
                    q_plan = analyze(raw, variant)
                    early_q, early_ids = capture(voice, raw, variant, 2)
                    q_phones, phone_ids = capture(voice, raw, variant, 1)
                    expected_vowels = [(q_phones[j].from_, q_phones[j].to)
                                       for j, slot in enumerate(phone_ids)
                                       if slot in (31, 34, 29, 27, 33, 32)]
                    assert [(x.from_, x.to) for x in early_q] == expected_vowels, text
                    q_id_array = (c.c_uint32 * len(early_ids))(*early_ids)
                    projected_q, n = (EarlyNucleus * len(early_q))(), S()
                    assert early_align(q_plan, len(q_plan), early_q, len(early_q), q_id_array,
                                       len(q_id_array), projected_q, len(projected_q), c.byref(n)) == 0, text
                    assert [(x.from_, x.to) for x in projected_q] == expected_vowels, text
                    final_q += 1
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
                # A word-final H must consume only the exact, contained
                # synthetic carrier. Reject a wrong identity or an interval
                # crossing the syllable boundary without returning anchors.
                raw = 'راه خانه'.encode('utf8')
                final_h = analyze(raw, 0)
                h_syllables, _ = capture(voice, raw, 0, 0)
                h_phones, h_ids = capture(voice, raw, 0, 1)
                h_slots = (c.c_uint32 * len(h_ids))(*h_ids)
                h_index = next(i for i, (phone, identity) in enumerate(zip(h_phones, h_ids))
                               if identity == 3 and phone.to == h_syllables[0].to)
                h_guard = (Anchor * len(h_syllables))()
                c.memset(h_guard, 0x5a, c.sizeof(h_guard)); h_before = bytes(h_guard)
                h_slots[h_index] = 999
                assert align(final_h, len(final_h), h_syllables, len(h_syllables), h_phones,
                             len(h_phones), h_slots, len(h_slots), h_guard, len(h_guard),
                             c.byref(n)) == 1 and n.value == 0 and bytes(h_guard) == h_before
                h_slots[h_index] = 3
                h_phones[h_index].to += 1
                assert align(final_h, len(final_h), h_syllables, len(h_syllables), h_phones,
                             len(h_phones), h_slots, len(h_slots), h_guard, len(h_guard),
                             c.byref(n)) == 1 and n.value == 0 and bytes(h_guard) == h_before
                # Early planning rejects a false provider identity, an
                # incorrect q count, and insufficient capacity atomically.
                q_events = analyze('قشنگ'.encode('utf8'), 0)
                q_early, q_ids = capture(voice, 'قشنگ'.encode('utf8'), 0, 2)
                q_slots = (c.c_uint32 * len(q_ids))(*q_ids)
                mixed_events = (Event * (len(q_events) + 1))(
                    *(list(q_events) + [Event(8, 0, 1, 0, 0)]))
                bad_cursor = H(123)
                assert cursor_create(mixed_events, len(mixed_events), c.byref(bad_cursor)) == 1
                assert not bad_cursor.value  # Reject English before any output.
                q_guard = (EarlyNucleus * len(q_early))()
                c.memset(q_guard, 0x5a, c.sizeof(q_guard)); q_before = bytes(q_guard)
                q_slots[0] ^= 1
                assert early_align(q_events, len(q_events), q_early, len(q_early), q_slots,
                                   len(q_slots), q_guard, len(q_guard), c.byref(n)) == 1
                assert n.value == 0 and bytes(q_guard) == q_before
                q_slots[0] = q_ids[0] ^ 0x100
                assert early_align(q_events, len(q_events), q_early, len(q_early), q_slots,
                                   len(q_slots), q_guard, len(q_guard), c.byref(n)) == 1
                assert n.value == 0 and bytes(q_guard) == q_before
                q_slots[0] = q_ids[0]
                assert early_align(q_events, len(q_events), q_early, len(q_early), q_slots,
                                   len(q_slots), q_guard, len(q_guard)-1, c.byref(n)) == -2
                assert n.value == 0 and bytes(q_guard) == q_before
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
    # Unbiased dictionary samples exercise more consonant combinations. A
    # still-unsupported mapping must decline cleanly, never return a guessed
    # vowel; accepted mappings must pass the same independent checks above.
    lexicon = Path(__file__).resolve().parents[1] / 'experiments/persian/direct_fa_frequency_words.inc'
    words = re.findall(r'^\s*\{"([^"\\]+)",', lexicon.read_text(encoding='utf8'), re.M)
    candidates = [word for word in words if len(word) >= 3 and all(ch.isalpha() for ch in word)]
    samples = random.Random(227).sample(candidates, 300)
    voice = dll.eciNewEx(0x90000)
    assert voice
    output = (c.c_short * 2048)()
    try:
        assert dll.eciSetOutputBuffer(voice, len(output), output) == 1
        dll.eciRegisterCallback(voice, callback, None)
        for parameter, value in ((5, 3), (0x1010, 0), (0x1007, 85), (0x100b, 0)):
            assert dll.eciSetParam(voice, parameter, value) >= 0
        lexical = sum(check(voice, text, 0, expect_bridged=None, expect_extended=None,
                            allow_unsupported=True) is not None for text in samples)
        assert lexical >= 250, lexical
    finally:
        dll.eciDelete(voice)
    # An otherwise valid extension must stop before a real comma pause.
    example = (Event * 8)(Event(1, 0, 0, 0, 0), Event(2, 13, 0, 0, 0),
                          Event(2, 1, 0, 0, 0), Event(3, 0, 0, 0, 0),
                          Event(4, 44, 0, 0, 0), Event(1, 0, 0, 0, 0),
                          Event(2, 2, 0, 0, 1), Event(3, 0, 0, 0, 0))
    syll = (NativeSyllable * 2)(NativeSyllable(0, 10), NativeSyllable(50, 60))
    phone = (NativeSyllable * 5)(NativeSyllable(0, 10), NativeSyllable(10, 25),
                                  NativeSyllable(30, 40), NativeSyllable(50, 60),
                                  NativeSyllable(70, 80))
    raw_slots = (c.c_uint32 * 5)(3, 31, 36, 34, 36)
    out, n = (Anchor * 2)(), S()
    assert align(example, len(example), syll, len(syll), phone, len(phone),
                 raw_slots, len(raw_slots), out, len(out), c.byref(n)) == 0
    assert out[0].syllable_extended and out[0].syllable_to == 25
    phone[2].from_ = 20
    c.memset(out, 0x5a, c.sizeof(out)); unchanged = bytes(out)
    assert align(example, len(example), syll, len(syll), phone, len(phone),
                 raw_slots, len(raw_slots), out, len(out), c.byref(n)) == 1
    assert n.value == 0 and bytes(out) == unchanged
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
    # Enabling diagnostics must not alter the generated control frames.
    voice = dll.eciNewEx(0x90000)
    assert voice
    output = (c.c_short * 2048)()
    try:
        assert dll.eciSetOutputBuffer(voice, len(output), output) == 1
        dll.eciRegisterCallback(voice, callback, None)
        for variant, text in ((0, 'سلام دنیا.'), (0, 'قشنگ'), (0, 'راه خانه'),
                              (0, '۱,۹۰۰,۰۰۰ ریال'), (1, 'tajjub raah')):
            raw = text.encode('utf8')
            _, _, normal = capture(voice, raw, variant, 0, include_frames=True)
            _, _, observed = capture(voice, raw, variant, 2, include_frames=True)
            assert normal == observed, (variant, text)
    finally:
        dll.eciDelete(voice)
    assert dll.eciPersianCaptureSetNucleusTrace(1) == -1
    assert dll.eciPersianCaptureBegin(100) == 0
    try:
        assert dll.eciPersianCaptureSetNucleusTrace(2) == -1
    finally:
        dll.eciPersianCaptureRelease()
    # The early mapping is scoped to the Persian projection, not ordinary
    # Finnish. A Finnish capture must decline it rather than inspect an
    # unrelated language's phone records or report a plausible wrong clock.
    voice = dll.eciNewEx(0x90000)
    assert voice
    output = (c.c_short * 2048)()
    try:
        assert dll.eciSetOutputBuffer(voice, len(output), output) == 1
        dll.eciRegisterCallback(voice, callback, None)
        assert dll.eciPersianCaptureBegin(30000) == 0
        try:
            assert dll.eciPersianCaptureSetControlsOnly(1) == 0
            assert dll.eciPersianCaptureSetNucleusTrace(1) == 0
            raw = 'Hei maailma.'.encode('utf8')
            assert dll.eciAddTextUtf8(voice, raw, len(raw), 0) == 1
            assert dll.eciSynthesize(voice) == 1
            count, failed = S(), I()
            dll.eciPersianCaptureEnd(c.byref(count), c.byref(failed))
            assert not failed.value and count.value
            dll.eciPersianCaptureSyllables(c.byref(count), c.byref(failed))
            assert failed.value and count.value == 0
        finally:
            dll.eciPersianCaptureRelease()
    finally:
        dll.eciDelete(voice)
    empty_cursor = H(123)
    assert cursor_create(None, 0, c.byref(empty_cursor)) == 1 and not empty_cursor.value
    assert cursor_finish(None) == -1
    cursor_release(None)
    print(f'Passed {checks} native vowel-clock mappings, {bridges} doubled-affricate bridges, '
          f'{final_q} final-q early mappings, {lexical}/300 dictionary words and '
          f'{simultaneous} concurrent captures.')
    if args.dump_identities:
        print('Observed provider phone slots:', sorted((phone, sorted(slots)) for phone, slots in identity_map.items()))


if __name__ == '__main__':
    main()
