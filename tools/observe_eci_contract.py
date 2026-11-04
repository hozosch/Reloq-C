"""Observe the public ABI of a supplied ECI DLL; retain no speech files.

The same probe runs in x86 Python for Eloquence_64's original host engine
and x64 Python for ReLoq. JSON records observations, not guessed contracts.
"""
import argparse
import ctypes as C
import hashlib
import json
import os
from pathlib import Path
import tempfile


def observe(path):
    path = path.resolve()
    if os.name == "nt":
        directory = os.add_dll_directory(str(path.parent))
    else:
        directory = None
    previous = Path.cwd()
    os.chdir(path.parent)
    try:
        lib = (C.WinDLL if os.name == "nt" else C.CDLL)(str(path))
        H, I = C.c_void_p, C.c_int
        CB = (C.WINFUNCTYPE if os.name == "nt" else C.CFUNCTYPE)(I, H, I, I, H)
        signatures = (
            ("eciVersion", None, (C.c_char_p,)),
            ("eciGetAvailableLanguages", I, (C.POINTER(I), C.POINTER(I))),
            ("eciNewEx", H, (I,)), ("eciDelete", H, (H,)),
            ("eciGetParam", I, (H, I)), ("eciSetParam", I, (H, I, I)),
            ("eciGetVoiceParam", I, (H, I, I)),
            ("eciSetVoiceParam", I, (H, I, I, I)), ("eciCopyVoice", I, (H, I, I)),
            ("eciRegisterCallback", None, (H, CB, H)),
            ("eciSetOutputBuffer", I, (H, I, C.POINTER(C.c_short))),
            ("eciAddText", I, (H, C.c_char_p)), ("eciSynthesize", I, (H,)),
            ("eciSynchronize", I, (H,)), ("eciClearInput", I, (H,)),
            ("eciNewDict", H, (H,)), ("eciGetDict", H, (H,)),
            ("eciSetDict", I, (H, H)), ("eciDeleteDict", I, (H, H)),
            ("eciLoadDict", I, (H, H, I, C.c_char_p)),
        )
        for name, result, arguments in signatures:
            function = getattr(lib, name)
            function.restype, function.argtypes = result, arguments
        version = C.create_string_buffer(128)
        lib.eciVersion(version)
        languages, count = (I * 64)(), I(64)
        enum_result = lib.eciGetAvailableLanguages(languages, C.byref(count))
        assert 0 <= count.value <= 64
        result = dict(version=version.value.decode("ascii", "replace"),
                      pointer_bits=C.sizeof(H) * 8,
                      dll_sha256=hashlib.sha256(path.read_bytes()).hexdigest(),
                      enumeration_result=enum_result,
                      languages=list(languages[:count.value]))
        handle = lib.eciNewEx(0x10000)
        assert handle, "US engine creation failed"
        try:
            result["initial_parameters"] = {str(i): lib.eciGetParam(handle, i)
                                             for i in (*range(18), 41, 42)}
            result["voice_slots"] = [[lib.eciGetVoiceParam(handle, voice, p)
                                       for p in range(8)] for voice in range(9)]
            result["voice_setters"] = []
            for parameter, value in enumerate((1, 40, 75, 47, 12, 30, 156, 60)):
                before = lib.eciGetVoiceParam(handle, 0, parameter)
                returned = lib.eciSetVoiceParam(handle, 0, parameter, value)
                after = lib.eciGetVoiceParam(handle, 0, parameter)
                result["voice_setters"].append([parameter, before, returned, after])
                lib.eciSetVoiceParam(handle, 0, parameter, before)
            lib.eciSetVoiceParam(handle, 0, 6, 156)
            result["copy_preset"] = dict(returned=lib.eciCopyVoice(handle, 1, 0),
                values=[lib.eciGetVoiceParam(handle, 0, p) for p in range(8)])
            result["parameter_aliases"] = {}
            for parameter in (3, 11, 41, 42):
                before = lib.eciGetParam(handle, parameter)
                returned = lib.eciSetParam(handle, parameter, 1)
                after = lib.eciGetParam(handle, parameter)
                result["parameter_aliases"][str(parameter)] = [before, returned, after]
                if before >= 0: lib.eciSetParam(handle, parameter, before)
            pcm = (C.c_short * 1100)()
            blocks = []

            @CB
            def receive(h, message, size, data):
                if message == 0:
                    assert 0 <= size <= len(pcm)
                    blocks.append(C.string_at(pcm, size * 2))
                return 1

            lib.eciRegisterCallback(handle, receive, None)
            assert lib.eciSetOutputBuffer(handle, len(pcm), pcm)
            lib.eciSetParam(handle, 1, 1)
            lib.eciSetVoiceParam(handle, 0, 6, 75)

            def speak(text):
                blocks.clear()
                assert lib.eciAddText(handle, text)
                assert lib.eciSynthesize(handle)
                assert lib.eciSynchronize(handle)
                data = b"".join(blocks)
                assert data
                return dict(bytes=len(data), sha256=hashlib.sha256(data).hexdigest())

            plain = speak(b"The contracttoken is ready.")
            with tempfile.TemporaryDirectory(prefix="reloq-eci-contract-") as temp:
                source = Path(temp) / "main.dic"
                source.write_bytes(b"contracttoken\tbanana\r\n")
                dictionary = lib.eciNewDict(handle)
                assert dictionary
                loaded = lib.eciLoadDict(handle, dictionary, 0, os.fsencode(source))
                selected = lib.eciSetDict(handle, dictionary)
                active = lib.eciGetDict(handle) == dictionary
                with_dictionary = speak(b"The contracttoken is ready.")
                source.unlink()
                retained = speak(b"The contracttoken is ready.")
                result["dictionary"] = dict(load_result=loaded, set_result=selected,
                    active=active, changes_pcm=plain != with_dictionary,
                    retains_source=retained == with_dictionary)
                lib.eciSetDict(handle, None)
                result["dictionary"]["delete_result"] = lib.eciDeleteDict(handle, dictionary)
            result["language_switches"] = []
            for language, text in ((0x40000, b"Dies ist ein Test."),
                                   (0x10000, b"This is a test.")):
                before = lib.eciGetParam(handle, 9)
                returned = lib.eciSetParam(handle, 9, language)
                after = lib.eciGetParam(handle, 9)
                audio = speak(text) if after == language else None
                result["language_switches"].append(dict(before=before, returned=returned,
                    after=after, produces_pcm=bool(audio)))
        finally:
            lib.eciDelete(handle)
        return result
    finally:
        os.chdir(previous)
        if directory is not None: directory.close()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--library", type=Path, required=True)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    report = observe(args.library)
    if args.output: args.output.write_text(json.dumps(report, indent=2) + "\n")
    print("ECI_CONTRACT_OBSERVATIONS=" + json.dumps(report, separators=(",", ":")))
