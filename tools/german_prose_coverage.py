#!/usr/bin/env python3
"""Measure German prose coverage at distinct C text and audio boundaries.

The fixed fixture includes user-supplied natural utterances, general prose,
and separate tiny calibration forms. An optional public Apple ECI comparison
captures fresh audio only for native successes. It does not supply any runtime
data to the independent C renderer. A successful pronunciation annotation is
never counted as successfully synthesized PCM.
"""

from __future__ import annotations

import argparse
import ast
from collections import Counter
from dataclasses import dataclass
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "nvda"))
from _reloq_eci_direct import DirectEciEngine, EciSynthesisError  # noqa: E402

WORD = re.compile(r"[A-Za-zÄÖÜäöüß]+")
SOURCES = {"user_capture", "general_prose_fixture", "calibration"}


@dataclass(frozen=True)
class FrontendResult:
    code: int
    bad_utf8_offset: int
    annotation_bytes: int

    @property
    def label(self) -> str:
        return {0: "known", -1: "invalid", -2: "uncovered"}.get(
            self.code, f"error_{self.code}")


def fixture_rows(path: Path) -> list[dict[str, str]]:
    rows: list[dict[str, str]] = []
    ids: set[str] = set()
    with path.open(encoding="utf-8") as stream:
        for lineno, line in enumerate(stream, 1):
            obj = json.loads(line)
            if not isinstance(obj, dict) or set(obj) != {"id", "source", "text"}:
                raise ValueError(f"{path}:{lineno}: expected id/source/text")
            if not all(isinstance(obj[key], str) for key in obj):
                raise ValueError(f"{path}:{lineno}: fields must be strings")
            if obj["source"] not in SOURCES or not obj["id"] or obj["id"] in ids:
                raise ValueError(f"{path}:{lineno}: invalid source or repeated ID")
            if not obj["text"] or "\n" in obj["text"] or "\r" in obj["text"] or "\0" in obj["text"]:
                raise ValueError(f"{path}:{lineno}: unsuitable line-based text")
            obj["text"].encode("cp1252")
            ids.add(obj["id"])
            rows.append(obj)
    if not rows or not any(row["source"] == "user_capture" for row in rows):
        raise ValueError("a coverage suite requires user-provided utterances")
    return rows


def build_text_probe(directory: Path) -> Path:
    binary = directory / "reloq_text_coverage"
    sources = sorted((ROOT / "src").glob("*.c"))
    sources = [source for source in sources if source.name != "reloq_host.c"]
    subprocess.run(["cc", "-std=c11", "-O2", "-Wall", "-Wextra",
                    "-Werror", "-pedantic", "-ffp-contract=off",
                    "-I" + str(ROOT / "include"),
                    str(ROOT / "tools" / "reloq_text_coverage.c"),
                    *(str(source) for source in sources), "-lm", "-o",
                    str(binary)], check=True, timeout=120)
    return binary


def front_end(probe: Path, inputs: list[str], lexicon: Path | None) -> list[FrontendResult]:
    argv = [str(probe)] + ([str(lexicon)] if lexicon else [])
    result = subprocess.run(argv, input=("\n".join(inputs) + "\n").encode(),
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                            check=True, timeout=30)
    lines = result.stdout.decode("ascii").splitlines()
    if len(lines) != len(inputs):
        raise RuntimeError(f"C frontend gave {len(lines)} lines for {len(inputs)} inputs")
    parsed = []
    for line in lines:
        status, offset, needed = map(int, line.split("\t"))
        if status not in (0, -1, -2) or offset < 0 or needed < 0:
            raise RuntimeError(f"unexpected C frontend status: {line!r}")
        parsed.append(FrontendResult(status, offset, needed))
    return parsed


def render_native(engine: DirectEciEngine, phrase: str) -> tuple[dict, bytes]:
    blocks: list[bytes] = []
    try:
        if not engine.speak((phrase,), blocks.append, lambda _: None):
            raise RuntimeError("unrequested cancellation")
    except EciSynthesisError as error:
        if blocks:
            raise RuntimeError("native ECI emitted partial audio on rejection") from error
        if error.status == -2 and error.stage == 2:
            label = "text_uncovered"
        elif error.status == -2 and error.stage == 3:
            label = "acoustic_unsupported"
        else:
            label = "other_error"
        return {"status": label, "native_status": error.status,
                "stage": error.stage, "utf8_offset": error.utf8_offset}, b""
    payload = b"".join(blocks)
    if not payload or len(payload) & 1:
        raise RuntimeError(f"native ECI returned malformed/empty PCM for {phrase!r}")
    return {"status": "pcm_produced", "samples": len(payload) // 2}, payload


def apple_pcm(probe: Path, eci: Path, workdir: Path, runtime: Path | None,
              phrase: str, output: Path) -> bytes:
    environment = os.environ.copy()
    if runtime:
        environment["LD_LIBRARY_PATH"] = os.pathsep.join(
            filter(None, (str(runtime), environment.get("LD_LIBRARY_PATH", ""))))
    run = subprocess.run((os.fsencode(probe), os.fsencode(eci),
                          phrase.encode("cp1252"), b"0", os.fsencode(output)),
                         cwd=workdir, env=environment,
                         capture_output=True, timeout=20, check=False)
    if run.returncode:
        raise RuntimeError(run.stderr.decode(errors="replace").strip()
                           or f"Apple probe exit {run.returncode}")
    payload = output.read_bytes()
    if not payload or len(payload) & 1:
        raise RuntimeError("Apple ECI returned malformed/empty PCM")
    return payload


def apple_isolated_class(probe: Path, eci: Path, workdir: Path,
                         runtime: Path | None, word: str) -> str:
    """Describe isolated reference SPR structure, never sentence realization."""
    environment = os.environ.copy()
    if runtime:
        environment["LD_LIBRARY_PATH"] = os.pathsep.join(
            filter(None, (str(runtime), environment.get("LD_LIBRARY_PATH", ""))))
    run = subprocess.run((os.fsencode(probe), os.fsencode(eci),
                          word.encode("cp1252"), b"0"), cwd=workdir,
                         env=environment, capture_output=True, timeout=20)
    if run.returncode:
        return "probe_error"
    try:
        response = ast.literal_eval(run.stdout.decode("ascii").splitlines()[-1])
    except (IndexError, ValueError, SyntaxError, UnicodeError):
        return "probe_error"
    if not isinstance(response, str):
        return "probe_error"
    groups = response.count("`[")
    if groups != 1:
        return "multiple_groups" if groups > 1 else "no_group"
    syllables = len(re.findall(r"\.[012]", response))
    return f"{syllables}_syllables" if syllables else "no_syllable_mark"


def summarize(rows: list[dict], word_queries: list[str],
              plain_words: list[FrontendResult],
              lex_words: list[FrontendResult] | None) -> dict:
    summaries: dict[str, dict] = {}
    for source in sorted({row["source"] for row in rows}):
        subset = [row for row in rows if row["source"] == source]
        summaries[source] = {
            "utterances": len(subset),
            "native": dict(sorted(Counter(row["native"]["status"] for row in subset).items())),
            "plain_text_known": sum(row["text_plain"]["status"] == "known" for row in subset),
            "lexicon_text_known": sum(row.get("text_lexicon", {}).get("status") == "known"
                                      for row in subset) if lex_words is not None else None,
            "apple_compared": sum(row["reference"] in ("exact", "different") for row in subset),
            "apple_exact": sum(row["reference"] == "exact" for row in subset),
        }
    word_freq = Counter(word for row in rows if row["source"] != "calibration"
                        for word in WORD.findall(row["text"]))
    word_map = dict(zip(word_queries, plain_words))
    lex_map = dict(zip(word_queries, lex_words)) if lex_words is not None else {}
    unrecognized = [(word, count) for word, count in word_freq.items()
                    if word_map[word].code != 0]
    unrecognized_lex = [(word, count) for word, count in word_freq.items()
                        if lex_words is not None and lex_map[word].code != 0]
    return {
        "by_source": summaries,
        "natural_word_occurrences": sum(word_freq.values()),
        "natural_distinct_word_forms": len(word_freq),
        "plain_known_occurrences": sum(count for word, count in word_freq.items()
                                        if word_map[word].code == 0),
        "lexicon_known_occurrences": (
            sum(count for word, count in word_freq.items()
                if lex_map[word].code == 0) if lex_words is not None else None),
        "top_unrecognized_words": sorted(unrecognized, key=lambda pair: (-pair[1], pair[0]))[:20],
        "top_unrecognized_after_lexicon": sorted(
            unrecognized_lex, key=lambda pair: (-pair[1], pair[0]))[:30],
        "unknown_distinct_after_lexicon": len(unrecognized_lex),
    }


def markdown_report(report: dict) -> str:
    summary = report["summary"]
    lines = ["# Deutsche Prosa: Text- und Audioabdeckung", "",
             "Gezählt werden vollständige Textaufträge. Eine bekannte Aussprache"
             " ist keine erfolgreiche PCM-Synthese. Das optionale `.rlex` wurde"
             " nur im Textfrontend geprüft; die aktuelle ECI-DLL lädt es nicht.", "",
             "| Quelle | Aufträge | Frontend | mit `.rlex` | C-PCM | Apple-genau / verglichen |",
             "| --- | ---: | ---: | ---: | ---: | ---: |"]
    for source, row in summary["by_source"].items():
        lex = str(row["lexicon_text_known"]) if row["lexicon_text_known"] is not None else "–"
        lines.append(f"| {source} | {row['utterances']} | {row['plain_text_known']} | "
                     f"{lex} | {row['native'].get('pcm_produced', 0)} | "
                     f"{row['apple_exact']} / {row['apple_compared']} |")
    words = summary
    lines.extend(["", f"Natürliche Wortvorkommen: {words['plain_known_occurrences']} / "
                  f"{words['natural_word_occurrences']} vom C-Frontend erkannt; "
                  + (f"mit dem geprüften Lexikon {words['lexicon_known_occurrences']} / "
                     f"{words['natural_word_occurrences']}." if words['lexicon_known_occurrences'] is not None
                     else "Lexikonvergleich nicht ausgeführt."), "",
                  "## Fälle", "",
                  "| ID | Frontend | `.rlex` | ECI | Referenz |",
                  "| --- | --- | --- | --- | --- |"])
    for row in report["cases"]:
        lines.append(f"| {row['id']} | {row['text_plain']['status']} | "
                     f"{row.get('text_lexicon', {}).get('status', '–')} | "
                     f"{row['native']['status']} | {row['reference']} |")
    lines.extend(["", "## Häufigste noch unbekannte Wortformen", "",
                  ", ".join(f"{word} ({count})" for word, count in
                            words["top_unrecognized_words"]) or "Keine.", ""])
    if report["lexicon_checked"]:
        lines.extend(["", "## Nach dem geprüften Lexikon noch unbekannt", "",
                      ", ".join(f"{word} ({count})" for word, count in
                                words["top_unrecognized_after_lexicon"]) or "Keine.", ""])
    if report.get("isolated_reference_inventory"):
        inventory = report["isolated_reference_inventory"]
        lines.extend(["", "## Isolierte Referenz-Lautstruktur der fehlenden Formen", "",
                      "Dies zählt Wortvorkommen nach frischen isolierten Apple-ECI-"
                      "Antworten. Ein Wort im Satz kann anders betont werden;"
                      " die Zählung ist kein Test vollständiger Satzprosodie.", "",
                      "| Klasse | Wortvorkommen |",
                      "| --- | ---: |"])
        lines.extend(f"| {kind} | {count} |" for kind, count in
                     inventory["weighted_classes"].items())
        lines.append("")
    return "\n".join(lines)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--fixture", type=Path,
                        default=ROOT / "tests/data/german_prose_coverage.jsonl")
    parser.add_argument("--native-eci", type=Path, default=ROOT / "eci/libreloq_eci.so")
    parser.add_argument("--lexicon", type=Path, default=ROOT / "data/german_apple_observed.rlex")
    parser.add_argument("--no-lexicon", action="store_true")
    parser.add_argument("--text-probe", type=Path, help="use an already compiled C batch probe")
    parser.add_argument("--apple-eci", type=Path)
    parser.add_argument("--apple-audio-probe", type=Path)
    parser.add_argument("--apple-phoneme-probe", type=Path,
                        help="classify isolated pronunciations of unknown word forms")
    parser.add_argument("--apple-workdir", type=Path)
    parser.add_argument("--apple-runtime-lib-dir", type=Path)
    parser.add_argument("--json-out", type=Path)
    parser.add_argument("--markdown-out", type=Path)
    args = parser.parse_args(argv)
    compare_apple = any((args.apple_eci, args.apple_audio_probe, args.apple_workdir))
    if compare_apple and not all((args.apple_eci, args.apple_audio_probe, args.apple_workdir)):
        parser.error("Apple comparison needs --apple-eci, --apple-audio-probe and --apple-workdir")
    if args.apple_phoneme_probe and not compare_apple:
        parser.error("isolated phoneme inventory also needs all Apple audio comparison arguments")
    fixtures = fixture_rows(args.fixture)
    texts = [row["text"] for row in fixtures]
    words = sorted({word for text in texts for word in WORD.findall(text)})
    with tempfile.TemporaryDirectory(prefix="reloq-prose-coverage-") as temp:
        work = Path(temp)
        probe = args.text_probe.resolve(strict=True) if args.text_probe else build_text_probe(work)
        plain = front_end(probe, texts + words, None)
        lex = None if args.no_lexicon else front_end(
            probe, texts + words, args.lexicon.resolve(strict=True))
        output = work / "apple.s16"
        records = []
        engine = DirectEciEngine(args.native_eci.resolve(strict=True))
        try:
            for i, item in enumerate(fixtures):
                text = item["text"]
                native, pcm = render_native(engine, text)
                reference = "not_compared"
                if native["status"] == "pcm_produced" and compare_apple:
                    try:
                        oracle = apple_pcm(args.apple_audio_probe.resolve(strict=True),
                                           args.apple_eci.resolve(strict=True),
                                           args.apple_workdir.resolve(strict=True),
                                           args.apple_runtime_lib_dir.resolve(strict=True)
                                           if args.apple_runtime_lib_dir else None,
                                           text, output)
                        reference = "exact" if pcm == oracle else "different"
                        native["apple_samples"] = len(oracle) // 2
                    except (OSError, RuntimeError, subprocess.TimeoutExpired) as exc:
                        reference = "reference_error"
                        native["reference_error"] = str(exc)
                record = dict(item)
                record["text_plain"] = {"status": plain[i].label,
                                        "utf8_offset": plain[i].bad_utf8_offset}
                if lex is not None:
                    record["text_lexicon"] = {"status": lex[i].label,
                                              "utf8_offset": lex[i].bad_utf8_offset}
                record["native"] = native
                record["reference"] = reference
                records.append(record)
        finally:
            engine.close()
    inventory = None
    if args.apple_phoneme_probe:
        occurrence = Counter(word for item in fixtures if item["source"] != "calibration"
                             for word in WORD.findall(item["text"]))
        front = lex[len(texts):] if lex is not None else plain[len(texts):]
        unresolved = {word: occurrence[word] for word, result in zip(words, front)
                      if word in occurrence and result.code != 0}
        classes = {word: apple_isolated_class(
            args.apple_phoneme_probe.resolve(strict=True),
            args.apple_eci.resolve(strict=True),
            args.apple_workdir.resolve(strict=True),
            args.apple_runtime_lib_dir.resolve(strict=True)
            if args.apple_runtime_lib_dir else None, word)
            for word in unresolved}
        inventory = {
            "scope": "isolated Apple ECI pronunciation, unknown C word forms",
            "lexicon_used_for_unknown_filter": lex is not None,
            "weighted_classes": dict(sorted(Counter(
                {kind: sum(count for word, count in unresolved.items()
                           if classes[word] == kind)
                 for kind in set(classes.values())}).items())),
            "unknown_word_forms": len(unresolved),
            "unknown_word_occurrences": sum(unresolved.values()),
            "word_classes": classes,
        }
    report = {"suite": "German natural text-to-PCM coverage, 11025 Hz",
              "reference_checked": bool(compare_apple),
              "lexicon_checked": lex is not None,
              "isolated_reference_inventory": inventory,
              "summary": summarize(records, words, plain[len(texts):],
                                   lex[len(texts):] if lex is not None else None),
              "cases": records}
    body = json.dumps(report, ensure_ascii=False, indent=2) + "\n"
    if args.json_out:
        args.json_out.write_text(body, encoding="utf-8")
    else:
        print(body)
    if args.markdown_out:
        args.markdown_out.write_text(markdown_report(report), encoding="utf-8")
    summary = report["summary"]
    print("German prose:", {source: row for source, row in summary["by_source"].items()},
          f"words {summary['plain_known_occurrences']}/{summary['natural_word_occurrences']}"
          + (f" lex {summary['lexicon_known_occurrences']}"
             if lex is not None else ""), file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
