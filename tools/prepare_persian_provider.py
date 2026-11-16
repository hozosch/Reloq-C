#!/usr/bin/env python3
"""Prepare the approved, thread-scoped Persian rules for the native provider.

The original provider files remain the source of the Finnish and donor
languages. Generate replacement translation units in the platform's build
directory, and compile those units instead of the unmodified originals.
No runtime Python dependency or environment-variable switch is introduced.
"""
from __future__ import annotations

import argparse
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('destination', type=Path)
    args = parser.parse_args()
    destination = args.destination.resolve()
    destination.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='persian-provider-', dir=destination.parent) as tmp:
        prepared = Path(tmp) / 'generated'
        subprocess.run([sys.executable, str(ROOT / 'experiments/persian/build_native_probe.py'),
                        '--dry-run', '--work-dir', str(prepared)], check=True)
        sources = list(prepared.glob('*.c'))
        if len(sources) != 18:
            raise RuntimeError(f'unexpected Persian provider source count: {len(sources)}')
        for source in sources:
            value = source.read_text(encoding='utf8')
            if 'getenv(' in value:
                value = ('extern const char *reloq_fa_scoped_get(const char *);\n' +
                         value.replace('getenv(', 'reloq_fa_scoped_get('))
                source.write_text(value, encoding='utf8')
        (prepared / '.stamp').write_text('Scoped Persian rules\n', encoding='ascii')
        if destination.exists():
            shutil.rmtree(destination)
        prepared.rename(destination)


if __name__ == '__main__':
    main()
