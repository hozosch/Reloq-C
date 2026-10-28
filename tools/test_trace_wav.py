"""End-to-end smoke checks for the numeric trace reader and WAV writer."""
import pathlib
import struct
import subprocess
import sys
import tempfile
import wave


def run(exe, trace, output, expected_ok):
    result = subprocess.run([str(exe), str(trace), str(output)],
                            capture_output=True, text=True)
    assert (result.returncode == 0) == expected_ok, result.stderr
    return result


def main():
    exe = pathlib.Path(sys.argv[1]).resolve()
    original = pathlib.Path(sys.argv[2]).read_text(encoding="ascii")
    with tempfile.TemporaryDirectory() as folder:
        folder = pathlib.Path(folder)
        trace = folder / "input.trace"
        output = folder / "result.wav"
        trace.write_text(original, encoding="ascii")
        run(exe, trace, output, True)
        content = output.read_bytes()
        assert content[:4] == b"RIFF" and content[8:12] == b"WAVE"
        assert struct.unpack_from("<I", content, 4)[0] == len(content) - 8
        assert struct.unpack_from("<I", content, 40)[0] == len(content) - 44
        with wave.open(str(output), "rb") as wav:
            assert (wav.getnchannels(), wav.getsampwidth(), wav.getframerate()) == (1, 2, 16000)
            assert wav.getnframes() == 120  # 3 * (2.5 ms * 16 samples/ms)
            samples = struct.unpack("<120h", wav.readframes(120))
            assert any(s != 0 for s in samples)
            assert len(set(samples)) > 10

        # The input may be longer than a customary fixed fgets buffer.
        verbose = original.replace("# Controlled numeric example:",
                                   "#" + " long comment" * 3000 + "\n# Controlled numeric example:")
        trace.write_text(verbose, encoding="ascii")
        run(exe, trace, output, True)
        assert output.read_bytes() == content

        # The unused trailing source slots have no mapped effect.
        reserved = original.replace("0 0 0 0 0 0 0 0\n", "0 0 0 0 0 0 1 1\n")
        trace.write_text(reserved, encoding="ascii")
        result = run(exe, trace, output, True)
        assert "reserved field values ignored" in result.stderr
        assert output.read_bytes() == content

        # Both independent source trim and output scaling accept natural
        # header values and alter the signal through the full WAV path.
        gained = original.replace("av_trim 0", "av_trim -5").replace(
            "output_gain 1", "output_gain 2.01999998")
        trace.write_text(gained, encoding="ascii")
        run(exe, trace, output, True)
        assert len(output.read_bytes()) == len(content)
        assert output.read_bytes() != content

        # The reference can emit DI=100; only values above that bound fail.
        di100 = original.replace("2.5 1200 50 50 7 0 0 18",
                                 "2.5 1200 50 50 7 0 100 18")
        trace.write_text(di100, encoding="ascii")
        run(exe, trace, output, True)
        assert len(output.read_bytes()) == len(content)

        # Exercise a frame that crosses the core's 200-sample work blocks.
        long_frames = original.replace("2.5 ", "25 ")
        trace.write_text(long_frames, encoding="ascii")
        run(exe, trace, output, True)
        with wave.open(str(output), "rb") as wav:
            assert wav.getnframes() == 1200

        # Malformed input leaves an existing WAV untouched while the output
        # is still in the temporary stream.
        for bad in (original.replace("duration_scale 1", "duration_scale nan"),
                    original.replace("rate 16000", "rate 12345"),
                    original.replace("2.5 1200 50 50 7 0 0 18",
                                     "2.5 1200 50 50 7 0 100.01 18"),
                    original.replace("2.5 1350 0 45", "2.5 1350 0"),
                    original.replace("2.5 1200 50 50", "200000000 1200 50 50")):
            output.write_bytes(b"original")
            trace.write_text(bad, encoding="ascii")
            run(exe, trace, output, False)
            assert output.read_bytes() == b"original"
    print("trace CLI: WAV header, samples, long comments, and malformed input OK")


if __name__ == "__main__":
    main()
