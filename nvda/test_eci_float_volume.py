"""Compare quiet speech with synthesis measured before PCM protection.

The diagnostic library has exactly the same DSP and a 1/64 output gain;
its final knee is disabled. All audio stays in memory. Scaling that signal
is independent evidence that the production limiter did not colour quiet
speech before the user's requested volume was applied.
"""
import argparse
import array
import json
from pathlib import Path
from _reloq_eci_direct import (DirectEciEngine, SAMPLE_RATE, HIGH_RATE_PROFILE,
    NATIVE_VOLUME, NATIVE_SPEED, GENDER, HEAD_SIZE, NATIVE_PITCH, BREATHINESS)

TEXTS = {
    0x10000: 'The warm morning voice should remain smooth. We are hearing more body in every vowel. Susan visits the zoo on Thursday.',
    0x40000: 'Am Morgen hören wir ruhige Musik. Zwölf Züge fahren zum Zentrum.',
    0x20000: 'Seis sacerdotes de Zaragoza escuchan esta suave voz por la mañana.',
}


def capture(e, text):
    chunks = []
    assert e.speak([text], chunks.append, lambda n: None)
    return array.array('h', b''.join(chunks))


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--library', type=Path, required=True)
    p.add_argument('--raw-library', type=Path, required=True)
    p.add_argument('--profile', type=int, choices=(1, 3), default=1)
    p.add_argument('--output', type=Path)
    a = p.parse_args()
    rows = []
    gain = 363.184 / 2768.533
    for language, text in TEXTS.items():
        engines = [DirectEciEngine(path.resolve(), dialect=language, encoding='utf8',
            experimental_generic=True) for path in (a.raw_library, a.library)]
        try:
            for e in engines:
                for param, value in ((HIGH_RATE_PROFILE, a.profile), (NATIVE_SPEED, 75), (GENDER, 1)):
                    e.set_parameter(param, value)
            engines[0].set_parameter(NATIVE_VOLUME, 100)
            engines[1].set_parameter(NATIVE_VOLUME, 50)
            for gender,head,pitch,breath in ((1,50,65,0),(1,29,90,60),(1,29,50,60),(0,80,30,0)):
                for e in engines:
                    for param,value in ((GENDER,gender),(HEAD_SIZE,head),(NATIVE_PITCH,pitch),(BREATHINESS,breath)):
                        e.set_parameter(param,value)
                for rate in (2, 3, 4, 6):
                    for e in engines:
                        e.set_parameter(SAMPLE_RATE, rate)
                    raw = capture(engines[0], text)
                    quiet = capture(engines[1], text)
                    assert len(raw) == len(quiet)
                    peak = max(abs(x) for x in raw) * 64
                    error = max(abs(y - x*64*gain) for x, y in zip(raw, quiet))
                    assert peak < 32767*64, 'Diagnostic output saturated'
                    assert peak*gain < 30000, 'Select a quieter diagnostic volume'
                    assert error < 16, (hex(language), rate, peak, error)
                    rows.append(dict(language=hex(language), gender=gender, head=head, pitch=pitch, breathiness=breath, rate=rate, raw_peak_upper_bound=peak+64,
                        samples_over_knee=sum(abs(x)*64 > 30000 for x in raw),
                        maximum_quiet_error=error))
        finally:
            for e in engines:
                e.close()
    assert any(r["samples_over_knee"] for r in rows), "The regression needs a raw over-knee witness"
    result = dict(checks=len(rows), all_quiet_signals_linear=True, rows=rows)
    if a.output:
        a.output.write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps(result))


if __name__ == '__main__':
    main()
