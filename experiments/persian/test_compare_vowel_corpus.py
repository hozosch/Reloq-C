"""Protect the corpus comparison against mismatched or misleading pairs."""
from compare_vowel_corpus import paired_rows


def main():
    # phone, class, stress, native length, original target, final target
    a = [(31, 1, 1, 1, 1000, 1200), (34, 1, 0, 0, 2000, 2000)]
    b = [(31, 1, 1, 1, 800, 900), (34, 1, 0, 0, 1600, 1600),
         (31, 1, 0, 1, 700, 700)]
    x = [('a', 1, 160), ('A', 0, 100)]
    y = [('a', 1, 120), ('A', 0, 75)]
    rows = paired_rows('test', a, b, x, y)
    assert len(rows) == 2 and rows[0]['native_ratio'] == 0.75 and rows[0]['espeak_ratio'] == 0.75
    assert rows[1]['native_ratio'] == 0.8 and rows[1]['espeak_ratio'] == 0.75
    assert not paired_rows('test', a, b, x[:-1], y), 'different vowel count must be excluded'
    assert not paired_rows('test', a, b, [('e', 1, 160), x[1]], y), 'different phone must be excluded'
    assert not paired_rows('test', a, b, [('a', 0, 160), x[1]], y), 'different stress must be excluded'
    assert not paired_rows('test', a, [(*b[0][:2], 0, *b[0][3:]), *b[1:]], x, y)
    print('Passed vowel pairing, context, and mismatch rejection checks.')


if __name__ == '__main__':
    main()
