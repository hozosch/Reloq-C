"""Observe r-to-next-word energy and ECI index timing without saving audio."""
import argparse
import array
import ctypes as C
import hashlib
import json
import math
import os
from pathlib import Path


def run(path):
    path = path.resolve()
    directory = os.add_dll_directory(str(path.parent)) if os.name == 'nt' else None
    previous = Path.cwd()
    os.chdir(path.parent)
    try:
        dll = (C.WinDLL if os.name == 'nt' else C.CDLL)(str(path))
        H, I = C.c_void_p, C.c_int
        CB = (C.WINFUNCTYPE if os.name == 'nt' else C.CFUNCTYPE)(I, H, I, I, H)
        for name, result, args in (
            ('eciNewEx', H, (I,)), ('eciDelete', H, (H,)),
            ('eciSetParam', I, (H, I, I)), ('eciSetVoiceParam', I, (H, I, I, I)),
            ('eciAddText', I, (H, C.c_char_p)), ('eciInsertIndex', I, (H, I)),
            ('eciSynthesize', I, (H,)), ('eciSynchronize', I, (H,)),
            ('eciRegisterCallback', None, (H, CB, H)),
            ('eciSetOutputBuffer', I, (H, I, C.POINTER(C.c_short))),
        ):
            f = getattr(dll, name); f.restype, f.argtypes = result, args
        handle = dll.eciNewEx(0x40000)
        assert handle, 'German reference unavailable'
        pcm = (C.c_short * 11)()
        data, indices = array.array('h'), []
        @CB
        def callback(_h, kind, size, _ctx):
            if kind == 0:
                data.extend(pcm[:size])
            elif kind == 2:
                indices.append((size, len(data)))
            return 1
        dll.eciRegisterCallback(handle, callback, None)
        assert dll.eciSetOutputBuffer(handle, len(pcm), pcm)
        assert dll.eciSetParam(handle, 1, 1) >= 0
        assert dll.eciSetParam(handle, 5, 1) >= 0
        for parameter, value in enumerate((0, 50, 65, 30, 0, 0, 75, 100)):
            assert dll.eciSetVoiceParam(handle, 0, parameter, value) >= 0

        def capture(left, right, marked):
            del data[:]; indices.clear()
            assert dll.eciAddText(handle, left.encode('cp1252'))
            if marked: assert dll.eciInsertIndex(handle, 71)
            assert dll.eciAddText(handle, right.encode('cp1252'))
            assert dll.eciSynthesize(handle)
            assert dll.eciSynchronize(handle)
            assert data
            return data.tobytes(), list(indices)

        result = {}
        for left, right in (('Der ', 'Zahn.'), ('Der ', 'Mann.'), ('Der ', 'Zug.'),
                            ('Vor ', 'Zahn.'), ('Mutter ', 'Zahn.'), ('Der, ', 'Zahn.')):
            plain, _ = capture(left, right, False)
            marked, marks = capture(left, right, True)
            assert len(marks) == 1 and marks[0][0] == 71, (left, right, marks)
            samples = array.array('h'); samples.frombytes(marked)
            anchor = marks[0][1]
            natural = array.array('h'); natural.frombytes(plain)
            natural_windows = []
            # Observe the unindexed sentence from its beginning: some
            # original ECI implementations place a public index early and
            # its insertion can change prosody. Values are analysis only.
            for ms in range(0, 351, 5):
                begin = round(ms * 11025 / 1000)
                sample = natural[begin:begin+220]
                if len(sample) != 220:
                    natural_windows.append(None); continue
                rms = math.sqrt(sum(v*v for v in sample) / len(sample))
                bands=[]
                for hz in range(400, 3401, 200):
                    real = sum(v*math.cos(2*math.pi*hz*i/11025) for i,v in enumerate(sample))
                    imag = sum(v*math.sin(2*math.pi*hz*i/11025) for i,v in enumerate(sample))
                    bands.append(round(math.hypot(real,imag)/len(sample),2))
                natural_windows.append([round(rms,2), bands])
            windows = []
            for offset in range(-150, 151, 5):
                begin = anchor + round(offset * 11025 / 1000)
                end = min(len(samples), begin + 55)
                if begin < 1 or end <= begin:
                    windows.append(None); continue
                window = samples[begin:end]
                rms = math.sqrt(sum(x*x for x in window) / len(window))
                high = math.sqrt(sum((samples[i] - samples[i-1])**2
                                     for i in range(begin, end)) / len(window))
                windows.append([round(rms, 2), round(high, 2)])
            result[left + right] = dict(samples=len(samples), mark=anchor,
                indexed_identical=plain == marked, rms_and_difference=windows,
                unindexed_windows=natural_windows)
        return dict(dll_sha256=hashlib.sha256(path.read_bytes()).hexdigest(),
                    sample_rate=11025, windows_start_ms=-150, windows_step_ms=5,
                    voices=[0,50,65,30,0,0,75,100], examples=result)
    finally:
        if 'handle' in locals() and handle: dll.eciDelete(handle)
        os.chdir(previous)
        if directory: directory.close()


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    args.output.resolve().write_text(json.dumps(run(args.library), indent=2), encoding='utf-8')
    print('German r-to-next-word observations recorded; PCM remained in RAM.')
