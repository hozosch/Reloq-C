"""Controls cross NVDA symbol processing without hiding ordinary text."""
import importlib.util
from pathlib import Path
import sys
import types
import unittest


class SynthCommand: pass
class CharacterModeCommand:
    def __init__(self, state): self.state = state

sys.modules["speech"] = types.ModuleType("speech")
sys.modules["speech.commands"] = types.SimpleNamespace(
    SynthCommand=SynthCommand, CharacterModeCommand=CharacterModeCommand)
path = Path(__file__).parent / "synth/synthDrivers/reloqC/_speech_tags.py"
spec = importlib.util.spec_from_file_location("tag_transport", path)
tags = importlib.util.module_from_spec(spec)
spec.loader.exec_module(tags)


def protected(text):
    return [("control", item.text) if isinstance(item, tags.EciTagCommand) else item
            for item in tags.protect_text(text)]


class Tags(unittest.TestCase):
    def test_boundaries(self):
        self.assertEqual(protected("das ist ein `vv50 test"),
            ["das ist ein ", ("control", "`vv50"), " test"])
        self.assertEqual(protected("a mes`vv50sa`vb80ge!"),
            ["a ", ("control", "`vv50"), ("control", "`vb80"), "message!"])
        self.assertEqual(protected("ein`vv50 test"),
            ["ein", ("control", "`vv50"), " test"])

    def test_public_syntax(self):
        for tag in ("vb0", "vb100", "vs250", "vv50", "vf10", "vg1", "vh60",
                    "vr20", "vy0", "v1", "v9", "l4.0.1252", "l32767.65535.65535",
                    "ts2", "da1", "pp0", "p60000", "00", "9", "[.1tEst]"):
            with self.subTest(tag=tag):
                self.assertEqual(protected("`" + tag), [("control", "`" + tag)])
        for tag in ("vv101", "vs251", "vg2", "vh-1", "v0", "v22", "l32768",
                    "l4.65536", "p60001", "ts3", "da2", "pp2", "11", "000", "vp50",
                    "vv", "other", "[unclosed", "[bad[nesting]"):
            with self.subTest(tag=tag):
                self.assertEqual(protected("`" + tag), ["`" + tag])

    def test_language_and_character_commands(self):
        marker = object()
        on, off = CharacterModeCommand(True), CharacterModeCommand(False)
        source = [on, "A`vv50B", marker, off, "das `vv50 Wort"]
        result = tags.protect_sequence(source)
        self.assertEqual(result[:4], source[:4])
        self.assertTrue(any(isinstance(item, tags.EciTagCommand) for item in result[4:]))
        self.assertEqual(protected("这是`vv50一个测试"),
            ["这是", ("control", "`vv50"), "一个测试"])
        self.assertEqual(protected("ma\u0308`vv50use"),
            [("control", "`vv50"), "ma\u0308use"])


if __name__ == "__main__":
    unittest.main()
