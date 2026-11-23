"""Check the backend-independent Persian number events exported by ECI."""
from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor
import ctypes
from pathlib import Path


class Event(ctypes.Structure):
    _fields_ = [(name, ctypes.c_uint32) for name in
                ('kind', 'value', 'linked', 'weaken', 'final_stress')]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    args = parser.parse_args()
    dll = ctypes.CDLL(str(args.library.resolve()))
    plan = dll.eciPersianNumberEvents
    plan.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.POINTER(ctypes.c_size_t),
                     ctypes.POINTER(Event), ctypes.c_size_t, ctypes.POINTER(ctypes.c_size_t))
    plan.restype = ctypes.c_int

    def run(text):
        source = text.encode('utf8')
        events = (Event * 49)()
        used, count = ctypes.c_size_t(), ctypes.c_size_t()
        status = plan(source, len(source), ctypes.byref(used), events, len(events), ctypes.byref(count))
        return status, used.value, tuple(tuple(getattr(e, name) for name, _ in e._fields_)
                                        for e in events[:count.value])

    examples = {
        '0': ((1, 0, 0, 0, 1),),
        '003': ((1, 0, 0, 1, 1), (1, 0, 0, 1, 1), (1, 3, 0, 0, 1)),
        '1234.05': ((2, 0, 1, 0, 0), (1, 234, 0, 2, 1),
                    (3, 0, 0, 0, 0), (1, 0, 0, 1, 1), (1, 5, 0, 0, 1)),
        '-3': ((4, 0, 0, 0, 0), (1, 3, 0, 0, 1)),
        '+3': ((4, 1, 0, 0, 0), (1, 3, 0, 0, 1)),
        '1.5.0': ((1, 1, 0, 2, 1), (3, 0, 0, 0, 0),
                  (1, 5, 0, 2, 1), (3, 0, 0, 0, 0), (1, 0, 0, 0, 1)),
    }
    for text, expected in examples.items():
        status, consumed, actual = run(text)
        assert status == 0 and consumed == len(text.encode('utf8')) and actual == expected, (text, actual)

    fa = str.maketrans('0123456789,.', '۰۱۲۳۴۵۶۷۸۹٬٫')
    ar = str.maketrans('0123456789,.', '٠١٢٣٤٥٦٧٨٩٬٫')
    for text in ('0', '12', '1,234.05', '0009', '999,999,999,999', '-3.14'):
        expected = run(text)[2]
        for spelling in (text, text.translate(fa), text.translate(ar)):
            status, used, got = run(spelling)
            assert status == 0 and used == len(spelling.encode('utf8')) and got == expected, spelling
    assert run('−۳')[2] == run('-3')[2]
    assert run('۱.۵.۰')[2] == run('1.5.0')[2]
    assert run('١.٥.٠')[2] == run('1.5.0')[2]
    assert run('۱٫۵٫۰')[2] == run('1.5.0')[2]
    assert run('١٫٥٫٠')[2] == run('1.5.0')[2]
    assert run('1.12.003')[2] == ((1, 1, 0, 2, 1), (3, 0, 0, 0, 0),
                                (1, 12, 0, 2, 1), (3, 0, 0, 0, 0),
                                (1, 0, 0, 1, 1), (1, 0, 0, 1, 1),
                                (1, 3, 0, 0, 1))
    assert run('3.' + '5' * 32)[0] == 0
    assert run('۱۲ ادامه')[1] == len('۱۲'.encode('utf8'))
    for text in ('3.1.4,5', '3,14', '۱٬۲۳', '1234567890123'):
        status, used, events = run(text)
        assert status == 2 and not used and not events, text

    source = '۱۲۳٫۴'.encode('utf8')
    used, count = ctypes.c_size_t(), ctypes.c_size_t()
    assert plan(source, len(source), ctypes.byref(used), None, 0, ctypes.byref(count)) == 0
    assert used.value == len(source) and count.value > 1
    guard = (Event * 1)(Event(99, 98, 97, 96, 95))
    assert plan(source, len(source), ctypes.byref(used), guard, 1, ctypes.byref(count)) == -2
    assert not used.value and not count.value and tuple(getattr(guard[0], n) for n, _ in Event._fields_) == (99, 98, 97, 96, 95)
    with ThreadPoolExecutor(max_workers=4) as pool:
        values = [str(n) for n in range(1000)]
        assert list(pool.map(run, values)) == [run(value) for value in values]
    print('Passed native Persian number events, three digit scripts, signs, capacity and concurrency.')


if __name__ == '__main__':
    main()
