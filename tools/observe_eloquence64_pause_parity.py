"""Compare actual Eloquence_64 v28 inputs against an external ECI library.

The library and its language files are obtained separately. Only aggregate
measurements and hashes are emitted; neither proprietary binaries nor audio
are included in the results. Run on Windows with Python matching the library.
"""
import argparse
import array
import ctypes as C
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys

# Languages common to the v28 32-bit host and ReLoq C.
CASES = (
    ('en_US', 0x10000, ['Hello, world!', 'Hello, world']),
    ('en_GB', 0x10001, ['Hello, world!', 'Hello, world']),
    ('es_ES', 0x20000, ['Hola, mundo.', 'Hola, mundo']),
    ('es_MX', 0x20001, ['Hola, mundo.', 'Hola, mundo']),
    ('fr_FR', 0x30000, ['Bonjour, monde.', 'Bonjour, monde']),
    ('fr_CA', 0x30001, ['Bonjour, monde.', 'Bonjour, monde']),
    ('de_DE', 0x40000, ['Hallo, Welt.', 'Hallo, Welt']),
    ('it_IT', 0x50000, ['Ciao, mondo.', 'Ciao, mondo']),
    ('zh_CN', 0x60000, ['你好，世界。', '你好,世界.', '你好，Hello!', '你好', 'Hello, world!']),
    ('pt_BR', 0x70000, ['Olá, mundo.', 'Olá, mundo']),
    ('ja_JP', 0x80000, ['こんにちは、世界。', 'こんにちは']),
    ('fi_FI', 0x90000, ['Hei, maailma.', 'Hei, maailma']),
    ('ko_KR', 0xa0000, ['안녕, 세계.', '안녕']),
)


def quiet_metrics(audio):
    """Approximate near-silence in 5 ms frames, including the trailing end."""
    frame = 55
    threshold_squared = 12 * 12
    quiet = []
    for start in range(0, len(audio) - frame + 1, frame):
        block = audio[start:start + frame]
        quiet.append(sum(int(x) * int(x) for x in block) <= threshold_squared * frame)
    spans = []
    start = None
    for i, bit in enumerate(quiet + [False]):
        if bit and start is None:
            start = i
        if not bit and start is not None:
            if i - start >= 6:
                spans.append((start * frame, i * frame))
            start = None
    trailing = spans[-1][1] - spans[-1][0] if spans and spans[-1][1] >= (len(quiet) - 1) * frame else 0
    return {
        'quiet_spans_ms': [[round(a / 11.025), round((b-a) / 11.025)] for a,b in spans if b-a >= 330],
        'trailing_quiet_ms': round(trailing / 11.025),
    }


def configure(dll):
    H, I = C.c_void_p, C.c_int
    callback = C.WINFUNCTYPE(I, H, I, I, H)
    signatures = (
        ('eciNewEx', H, (I,)), ('eciDelete', H, (H,)),
        ('eciSetParam', I, (H,I,I)), ('eciGetParam', I, (H,I)),
        ('eciCopyVoice', I, (H,I,I)), ('eciSetVoiceParam', I, (H,I,I,I)),
        ('eciRegisterCallback', None, (H,callback,H)),
        ('eciSetOutputBuffer', I, (H,I,C.POINTER(C.c_short))),
        ('eciAddText', I, (H,C.c_char_p)), ('eciInsertIndex', I, (H,I)),
        ('eciSynthesize', I, (H,)), ('eciSynchronize', I, (H,)),
    )
    for name, result, args in signatures:
        f = getattr(dll, name); f.restype = result; f.argtypes = args
    return callback


def child(library, driver_root, case_number, implementation):
    case_name, language, texts = CASES[case_number]
    sys.path.insert(0, str(driver_root.resolve()))
    from synthDrivers import _eloquence_text as text_builder
    library = library.resolve()
    os.chdir(library.parent)
    os.environ['PATH'] = str(library.parent) + os.pathsep + os.environ.get('PATH', '')
    locale_id = {0x60000: 0x0804, 0x80000: 0x0411, 0xa0000: 0x0412}.get(language)
    if locale_id:
        C.windll.kernel32.SetThreadLocale(locale_id)
    directory = os.add_dll_directory(str(library.parent))
    dll = C.WinDLL(str(library))
    cbtype = configure(dll)

    def render(parts, speed):
        h = dll.eciNewEx(language)
        if not h:
            h = dll.eciNewEx(0x10000)
            if not h or dll.eciSetParam(h,9,language)<0 or dll.eciGetParam(h,9)!=language:
                raise RuntimeError('language initialization failed')
        buf = (C.c_short * 1100)()
        audio = array.array('h')
        indices = []
        @cbtype
        def callback(handle, msg, value, userdata):
            if msg == 0:
                if value < 0 or value > len(buf): return 2
                audio.extend(buf[:value])
            elif msg == 2:
                indices.append([value,len(audio)])
            return 1
        try:
            dll.eciRegisterCallback(h,callback,None)
            if dll.eciSetOutputBuffer(h,len(buf),buf)<=0: raise RuntimeError('output buffer refused')
            if dll.eciSetParam(h,1,1)<0 or dll.eciSetParam(h,5,1)<0: raise RuntimeError('ECI setup refused')
            if dll.eciCopyVoice(h,1,0)<0: raise RuntimeError('voice preset refused')
            if implementation == 'reloq' and dll.eciSetParam(h,0x101b,1)<0:
                raise RuntimeError('Eloquence_64 original 5 ms clock refused')
            if dll.eciSetVoiceParam(h,0,6,speed)<0: raise RuntimeError('speed refused')
            for part in parts:
                if not dll.eciAddText(h,part): raise RuntimeError('eciAddText refused')
            if not dll.eciInsertIndex(h,0xffff): raise RuntimeError('final index refused')
            if not dll.eciSynthesize(h) or not dll.eciSynchronize(h): raise RuntimeError('synthesis failed')
            if not audio: raise RuntimeError('no PCM')
            return {'samples':len(audio), 'audio_sha256':hashlib.sha256(audio.tobytes()).hexdigest(),
                    'indices':indices, **quiet_metrics(audio)}
        finally:
            dll.eciDelete(h)

    records = []
    try:
        for speed in (50,95,135):
            for sample_index, text in enumerate(texts):
                for mode in (0,1,2):
                    options = text_builder.BuildOptions(volume=80, rate=round((speed-40)*100/110), pause_mode=mode,
                        backquote_tags=False, abbreviation_dict=False, phrase_prediction=False)
                    text_bytes = text_builder.build(text,language,options)
                    tail = text_builder.trailing_pause(text_bytes,options)
                    parts = [text_bytes] + ([tail] if tail else [])
                    result = render(parts,speed)
                    records.append({'language':case_name,'kind':'driver','sample':sample_index,
                        'text':text,'mode':mode,'speed':speed,'input_hex':[p.hex() for p in parts],**result})
            if language == 0x60000:
                for before in ('你好','Hello'):
                    for pause in (0,1,50,250):
                        message = f'`pp0 `da0 `vv80 {before} `p{pause}, 世界。'.encode('gb18030')
                        result = render([message],speed)
                        records.append({'language':case_name,'kind':'explicit','text':before,
                            'pause':pause,'speed':speed,'input_hex':[message.hex()],**result})
        return records
    finally:
        directory.close()


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--library',type=Path,required=True)
    p.add_argument('--driver-root',type=Path,required=True)
    p.add_argument('--engine',choices=('original','reloq'),required=True)
    p.add_argument('--output',type=Path,required=True)
    p.add_argument('--child',type=int)
    args=p.parse_args()
    if args.child is not None:
        print('RESULT='+json.dumps(child(args.library,args.driver_root,args.child,args.engine),ensure_ascii=True),flush=True)
        return
    args.output.parent.mkdir(parents=True,exist_ok=True)
    out = {'engine':args.engine,'pointer_bits':C.sizeof(C.c_void_p)*8,
           'dll_sha256':hashlib.sha256(args.library.read_bytes()).hexdigest(), 'cases':[], 'errors':[]}
    for case_number, (name, _, _) in enumerate(CASES):
        cmd=[sys.executable,str(Path(__file__).resolve()),'--library',str(args.library.resolve()),
             '--driver-root',str(args.driver_root.resolve()),'--engine',args.engine,
             '--output',str(args.output.resolve()),'--child',str(case_number)]
        try:
            result=subprocess.run(cmd,capture_output=True,text=True,encoding='utf-8',errors='replace',timeout=90)
            rows=[x[len('RESULT='):] for x in result.stdout.splitlines() if x.startswith('RESULT=')]
            if result.returncode or len(rows)!=1:
                out['errors'].append({'language':name,'returncode':result.returncode,
                    'stdout':result.stdout[-600:],'stderr':result.stderr[-1000:]})
            else:
                values=json.loads(rows[0]);out['cases'].extend(values)
                print(f'{args.engine}: {name}: {len(values)} samples',flush=True)
        except subprocess.TimeoutExpired:
            out['errors'].append({'language':name,'error':'timeout'})
        args.output.write_text(json.dumps(out,indent=2,ensure_ascii=True)+'\n',encoding='utf8')
    print('MEASUREMENTS='+json.dumps(out,ensure_ascii=True),flush=True)
    if out['errors']: raise SystemExit(1)

if __name__=='__main__': main()
