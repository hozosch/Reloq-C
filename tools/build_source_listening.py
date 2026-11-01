"""Compare native S levels and periodic-source harmonics at slower speed.

This is a listening experiment, not a DECtalk richness implementation.
No production source change beyond the requested S/Z rollback is included.
"""
from pathlib import Path
import argparse
import ctypes as C
import hashlib
import json
import math
import os
import random
import zipfile

import numpy as np

from build_wideband_listening import (TEXTS, RATES, create, speak, band_power,
    assemble_rate, write_wave, rate_label, SAMPLE_RATE)

PROFILES = dict(A=0, B=40, C=60)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--reference', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    library, reference, root = args.library.resolve(), args.reference.resolve(), args.output.resolve()
    root.mkdir(parents=True, exist_ok=True)
    dll = C.CDLL(str(library)); setter = dll.reloq_source_listening_set
    setter.argtypes = (C.c_uint, C.c_double); setter.restype = C.c_int
    for language, (dialect, cases) in TEXTS.items():
        folder = root / ('Reloq_Quellobertoene_Original_S_' + language)
        folder.mkdir(exist_ok=False)
        engine, truth = create(library, dialect, 75), create(reference, dialect, 75)
        captures, metrics, gains = {}, [], {}
        def render(profile, gain, text):
            assert setter(profile, gain) == 0
            return speak(engine, text)
        try:
            for code, rate in RATES:
                engine.set_parameter(SAMPLE_RATE, code); truth.set_parameter(SAMPLE_RATE, code)
                body = cases[2][1]
                a = render(0, 1, body).astype(float); noise = render(0, 0, body).astype(float)
                gains[rate, 'A'] = 1.0
                for label in ('B', 'C'):
                    profile = PROFILES[label]
                    assert np.array_equal(noise, render(profile, 0, body))
                    pilot = render(profile, .25, body).astype(float)
                    voice = 4 * (pilot - noise)
                    aa = np.mean(voice * voice); b = np.mean(voice * noise)
                    target = np.mean(a * a); cc = np.mean(noise * noise)
                    gain = float((-b + np.sqrt(b * b + aa * (target - cc))) / aa)
                    gains[rate, label] = 1.0 if rate == 11025 else gain
                    assert 0 < gains[rate, label] < 4
                for case, text in cases:
                    captures['R', rate, case] = speak(truth, text)
                    noise = render(0, 0, text)
                    for label, profile in PROFILES.items():
                        assert np.array_equal(noise, render(profile, 0, text)), (language, rate, case, 'noise changed')
                        captures[label, rate, case] = render(profile, gains[rate, label], text)
                        assert len(captures[label, rate, case]) == len(captures['R', rate, case])
                        if rate == 11025:
                            assert np.array_equal(captures[label, rate, case], captures['A', rate, case])
                    if case == cases[2][0]:
                        assert np.array_equal(captures['R', rate, case], captures['A', rate, case]), 'S rollback changed body'
                        powers = {}
                        for label in PROFILES:
                            v = captures[label, rate, case].astype(float) - noise
                            powers[label] = {name: band_power(v, rate, lo, hi) for name, lo, hi in (
                                ('lower', 80, 800), ('upper', 2500, 5000))}
                            level = 10 * math.log10(np.mean(captures[label, rate, case].astype(float)**2) / np.mean(a*a))
                            assert abs(level) < .05, (language, rate, label, 'body gain', level)
                        relative = {label: 10 * math.log10(p['upper']/p['lower'] /
                            (powers['A']['upper']/powers['A']['lower'])) for label, p in powers.items()}
                        metrics.append(dict(rate=rate, powers=powers, upper_relative_to_lower_db=relative,
                            voice_gains={label:gains[rate,label] for label in PROFILES}))
                print(language, rate, 'upper/lower', metrics[-1]['upper_relative_to_lower_db'], flush=True)
        finally:
            engine.close(); truth.close()
        # These guard actual output spectral changes; they do not establish
        # audibility, preference or equivalence to DECtalk's ri parameter.
        assert metrics[-1]['upper_relative_to_lower_db']['B'] > 3
        assert metrics[-1]['upper_relative_to_lower_db']['C'] > 5
        levels = {}
        for _, rate in RATES:
            body = captures['A', rate, cases[2][0]].astype(float)
            active = body[np.abs(body) >= 64]
            levels[rate] = 3000 / np.sqrt(np.mean(active * active))
        peak = max(np.max(np.abs(x.astype(float))) * levels[rate] / 32768
                   for (_, rate, _), x in captures.items())
        trim = min(1, .65 / peak); files, scaled = [], {}
        for (label, rate, case), x in captures.items():
            data = x.astype(float) / 32768 * levels[rate] * trim
            scaled[label, rate, case] = data
            if rate == 11025 and label in ('B', 'C'): continue
            order = next(i for i, (_, hz) in enumerate(RATES, 1) if hz == rate)
            write_wave(folder / 'Einzeldateien' / case / f'{order:02d}_{rate}_{label}.wav',
                       data, rate, folder, files)
        labels = create(reference, dialect, 75); labels.set_parameter(SAMPLE_RATE, 1)
        cache, gap = {}, np.zeros(round(.35 * 48000))
        def announcement(text):
            if text not in cache:
                x = assemble_rate(speak(labels, text).astype(float) / 32768, 11025)
                cache[text] = x * .3 / max(np.max(np.abs(x)), .001)
            return cache[text]
        def sequence(name, entries, blind=False):
            parts, key = [], []
            for label, rate, case, group, position in entries:
                i = next(i for i, row in enumerate(cases) if row[0] == case)
                kind = (('Stimmloses S.', 'Stimmhaftes S.', 'Stimmquelle.') if language == 'Deutsch'
                        else ('Unvoiced S.', 'Voiced Z.', 'Voice source.'))[i]
                tag = ((f'Gruppe {group}. Fassung {position}.' if language == 'Deutsch'
                        else f'Group {group}. Version {position}.') if blind else label + '.')
                parts += [announcement(kind + ' ' + rate_label(language, rate) + ' ' + tag), gap,
                          assemble_rate(scaled[label,rate,case], rate), gap]
                key.append(dict(group=group, position=position, variant=label, rate=rate, case=case))
            write_wave(folder / name, np.concatenate(parts), 48000, folder, files)
            return key
        try:
            direct = [(v,hz,cases[2][0],0,0) for hz in (22050,48000) for v in PROFILES]
            direct += [(v,22050,case,0,0) for case,_ in cases[:2] for v in ('R','A')]
            direct += [(v,48000,cases[1][0],0,0) for v in PROFILES]
            sequence('00_Direktvergleich.wav', direct)
            for v in PROFILES:
                sequence(f'01_Ratenfolge_{v}.wav', [(v,hz,cases[2][0],0,0) for _,hz in RATES])
            rng = random.Random(83951); entries = []
            for group, hz in enumerate((16000,22050,48000), 1):
                variants = list(PROFILES); rng.shuffle(variants)
                entries += [(v,hz,cases[2][0],group,i) for i,v in enumerate(variants,1)]
            key = sequence('02_Blindvergleich_Stimmquelle.wav', entries, True)
        finally:
            labels.close()
        german = language == 'Deutsch'
        group_word, version_word = ('Gruppe','Fassung') if german else ('Group','Version')
        answer = '\n'.join(f"{group_word} {k['group']}, {version_word} {k['position']}: {k['variant']}, {k['rate']} Hz" for k in key)
        (folder/'Aufloesung_Blindvergleich.txt').write_text(answer+'\n', encoding='utf-8-sig')
        readme = ('''Obere Stimmanteile und ursprüngliche S-Quellpegel

R: Release 1.0, noch mit der zusätzlichen S/Z-Anhebung.
A: dieselbe Stimme ohne die zusätzliche S/Z-Anhebung von 4 dB.
B: A mit stärkerem oberen Obertonspektrum der periodischen Stimmquelle.
C: stärkere Fassung dieses Quellversuchs.

00_Direktvergleich.wav: Stimmquelle A/B/C bei 22,05 und 48 kHz;
dann S und stimmhaftes S jeweils R/A bei 22,05 kHz; zuletzt A/B/C
im Satz mit stimmhaftem S bei 48 kHz.
01_Ratenfolge_A/B/C.wav: der Stimmquellensatz bei allen sechs Raten.
02_Blindvergleich_Stimmquelle.wav: drei Gruppen, jeweils drei neutral
nummerierte Fassungen bei 16, 22,05 und 48 kHz. Auflösung separat.

Diese Versuche betreffen die oberen periodischen Anteile. Die verworfenen
500-Hz- und 900-Hz-Anhebungen sind nicht enthalten. Die neue Quellenformung
verändert keine S-Geräusche, Behauchtheit, Formantfrequenzen oder Dauern.
Sie orientiert sich am gewünschten Obertonreichtum, ist aber keine
Nachbildung des DECtalk-Richness-Reglers. Ein weicher Hochton-Shelf bei
1,8 kHz formt nur die periodische Quelle, bevor Behauchtheit hinzukommt.
B nutzt vor dem Pegelabgleich 1,6 bis 8 dB, C 2,4 bis 12 dB, von 16 bis
48 kHz. Der Pegelabgleich erhält die Gesamtenergie. Impulsform, Öffnungsphase,
Tonhöhenverlauf und die bisherigen Takt- und Geräuschmasken bleiben erhalten.
Das ursprüngliche S-Quellniveau gilt in A/B/C bei allen Raten. Die bestehende
hohe Bandbreite bleibt; F/V/W und andere Konsonanten werden nicht abgesenkt.

Tempo NVDA 30, native Geschwindigkeit 75, Tonhöhe 65, Betonung 30,
Lautstärke 100, männliches Modell, Kopfgröße 50, keine Benutzerwörterbücher.
Ein Quellpegelabgleich pro Rate und Fassung, am Stimmquellensatz ermittelt,
gilt unverändert für alle Sätze. Ein gemeinsamer Dateipegel pro Rate erhält
auch den Pegelunterschied R/A beim S. Kein Limiter, keine Kompression,
kein zeitliches Strecken. Nur die beschrifteten Zusammenstellungen werden
offline auf 48 kHz gebracht; Einzeldateien behalten ihre nativen Raten.
Bei 11,025 kHz sind A/B/C identisch. Noch keine bestätigte Klangverbesserung
und keine neue Add-on-Version.
''' if german else '''Upper voiced harmonics and native S source levels

R: release 1.0, including its additional S/Z source boost.
A: the same voice with the additional 4-dB S/Z boost removed.
B: A with stronger upper harmonics in the periodic voice source.
C: a stronger version of that source experiment.

00_Direktvergleich.wav: source A/B/C at 22.05 and 48 kHz;
unvoiced S and voiced Z with R/A at 22.05 kHz; finally A/B/C
in the voiced-Z sentence at 48 kHz.
01_Ratenfolge_A/B/C.wav: the source sentence at all six sample rates.
02_Blindvergleich_Stimmquelle.wav: three groups of three neutrally numbered
versions at 16, 22.05 and 48 kHz. The answer key is a separate file.

These trials address upper periodic harmonics. Neither rejected 500-Hz nor
900-Hz boost is present. The new source shaping leaves S noise, aspiration,
formant frequencies and timing unchanged. It targets the requested harmonic
richness, but is not DECtalk's richness implementation. A gentle 1.8-kHz high
shelf shapes only periodic excitation before aspiration joins it. B uses
1.6 to 8 dB before level matching, C 2.4 to 12 dB, from 16 to 48 kHz. Voice
level is matched for equal total energy. Pulse shape, open phase, pitch,
period clock and noise masks remain native. A/B/C use native S source levels
at every rate. Existing wide bandwidth and F/V/W adjustments are retained.

Speed NVDA 30/native 75, pitch 65, inflection 30, volume 100, male model,
head 50, no user dictionaries. One voice-level calibration per rate/profile
uses the source sentence and is then fixed for every sentence. A shared
file gain per rate preserves the R/A S-level difference. No limiter,
compression or time stretching. Only labelled sequences are assembled
offline at 48 kHz; individual files retain native rates. A/B/C are identical
at 11.025 kHz. No preferred sound is established; this is not an add-on.
''')
        readme += ''.join('\n'+case+': '+text for case,text in cases)+'\n'
        (folder/'README.txt').write_text(readme, encoding='utf-8-sig')
        report = dict(language=language, native_speed=75, nvda_rate=30,
            reference_sha256=hashlib.sha256(reference.read_bytes()).hexdigest(),
            prototype_sha256=hashlib.sha256(library.read_bytes()).hexdigest(),
            removed_s_source_gain_db=4, source_noise_independence=True,
            body_reference_unchanged=True, identical_ABC_11025=True,
            spectral_metrics=metrics, blind_key=key, files=files)
        (folder/'validation.json').write_text(json.dumps(report,indent=2,allow_nan=False)+'\n')
        archive=folder.with_suffix('.zip'); pending=archive.with_suffix('.zip.partial')
        with zipfile.ZipFile(pending,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
            for p in sorted(folder.rglob('*')):
                if p.is_file(): z.write(p,str(p.relative_to(folder)))
        with zipfile.ZipFile(pending) as z: assert z.testzip() is None
        os.replace(pending,archive)
        direct=root/(folder.name+'_Direktvergleich.wav')
        direct.write_bytes((folder/'00_Direktvergleich.wav').read_bytes())
        print(json.dumps(dict(package=str(archive),bytes=archive.stat().st_size,
            wav_files=len(files),source_48000=metrics[-1])),flush=True)


if __name__ == '__main__':
    main()
