"""Create labelled, lossless S/Z listening packages from three native builds.

WAV output is intentional: these are requested listening deliverables, not
the numeric regression suite. Offline resampling is used only to assemble
the guided 48-kHz comparisons; individual files retain their native rates.
"""
from pathlib import Path
import argparse
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
LANGUAGES = {
    'Deutsch': (0x40000, (
        ('01_stimmlos', 'Es ist nass. Das Fass ist weiß. Klaus isst Reis.'),
        ('02_stimmhaft', 'Susi summt. Sie sehen sieben Segel.'),
        ('03_Stimmkoerper', 'Eine warme, ruhige Melodie am Meer.'))),
    'US_English': (0x10000, (
        ('01_unvoiced', 'Sue sips soup. Sam sits still beside a silent sea.'),
        ('02_voiced', 'Lazy bees buzz by the roses.'),
        ('03_voice_body', 'A warm, mellow aroma.'))),
}


def engine(path, language):
    result = DirectEciEngine(path, dialect=language, encoding='utf8',
                             experimental_generic=True, lexicon_path=None)
    for parameter, value in ((NATIVE_SPEED, 98), (NATIVE_PITCH, 65),
                             (NATIVE_INFLECTION, 30), (NATIVE_VOLUME, 100),
                             (GENDER, 0), (HEAD_SIZE, 50)):
        result.set_parameter(parameter, value)
    return result


def speak(native, text):
    chunks = []
    assert native.speak([text], chunks.append, None)
    pcm = np.frombuffer(b''.join(chunks), dtype='<i2').copy()
    assert len(pcm) and not np.any((pcm == 32767) | (pcm == -32768))
    return pcm


def active_rms(pcm):
    data = pcm.astype(np.float64)
    active = data[np.abs(data) >= 64]
    assert len(active)
    return float(np.sqrt(np.mean(active * active)))


def converted(pcm, rate):
    data = pcm.astype(np.float64) / 32768
    if rate == 48000:
        return data
    divisor = math.gcd(rate, 48000)
    return resample_poly(data, 48000 // divisor, rate // divisor,
                         window=('kaiser', 8.6))


def save_wave(path, data, rate, manifest, root):
    assert np.isfinite(data).all() and np.max(np.abs(data)) < 1
    pcm = np.rint(data * 32768).astype('<i2')
    assert not np.any((pcm == 32767) | (pcm == -32768))
    path.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(path), 'wb') as output:
        output.setnchannels(1)
        output.setsampwidth(2)
        output.setframerate(rate)
        output.writeframes(pcm.tobytes())
    with wave.open(str(path), 'rb') as check:
        assert (check.getnchannels(), check.getsampwidth(), check.getframerate(),
                check.getnframes()) == (1, 2, rate, len(pcm))
    manifest.append(dict(file=str(path.relative_to(root)), rate=rate, samples=len(pcm),
                         peak=int(np.max(np.abs(pcm.astype(np.int32)))),
                         sha256=hashlib.sha256(path.read_bytes()).hexdigest()))


def rate_name(language, rate):
    if language == 'Deutsch':
        return {11025: 'Elf Komma null zwei fünf', 16000: 'Sechzehn',
                22050: 'Zweiundzwanzig Komma null fünf', 32000: 'Zweiunddreißig',
                44100: 'Vierundvierzig Komma eins', 48000: 'Achtundvierzig'}[rate] + ' Kilohertz.'
    return {11025: 'Eleven point zero two five', 16000: 'Sixteen',
            22050: 'Twenty two point zero five', 32000: 'Thirty two',
            44100: 'Forty four point one', 48000: 'Forty eight'}[rate] + ' kilohertz.'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--reference', type=Path, required=True)
    parser.add_argument('--soft', type=Path, required=True)
    parser.add_argument('--clear', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    libraries = dict(A=args.reference.resolve(), B=args.soft.resolve(), C=args.clear.resolve())
    args.output = args.output.resolve()
    args.output.mkdir(parents=True, exist_ok=True)
    for language, (dialect, sentences) in LANGUAGES.items():
        folder = args.output / ('Reloq_S_Hoervergleich_' + language)
        folder.mkdir(exist_ok=False)
        captures, versions = {}, {}
        for variant, library in libraries.items():
            native = engine(library, dialect)
            try:
                versions[variant] = dict(eci_version=native.native_version,
                    sha256=hashlib.sha256(library.read_bytes()).hexdigest())
                for code, rate in RATES:
                    native.set_parameter(SAMPLE_RATE, code)
                    for case, text in sentences:
                        captures[variant, rate, case] = speak(native, text)
            finally:
                native.close()
        for _, rate in RATES:
            for case, _ in sentences:
                rows = [captures[v, rate, case] for v in libraries]
                assert len({len(row) for row in rows}) == 1, (language, rate, case)
                if rate == 11025:
                    assert all(np.array_equal(rows[0], row) for row in rows[1:])
        # One reference-derived gain per sentence/rate is applied identically
        # to A/B/C. Thus the rate comparison is level matched without hiding
        # any relative S-level changes between the three source builds.
        gain, normalized, metadata = {}, {}, []
        for case, _ in sentences:
            for _, rate in RATES:
                gain[rate, case] = 3000 / active_rms(captures['A', rate, case])
            peak = max(np.max(np.abs(captures[v, rate, case].astype(np.float64))) *
                       gain[rate, case] / 32768 for v in libraries for _, rate in RATES)
            safety = min(1, .65 / peak)
            for variant in libraries:
                for order, (_, rate) in enumerate(RATES, 1):
                    data = captures[variant, rate, case].astype(np.float64) / 32768
                    normalized[variant, rate, case] = data * gain[rate, case] * safety
                    if rate == 11025 and variant != 'A':
                        continue
                    name = f'{order:02d}_{rate}_' + ('Referenz' if rate == 11025 else variant) + '.wav'
                    save_wave(folder / 'Einzeldateien' / case / name,
                              normalized[variant, rate, case], rate, metadata, folder)
        labels = engine(libraries['A'], dialect)
        labels.set_parameter(SAMPLE_RATE, 1)
        label_cache = {}
        def label(text):
            if text not in label_cache:
                pcm = speak(labels, text)
                data = converted(pcm, 11025)
                data *= .32 / max(np.max(np.abs(data)), .001)
                label_cache[text] = data
            return label_cache[text]
        gap = np.zeros(round(48000 * .35))
        def sequence(path, entries):
            parts = []
            for variant, rate, case in entries:
                case_index = next(i for i, row in enumerate(sentences) if row[0] == case)
                category = (('Stimmlos.', 'Stimmhaft.', 'Stimmkörper.') if language == 'Deutsch'
                            else ('Unvoiced S.', 'Voiced Z.', 'Voice body.'))[case_index]
                parts.extend((label(category + ' ' + rate_name(language, rate) + ' ' + variant + '.'), gap,
                              converted(normalized[variant, rate, case] * 32768, rate), gap))
            save_wave(folder / path, np.concatenate(parts), 48000, metadata, folder)
        try:
            sequence('00_Direktvergleich_16_48.wav',
                     [(variant, rate, case) for case, _ in sentences[:2]
                      for rate in (16000, 48000) for variant in libraries])
            for variant in libraries:
                sequence('01_Ratenfolge_' + variant + '.wav',
                         [(variant, rate, case) for case, _ in sentences[:2] for _, rate in RATES])
            sequence('02_Stimmkoerper_Ratenfolge.wav',
                     [('A', rate, sentences[2][0]) for _, rate in RATES])
        finally:
            labels.close()
        if language == 'Deutsch':
            readme = '''Hörvergleich für S, Z und die Abtastraten

Zum Einstieg: 00_Direktvergleich_16_48.wav.
Die Ansagen nennen Stimmlos/Stimmhaft, die Rate und A/B/C.
01_Ratenfolge_A/B/C.wav enthalten jeweils beide S-Sätze in aufsteigenden Raten.
02_Stimmkoerper_Ratenfolge.wav enthält einen überwiegend stimmhaften Vergleichssatz.
Einzeldateien enthält die Clips mit ihrer tatsächlichen nativen Abtastrate.
Die 11,025-kHz-Fassung ist in allen Varianten identisch und liegt dort nur einmal vor.

A: bisheriger Klang aus Release 1.0 (im S-Pfad wie 0.2.205).
B: breitere Hauptresonanz und stärker abgestufte obere Anteile.
C: wie B, mit stärkerer Orientierung der unteren Rauschflanke am 11-kHz-S.
B/C sind Hörversuche. Die Messwerte belegen keine subjektive Verbesserung.

Die gesprochenen Vergleiche sind verlustfreie Mono-WAVs mit 48 kHz.
Für diese Zusammenstellung wurden die nativen Clips nur offline umgerechnet;
ihre ursprüngliche Bandbreite bleibt erhalten. Im Synthesizer ist kein Resampler hinzugekommen.
Pro Satz/Rate gilt derselbe aus A abgeleitete Lautstärkefaktor für A/B/C.
Es gibt keine Dynamikkompression, Begrenzung oder einzelne Spitzen-Normalisierung.
Parameter: männlich, Kopfgröße 50, native Geschwindigkeit 98, Tonhöhe 65,
Betonung 30, Lautstärke 100, keine Benutzerwörterbücher.

Beurteile die Klarheit, Körnigkeit, ein mögliches Lispeln und die Schärfe des S.
Bei den Ratenfolgen außerdem: Wird der Klang angenehm breiter oder nur heller?
'''
        else:
            readme = '''S/Z and sample-rate listening comparison

Start with 00_Direktvergleich_16_48.wav. Spoken labels identify unvoiced/voiced,
the sample rate and A/B/C. The 01_Ratenfolge_A/B/C.wav files each contain both
S/Z sentences at ascending sample rates. 02_Stimmkoerper_Ratenfolge.wav adds
a mostly voiced sentence for comparing the voice body.
Einzeldateien contains individual clips at their true native sample rates.
The 11.025-kHz versions are identical in A/B/C and stored only once there.

A: current release 1.0 sound (same S path as 0.2.205).
B: a broader principal S resonance and more progressive upper-band energy.
C: B with the lower noise skirt more closely anchored to the 11-kHz S.
B/C are listening experiments; numeric measurements do not establish preference.

Guided comparisons are lossless mono 48-kHz WAV files. Native clips were
resampled offline for assembly only; their original bandwidth is retained.
No resampler was added to the synthesizer. The same reference-derived gain
is applied to A/B/C for each sentence/rate. No compression, limiting or
individual peak normalization is used.
Settings: male, head size 50, native speed 98, pitch 65, inflection 30,
volume 100, no user dictionaries.

Compare S clarity, grain, lispiness and sharpness. Across rates, consider
whether the voice becomes pleasantly broader or merely brighter.
'''
        readme += '\n'.join('\n' + case + ': ' + text for case, text in sentences) + '\n'
        (folder / 'README.txt').write_text(readme, encoding='utf-8-sig')
        report = dict(language=language, builds=versions, rates=[rate for _, rate in RATES],
                      settings=dict(speed=98, pitch=65, inflection=30, volume=100, head=50, gender=0),
                      equal_native_durations=True, identical_11025=True,
                      relative_levels_preserved=True, files=metadata)
        (folder / 'validation.json').write_text(json.dumps(report, indent=2, allow_nan=False) + '\n')
        archive = folder.with_suffix('.zip')
        pending = archive.with_suffix('.zip.partial')
        with zipfile.ZipFile(pending, 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as zipped:
            for path in sorted(folder.rglob('*')):
                if path.is_file():
                    zipped.write(path, str(path.relative_to(folder)))
        with zipfile.ZipFile(pending) as zipped:
            assert zipped.testzip() is None
        os.replace(pending, archive)
        print(json.dumps(dict(package=str(archive), bytes=archive.stat().st_size,
                              wav_files=len(metadata), sha256=hashlib.sha256(archive.read_bytes()).hexdigest())))


if __name__ == '__main__':
    main()
