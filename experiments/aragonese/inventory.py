"""Inspectable Aragonese pronunciation targets and Castilian baseline.

This is a research corpus, not a text-to-speech implementation. In particular,
the orthography does not determine whether every written x is /ʃ/ or /ks/.
The cases and alternatives are from the 2023 Academia Aragonesa orthography;
see README.md. Audio and phoneme output from the parent is diagnostic only.
"""
from __future__ import annotations

import argparse
from dataclasses import asdict, dataclass
import json
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from probe import spanish_probe_input


@dataclass(frozen=True)
class Case:
    group: str
    word: str
    target: str
    reference: str | None = None


CASES = (
    Case('ny', 'sinyal', 'ny = ɲ', 'siñal'),
    Case('ny', 'cabanyera', 'ny = ɲ', 'cabañera'),
    Case('ny', 'nyapir', 'ny = ɲ', 'ñapir'),
    Case('ny', 'bony', 'ny = ɲ', 'boñ'),
    Case('ny', 'Espanya', 'ny = ɲ', 'España'),
    Case('sh', 'xada', 'x = ʃ'),
    Case('sh', 'xuela', 'x = ʃ'),
    Case('sh', 'caixa', 'ix = ʃ or i̯ʃ'),
    Case('sh', 'peix', 'ix = ʃ or i̯ʃ'),
    Case('sh', 'mixina', 'x = ʃ'),
    Case('ks', 'xenofobo', 'x = ks'),
    Case('ks', 'taxi', 'x = ks'),
    Case('ks', 'sexo', 'x = ks'),
    Case('ks', 'examen', 'x = ks'),
    Case('ks', 'externo', 'x = ks'),
    Case('theta', 'cinglo', 'c before i = θ'),
    Case('theta', 'mocet', 'c before e = θ'),
    Case('theta', 'mocez', 'c before e and final z = θ'),
    Case('theta', 'panizo', 'z = θ'),
    Case('palatal_l', 'llebre', 'll = ʎ'),
    Case('palatal_l', 'palla', 'll = ʎ'),
    Case('palatal_l', 'viello', 'll = ʎ'),
    Case('au', 'auca', 'au ends in rounded u̯'),
    Case('au', 'pauto', 'au ends in rounded u̯'),
    Case('au', 'auto', 'au ends in rounded u̯'),
    Case('au', 'cudiau', 'au ends in rounded u̯'),
    Case('final_t', 'puent', 'final nt is pronounced n'),
    Case('final_t', 'chent', 'final nt is pronounced n'),
    Case('final_t', 'dolent', 'final nt is pronounced n'),
    Case('final_t', 'rebost', 'final st is pronounced s'),
    Case('final_t', 'cort', 'final rt is pronounced r'),
    Case('stress', 'malament', 'final stress'),
    Case('stress', 'grafico', 'penultimate stress'),
    Case('stress', 'ráfil', 'written penultimate stress'),
)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    args = parser.parse_args()
    from _reloq_eci_direct import DirectEciEngine
    engine = DirectEciEngine(args.library.resolve(), dialect=0x20000, encoding='utf8', native_voice=True)
    try:
        rows = []
        for case in CASES:
            row = asdict(case)
            row['castilian'] = engine._phonemes_for_probe(case.word)
            if case.group == 'ny':
                adapted = spanish_probe_input(case.word)
                assert adapted == case.reference
                row['ny_probe'] = engine._phonemes_for_probe(adapted)
            rows.append(row)
        print(json.dumps(rows, ensure_ascii=False, indent=2))
    finally:
        engine.close()


if __name__ == '__main__':
    main()
