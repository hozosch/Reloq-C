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
from scipy.signal import resample_poly, welch
import random

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import (DirectEciEngine, SAMPLE_RATE, NATIVE_SPEED,
    NATIVE_PITCH, NATIVE_INFLECTION, NATIVE_VOLUME, GENDER, HEAD_SIZE)

RATES = ((1, 11025), (3, 16000), (2, 22050), (4, 32000), (5, 44100), (6, 48000))
MODES = dict(A=0, B=3, C=4, D=12, E=15)
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


def band_power(data, rate, low, high):
    f, p = welch(np.asarray(data, dtype=np.float64), fs=rate,
                 nperseg=round(rate * .04), noverlap=round(rate * .02))
    return float(np.sum(p[(f >= low) & (f < high)]))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--reference', type=Path, required=True)
    parser.add_argument('--nvda-rate', type=int, default=30)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    assert 0 <= args.nvda_rate <= 100
    speed = 40 + (116 * args.nvda_rate + 50) // 100
    library, reference = args.library.resolve(), args.reference.resolve()
    dll = C.CDLL(str(library)); setter = dll.reloq_listening_set_profile
    setter.argtypes = (C.c_uint, C.c_double); setter.restype = C.c_int
    args.output = args.output.resolve(); args.output.mkdir(parents=True, exist_ok=True)
    for language, (dialect, cases) in TEXTS.items():
        folder = args.output / ('Reloq_Breitband_Koerper_S_' + language)
        folder.mkdir(exist_ok=False)
        engine, truth = create(library, dialect, speed), create(reference, dialect, speed)
        captures, checks, metrics, separation = {}, [], [], []
        gains = {}
        def render(profile, gain, text):
            assert setter(profile, gain) == 0
            return speak(engine, text)
        try:
            for code, rate in RATES:
                engine.set_parameter(SAMPLE_RATE, code); truth.set_parameter(SAMPLE_RATE, code)
                body_text = cases[2][1]
                base_body = render(0, 1, body_text).astype(np.float64)
                noise_body = render(3, 0, body_text).astype(np.float64)
                assert np.array_equal(noise_body, render(0, 0, body_text))
                if rate == 11025:
                    voice_gain = 1.0
                else:
                    pilot = render(3, .5, body_text).astype(np.float64)
                    voice = 2 * (pilot - noise_body)
                    aa = np.mean(voice * voice); b = np.mean(voice * noise_body)
                    target = np.mean(base_body * base_body); cc = np.mean(noise_body * noise_body)
                    voice_gain = float((-b + np.sqrt(b * b + aa * (target - cc))) / aa)
                    assert 0 < voice_gain < 4
                gains[rate] = voice_gain
                for case, text in cases:
                    a = render(0, 1, text)
                    assert np.array_equal(a, speak(truth, text)), (language, rate, case, 'reference')
                    noise_a = render(0, 0, text); noise_c = render(4, 0, text); noise_d = render(12, 0, text)
                    assert np.array_equal(noise_a, render(3, 0, text)), 'Voice tuning changed noise'
                    assert np.array_equal(noise_d, render(15, 0, text)), 'Combined tuning changed S noise'
                    for label, profile in MODES.items():
                        gain = voice_gain if label in ('B', 'E') else 1.0
                        final = render(profile, gain, text)
                        assert len(final) == len(a)
                        if rate == 11025:
                            assert np.array_equal(a, final), (language, label, '11-kHz changed')
                        captures[label, rate, case] = final
                        delta = 10 * np.log10(np.mean(final.astype(float) ** 2) / np.mean(a.astype(float) ** 2))
                        checks.append(dict(variant=label, rate=rate, case=case, voiced_gain=gain, total_level_db=float(delta)))
                    voice_a = a.astype(np.int32) - noise_a.astype(np.int32)
                    for label, noise in (('C', noise_c), ('D', noise_d)):
                        voiced = captures[label, rate, case].astype(np.int32) - noise.astype(np.int32)
                        residual = voiced - voice_a
                        # Native PCM conversion truncates before and after
                        # output gain (2.02 in the shared voice defaults).
                        # Subtracting two separately quantized mixes is not
                        # equivalent to quantizing their difference once.
                        peak_error = int(np.max(np.abs(residual)))
                        rms_error = float(np.sqrt(np.mean(residual.astype(float) ** 2)))
                        assert peak_error <= 8 and rms_error < 1, (label, 'S tuning changed voicing', peak_error, rms_error)
                        separation.append(dict(rate=rate, case=case, variant=label,
                            peak_pcm_error=peak_error, rms_pcm_error=rms_error))
                    voice_b = captures['B', rate, case].astype(np.int32) - noise_a.astype(np.int32)
                    voice_e = captures['E', rate, case].astype(np.int32) - noise_d.astype(np.int32)
                    residual = voice_b - voice_e
                    assert np.max(np.abs(residual)) <= 8 and np.sqrt(np.mean(residual.astype(float) ** 2)) < 1
                    if case == cases[2][0]:
                        assert np.array_equal(a, captures['C', rate, case])
                        assert np.array_equal(a, captures['D', rate, case])
                        assert np.array_equal(captures['B', rate, case], captures['E', rate, case])
                        body_level = 10 * np.log10(np.mean(captures['B', rate, case].astype(float) ** 2) / np.mean(a.astype(float) ** 2))
                        assert abs(body_level) < .04
                        bands = {}
                        for label, v in (('A', voice_a), ('B', voice_b)):
                            bands[label] = {name: band_power(v, rate, lo, hi) for name, lo, hi in (
                                ('fundamental', 80, 250), ('body', 300, 900), ('clarity', 2500, 5200))}
                        metrics.append(dict(kind='voiced_body', rate=rate,
                            calibration_gain_db=20 * math.log10(voice_gain),
                            body_to_fundamental_change_db=10 * math.log10(
                                bands['B']['body'] / bands['B']['fundamental'] / (bands['A']['body'] / bands['A']['fundamental'])),
                            clarity_to_body_change_db=10 * math.log10(
                                bands['B']['clarity'] / bands['B']['body'] / (bands['A']['clarity'] / bands['A']['body'])),
                            powers=bands))
                probe = 'Ass. Ass. Ass. Ass.' if language == 'Deutsch' else 'Us. Us. Us. Us.'
                for label in ('A', 'C', 'D'):
                    noise = render(MODES[label], 0, probe)
                    all_power = band_power(noise, rate, 0, rate / 2 + 1)
                    upper = band_power(noise, rate, 5500, rate / 2 + 1) / all_power
                    metrics.append(dict(kind='S_noise', rate=rate, variant=label, above_5500_fraction=upper))
                print(language, rate, 'voice gain', round(voice_gain, 4), flush=True)
        finally:
            engine.close(); truth.close()
        body48 = next(m for m in metrics if m['kind'] == 'voiced_body' and m['rate'] == 48000)
        assert body48['body_to_fundamental_change_db'] > 1.5, body48
        for _, rate in RATES[1:]:
            rows = {m['variant']: m['above_5500_fraction'] for m in metrics if m['kind'] == 'S_noise' and m['rate'] == rate}
            for v in ('C', 'D'):
                assert rows[v] > rows['A'] * .5, (language, rate, v, rows)
        scaled, levels = {}, {}
        for case, _ in cases:
            for _, rate in RATES:
                ref = captures['A', rate, case].astype(float)
                active = ref[np.abs(ref) >= 64]
                levels[rate, case] = 3000 / np.sqrt(np.mean(active * active))
        peak = max(np.max(np.abs(x.astype(float))) * levels[rate, case] / 32768
                   for (_, rate, case), x in captures.items())
        headroom = min(1, .65 / peak); files = []
        for (variant, rate, case), x in captures.items():
            data = x.astype(float) / 32768 * levels[rate, case] * headroom
            scaled[variant, rate, case] = data
            if rate == 11025 and variant != 'A': continue
            order = next(i for i, (_, hz) in enumerate(RATES, 1) if hz == rate)
            name = f'{order:02d}_{rate}_' + ('Referenz' if rate == 11025 else variant) + '.wav'
            write_wave(folder / 'Einzeldateien' / case / name, data, rate, folder, files)
        labels = create(reference, dialect, speed); labels.set_parameter(SAMPLE_RATE, 1)
        cache, gap = {}, np.zeros(round(.35 * 48000))
        def label(text):
            if text not in cache:
                x = assemble_rate(speak(labels, text).astype(float) / 32768, 11025)
                cache[text] = x * .3 / max(np.max(np.abs(x)), .001)
            return cache[text]
        def sequence(name, entries, blind=False):
            parts, key = [], []
            for number, (variant, rate, case, group, position) in enumerate(entries, 1):
                index = next(i for i, row in enumerate(cases) if row[0] == case)
                kind = (('Stimmloses S.', 'Stimmhaftes S.', 'Stimmkörper.') if language == 'Deutsch'
                        else ('Unvoiced S.', 'Voiced Z.', 'Voice body.'))[index]
                announcement = (f'Gruppe {group}. Fassung {position}.' if language == 'Deutsch'
                                else f'Group {group}. Version {position}.') if blind else variant + '.'
                parts += [label(kind + ' ' + rate_label(language, rate) + ' ' + announcement), gap,
                          assemble_rate(scaled[variant, rate, case], rate), gap]
                key.append(dict(clip=number, group=group, position=position, variant=variant, rate=rate, case=case))
            write_wave(folder / name, np.concatenate(parts), 48000, folder, files)
            return key
        try:
            direct = [('A',48000,cases[2][0],0,0),('B',48000,cases[2][0],0,0)]
            direct += [(v, hz, case, 0, 0) for case, _ in cases[:2] for hz in (16000,48000) for v in ('A','C','D')]
            direct += [(v,48000,case,0,0) for case, _ in cases for v in ('A','E')]
            sequence('00_Direktvergleich.wav', direct)
            for v in ('A', 'E'):
                sequence('01_Ratenfolge_' + v + '.wav', [(v,hz,case,0,0) for case, _ in cases for _,hz in RATES])
            rng = random.Random(72519); entries=[]; group=0
            for case, _ in cases[:2]:
                for hz in (16000,22050,48000):
                    group+=1; variants=['A','C','D']; rng.shuffle(variants)
                    entries += [(v,hz,case,group,i) for i,v in enumerate(variants,1)]
            key = sequence('02_Blindvergleich_S.wav', entries, blind=True)
        finally:
            labels.close()
        group_word, version_word = ('Gruppe', 'Fassung') if language == 'Deutsch' else ('Group', 'Version')
        answer = '\n'.join(f"{group_word} {k['group']}, {version_word} {k['position']}: {k['variant']}, {k['rate']} Hz, {k['case']}" for k in key)
        (folder / 'Aufloesung_Blindvergleich.txt').write_text(answer + '\n', encoding='utf-8-sig')
        readme = f'''Breitbandiger Hörvergleich: Stimme und zwei S-Ansätze

A: unverändertes Release 1.0.
B: gezieltere untere Mitten und angepasste obere stimmhafte Anteile; S unverändert.
C: nur die untere S-Flanke stärker am klassischen Spektrum orientiert; obere Bandbreite bleibt.
D: wie C, zusätzlich geglätteter inverser Filterabgleich als Versuch gegen Körnung.
E: B und D zusammen.

00_Direktvergleich.wav: Stimme A/B bei 48 kHz, dann stimmloses und stimmhaftes
S mit A/C/D bei 16 und 48 kHz, schließlich alle drei Sätze A/E bei 48 kHz.
01_Ratenfolge_A/E.wav: je Satz 11,025, 16, 22,05, 32, 44,1 und 48 kHz.
02_Blindvergleich_S.wav: sechs Gruppen mit je drei neutral nummerierten
Fassungen, bei 16, 22,05 und 48 kHz; Zuordnung in Aufloesung_Blindvergleich.txt.
Einzeldateien: alle Fassungen mit ihrer tatsächlichen nativen Abtastrate.

Die Blindvergleiche eignen sich auch für jemanden, der das Lispeln hört.
Pro Gruppe können die bevorzugte Nummer und getrennt Lispeln,
Körnung/Verzerrung sowie Schärfe beurteilt werden. Kein Unterschied ist
auch ein sinnvolles Ergebnis. Keine Fassung ist bereits als Verbesserung
bestätigt. Die Auflösung lässt sich bis nach dem Hören beiseitelegen.

Tempo: NVDA {args.nvda_rate}, native Geschwindigkeit {speed}; Tonhöhe 65, Betonung 30,
Lautstärke 100, männliches Modell, Kopfgröße 50, keine Benutzerwörterbücher.
Das langsamere Tempo wird nativ erzeugt, ohne zeitliches Strecken.

Die Stimmformung und S-Korrekturen sind unabhängig. Die Stimme nutzt eine
begrenzte Mittenanhebung statt der verworfenen breiten Bassanhebung. Ihr
Pegel wird einmal pro Rate am stimmhaften Referenzsatz abgeglichen, nicht
für jedes Wort oder jeden S-Satz neu. A/C/D behalten dieselbe Stimmquelle;
B/E teilen dieselbe veränderte Stimmquelle. Das S wird nicht auf die
11-kHz-Bandbreite begrenzt, seine Resonanz wird nicht verbreitert, und es
bekommt keine zusätzliche Höhenanhebung. D glättet nur die schmalen
Korrekturen des Filterabgleichs, anschließend bleibt dessen bestehender
Leistungsabgleich aktiv. Bei 11,025 kHz sind alle Fassungen bitgleich.

Alle Fassungen eines Satzes und einer Rate teilen denselben aus A
abgeleiteten Dateipegel und dieselbe Pegelreserve. Keine Kompression,
kein Limiter und keine einzelne Spitzennormalisierung der Testclips.
Die gesprochen beschrifteten Vergleiche werden ausschließlich zur
Zusammenstellung offline auf 48 kHz gebracht; die Einzeldateien behalten
ihre nativen Raten. Es sind Hörversuche, keine neue Add-on-Version.
'''
        if language == 'US_English':
            readme = f'''Wideband listening comparison: voice body and two S hypotheses

A: unchanged release 1.0.
B: targeted lower mids and adjusted upper voiced harmonics; S unchanged.
C: only the lower S flank moves toward the classic spectrum; upper bandwidth is retained.
D: C plus a smoother inverse correction, tested as a possible way to reduce graininess.
E: B and D together.

00_Direktvergleich.wav: voice A/B at 48 kHz, unvoiced/voiced S with A/C/D
at 16 and 48 kHz, then all three sentences with A/E at 48 kHz.
01_Ratenfolge_A/E.wav: each sentence at 11.025, 16, 22.05, 32, 44.1 and 48 kHz.
02_Blindvergleich_S.wav: six groups of three neutrally numbered versions
at 16, 22.05 and 48 kHz. The key is in Aufloesung_Blindvergleich.txt.
Einzeldateien: individual files at their actual native sample rates.

The blind comparisons can be assessed by the listener who hears the lisp.
For each group, note a preferred number and rate lispiness, grain/distortion
and sharpness separately. No audible difference is also useful feedback.
Neither S hypothesis is an established improvement. The key can be left
unread until after listening.

Speed: NVDA {args.nvda_rate}, native {speed}; pitch 65, inflection 30, volume 100,
male model, head 50, no user dictionaries. The slower speech is synthesized
natively, without time stretching.

Voice and S processing are independent. A limited midrange bell replaces
the rejected broad low shelf. Voice level is calibrated once per rate using
the body sentence, then kept fixed for every sentence. A/C/D share exactly
the same periodic voice; B/E share the modified voice. The S is not limited
to 11-kHz bandwidth, its resonance is not widened, and no extra upper boost
is applied. D smooths narrow features of the inverse correction; the existing
integrated-power compensation remains active. All versions are bit-identical
at 11.025 kHz.

All versions of a sentence/rate share the same reference-derived file gain
and headroom. No compression, limiter or individual test-clip peak matching.
Only the guided comparisons are assembled offline at 48 kHz. Individual WAVs
retain their native rates. These are listening experiments, not an add-on.
'''
        readme += ''.join('\n' + case + ': ' + text for case, text in cases)
        (folder / 'README.txt').write_text(readme + '\n', encoding='utf-8-sig')
        report = dict(language=language, nvda_rate=args.nvda_rate, native_speed=speed,
            prototype_sha256=hashlib.sha256(library.read_bytes()).hexdigest(),
            reference_sha256=hashlib.sha256(reference.read_bytes()).hexdigest(),
            reference_matches_release_1_0=True, identical_11025=True,
            periodic_noise_separation_verified=True, separation_pcm=separation,
            gain_calibration='one body sentence per rate, same profile gains for every sentence',
            checks=checks, spectral_metrics=metrics, blind_key=key, files=files)
        (folder / 'validation.json').write_text(json.dumps(report, indent=2, allow_nan=False) + '\n')
        archive=folder.with_suffix('.zip'); pending=archive.with_suffix('.zip.partial')
        with zipfile.ZipFile(pending,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
            for p in sorted(folder.rglob('*')):
                if p.is_file():z.write(p,str(p.relative_to(folder)))
        with zipfile.ZipFile(pending) as z:assert z.testzip() is None
        os.replace(pending,archive)
        direct=args.output/(folder.name+'_Direktvergleich.wav')
        direct.write_bytes((folder/'00_Direktvergleich.wav').read_bytes())
        print(json.dumps(dict(package=str(archive),bytes=archive.stat().st_size,wav_files=len(files),native_speed=speed,body_metric_48000=body48)),flush=True)


if __name__ == '__main__':
    main()
