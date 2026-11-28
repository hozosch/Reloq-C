"""Make temporary, reproducible PCM comparisons from the actual NVDA engines."""
import argparse
import ctypes
import hashlib
import importlib.util
import json
import sys
import wave
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "nvda"))
from _reloq_eci_direct import DirectEciEngine, NATIVE_WPM, PROCESS_TAGS, SAMPLE_RATE

WORDS = ("see", "ski", "key", "say", "stay", "day", "pay", "spay", "bay",
         "scout", "school", "describe", "transcribe", "transcript",
         "spin", "stop", "spill", "ask", "disgust")
RATES = (200, 320)


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def save_pcm(path, pcm):
    assert len(pcm) > 2000 and len(pcm) % 2 == 0, (path, len(pcm))
    with wave.open(str(path), "wb") as f:
        f.setnchannels(1)
        f.setsampwidth(2)
        f.setframerate(11025)
        f.writeframes(pcm)


def unique_file(root, name):
    hits = list(Path(root).rglob(name))
    if len(hits) != 1:
        raise RuntimeError(f"expected one {name} under {root}, got {hits}")
    return hits[0]


def dec_module(root):
    path = unique_file(root, "_dectalk.py")
    spec = importlib.util.spec_from_file_location("_dectalk_for_compare", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module, path


def register_phoneme_capture(engine, module):
    # This is the optional phoneme metadata in DECtalk's public in-memory
    # API. The NVDA wrapper normally requests no phoneme entries.
    class Phone(ctypes.Structure):
        _fields_ = [("phone", module.DWORD), ("sample", module.DWORD),
                    ("duration", module.DWORD), ("reserved", module.DWORD)]
    arrays = []
    for buf in engine._buffers:
        arr = (Phone * 1024)()
        arrays.append(arr)
        buf.lpPhonemeArray = ctypes.cast(arr, ctypes.c_void_p)
        buf.dwMaximumNumberOfPhonemeChanges = len(arr)
    events = []
    original = engine._consume

    def consume(buf):
        arr = ctypes.cast(buf.lpPhonemeArray, ctypes.POINTER(Phone))
        for i in range(buf.dwNumberOfPhonemeChanges):
            p = arr[i]
            events.append({"phone": int(p.phone), "sample": int(p.sample),
                           "duration": int(p.duration)})
        original(buf)
    engine._consume = consume
    return arrays, events


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--dectalk-dir", required=True, type=Path)
    p.add_argument("--candidate-dir", required=True, type=Path)
    p.add_argument("--reference-dir", required=True, type=Path)
    p.add_argument("--out", required=True, type=Path)
    args = p.parse_args()
    args.out.mkdir(parents=True, exist_ok=True)

    dt, dt_path = dec_module(args.dectalk_dir)
    candidate_dll = unique_file(args.candidate_dir, "eci.dll")
    reference_dll = unique_file(args.reference_dir, "eci.dll")
    dt_dll = Path(dt.default_lib_path())
    dt_dic = Path(dt.default_dic_path())
    manifest = {
        "reference": {"description": "masonasons/DECTalkNVDA v0.1.2, Perfect Paul",
                      "wrapper_sha256": sha(dt_path), "dll_sha256": sha(dt_dll),
                      "dictionary_sha256": sha(dt_dic)},
        "reloq_dev3_dll_sha256": sha(candidate_dll),
        "reloq_213_dll_sha256": sha(reference_dll),
        "rate_units": "nominal words per minute; both engines may realize rates differently",
        "dectalk_phoneme_times": "public API samples, at 11025 Hz; duration units are engine-defined",
        "words": WORDS, "rates": RATES, "samples": {}, "phonemes": {},
    }
    dec = dt.Engine()
    arrays, events = register_phoneme_capture(dec, dt)
    current = DirectEciEngine(candidate_dll, dialect=0x10000, encoding="utf8", native_voice=True)
    previous = DirectEciEngine(reference_dll, dialect=0x10000, encoding="utf8", native_voice=True)
    dictionary = DirectEciEngine(candidate_dll, dialect=0x10000, encoding="utf8", native_voice=True)
    dictionary.set_dictionary_path(candidate_dll.parent / "communityDictionaries")
    engines = (("dev3", current), ("2_1_3", previous), ("dev3_dictionary", dictionary))
    try:
        dec.set_speaker(0)
        for name, eng in engines:
            eng.set_parameter(SAMPLE_RATE, 1)
            eng.set_parameter(PROCESS_TAGS, 1)
        for rate in RATES:
            dec.set_rate(rate)
            for name, eng in engines:
                eng.set_parameter(NATIVE_WPM, rate)
            for word in WORDS:
                events.clear()
                data = bytearray()
                dec.speak(f"[:np][:rate {rate}] {word}.", lambda pcm, marks: data.extend(pcm))
                key = f"dectalk_{rate}_{word}"
                save_pcm(args.out / (key + ".wav"), bytes(data))
                manifest["samples"][key] = len(data) // 2
                manifest["phonemes"][key] = list(events)
                for name, eng in engines:
                    if name == "dev3_dictionary" and word not in ("transcript", "transcribe", "scout"):
                        continue
                    data = bytearray()
                    if not eng.speak([word + "."], data.extend, lambda index: None):
                        raise RuntimeError(f"synthesis cancelled: {name} {rate} {word}")
                    key = f"{name}_{rate}_{word}"
                    save_pcm(args.out / (key + ".wav"), bytes(data))
                    manifest["samples"][key] = len(data) // 2
        # Keep the arrays strongly referenced throughout synthesis.
        assert len(arrays) == len(dec._buffers)
    finally:
        dictionary.close()
        previous.close()
        current.close()
        dec.close()
    (args.out / "manifest.json").write_text(json.dumps(manifest, indent=2), encoding="utf8")
    print(f"rendered {len(manifest['samples'])} utterances")
    print(f"DECtalk phoneme events: {sum(map(len, manifest['phonemes'].values()))}")
    print(f"DECtalk library SHA256: {manifest['reference']['dll_sha256']}")
    print(f"ReLoq dev3 library SHA256: {manifest['reloq_dev3_dll_sha256']}")


if __name__ == "__main__":
    main()
