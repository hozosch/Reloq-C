"""Check whether ECI's callback and phoneme reporting change the waveform."""
import json
from pathlib import Path
import sys

from observe_high_speed_reference import capture

library, kind, output = Path(sys.argv[1]), sys.argv[2], Path(sys.argv[3])
output.mkdir(parents=True, exist_ok=True)
records = []
for text in ('adictive drums', 'six ships and fresh cups'):
    for speed in (95, 106, 128):
        for size in (110, 1100):
            for phones in ((False, True) if kind == 'original' else (False,)):
                name = f"{kind}-{speed}-{size}-{int(phones)}-{text.replace(' ', '_')}.wav"
                record = capture(library, kind == 'original', text, speed,
                                 (output / name).resolve(), buffer_samples=size,
                                 want_phones=phones)
                record.update(file=name, buffer_samples=size, want_phones=phones)
                print(f"{name}: {record['samples']} samples, {len(record['events'])} phones", flush=True)
                records.append(record)
(output / f'{kind}.json').write_text(json.dumps(records, indent=2) + '\n')
