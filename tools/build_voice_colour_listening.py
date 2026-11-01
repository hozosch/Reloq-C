"""Build slower, labelled body/clarity/S listening packages from the 1.0 host.

The listening-only native wrapper supplies separate periodic and S targets.
Voicing gain is calibrated by native re-synthesis. Noise is never level
matched by filtering or scaling a mixed recording. Only guided comparison
assembly resamples files offline; raw WAVs retain their original clocks.
"""
from pathlib import Path
import argparse
import ctypes as C
import hashlib
import json
import math
import os
import sys
import wave
import zipfile

import numpy as np
from scipy.signal import resample_poly

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import (DirectEciEngine, SAMPLE_RATE, NATIVE_SPEED,
    NATIVE_PITCH, NATIVE_INFLECTION, NATIVE_VOLUME, GENDER, HEAD_SIZE)

RATES = ((1, 11025), (3, 16000), (2, 22050), (4, 32000), (5, 44100), (6, 48000))
MODES = dict(A=0, B=1, C=2, D=3, E=4, F=7)
TEXTS = {
    'Deutsch': (0x40000, (
        ('01_stimmlos', 'Es ist nass. Das Fass ist weiß. Klaus isst Reis.'),
        ('02_stimmhaft', 'Susi summt. Sie sehen sieben Segel.'),
        ('03_Stimmkoerper', 'Eine warme, ruhige Melodie am Meer.'))),
    'US_English': (0x10000, (
        ('01_unvoiced', 'Sue sips soup. Sam sits still beside a silent sea.'),
        ('02_voiced', 'Lazy bees buzz by the roses.'),
        ('03_voice_body', 'A warm, mellow aroma.'))),
}


def create(path, dialect, speed):
    e = DirectEciEngine(path, dialect=dialect, encoding='utf8',
                        experimental_generic=True, lexicon_path=None)
    for p, v in ((NATIVE_SPEED, speed), (NATIVE_PITCH, 65),
                 (NATIVE_INFLECTION, 30), (NATIVE_VOLUME, 100),
                 (GENDER, 0), (HEAD_SIZE, 50)):
        e.set_parameter(p, v)
    return e


def speak(e, text):
    chunks = []
    assert e.speak([text], chunks.append, None)
    x = np.frombuffer(b''.join(chunks), dtype='<i2').copy()
    assert len(x) and not np.any((x == 32767) | (x == -32768))
    return x


def assemble_rate(data, rate):
    if rate == 48000:
        return data
    divisor = math.gcd(rate, 48000)
    return resample_poly(data, 48000 // divisor, rate // divisor,
                         window=('kaiser', 8.6))


def write_wave(path, data, rate, root, records):
    assert np.isfinite(data).all() and np.max(np.abs(data)) < .99
    pcm = np.rint(data * 32768).astype('<i2')
    path.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(path), 'wb') as w:
        w.setnchannels(1); w.setsampwidth(2); w.setframerate(rate)
        w.writeframes(pcm.tobytes())
    records.append(dict(file=str(path.relative_to(root)), rate=rate,
                        samples=len(pcm), sha256=hashlib.sha256(path.read_bytes()).hexdigest()))


def rate_label(language, rate):
    labels = ({11025: 'Elf Komma null zwei fünf', 16000: 'Sechzehn',
               22050: 'Zweiundzwanzig Komma null fünf', 32000: 'Zweiunddreißig',
               44100: 'Vierundvierzig Komma eins', 48000: 'Achtundvierzig'}
              if language == 'Deutsch' else
              {11025: 'Eleven point zero two five', 16000: 'Sixteen',
               22050: 'Twenty two point zero five', 32000: 'Thirty two',
               44100: 'Forty four point one', 48000: 'Forty eight'})
    return labels[rate] + (' Kilohertz.' if language == 'Deutsch' else ' kilohertz.')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--reference', type=Path, required=True)
    parser.add_argument('--nvda-rate', type=int, default=30)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    assert 0 <= args.nvda_rate <= 100
    # The public driver's mapping without rate boost.
    speed = 40 + (116 * args.nvda_rate + 50) // 100
    library, reference = args.library.resolve(), args.reference.resolve()
    setter_lib = C.CDLL(str(library))
    setter = setter_lib.reloq_listening_set_profile
    setter.argtypes = (C.c_uint, C.c_double); setter.restype = C.c_int
    args.output = args.output.resolve()
    args.output.mkdir(parents=True, exist_ok=True)
    for language, (dialect, cases) in TEXTS.items():
        folder = args.output / ('Reloq_Koerper_Klarheit_S_Langsam_' + language)
        folder.mkdir(exist_ok=False)
        e, truth = create(library, dialect, speed), create(reference, dialect, speed)
        captures, checks = {}, []
        def render(mode, gain, text):
            assert setter(mode, gain) == 0
            return speak(e, text)
        try:
            for code, rate in RATES:
                e.set_parameter(SAMPLE_RATE, code); truth.set_parameter(SAMPLE_RATE, code)
                for case, text in cases:
                    original = render(0, 1, text)
                    assert np.array_equal(original, speak(truth, text)), (language, rate, case, 'A != 1.0')
                    captures['A', rate, case] = original
                    noise_a = render(0, 0, text)
                    noise_e = render(4, 0, text)
                    a = original.astype(np.float64)
                    target = np.mean(a * a)
                    for variant, mode in list(MODES.items())[1:]:
                        noise = render(mode, 0, text)
                        assert np.array_equal(noise, noise_e if mode & 4 else noise_a), (
                            language, rate, case, variant, 'noise branch changed unexpectedly')
                        if rate == 11025:
                            gain = 1.0
                        else:
                            # A half-level pilot leaves headroom for stronger
                            # harmonics. Solve the full-signal energy equation
                            # with the unchanged native noise term retained.
                            pilot = render(mode, .5, text).astype(np.float64)
                            n = noise.astype(np.float64); voice = 2 * (pilot - n)
                            aa, b, cc = np.mean(voice * voice), np.mean(voice * n), np.mean(n * n)
                            assert aa > 0 and b * b + aa * (target - cc) >= 0
                            gain = float((-b + np.sqrt(b * b + aa * (target - cc))) / aa)
                            assert 0 < gain < 4
                        final = render(mode, gain, text)
                        assert len(final) == len(original)
                        level = float(10 * np.log10(np.mean(final.astype(np.float64) ** 2) / target))
                        assert abs(level) < .04, (language, variant, rate, case, level)
                        if rate == 11025:
                            assert np.array_equal(original, final)
                        captures[variant, rate, case] = final
                        checks.append(dict(variant=variant, rate=rate, case=case,
                                           voiced_gain=gain, total_level_db=level,
                                           noise_branch_exact=True))
            # Measure the old tempo too, rather than interpreting either
            # native speed number as an exact words-per-minute value.
            truth.set_parameter(SAMPLE_RATE, 1); truth.set_parameter(NATIVE_SPEED, 98)
            old_duration = len(speak(truth, cases[0][1])) / 11025
            new_duration = len(captures['A', 11025, cases[0][0]]) / 11025
        finally:
            e.close(); truth.close()
        scaled, gain = {}, {}
        for case, _ in cases:
            for _, rate in RATES:
                a = captures['A', rate, case].astype(np.float64)
                active = a[np.abs(a) >= 64]
                gain[rate, case] = 3000 / np.sqrt(np.mean(active * active))
        peak = max(np.max(np.abs(x.astype(np.float64))) * gain[rate, case] / 32768
                   for (variant, rate, case), x in captures.items())
        safety = min(1, .65 / peak)
        files = []
        for (variant, rate, case), x in captures.items():
            data = x.astype(np.float64) / 32768 * gain[rate, case] * safety
            scaled[variant, rate, case] = data
            if rate == 11025 and variant != 'A':
                continue
            order = next(i for i, (_, hz) in enumerate(RATES, 1) if hz == rate)
            name = f'{order:02d}_{rate}_' + ('Referenz' if rate == 11025 else variant) + '.wav'
            write_wave(folder / 'Einzeldateien' / case / name, data, rate, folder, files)
        labels = create(reference, dialect, speed)
        labels.set_parameter(SAMPLE_RATE, 1)
        label_cache, gap = {}, np.zeros(round(.35 * 48000))
        def label(text):
            if text not in label_cache:
                data = assemble_rate(speak(labels, text).astype(np.float64) / 32768, 11025)
                label_cache[text] = data * .30 / max(np.max(np.abs(data)), .001)
            return label_cache[text]
        def sequence(name, entries):
            parts = []
            for variant, rate, case in entries:
                idx = next(i for i, row in enumerate(cases) if row[0] == case)
                kind = (('Stimmloses S.', 'Stimmhaftes S.', 'Stimmkörper.')
                        if language == 'Deutsch' else ('Unvoiced S.', 'Voiced Z.', 'Voice body.'))[idx]
                parts += [label(kind + ' ' + rate_label(language, rate) + ' ' + variant + '.'), gap,
                          assemble_rate(scaled[variant, rate, case], rate), gap]
            write_wave(folder / name, np.concatenate(parts), 48000, folder, files)
        try:
            entries = [(v, 48000, cases[2][0]) for v in ('A', 'B', 'C', 'D')]
            entries += [(v, hz, case) for case, _ in cases[:2]
                        for hz in (16000, 48000) for v in ('A', 'E', 'F')]
            sequence('00_Direktvergleich.wav', entries)
            for v in ('A', 'D', 'F'):
                sequence('01_Ratenfolge_' + v + '.wav',
                         [(v, hz, case) for case, _ in cases for _, hz in RATES])
        finally:
            labels.close()
        descriptions = ('A: Release 1.0.\nB: Nur Körper.\nC: Nur Klarheit.\nD: Körper und Klarheit.\n'
                        'E: Nur S-Korrektur mit 11-kHz-Klangfarbe.\nF: Körper, Klarheit und S-Korrektur.\n'
                        if language == 'Deutsch' else
                        'A: Release 1.0.\nB: Body only.\nC: Clarity only.\nD: Body and clarity.\n'
                        'E: S-only correction targeting the 11-kHz colour.\nF: Body, clarity and S correction.\n')
        readme = ('Langsamer Hörvergleich: Körper, Klarheit und S\n\n' if language == 'Deutsch'
                  else 'Slower listening comparison: body, clarity and S\n\n')
        readme += descriptions
        readme += ((f'\nNVDA {args.nvda_rate}, native Geschwindigkeit {speed}, Tonhöhe 65, '
                    'Betonung 30, Lautstärke 100, männlich, Kopfgröße 50, keine Benutzerwörterbücher.\n')
                   if language == 'Deutsch' else
                   (f'\nNVDA {args.nvda_rate}, native speed {speed}, pitch 65, inflection 30, '
                    'volume 100, male, head 50, no user dictionaries.\n'))
        readme += ('''
00_Direktvergleich.wav: zuerst Stimmkörper A/B/C/D bei 48 kHz, danach
stimmloses und stimmhaftes S mit A/E/F bei 16 und 48 kHz.
Die gesprochenen Ansagen nennen jeweils Satztyp, Abtastrate und Fassung.
01_Ratenfolge_A/D/F.wav: alle drei Sätze mit aufsteigenden Abtastraten.
Einzeldateien: einzelne WAVs mit ihrer tatsächlichen nativen Abtastrate.
Bei 11,025 kHz sind die sechs Fassungen bitgenau identisch.

Körper und Klarheit verändern nur die periodische Stimmquelle. Bei A/B/C/D
bleibt der Rauschanteil bitgenau gleich. E/F verändern das S-Rauschziel
anhand der klassischen 11,025-kHz-Bank. Dafür wird der vorhandene kausale
Filterabgleich verwendet; im Synthesizer kommt kein Resampler hinzu.
Es sind Hörversuche, deren Gefallen erst beurteilt werden muss.

Die Aufnahmen wurden direkt mit der niedrigeren Geschwindigkeit erzeugt,
nicht durch nachträgliches Strecken verlangsamt. Die Sammelvergleiche sind
verlustfreie 48-kHz-WAVs; nur für deren Zusammenstellung wurden die Clips
offline umgerechnet. Die Einzeldateien behalten ihre nativen Raten.
Der Pegel der Stimmquelle wurde durch erneute Synthese abgeglichen, ohne
den Rauschanteil mitzuskalieren. Für alle Fassungen eines Satzes und einer
Rate gelten anschließend derselbe aus A abgeleitete Dateipegel und
dieselbe Pegelreserve. Keine Kompression, kein Limiter und keine einzelne
Spitzennormalisierung der Testclips.
''' if language == 'Deutsch' else '''
00_Direktvergleich.wav: body A/B/C/D at 48 kHz, then unvoiced/voiced S
A/E/F at 16 and 48 kHz. Spoken labels identify every sample.
01_Ratenfolge_A/D/F.wav: all three sentences at ascending native rates.
Einzeldateien: individual WAVs at their true native sample rates.
The 11.025-kHz versions are bit-identical across all six profiles.

Body and clarity act on periodic excitation only. A/B/C/D preserve the
original noise exactly. E/F change only the S noise target, using the classic
11.025-kHz bank, with the existing causal fit and no runtime resampler.
These are listening experiments, not established improvements.

The recordings are re-synthesized at the lower native speed, not slowed by
stretching a recording. Guided comparisons use lossless 48-kHz WAV assembly;
only that offline assembly resamples clips. Individual clips retain their
native rates. Voicing gains are calibrated by native re-synthesis to match
overall energy without scaling the noise branch. A common reference-derived
file gain and headroom trim are applied to all variants of each sentence/rate.
No compression, clipping, limiter or per-file peak normalization is used.
''')
        readme += ''.join('\n' + case + ': ' + text for case, text in cases)
        (folder / 'README.txt').write_text(readme + '\n', encoding='utf-8-sig')
        report = dict(language=language, nvda_rate=args.nvda_rate, native_speed=speed,
                      old_native_speed=98, old_reference_seconds=old_duration,
                      new_reference_seconds=new_duration,
                      prototype_sha256=hashlib.sha256(library.read_bytes()).hexdigest(),
                      reference_sha256=hashlib.sha256(reference.read_bytes()).hexdigest(),
                      reference_matches_release_1_0=True, identical_11025=True,
                      noise_branch_separation_verified=True, checks=checks, files=files)
        (folder / 'validation.json').write_text(json.dumps(report, indent=2, allow_nan=False) + '\n')
        archive = folder.with_suffix('.zip'); pending = archive.with_suffix('.zip.partial')
        with zipfile.ZipFile(pending, 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as z:
            for p in sorted(folder.rglob('*')):
                if p.is_file(): z.write(p, str(p.relative_to(folder)))
        with zipfile.ZipFile(pending) as z: assert z.testzip() is None
        os.replace(pending, archive)
        direct = args.output / (folder.name + '_Direktvergleich.wav')
        direct.write_bytes((folder / '00_Direktvergleich.wav').read_bytes())
        print(json.dumps(dict(package=str(archive), bytes=archive.stat().st_size,
                              files=len(files), native_speed=speed, old_seconds=old_duration,
                              new_seconds=new_duration)), flush=True)


if __name__ == '__main__':
    main()
