"""Compare raw closure with a 32x oversampled reference; requires NumPy/SciPy.

This is a source-model error measurement, not a recording or an NVDA test.
The probe is compiled in a temporary directory; all samples remain in RAM.
"""
import json
import os
from pathlib import Path
import shlex
import subprocess
import tempfile

import numpy as np
from scipy.signal import resample_poly

ROOT=Path(__file__).resolve().parents[1]


def main():
    rows=[]
    with tempfile.TemporaryDirectory(prefix='reloq-source-reference-') as directory:
        probe=Path(directory)/'source-probe'
        command=shlex.split(os.environ.get('CC','cc'))
        command+=['-O2','-std=c11','-ffp-contract=off','-ffunction-sections','-fdata-sections',
            '-I',str(ROOT/'include'),str(ROOT/'tools/glottal_source_probe.c'),
            *(str(ROOT/'src'/name) for name in ('reloq_core.c','reloq_rate_transfer.c','reloq_noise_profile.c')),
            '-Wl,--gc-sections','-lm','-o',str(probe)]
        subprocess.run(command,check=True)
        for rate in (16000,22050,32000,44100,48000):
            for pitch in (137,233,430):
                for opening in (45,70,85):
                    samples=[np.frombuffer(subprocess.check_output([str(probe),str(rate),str(mode),
                        str(pitch*10),str(opening)]),dtype=np.float32) for mode in (0,1)]
                    n=len(samples[0]);ratio=32;period=rate/pitch;duration=period*opening/100
                    t=np.arange(n*ratio,dtype=np.float64)/ratio%period
                    base=1.5/duration;gain=10**(55/20)*float.fromhex('0x1.278002p+4')
                    ideal=np.where(t<duration,gain*(base*t-(base*t)**2),0)
                    trim=rate//4
                    ideal=resample_poly(ideal,1,ratio)[trim:-trim]
                    errors=[float(np.sqrt(np.mean((a[trim:-trim]-ideal)**2))) for a in samples]
                    improvement=float(20*np.log10(errors[0]/errors[1]))
                    assert improvement>5,(rate,pitch,opening,errors,improvement)
                    rows.append({'rate':rate,'f0_hz':pitch,'open_percent':opening,
                        'legacy_rms_error':errors[0],'corrected_rms_error':errors[1],
                        'error_reduction_db':improvement})
    print(json.dumps({'reference':'32x analytic polynomial pulse, anti-alias resample_poly; no tract/noise',
        'cases':len(rows),'minimum_error_reduction_db':min(r['error_reduction_db'] for r in rows),
        'rows':rows},indent=2))


if __name__=='__main__':main()
