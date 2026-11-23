"""Host checks for automatic dialect routing and number processing."""
from pathlib import Path
import argparse
import os
import subprocess
import tempfile

from codegen import generate

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--java-home", type=Path, default=os.environ.get("JAVA_HOME"), required=not os.environ.get("JAVA_HOME"))
    args = parser.parse_args()
    javac = args.java_home / "bin" / ("javac.exe" if os.name == "nt" else "javac")
    java = args.java_home / "bin" / ("java.exe" if os.name == "nt" else "java")
    with tempfile.TemporaryDirectory() as directory:
        folder = Path(directory)
        generate(folder / "generated")
        source = ROOT / "android/java/org/reloq/tts"
        test = ROOT / "android/test"
        files = [source / (name + ".java") for name in
                 ("Catalog", "VoiceSettings", "AutoLanguage", "NumberReading", "VolumeGain")]
        files += [test / (name + ".java") for name in
                  ("AutoLanguageTest", "NumberReadingTest", "VolumeGainTest")]
        files += list((folder / "generated").rglob("RateCurve.java"))
        subprocess.run([str(javac), "-encoding", "UTF-8", "-d", str(folder), *map(str, files)], check=True)
        for name in ("AutoLanguageTest", "NumberReadingTest", "VolumeGainTest"):
            subprocess.run([str(java), "-cp", str(folder), "org.reloq.tts." + name], check=True, timeout=30)


if __name__ == "__main__":
    main()
