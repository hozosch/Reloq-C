"""Share the C speed curve and rate controls with Android Java."""
from pathlib import Path
import os
import re


def build_epoch():
    value=os.environ.get('RELOQ_BUILD_EPOCH', '0') or '0'
    if not re.fullmatch(r'(?:0|[1-9]\d{0,11})', value) or int(value)>253402300799:
        raise ValueError('Invalid release build timestamp')
    return int(value)


def generate(destination, package_arch="arm64"):
    if package_arch not in ("arm64", "x86_64"):
        raise ValueError("Unsupported Android package architecture")
    root=Path(__file__).resolve().parents[1]
    target=Path(destination)/"org/reloq/tts"
    target.mkdir(parents=True,exist_ok=True)
    source=(root/"src/reloq_voice_controls.c").read_text()
    block=source.split("time_scale[] = {",1)[1].split("};",1)[0]
    rows=re.findall(r"\{(\d+),\s*([\d.]+)(?:\s*/\s*([\d.]+))?\}",block)
    assert len(rows)>10 and int(rows[0][0])==40 and int(rows[-1][0])==250
    speeds=[int(a) for a,b,c in rows]
    times=[float(b)/float(c or 1) for a,b,c in rows]
    assert all(a>b for a,b in zip(times,times[1:]))
    (target/"RateCurve.java").write_text(
        'package org.reloq.tts;\n'
        '// Generated from src/reloq_voice_controls.c; no separate tempo model.\n'
        'final class RateCurve {\n'
        ' static final int[] SPEEDS={'+','.join(map(str,speeds))+'};\n'
        ' static final double[] TIMES={'+','.join(repr(v) for v in times)+'};\n}\n')
    (target/"BuildData.java").write_text(
        'package org.reloq.tts;\n'
        'final class BuildData { static final long BUILD_EPOCH='+str(build_epoch())+'L; '
        'static final String PACKAGE_ARCH="'+package_arch+'"; }\n')
