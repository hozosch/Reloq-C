#!/usr/bin/env python3
"""Build standard NVDA gettext catalogs and translated manifest fields.

GNU gettext's msgfmt is a build dependency, not a runtime dependency.
The add-on itself uses NVDA's addonHandler.initTranslation().
"""
from __future__ import annotations

import argparse
import ast
import configparser
import gettext
import io
import json
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "addon" / "synth"


def N_(message: str) -> str:
    return message


# Mark manifest messages for xgettext as well as validating the English base.
MANIFEST_MESSAGES = {
    # Translators: Add-on name shown in NVDA's add-on manager.
    "summary": N_("Eloquence C: fifteen language variants"),
    # Translators: Add-on description; this refers to the spoken language.
    "description": N_("Native C synthesizer with fifteen language variants at 8 to 48 kHz, including Japanese, Korean, Chinese and Greek."),
}


def manifest_values() -> dict[str, str]:
    parser = configparser.ConfigParser(interpolation=None)
    parser.read_string("[addon]\n" + (SOURCE / "manifest.ini").read_text(encoding="utf-8"))
    values = {key: ast.literal_eval(value) for key, value in parser["addon"].items()}
    for key, message in MANIFEST_MESSAGES.items():
        if values[key] != message:
            raise ValueError(f"English manifest and translation marker differ: {key}")
    return values


def translation_entries() -> dict[str, bytes]:
    """Compile each PO; missing translations retain the English msgid."""
    manifest_values()
    entries = {}
    for source in sorted((SOURCE / "locale").glob("*/LC_MESSAGES/nvda.po")):
        with tempfile.TemporaryDirectory() as temporary:
            output = Path(temporary) / "nvda.mo"
            subprocess.run(["msgfmt", "--check", "--check-format", "--output-file",
                            str(output), str(source)], check=True)
            compiled = output.read_bytes()
        translations = gettext.GNUTranslations(io.BytesIO(compiled))
        locale = source.parent.parent.name
        entries[f"locale/{locale}/LC_MESSAGES/nvda.mo"] = compiled
        # NVDA localizes these manifest fields via locale/<lang>/manifest.ini.
        # Generate them from the SAME PO, avoiding a second translation source.
        text = "".join(f"{key} = {json.dumps(translations.gettext(message), ensure_ascii=False)}\n"
                       for key, message in MANIFEST_MESSAGES.items())
        entries[f"locale/{locale}/manifest.ini"] = text.encode("utf-8")
    return entries


def update_template() -> Path:
    values = manifest_values()
    output = SOURCE / "locale" / "reloqC.pot"
    output.parent.mkdir(parents=True, exist_ok=True)
    subprocess.run([
        "xgettext", "--language=Python", "--from-code=UTF-8",
        "--keyword=_", "--keyword=N_", "--add-comments=Translators:",
        "--package-name=Eloquence C", "--package-version=" + values["version"],
        "--output=" + str(output),
        "addon/synth/synthDrivers/reloqC/__init__.py",
        "nvda/_reloq_languages.py", "addon/build_translations.py",
        "addon/synth/globalPlugins/reloqCDictionaries/__init__.py",
    ], cwd=ROOT, check=True)
    return output


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--update-template", action="store_true")
    args = parser.parse_args()
    if args.update_template:
        print(update_template())
    else:
        entries = translation_entries()
        print(len(entries) // 2, "gettext catalogs compiled and validated")


if __name__ == "__main__":
    main()
