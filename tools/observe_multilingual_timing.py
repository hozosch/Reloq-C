"""Reference cases for comparing the acoustic control clock across languages."""
import json
from pathlib import Path
import sys
from observe_high_speed_reference import capture

library, output = Path(sys.argv[1]), Path(sys.argv[2])
output.mkdir(parents=True, exist_ok=True)
cases = (
    ('en_us', 0x10000, 'adictive drums'),
    ('en_us_sibilants', 0x10000, 'six ships and fresh cups'),
    ('en_gb', 0x10001, 'please speak clearly'),
    ('de', 0x40000, 'guten tag wie geht es dir'),
    ('pt_br', 0x70000, 'bom dia tudo bem'),
    ('es', 0x20000, 'buenos dias como estas'),
    ('fr', 0x30000, 'bonjour comment allez vous'),
)
records = []
for label, dialect, text in cases:
    for speed in (84, 106):
        for gender in (0, 1):
            for rate in (8000, 11025):
                name = f'{label}-{speed}-{gender}-{rate}.wav'
                try:
                    record = capture(library, True, text, speed, (output/name).resolve(),
                                     buffer_samples=1100, want_phones=False,
                                     dialect=dialect, sample_rate=rate, gender=gender)
                except Exception as exc:
                    print(f'{name}: {exc}', flush=True)
                    records.append(dict(case=name, error=repr(exc)))
                    continue
                record.update(case=name, label=label, dialect=dialect,
                              sample_rate=rate, gender=gender)
                print(f"{name}: {record['samples']} samples", flush=True)
                records.append(record)
(output/'references.json').write_text(json.dumps(records, indent=2)+'\n')
