"""Build the common frame host with the probe's legacy exports enabled."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def prepare(work: Path) -> Path:
    output = work / 'reloq_rules61_continuous.c'
    output.write_text((ROOT / 'src/reloq_rules61.c').read_text(encoding='utf8'), encoding='utf8')
    return output


def prepare_eci(work: Path) -> Path:
    output = work / 'reloq_eci_continuous.c'
    text = (ROOT / 'src/reloq_eci.c').read_text(encoding='utf8')
    output.write_text('#define RELOQ_FA_PROBE_EXPORT 1\n' + text, encoding='utf8')
    return output
