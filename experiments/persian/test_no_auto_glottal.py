"""Retain the first onset and remove only added inter-word glottalization."""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import re
import sys
import tempfile

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import EciIndex, GENDER, NATIVE_INFLECTION, NATIVE_PITCH, NATIVE_SPEED, SAMPLE_RATE, SAMPLE_RATES
from contextual_continuation import _context
from espeak_ng_probe import EspeakPhonemeProbe
from listening import render
from shared_sentence_contour import capture
from test_selected_probe import engine, set_confirmed_persian_probe
from text_frontend import translate_text


def mark_words(text: str) -> list:
    matches = list(re.finditer(r'`\[[^]]+\]', text))
    result, pos = [], 0
    for i, match in enumerate(matches):
        result.append(text[pos:match.start()])
        if i:
            result.append(EciIndex(i))
        result.append(match.group())
        pos = match.end()
    result.append(text[pos:])
    return result


def boundary_controls(item, index: int, rate: int) -> np.ndarray:
    t = round(item.indexes.get(index, 0) * 1000 / rate)
    ticks = item.frames['tick']
    begin = int(np.searchsorted(ticks, t, side='left'))
    end = int(np.searchsorted(ticks, t + 40, side='left'))
    assert end - begin >= 5
    return item.frames['controls'][begin:end]


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--previous', type=Path, required=True)
    parser.add_argument('--probe', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    args = parser.parse_args()
    old, new, en, de = ((engine(args.previous, 0x90000), engine(args.probe, 0x90000),
                         engine(args.probe, 0x10000), engine(args.probe, 0x40000)))
    checks = 0
    try:
        with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as api, tempfile.TemporaryDirectory() as temp:
            assert '?' in api.translate('ساعت', separator='|')[0].phonemes
            assert '?' not in api.translate('ایران', separator='|')[0].phonemes
            cases = ('من ایران را دوست دارم.', 'او آمد.', 'خوب است.',
                     'یک آدم.', 'ساعت ایران.', 'ایران آمد.', 'من امروز اینجا هستم.')
            for mode in (1, 3, 6):
                for gender in (0, 1):
                    rate = SAMPLE_RATES[mode]
                    for voice in (old, new, en, de):
                        for parameter, value in ((SAMPLE_RATE, mode), (GENDER, gender),
                                                 (NATIVE_PITCH, 81 if gender else 65),
                                                 (NATIVE_INFLECTION, 30), (NATIVE_SPEED, 50)):
                            voice.set_parameter(parameter, value)
                    for sentence in cases:
                        part = translate_text(api, sentence)
                        assert len(part) == 1 and part[0].language == 'fa'
                        text = part[0].text
                        set_confirmed_persian_probe(no_auto_glottal=0)
                        assert render(old, text) == render(new, text), ('disabled', sentence, mode, gender)
                        marked = mark_words(text)
                        folder = Path(temp)
                        before = capture(new, marked, folder / 'before.f0', frames=folder / 'before.frames')
                        set_confirmed_persian_probe(no_auto_glottal=1)
                        after = capture(new, marked, folder / 'after.f0', frames=folder / 'after.frames')
                        assert before.pcm != after.pcm
                        word = 1
                        b = boundary_controls(before, word, rate)
                        a = boundary_controls(after, word, rate)
                        normal = float(np.median(a[:, 3]))
                        assert float(np.min(b[:, 3])) < normal * .5, (sentence, mode, gender)
                        assert np.min(a[:, 3]) >= normal - 1 and np.max(a[:, 6]) == 0, (sentence, mode, gender)
                        checks += 1
                        if sentence == 'من امروز اینجا هستم.':
                            b = boundary_controls(before, 2, rate)
                            a = boundary_controls(after, 2, rate)
                            normal = float(np.median(a[:, 3]))
                            assert np.min(b[:, 3]) < normal * .5 and np.min(a[:, 3]) >= normal - 1
                            checks += 1
                    # Preserve the original first vowel onset, including
                    # at the beginning of a multi-word utterance. Later
                    # changes can replan pitch/timing, so compare the onset
                    # controls as well as exact isolated-word audio.
                    for sentence in ('ایران', 'ایران آمد.', 'او آمد.'):
                        text = translate_text(api, sentence)[0].text
                        marked = mark_words(text)
                        folder = Path(temp)
                        set_confirmed_persian_probe(no_auto_glottal=0)
                        before = capture(new, marked, folder / 'first0.f0', frames=folder / 'first0.frames')
                        set_confirmed_persian_probe(no_auto_glottal=1)
                        after = capture(new, marked, folder / 'first1.f0', frames=folder / 'first1.frames')
                        b = boundary_controls(before, 0, rate)
                        a = boundary_controls(after, 0, rate)
                        assert np.min(b[:, 3]) < 40 and np.min(a[:, 3]) < 40, (sentence, mode, gender)
                        assert np.max(b[:, 6]) > 0 and np.max(a[:, 6]) > 0, (sentence, mode, gender)
                        if sentence == 'ایران':
                            assert before.pcm == after.pcm
                        checks += 1
                    # The first *Persian segment* is not the first word of
                    # an utterance that starts with English. Its planning
                    # proxy must count as preceding context.
                    parts = translate_text(api, 'NVDA ایران است.')
                    assert len(parts) == 2 and parts[0].language == 'en-us'
                    planned = _context(parts, 'fa', en, new, True)
                    folder = Path(temp)
                    set_confirmed_persian_probe(no_auto_glottal=0)
                    before = capture(new, planned, folder / 'mix0.f0', frames=folder / 'mix0.frames')
                    set_confirmed_persian_probe(no_auto_glottal=1)
                    after = capture(new, planned, folder / 'mix1.f0', frames=folder / 'mix1.frames')
                    b = boundary_controls(before, 1, rate)
                    a = boundary_controls(after, 1, rate)
                    normal = float(np.median(a[:, 3]))
                    assert np.min(b[:, 3]) < normal * .5 and np.min(a[:, 3]) >= normal - 1, (
                        'English-to-Persian onset', mode, gender)
                    checks += 1
                    # A full stop starts a new native sentence and its first
                    # word retains the onset. A comma does not reset the
                    # word number and later vowel onsets remain suppressed.
                    for sentence, initial_indexes, later_indexes in (
                        ('ایران آمد. ایران آمد.', (0, 2), (1, 3)),
                    ):
                        text = translate_text(api, sentence)[0].text
                        planned = mark_words(text)
                        set_confirmed_persian_probe(no_auto_glottal=0)
                        before = capture(new, planned, folder / 'punct0.f0', frames=folder / 'punct0.frames')
                        set_confirmed_persian_probe(no_auto_glottal=1)
                        after = capture(new, planned, folder / 'punct1.f0', frames=folder / 'punct1.frames')
                        for index in initial_indexes:
                            a = boundary_controls(after, index, rate)
                            assert np.min(a[:, 3]) < 40 and np.max(a[:, 6]) > 0, (sentence, index)
                        for index in later_indexes:
                            b = boundary_controls(before, index, rate)
                            a = boundary_controls(after, index, rate)
                            normal = float(np.median(a[:, 3]))
                            assert np.min(b[:, 3]) < normal * .5 and np.min(a[:, 3]) >= normal - 1, (
                                sentence, index)
                        checks += 1
                    # Inserting a marker after a comma can be delivered only
                    # after the sentence by this ECI path. Inspect the later
                    # controls directly instead of treating that index as
                    # an accurate phonetic boundary.
                    comma = translate_text(api, 'ایران، آمد.')[0].text
                    set_confirmed_persian_probe(no_auto_glottal=0)
                    before = capture(new, [comma], folder / 'comma0.f0', frames=folder / 'comma0.frames')
                    set_confirmed_persian_probe(no_auto_glottal=1)
                    after = capture(new, [comma], folder / 'comma1.f0', frames=folder / 'comma1.frames')
                    b = before.frames['controls'][before.frames['tick'] > 200]
                    a = after.frames['controls'][after.frames['tick'] > 200]
                    assert np.min(b[:, 3]) < 40 and np.max(b[:, 6]) > 0
                    assert np.min(a[:, 3]) >= 48 and np.max(a[:, 6]) == 0
                    checks += 1
                    # An explicitly marked lexical stop must sound identical;
                    # switching the marker off must still change the result.
                    for sentence in ('ساعت', 'من ساعت دارم.', 'مسئله', 'مسئول', 'مأمور', 'قرآن'):
                        text = translate_text(api, sentence)[0].text
                        assert 'w' in text
                        set_confirmed_persian_probe(no_auto_glottal=0)
                        before = render(new, text)
                        set_confirmed_persian_probe(no_auto_glottal=1)
                        after = render(new, text)
                        assert before == after, ('lexical stop changed', sentence, mode, gender)
                        os.environ['RELOQ_FA_PROBE_GLOTTAL'] = '0'
                        assert after != render(new, text), ('explicit stop missing', sentence, mode, gender)
                        checks += 1
                    # An unrelated final-H carrier must survive the option.
                    # Its following word starts with a consonant, so its
                    # native sentence should remain byte-identical.
                    h_text = translate_text(api, 'ده نفر')[0].text
                    assert 'hw' in h_text
                    set_confirmed_persian_probe(no_auto_glottal=0)
                    h_before = render(new, h_text)
                    set_confirmed_persian_probe(no_auto_glottal=1)
                    assert render(new, h_text) == h_before
                    assert render(new, '`[.1dAhw]') != render(new, '`[.1dAw]')
                    checks += 1
                    # Other engines do not use this Finnish private rule.
                    for voice, text in ((en, 'NVDA is good.'), (de, 'Guten Morgen.')):
                        set_confirmed_persian_probe(no_auto_glottal=0)
                        off = render(voice, text)
                        set_confirmed_persian_probe(no_auto_glottal=1)
                        assert render(voice, text) == off
                        checks += 1
            # Verify that accepted explicit /ʔ/ and /h/ markers remain
            # active even when automatic Finnish word onsets are disabled.
            for speed in (40, 98, 200):
                for parameter, value in ((SAMPLE_RATE, 3), (GENDER, 0), (NATIVE_PITCH, 65),
                                         (NATIVE_SPEED, speed)):
                    new.set_parameter(parameter, value)
                for sentence in ('ساعت', 'من ساعت دارم.'):
                    tag = translate_text(api, sentence)[0].text
                    set_confirmed_persian_probe(no_auto_glottal=0)
                    before = render(new, tag)
                    set_confirmed_persian_probe(no_auto_glottal=1)
                    assert render(new, tag) == before, ('lexical stop at speed', speed, sentence)
                    checks += 1
                first_tag = translate_text(api, 'ایران')[0].text
                set_confirmed_persian_probe(no_auto_glottal=0)
                original = render(new, first_tag)
                set_confirmed_persian_probe(no_auto_glottal=1)
                assert render(new, first_tag) == original, ('first word at speed', speed)
                checks += 1
                sentence = translate_text(api, 'من ایران را دوست دارم.')[0].text
                planned = mark_words(sentence)
                folder = Path(temp)
                set_confirmed_persian_probe(no_auto_glottal=0)
                before = capture(new, planned, folder / 'speed0.f0', frames=folder / 'speed0.frames')
                set_confirmed_persian_probe(no_auto_glottal=1)
                after = capture(new, planned, folder / 'speed1.f0', frames=folder / 'speed1.frames')
                a = boundary_controls(after, 1, 16000)
                assert np.min(a[:, 3]) >= 48 and np.max(a[:, 6]) == 0, ('later word at speed', speed)
                if speed != 200:
                    b = boundary_controls(before, 1, 16000)
                    assert np.min(b[:, 3]) < np.median(a[:, 3]) * .5
                checks += 1
    finally:
        for voice in (old, new, en, de):
            voice.close()
    print(f'Passed {checks} automatic-onset, explicit-stop and isolation checks.')


if __name__ == '__main__':
    main()
