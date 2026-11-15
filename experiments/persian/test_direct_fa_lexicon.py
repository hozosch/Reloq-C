"""Compare the native Persian word/phrase probe with eSpeak and the C core."""
from __future__ import annotations

import argparse
import ctypes
import json
from pathlib import Path

from compile_direct_replacements import SOURCE, replacements, render as render_replacements
from espeak_ng_probe import EspeakPhonemeProbe
from listening import render
from test_selected_probe import engine, set_confirmed_persian_probe
from text_frontend import native_tag_accepted, translate_text, word_tag


HERE = Path(__file__).resolve().parent


def entries():
    rows = []
    for line in (HERE / 'direct_fa_lexicon.inc').read_text(encoding='utf8').splitlines():
        line = line.strip()
        if line.startswith('{') and line.endswith('},'):
            rows.append(tuple(json.loads('[' + line[1:-2] + ']')))
    return rows


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--probe', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    args = parser.parse_args()
    dll = ctypes.CDLL(str(args.library.resolve()))
    dll.reloq_fa_direct_word.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.c_void_p, ctypes.c_size_t)
    dll.reloq_fa_direct_word.restype = ctypes.c_int
    dll.reloq_fa_direct_word_count.restype = ctypes.c_size_t
    dll.reloq_fa_direct_text.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.POINTER(ctypes.c_void_p),
                                        ctypes.POINTER(ctypes.POINTER(ctypes.c_size_t)), ctypes.POINTER(ctypes.c_size_t))
    dll.reloq_fa_direct_text.restype = ctypes.c_int
    dll.reloq_fa_direct_free.argtypes = (ctypes.c_void_p,)
    dll.reloq_fa_direct_normalize.argtypes = dll.reloq_fa_direct_text.argtypes
    dll.reloq_fa_direct_normalize.restype = ctypes.c_int

    def lookup(word: str, cap: int = 2048):
        encoded = word.encode('utf8')
        buffer = ctypes.create_string_buffer(cap)
        status = dll.reloq_fa_direct_word(encoded, len(encoded), buffer, cap)
        return status, buffer.value.decode('ascii')

    def phrase(text: bytes):
        output = ctypes.c_void_p()
        mapping = ctypes.POINTER(ctypes.c_size_t)()
        offset = ctypes.c_size_t()
        status = dll.reloq_fa_direct_text(text, len(text), ctypes.byref(output),
                                          ctypes.byref(mapping), ctypes.byref(offset))
        if status:
            assert not output.value and not bool(mapping)
            return status, offset.value, None, None
        try:
            return status, 0, ctypes.string_at(output).decode('ascii'), tuple(mapping[i] for i in range(len(text) + 1))
        finally:
            dll.reloq_fa_direct_free(output)
            dll.reloq_fa_direct_free(mapping)

    def normalize(text: str):
        encoded = text.encode('utf8')
        output = ctypes.c_void_p()
        mapping = ctypes.POINTER(ctypes.c_size_t)()
        offset = ctypes.c_size_t()
        status = dll.reloq_fa_direct_normalize(encoded, len(encoded), ctypes.byref(output),
                                                ctypes.byref(mapping), ctypes.byref(offset))
        assert status == 0
        try:
            return ctypes.string_at(output).decode('utf8'), tuple(mapping[i] for i in range(len(encoded) + 1))
        finally:
            dll.reloq_fa_direct_free(output)
            dll.reloq_fa_direct_free(mapping)

    rows = entries()
    assert (HERE / 'direct_fa_replacements.inc').read_text(encoding='utf8') == render_replacements(SOURCE)
    for cp, target in replacements(SOURCE):
        actual, mapping = normalize(chr(cp))
        assert actual == target and mapping[-1] == len(target.encode('utf8')), hex(cp)
        assert mapping == tuple(sorted(mapping))
    assert len(rows) == dll.reloq_fa_direct_word_count() >= 15000
    assert [word.encode('utf8') for word, _ in rows] == sorted({word.encode('utf8') for word, _ in rows})
    assert lookup('ایران', 3) == (-2, '')
    assert lookup('من') == (0, '')  # this word changes stress in context
    assert lookup('ناموجودِمطمئن') == (0, '')
    assert phrase('سلام NVDA'.encode('utf8'))[:2] == (1, len('سلام '.encode('utf8')))
    assert phrase(b'\xd8broken')[0] == -1
    text = 'ﺳﻼﻡ دنیا؟'
    status, _, tagged, mapping = phrase(text.encode('utf8'))
    first_word = len('ﺳﻼﻡ'.encode('utf8'))
    assert status == 0 and tagged is not None and mapping is not None
    assert mapping[0] == 0 and mapping[first_word] == len('`[.0sA.1lam]')
    assert mapping[first_word + 1] == len('`[.0sA.1lam] ')
    assert all(value == 0 for value in mapping[:first_word])

    fa = engine(args.probe, 0x90000)
    checks = 0
    try:
        set_confirmed_persian_probe()
        with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as api:
            # Every seventeenth entry is spread over the full Persian alphabet.
            for word, expected in rows[::17]:
                assert lookup(word) == (1, expected), word
                for text, index in ((word, 0), ('من ' + word + ' آمد.', 1), ('من ' + word + '.', 1)):
                    phones = api.translate(text, separator='|')[0].phonemes.strip().split()
                    assert word_tag(phones[index]) == expected, (word, text)
                checks += 1
            # Check the native Finnish phoneme parser, not only the table.
            for word, tag in rows[::79]:
                shown = fa._phonemes_for_probe(tag)
                assert native_tag_accepted(tag, shown), (word, tag, shown)
            texts = ('سلام دنیا؟', 'او آمد.', 'ایران نقطه.', 'کاغذ آمد.', 'سلام خوب؟',
                     'ايران نقطه.', 'كاغذ آمد.', 'ﺳﻼﻡ دنیا؟', 'سلامـ دنیا؟')
            for text in texts:
                parts = translate_text(api, text)
                assert len(parts) == 1 and parts[0].language == 'fa'
                status, _, actual, offsets = phrase(text.encode('utf8'))
                assert status == 0 and actual == parts[0].text, (text, actual, parts)
                assert offsets == tuple(sorted(offsets)) and offsets[-1] == len(actual.encode('utf8'))
                assert render(fa, actual) == render(fa, parts[0].text)
    finally:
        fa.close()
    print(f'Passed {checks} native word comparisons, {len(rows[::79])} native tag checks, '
          f'{len(replacements(SOURCE))} direct replacement patterns, '
          f'{len(texts)} direct-text/audio comparisons and failure/index checks.')


if __name__ == '__main__':
    main()
