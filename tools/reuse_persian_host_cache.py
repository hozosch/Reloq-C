#!/usr/bin/env python3
"""Reuse only a verified dev144 Windows host cache after Persian rule edits."""
from __future__ import annotations

import os
from pathlib import Path
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[1]
BASE = '3c3d3f27635e48974487d6c8769a2b98dd97ae0c'
EXPECTED_KEY = ('persian-windows-host-gcc14-v2-'
                '095ec9297122d7ae23141480a61b623ca85d47fc13ed7e5a61e180300e5207bf')
RECENT_BASE = '5675c59f29e690b2e15ba7ba9c209934a09632c9'
RECENT_KEY = ('persian-windows-host-gcc14-v2-'
              'e69d174816ad2c632f7786280780e6d8f87ca561fcef6adc1212928b2e58f6b6')
RECENT_ALLOWED = {'include/reloq_eci.h', 'include/reloq_persian_numbers.h',
                  'src/reloq_eci.c', 'experiments/persian/direct_fa_numbers.c',
                  'experiments/persian/direct_fa_numbers.h',
                  'experiments/persian/direct_fa_characters.c',
                  'experiments/persian/direct_fa_characters.h'}
NEWEST_BASE = 'ca30814b6041007ecbe94c7681ef2e39a2ec09d9'
NEWEST_KEY = ('persian-windows-host-gcc14-v2-'
              'f4aa931982b6b718f119c59251c1b630a0645e043c0d790f358088e893c079a6')
NEWEST_ALLOWED = {'experiments/persian/direct_fa_lexicon.c',
                  'experiments/persian/direct_fa_text.c'}
TRACE_BASE = '14bfa3132c9936d2798d3281ac5693b550f002cb'
TRACE_KEY = ('persian-windows-host-gcc14-v2-'
             '9fbaf447f57f00e8aa4e28add7806baf049e17728506d6b7be26561f5313afaa')
DEV149_BASE = 'ccdc6680e8d698c6cf4834556706021aa0027f25'
DEV149_KEY = ('persian-windows-host-gcc14-v2-'
              '4419e090258b54c489fe7ecfba709236760771647c2b523f2865fd3ebe3fd944')
DEV149_ALLOWED = {'include/reloq_syllable_trace.h',
                  'experiments/persian/direct_fa_lexicon.c',
                  'vendor/openevv61/src/reloq61_bridge.c',
                  'vendor/openevv61/lang/fifi/delta_rules_c30_fifi.c',
                  'vendor/openevv61/lang/enus61/delta_rules_c29_enus61.c'}
DEV150_BASE = '0c2120cc341b6f9034564df99539f3c16e94e51c'
DEV150_KEY = ('persian-windows-host-gcc14-v2-'
              'dac446486a22d19a77d825d49f9646341087763174c54ba85cd8cda4c8791601')
DEV150_ALLOWED = {'experiments/persian/build_native_probe.py',
                  'src/reloq_persian_acoustics.c'}
DEV151_BASE = '64cecf6a0541bcc74a6f39677b726b576b9f1b25'
DEV151_KEY = ('persian-windows-host-gcc14-v2-'
              'a9bee5067b5866b06b93d8cc979749e88a6150b3e977d397276e2e911f96023e')
DEV151_ALLOWED = {'experiments/persian/build_native_probe.py',
                  'src/reloq_persian_acoustics.c'}
DEV152_BASE = '812e03b4fe7f0a8331b7a433c352a86a34710498'
DEV152_KEY = ('persian-windows-host-gcc14-v2-'
              '56a9101fabd669ff59d3badaef2dc36a73677c46a44d562608d88877b274f13a')
DEV152_ALLOWED = {'experiments/persian/build_native_probe.py',
                  'src/reloq_persian_acoustics.c',
                  'vendor/openevv61/lang/jajp61/delta_rules_c17_jajp61.c',
                  'vendor/openevv61/lang/jajp61/delta_rules_c30_jajp61.c'}
DEV153_BASE = '0d8ccd6e773bcd9e1af0a3ecbd4b3ddc956fbd82'
DEV153_KEY = ('persian-windows-host-gcc14-v2-'
              '9334727f647864f5d57874fced644ee7c81c02f472b7147f391038ea8999ee37')
DEV153_ALLOWED = {'experiments/persian/build_native_probe.py',
                  'src/reloq_persian_acoustics.c',
                  'vendor/openevv61/Makefile.native', 'tools/prepare_persian_provider.py'}
DEV153_CHECKED = {'vendor/openevv61/Makefile.native': '0a483b13d681dfcb0e61f886cc677259d40b0ba9',
                  'tools/prepare_persian_provider.py': 'b384362ec6ba15c0fd412b6f4f94201d738c2470'}
DEV154_BASE = 'bd28d74c148d9b88b2995d9267293c40a0f78be5'
DEV154_KEY = ('persian-windows-host-gcc14-v2-'
              '28334eae4866b95529b1c12304a1fbb6cb2012e9886ee8436249506ffa838835')
DEV154_ALLOWED = {'experiments/persian/build_native_probe.py',
                  'src/reloq_persian_acoustics.c',
                  'vendor/openevv61/src/eci/bridge/eci_stmarray.c'}
DEV155_BASE = 'b434e5cb96f9f824cc4d0287d1e1427a96853f41'
DEV155_KEY = ('persian-windows-host-gcc14-v2-'
              'd60887a553bd146e3d780f72638facb392f554e616ff3c8ce78dc1982edd5417')
DEV155_ALLOWED = {'src/reloq_persian_acoustics.c',
                  'experiments/persian/direct_fa_lexicon.c',
                  'experiments/persian/direct_fa_text.c',
                  'experiments/persian/direct_fa_characters.inc'}
DEV156_BASE = '2228cec2756a974b3a939d0a20fd24807cfa9795'
DEV156_KEY = ('persian-windows-host-gcc14-v2-'
              'f9388dd3e51b71dced57f8f35bbd18bc125d4abba4535fbfcf772d72be27a199')
DEV156_ALLOWED = {'experiments/persian/build_native_probe.py',
                  'src/reloq_persian_acoustics.c'}
DEV157_BASE = 'd6cf01188733063aacbd0637f00e9643cec0cfc9'
DEV157_KEY = ('persian-windows-host-gcc14-v2-'
              '6484c46e5ee8e46a391f4360c8036e02a5cecc9033d4ce66b167a9703f0a4b1f')
DEV157_ALLOWED = {'experiments/persian/build_native_probe.py',
                  'src/reloq_persian_acoustics.c',
                  'experiments/persian/direct_fa_pinglish.c'}
DEV158_BASE = '54829010074cbe0b9a66d9b218769257e7fdcd42'
DEV158_KEY = ('persian-windows-host-gcc14-v2-'
              'f8652d8ddfddfd4d3df0334cd6c65a2b5be31cdb60fb2238c0834176d3c57902')
DEV158_ALLOWED = {'experiments/persian/build_native_probe.py',
                  'src/reloq_persian_acoustics.c', 'src/reloq_noise_profile.c'}
DEV159_BASE = '0198909fe09a5a36c00d68b7276ef3f76f5fdb42'
DEV159_KEY = ('persian-windows-host-gcc14-v2-'
              'ea351e41abbbd5cf7f9bf3b9aac8bbeeb04e6730c38512719e4ca99030a93b76')
DEV159_ALLOWED = {'experiments/persian/build_native_probe.py'}
DEV160_BASE = 'ae05979e72f5b2b5b695b23900abc9a02f84f1fd'
DEV160_KEY = ('persian-windows-host-gcc14-v2-'
              '26f76276c275cb8c4eab548c53efc73c3145f7f77fbb53825d928cf75af24a3e')
DEV160_ALLOWED = {'experiments/persian/build_native_probe.py'}
DEV161_BASE = '3ed3f000f9cddf54db0560d7efe7b805f422f7f6'
DEV161_KEY = ('persian-windows-host-gcc14-v2-'
              '8e622f843ed25aaf518c3edf2bc09060bcde28cefcbc1866dbf2f570c2d7b8c8')
DEV162_BASE = 'dcb158357f9327bf20a3f9dc7ea84e0616c16878'
DEV162_KEY = ('persian-windows-host-gcc14-v2-'
              'dc698865629915348179f0cd6d7ee667fb27457cb4aba0a9f3961eb660d5d42b')
DEV162_ALLOWED = {'experiments/persian/build_native_probe.py'}
DEV163_BASE = '573598d47a37057e183fde20c8dad97482fe2bb6'
DEV163_KEY = ('persian-windows-host-gcc14-v2-'
              '64655db88d9d2921c335cc3fce13b44126d168864cfffa3fc0b453bfc582b492')
DEV163_ALLOWED = {'experiments/persian/build_native_probe.py',
                  'include/reloq_eci.h', 'include/reloq_persian_tuning.h',
                  'vendor/openevv61/src/reloq61_bridge.c',
                  'src/reloq_rules61.c', 'src/reloq_persian_frames.c',
                  'src/reloq_persian_replay.inc', 'src/reloq_eci.c'}
DEV164_BASE = 'dcff146b028fd0c254017c44eda9bbf56531606f'
DEV164_KEY = ('persian-windows-host-gcc14-v2-'
              'de500316e8c7bfea087b7eb9bdd6965ce21ea030dfedbb4def6c30f111d52d42')
DEV164_ALLOWED = {'experiments/persian/direct_fa_lexicon.c',
                  'experiments/persian/direct_fa_lexicon.h',
                  'experiments/persian/direct_fa_text.c'}
DEV165_ALLOWED = DEV164_ALLOWED | {'src/reloq_persian_frame_internal.h',
                                  'src/reloq_persian_frames.c',
                                  'src/reloq_persian_replay.inc',
                                  'src/reloq_rules61.c',
                                  'experiments/persian/direct_fa_letters.inc',
                                  'experiments/persian/direct_fa_numbers.c'}
DEV166_BASE = '2b20c1c385f9412a1c22e32a18efe9308d82401c'
DEV166_KEY = ('persian-windows-host-gcc14-v2-'
              '278f51539cb847a4e95abe4bc1fc20ee9a8d0949090162e7ede927b0fa2834ba')
DEV166_ALLOWED = {'experiments/persian/build_native_probe.py',
                  'experiments/persian/direct_fa_lexicon.c'}
DEV168_BASE = '629f88e138a2189e65bfb4c167db94853b43e106'
DEV168_KEY = ('persian-windows-host-gcc14-v2-'
              '2d28d855a417923c98c03817e565440186fb9c17a0d6508a945bb3d8b3be9fc5')
DEV168_ALLOWED = {'experiments/persian/build_native_probe.py',
                  'experiments/persian/direct_fa_lexicon.c'}
DEV161_ALLOWED = {
    'experiments/persian/build_native_probe.py',
    'include/reloq_eci.h', 'include/reloq_syllable_trace.h', 'src/reloq_eci.c',
    'vendor/openevv61/src/reloq61_bridge.c',
    'vendor/openevv61/lang/fifi/delta_rules_c13_fifi.c',
    'vendor/openevv61/lang/dede61/delta_rules_c24_dede61.c',
    'vendor/openevv61/lang/enus61/delta_rules_c31_enus61.c',
    'vendor/openevv61/lang/jajp61/delta_rules_c25_jajp61.c',
}
TRACE_ALLOWED = {'include/reloq_eci.h', 'include/reloq_persian_frames.h',
                 'include/reloq_syllable_trace.h', 'src/reloq_eci.c',
                 'src/reloq_persian_frames.c', 'src/reloq_persian_frame_internal.h',
                 'experiments/persian/direct_fa_lexicon.c',
                 'vendor/openevv61/src/reloq61_bridge.c',
                 'vendor/openevv61/lang/fifi/delta_rules_c30_fifi.c',
                 'vendor/openevv61/lang/enus61/delta_rules_c29_enus61.c'}
ALLOWED = {'experiments/persian/direct_fa_lexicon.c',
           'experiments/persian/direct_fa_lexicon.h',
           'experiments/persian/direct_fa_text.c'}
INPUTS = ('eci/Makefile', 'eci/reloq_eci_fixed_math.c', 'src', 'include',
          'vendor/openevv61', 'tools/prepare_persian_provider.py',
          'experiments/persian/build_native_probe.py',
          'experiments/aragonese/build_native_probe.py',
          'experiments/persian/direct_fa_*.c', 'experiments/persian/direct_fa_*.h',
          'experiments/persian/direct_fa_*.inc')


def eligible(key, changed):
    return bool(changed) and ((key == EXPECTED_KEY and set(changed) <= ALLOWED) or
                              (key == RECENT_KEY and set(changed) <= RECENT_ALLOWED) or
                              (key == NEWEST_KEY and set(changed) <= NEWEST_ALLOWED) or
                              (key == TRACE_KEY and set(changed) <= TRACE_ALLOWED) or
                              (key == DEV149_KEY and set(changed) == DEV149_ALLOWED) or
                              (key == DEV150_KEY and set(changed) == DEV150_ALLOWED) or
                              (key == DEV151_KEY and set(changed) == DEV151_ALLOWED) or
                              (key == DEV152_KEY and set(changed) == DEV152_ALLOWED) or
                              (key == DEV153_KEY and set(changed) == DEV153_ALLOWED) or
                              (key == DEV154_KEY and set(changed) == DEV154_ALLOWED) or
                              (key == DEV155_KEY and set(changed) == DEV155_ALLOWED) or
                              (key == DEV156_KEY and set(changed) == DEV156_ALLOWED) or
                              (key == DEV157_KEY and set(changed) == DEV157_ALLOWED) or
                              (key == DEV158_KEY and set(changed) == DEV158_ALLOWED) or
                              (key == DEV159_KEY and set(changed) == DEV159_ALLOWED) or
                              (key == DEV160_KEY and set(changed) == DEV160_ALLOWED) or
                              (key == DEV161_KEY and set(changed) == DEV161_ALLOWED) or
                              (key == DEV162_KEY and set(changed) == DEV162_ALLOWED) or
                              (key == DEV163_KEY and set(changed) == DEV163_ALLOWED) or
                              (key == DEV164_KEY and set(changed) in (DEV164_ALLOWED, DEV165_ALLOWED)) or
                              (key == DEV166_KEY and set(changed) == DEV166_ALLOWED) or
                              (key == DEV168_KEY and set(changed) == DEV168_ALLOWED))


def main():
    build, dll = ROOT / 'eci/build/windows', ROOT / 'eci/eci.dll'
    try:
        key = os.environ.get('PERSIAN_HOST_CACHE_KEY', '')
        base = DEV168_BASE if key == DEV168_KEY else DEV166_BASE if key == DEV166_KEY else DEV164_BASE if key == DEV164_KEY else DEV163_BASE if key == DEV163_KEY else DEV162_BASE if key == DEV162_KEY else DEV161_BASE if key == DEV161_KEY else DEV160_BASE if key == DEV160_KEY else DEV159_BASE if key == DEV159_KEY else DEV158_BASE if key == DEV158_KEY else DEV157_BASE if key == DEV157_KEY else DEV156_BASE if key == DEV156_KEY else DEV155_BASE if key == DEV155_KEY else DEV154_BASE if key == DEV154_KEY else DEV153_BASE if key == DEV153_KEY else DEV152_BASE if key == DEV152_KEY else DEV151_BASE if key == DEV151_KEY else DEV150_BASE if key == DEV150_KEY else DEV149_BASE if key == DEV149_KEY else TRACE_BASE if key == TRACE_KEY else NEWEST_BASE if key == NEWEST_KEY else RECENT_BASE if key == RECENT_KEY else BASE
        changed = subprocess.check_output(['git', 'diff', '--name-only', base, 'HEAD', '--', *INPUTS],
                                          cwd=ROOT, text=True).splitlines()
        if not eligible(key, changed):
            raise ValueError('cached host source transition was not verified')
        if key == DEV153_KEY:
            for path, blob in DEV153_CHECKED.items():
                current = subprocess.check_output(['git', 'rev-parse', 'HEAD:' + path], cwd=ROOT, text=True).strip()
                if current != blob:
                    raise ValueError('Persian provider build instructions changed')
        if not build.is_dir() or not dll.is_file():
            raise ValueError('cached host objects or DLL are missing')
        for path in build.rglob('*'):
            if path.is_file():
                os.utime(path, None)
        os.utime(dll, None)
        # All native Persian direct components depend on direct_fa_*.h.
        for obj in build.glob('reloq_fa_direct_*.o'):
            obj.unlink()
        if key in (RECENT_KEY, TRACE_KEY, DEV149_KEY, DEV150_KEY, DEV151_KEY, DEV152_KEY, DEV153_KEY, DEV154_KEY, DEV155_KEY, DEV156_KEY, DEV157_KEY, DEV158_KEY, DEV159_KEY, DEV160_KEY, DEV161_KEY, DEV162_KEY, DEV163_KEY, DEV164_KEY, DEV166_KEY, DEV168_KEY):
            # These are the only host sources which include reloq_eci.h;
            # direct Persian objects also cover the changed private headers.
            for name in ('reloq_eci.o', 'reloq_rules61.o'):
                (build / name).unlink(missing_ok=True)
            if key in (TRACE_KEY, DEV149_KEY, DEV163_KEY, DEV164_KEY):
                (build / 'reloq_persian_frames.o').unlink(missing_ok=True)
            if key == DEV158_KEY:
                (build / 'reloq_noise_profile.o').unlink(missing_ok=True)
            if key == DEV161_KEY:
                for dep in build.glob('*.d'):
                    contents=dep.read_text(encoding='utf8', errors='replace')
                    if 'reloq_eci.h' in contents or 'reloq_syllable_trace.h' in contents:
                        dep.with_suffix('.o').unlink(missing_ok=True)
            if key == DEV166_KEY:
                for dep in build.glob('*.d'):
                    if 'reloq_persian_tuning.h' in dep.read_text(encoding='utf8', errors='replace'):
                        dep.with_suffix('.o').unlink(missing_ok=True)
            dll.unlink()
        print('Reused verified Windows host objects; rebuilding Persian text components.')
    except (OSError, ValueError, subprocess.CalledProcessError) as exc:
        print(f'Cannot safely reuse host cache ({exc}); rebuilding the host cleanly.')
        shutil.rmtree(build, ignore_errors=True)
        dll.unlink(missing_ok=True)


if __name__ == '__main__':
    main()
