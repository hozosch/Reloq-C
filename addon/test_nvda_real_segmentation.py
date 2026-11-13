"""Exercise the real NVDA speech pipeline with its built-in dictionaries."""
import argparse
import json
import os
from pathlib import Path
import shutil
import subprocess
import time
import zipfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--nvda', type=Path, required=True)
    parser.add_argument('--addon', type=Path, required=True)
    parser.add_argument('--driver', type=Path,
                        help='use this candidate driver with the packaged native DLL')
    parser.add_argument('--plugin', type=Path,
                        default=Path(__file__).with_name('test_nvda_real_segmentation_plugin.py'))
    parser.add_argument('--work', type=Path, required=True)
    parser.add_argument('--diagnose', action='store_true')
    args = parser.parse_args()
    work = args.work.resolve()
    if work.exists(): shutil.rmtree(work)
    profile = work / 'config'
    target = profile / 'addons' / 'reloqC'
    target.mkdir(parents=True)
    legacy = work / 'legacy-nvda-dictionary.dic'
    legacy.write_text('# A previously selected NVDA dictionary\nlegacyprobe\tSonderwort\t0\t2\n', encoding='utf-8-sig')
    (profile / 'reloqCDictionaries.json').write_text(json.dumps({
        'version': 1, 'repository': 'community', 'sources': [
            {'path': str(legacy), 'language': 0x40000, 'volume': 0}]}), encoding='utf8')
    with zipfile.ZipFile(args.addon) as bundle:
        if bundle.testzip() is not None: raise RuntimeError('corrupt add-on')
        bundle.extractall(target)
    if args.driver:
        shutil.copy2(args.driver, target / 'synthDrivers' / 'reloqC' / '__init__.py')
    shutil.copy2(args.plugin, target / 'globalPlugins' / 'reloqSegmentationProbe.py')
    (profile / 'nvda.ini').write_text('[general]\nplayStartAndExitSounds = False\n[speech]\nsynth = silence\n',encoding='utf8')
    result, log = work / 'result.json', work / 'nvda.log'
    env = dict(os.environ, RELOQ_SEGMENTATION_PROBE_RESULT=str(result),
               RELOQ_SEGMENTATION_DIAGNOSE='1' if args.diagnose else '0')
    process = subprocess.Popen([str(args.nvda.resolve()), '--minimal', '--no-sr-flag',
        '--debug-logging', '--config-path=' + str(profile), '--log-file=' + str(log)], env=env)
    try:
        # The complete issue #34/#39/#40 matrix uses the real speech manager,
        # both English dialects and the stock/empty in-memory builtin dictionary.
        deadline = time.monotonic() + 600
        while time.monotonic() < deadline and not result.exists():
            if process.poll() is not None: raise RuntimeError(f'NVDA exited early: {process.returncode}')
            time.sleep(.1)
        if not result.exists(): raise RuntimeError('real NVDA speech test timed out')
        data = json.loads(result.read_text(encoding='utf8'))
        print(json.dumps(data, ensure_ascii=False))
        if not data.get('ok'): raise RuntimeError(data.get('error', 'real NVDA speech failed'))
    finally:
        process.terminate()
        try: process.wait(timeout=10)
        except subprocess.TimeoutExpired: process.kill();process.wait()
        if log.exists() and not result.exists():
            print(log.read_text(encoding='utf8', errors='replace')[-12000:])


if __name__ == '__main__': main()
