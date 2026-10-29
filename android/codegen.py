"""Share the C speed curve and immutable lexical identity with Android Java."""
from pathlib import Path
import hashlib
import re


def generate(destination, lexicon):
    root=Path(__file__).resolve().parents[1]
    target=Path(destination)/"org/reloq/tts/preview"
    target.mkdir(parents=True,exist_ok=True)
    source=(root/"src/reloq_voice_controls.c").read_text()
    block=source.split("time_scale[] = {",1)[1].split("};",1)[0]
    rows=re.findall(r"\{(\d+),\s*([\d.]+)(?:\s*/\s*([\d.]+))?\}",block)
    assert len(rows)>10 and int(rows[0][0])==40 and int(rows[-1][0])==250
    speeds=[int(a) for a,b,c in rows]
    times=[float(b)/float(c or 1) for a,b,c in rows]
    assert all(a>b for a,b in zip(times,times[1:]))
    (target/"RateCurve.java").write_text(
        'package org.reloq.tts.preview;\n'
        '// Generated from src/reloq_voice_controls.c; no separate tempo model.\n'
        'final class RateCurve {\n'
        ' static final int[] SPEEDS={'+','.join(map(str,speeds))+'};\n'
        ' static final double[] TIMES={'+','.join(repr(v) for v in times)+'};\n}\n')
    digest=hashlib.sha256(lexicon).hexdigest()
    (target/"BuildData.java").write_text(
        'package org.reloq.tts.preview;\nfinal class BuildData {\n'
        f' static final String LEXICON_NAME="german-{digest}.rlex";\n'
        f' static final String LEXICON_SHA256="{digest}";\n'
        f' static final int LEXICON_BYTES={len(lexicon)};\n'+'}\n')
