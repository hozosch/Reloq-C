"""Package the independent NVDA acoustic listening laboratory."""
from pathlib import Path
import argparse
import zipfile
from build_experimental_nvda import MODULES

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / 'addon/soundlab'
PREFIX = 'globalPlugins/reloqSoundLab/'


def entries():
    result = {str(p.relative_to(SOURCE)).replace('\\','/'): p.read_bytes()
              for p in SOURCE.rglob('*') if p.is_file() and '__pycache__' not in p.parts}
    for name in ('_reloq_eci_direct', '_reloq_languages', '_reloq_persian_frames',
                 '_reloq_persian_glide', '_reloq_persian_projection'):
        result[PREFIX + name + '.py'] = (ROOT / 'nvda' / (name + '.py')).read_bytes()
    result[PREFIX + '_persian/__init__.py'] = b'"""Private copy of the Persian frontend for isolated listening tests."""\n'
    for name in MODULES:
        result[PREFIX + '_persian/' + name + '.py'] = (ROOT / 'experiments/persian' / (name + '.py')).read_bytes()
    result['doc/licenses/ReLoq-C-LICENSE.txt'] = (ROOT / 'LICENSE').read_bytes()
    result['doc/licenses/ReLoq-C-NOTICE.txt'] = (ROOT / 'NOTICE').read_bytes()
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', required=True, type=Path)
    args = parser.parse_args()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(args.output, 'w', compression=zipfile.ZIP_DEFLATED, compresslevel=9) as z:
        for name, data in sorted(entries().items()):
            info = zipfile.ZipInfo(name, (1980,1,1,0,0,0))
            info.compress_type = zipfile.ZIP_DEFLATED
            z.writestr(info, data)
    print('Created independent sound laboratory:', args.output)

if __name__ == '__main__': main()
