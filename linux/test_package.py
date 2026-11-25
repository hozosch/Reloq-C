#!/usr/bin/env python3
"""Build the Linux archive, then install, use and remove it in a scratch home.

HOME and the XDG directories point into a temporary directory, so neither the
user's files nor a running Speech Dispatcher are touched. The installed module
is started with its installed configuration and the community dictionaries
switched on, and must load them without a warning.
"""

import argparse
import os
from pathlib import Path
import subprocess
import sys
import tarfile
import tempfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "linux"))
sys.path.insert(0, str(ROOT / "speechd"))
import package  # noqa: E402
from test_speechd import start  # noqa: E402


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--library", type=Path, default=ROOT / "eci/libreloq_eci.so")
    parser.add_argument("--module", type=Path, default=ROOT / "eci/sd_reloq")
    parser.add_argument("--archive", type=Path, help="test this archive instead of building one")
    args = parser.parse_args()

    with tempfile.TemporaryDirectory(prefix="reloq-linux-package-") as scratch:
        scratch = Path(scratch)
        version = package.addon_version()
        archive = args.archive or package.build(scratch / package.output_name(version),
                                                args.library, args.module, version)
        if not args.archive:
            again = package.build(scratch / "again.tar.gz", args.library, args.module, version)
            assert again.read_bytes() == archive.read_bytes(), "the archive is not reproducible"
        top = package.output_name(version).removesuffix(".tar.gz")
        with tarfile.open(archive) as tar:
            names = set(tar.getnames())
            for name in ("install.sh", "sd_reloq", "libreloq_eci.so", "reloq.conf", "reloq-speechd-enable",
                         "README.md", "include/reloq_eci.h", "dictionaries/ENUmain.dic",
                         "dictionaries/DEURoot.dic", "licenses/OpenEVV-LICENSE.txt"):
                assert f"{top}/{name}" in names, name
            assert all(name == top or name.startswith(top + "/") for name in names)
            assert all(member.uid == 0 and not member.issym() and not member.islnk()
                       for member in tar.getmembers())
            assert tar.getmember(f"{top}/install.sh").mode == 0o755
            tar.extractall(scratch / "unpacked", filter="data")
        unpacked = scratch / "unpacked" / top

        home = scratch / "home"
        home.mkdir()
        config_root = home / ".config/speech-dispatcher"
        config_root.mkdir(parents=True)
        # A configuration that relies on discovery, as most systems ship.
        (config_root / "speechd.conf").write_text('LogLevel 3\nDefaultModule espeak-ng\n')
        environment = {key: value for key, value in os.environ.items() if not key.startswith("XDG_")}
        environment["HOME"] = str(home)

        print("linux package: user installation", flush=True)
        result = subprocess.run([str(unpacked / "install.sh"), "--user"], env=environment,
                                capture_output=True, text=True, timeout=120)
        assert result.returncode == 0, result.stdout + result.stderr
        module = home / ".local/libexec/speech-dispatcher-modules/sd_reloq"
        module_config = config_root / "modules/reloq.conf"
        dictionaries = home / ".local/share/reloq-c/dictionaries"
        for path in (module, module_config, home / ".local/lib/libreloq_eci.so",
                     home / ".local/bin/reloq-speechd-enable", dictionaries / "ENUmain.dic",
                     home / ".local/include/reloq-c/reloq_eci.h"):
            assert path.is_file(), path
        assert os.access(module, os.X_OK)
        text = module_config.read_text()
        assert f"#DictionaryDirectory {dictionaries}\n" in text, text
        speechd_conf = (config_root / "speechd.conf").read_text()
        assert f'AddModule "reloq" "{module}" "{module_config}"' in speechd_conf, speechd_conf
        assert "DefaultModule espeak-ng" in speechd_conf

        print("linux package: installed module with dictionaries", flush=True)
        module_config.write_text(text.replace("#DictionaryDirectory", "DictionaryDirectory"))
        process = start(str(module), str(module_config))
        try:
            audio, _, _, event = process.speak("The installed voice speaks.")
            assert event == "702 END" and audio
        finally:
            assert process.close() == 0
        errors = process.process.stderr.read().decode("utf-8", "replace")
        assert "could not be loaded" not in errors and "ignoring" not in errors, errors

        print("linux package: a second installation keeps the edited configuration", flush=True)
        edited = module_config.read_text()
        result = subprocess.run([str(unpacked / "install.sh"), "--user"], env=environment,
                                capture_output=True, text=True, timeout=120)
        assert result.returncode == 0 and "Kept your existing" in result.stdout, result.stdout + result.stderr
        assert module_config.read_text() == edited
        assert (config_root / "speechd.conf").read_text().count('AddModule "reloq"') == 1

        print("linux package: removal", flush=True)
        result = subprocess.run([str(unpacked / "install.sh"), "--user", "--uninstall"], env=environment,
                                capture_output=True, text=True, timeout=120)
        assert result.returncode == 0, result.stdout + result.stderr
        assert not module.exists() and not (home / ".local/lib/libreloq_eci.so").exists()
        assert not (home / ".local/share/reloq-c").exists()
        assert module_config.exists(), "the user's reloq.conf is kept"
        assert (config_root / "speechd.conf").read_text() == 'LogLevel 3\nDefaultModule espeak-ng\n'
    print("linux package: all checks passed")


if __name__ == "__main__":
    main()
