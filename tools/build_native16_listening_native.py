"""Build a listening-only native-16-kHz consonant reference host."""
from pathlib import Path
import argparse
import subprocess

ROOT = Path(__file__).resolve().parents[1]

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output',type=Path,required=True)
    p.add_argument('--object-dir',type=Path,default=ROOT/'eci/build/posix')
    p.add_argument('--provider',type=Path,default=ROOT/'vendor/openevv61/build/release/libreloq61.a')
    a=p.parse_args(); output=a.output.resolve(); output.parent.mkdir(parents=True,exist_ok=True)
    flags=['cc','-std=c11','-O2','-Wall','-Wextra','-Werror','-pedantic',
        '-ffp-contract=off','-fPIC','-fvisibility=hidden','-I'+str(ROOT/'include')]
    changed=['src/reloq_eci.c','src/reloq_german_generic_voice.c',
        'tests/reloq_listening_native16.c','tests/reloq_listening_source.c']
    objects=[]
    for name in changed:
        obj=output.parent/(Path(name).stem+'.o')
        subprocess.run(flags+['-c',str(ROOT/name),'-o',str(obj)],check=True);objects.append(obj)
    skip={'reloq_eci.o','reloq_german_generic_voice.o','reloq_rate_transfer.o'}
    shared=sorted(p for p in a.object_dir.resolve().glob('*.o') if p.name not in skip)
    assert shared, 'Build production POSIX objects first'
    subprocess.run(['cc','-O3','-flto=2','-save-temps=obj','-fno-strict-aliasing','-ffp-contract=off',
        '-shared',*map(str,shared+objects),str(a.provider.resolve()),'-pthread',
        '-Wl,--gc-sections,--no-undefined','-Wl,--wrap=reloq_noise_profile_prepare',
        '-Wl,--wrap=reloq_rate_transfer_prepare_noise_mix_profile',
        '-Wl,--wrap=reloq_rate_transfer_process_voice','-lm','-o',str(output)],check=True)
    print(output)

if __name__=='__main__': main()
