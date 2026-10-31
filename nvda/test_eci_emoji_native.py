"""CLDR emoji names through the public UTF-8, spelling and fragment API.

PCM hashes stay in RAM. This verifies that the Android/NVDA shared core
speaks each complete localized name, including source marks inside a joined
emoji, rather than requiring a platform-side symbol replacement.
"""
from pathlib import Path
import argparse
import hashlib
import json
import re
from _reloq_eci_direct import DirectEciEngine, EciText, EciIndex, SAMPLE_RATE, NATIVE_SPEED
from _reloq_languages import LANGUAGES

ROOT = Path(__file__).resolve().parents[1]


def pool(source, name):
    raw = source.split(f"static const unsigned char {name}[] = {{", 1)[1].split("};", 1)[0]
    return bytes(map(int, re.findall(r"\d+", raw)))


def labels():
    source = (ROOT / "src/reloq_unicode_data.inc").read_text()
    strings = pool(source, "unicode_strings")
    string = lambda offset: strings[offset:strings.index(0, offset)].decode()
    western = {string(int(k)): (string(int(de)), string(int(en)))
        for k, de, en in re.findall(r"\{(\d+),(\d+),(\d+)\}", source.split("unicode_cldr[] = {", 1)[1].split("};", 1)[0])
        if int(de) and int(en)}
    source = (ROOT / "src/reloq_unicode_multilingual.inc").read_text()
    strings = pool(source, "multi_name_strings")
    other = {string(int(k)): tuple(string(int(v)) if int(v) else None for v in values.split(','))
        for k, values in re.findall(r"\{(\d+),\{([^}]+)\}\}", source.split("multi_names[] = {", 1)[1].split("};", 1)[0])}
    return western, other


def capture(engine, parts):
    digest = hashlib.sha256(); samples = 0; marks = []
    def pcm(data):
        nonlocal samples
        digest.update(data); samples += len(data) // 2
    assert engine.speak([*parts, EciIndex(29)], pcm, lambda n: marks.append((n, samples)))
    assert samples > 0 and marks[-1] == (29, samples), marks
    return samples, digest.hexdigest(), marks


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, default=ROOT / 'eci/libreloq_eci.so')
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    western, other = labels(); checks = 0
    locale = {2: 0, 3: 1, 5: 2, 6: 3, 7: 4, 8: 5, 9: 6, 10: 7, 18: 8}
    for language in LANGUAGES.values():
        engine = DirectEciEngine(args.library.resolve(), dialect=language.id,
            encoding='utf8', experimental_generic=True)
        try:
            engine.set_parameter(NATIVE_SPEED, 98)
            for rate in (1, 6):
                engine.set_parameter(SAMPLE_RATE, rate)
                for emoji in ('😀', '👩🏽‍💻', '🇩🇪', '1️⃣', '\U0001fa8a'):
                    key = emoji.replace('\ufe0f', '')
                    family = language.id >> 16
                    name = western[key][family == 1] if family in (1, 4) else other[key][locale[family]]
                    assert name
                    expected = capture(engine, [' ' + name + ' '])
                    for parts in ([emoji], list(emoji), [EciText(emoji, True)]):
                        actual = capture(engine, parts)
                        assert actual == expected, (language.locale, rate, emoji, name, parts, actual, expected)
                        checks += 1
                    if len(emoji) > 1:
                        actual = capture(engine, [emoji[0], EciIndex(11), emoji[1:]])
                        assert actual[:2] == expected[:2]
                        assert [n for n, _ in actual[2]] == [11, 29]
                        assert actual[2][0][1] == actual[0], (language.locale, emoji, actual)
                        checks += 1
        finally:
            engine.close()
    report = dict(checks=checks, languages=len(LANGUAGES), rates=[11025, 48000],
        no_audio_files=True, localized_name_pcm_equal=True,
        fragment_and_inside_sequence_indexes=True)
    if args.output:
        args.output.write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report))


if __name__ == '__main__':
    main()
