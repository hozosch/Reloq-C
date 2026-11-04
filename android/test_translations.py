"""Validate real Android resource coverage and the public speech catalogue."""
import argparse
from collections import Counter
from pathlib import Path
import re
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]
RES = ROOT / "android/res"
FORMAT = re.compile(r"%(?:\d+\$)?[ds]")


def resources(path):
    nodes = list(ET.parse(path).getroot())
    result = {x.attrib["name"]: x for x in nodes}
    assert len(result) == len(nodes), (path, "Duplicate resource")
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--library", type=Path)
    args = parser.parse_args()
    base = resources(RES / "values/strings.xml")
    expected = set(base) - {"app_name"}
    paths = sorted(RES.glob("values-*/strings.xml"))
    locales = {p.parent.name[7:] for p in paths}
    required = {"de", "pt-rBR", "es", "fr", "it", "fi", "el", "zh-rCN", "zh-rTW", "ja", "ko"}
    assert required <= locales, ("Missing UI locales", required - locales)
    checks = 0
    for path in paths:
        translated = resources(path)
        assert expected <= set(translated) <= set(base), (path, "Missing/extra resource")
        for key, node in translated.items():
            reference = base[key]
            assert node.tag == reference.tag, (path, key, "Wrong resource type")
            if node.tag == "string-array":
                assert len(node) == len(reference), (path, key, "Array changes control IDs")
                pairs = zip(reference, node)
            else:
                pairs = ((reference, node),)
            for before, after in pairs:
                assert after.text and after.text.strip(), (path, key, "Empty translation")
                assert Counter(FORMAT.findall(before.text or "")) == Counter(FORMAT.findall(after.text)), (path, key, "Format arguments changed")
                checks += 1
    catalog = (ROOT / "android/java/org/reloq/tts/Catalog.java").read_text()
    identifiers = [int(value, 16) for value in re.findall(r"new Language\((0x[0-9a-f]+),", catalog)]
    assert len(identifiers) == len(set(identifiers)) == 15
    # Existing persisted indices must retain their language after an update.
    assert identifiers[:4] == [0x60000, 0x40000, 0x10001, 0x10000]
    assert identifiers[-1] == 0x60001
    if args.library:
        import ctypes as C
        lib = (C.WinDLL if hasattr(C, "WinDLL") else C.CDLL)(str(args.library.resolve()))
        enum = lib.eciGetAvailableLanguages
        enum.argtypes = (C.POINTER(C.c_uint), C.POINTER(C.c_int))
        enum.restype = C.c_int
        values, count = (C.c_uint * 64)(), C.c_int(64)
        assert enum(values, C.byref(count)) == 0
        assert set(values[:count.value]) == set(identifiers), "Android/public ECI languages differ"
    print(f"Android UI: {len(paths) + 1} locales, {checks} text/format/array checks; all 15 language variants and stored selection order covered")


if __name__ == "__main__": main()
