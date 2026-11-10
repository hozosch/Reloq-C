"""Run the real NVDA settings panel on an isolated portable NVDA profile."""
import argparse
import json
import os
from pathlib import Path
import shutil
import subprocess
import time
import zipfile


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--nvda',type=Path,required=True)
    parser.add_argument('--addon',type=Path,required=True)
    parser.add_argument('--work',type=Path,required=True)
    args=parser.parse_args()
    work=args.work.resolve()
    if work.exists(): shutil.rmtree(work)
    profile=work/'config'; target=profile/'addons'/'reloqC'
    target.mkdir(parents=True)
    with zipfile.ZipFile(args.addon) as bundle:
        assert bundle.testzip() is None
        bundle.extractall(target)
    shutil.copy2(Path(__file__).with_name('test_nvda_real_settings_plugin.py'),
                 target/'globalPlugins'/'reloqSettingsProbe.py')
    (profile/'nvda.ini').write_text('[general]\nplayStartAndExitSounds = False\n[speech]\nsynth = silence\n',encoding='utf8')
    result=work/'result.json'; log=work/'nvda.log'
    env=dict(os.environ,RELOQ_SETTINGS_PROBE_RESULT=str(result))
    process=subprocess.Popen([str(args.nvda.resolve()),'--minimal','--no-sr-flag',
        '--debug-logging','--config-path='+str(profile),'--log-file='+str(log)],env=env)
    try:
        deadline=time.monotonic()+90
        while time.monotonic()<deadline and not result.exists():
            if process.poll() is not None: raise RuntimeError(f'NVDA exited early: {process.returncode}')
            time.sleep(.1)
        if not result.exists(): raise RuntimeError('NVDA settings probe timed out')
        data=json.loads(result.read_text(encoding='utf8'))
        if not data.get('ok'): raise RuntimeError(data.get('error','unknown NVDA settings failure'))
        print('PASS',data['details'])
    finally:
        process.terminate()
        try: process.wait(timeout=10)
        except subprocess.TimeoutExpired: process.kill();process.wait()
        if log.exists() and not result.exists():
            print(log.read_text(encoding='utf8',errors='replace')[-12000:])


if __name__=='__main__':main()
