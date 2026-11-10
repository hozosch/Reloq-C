"""Original ECI utterances for checking slow-speed acoustic continuity."""
import json
from pathlib import Path
import sys

from observe_high_speed_reference import capture

library, output = Path(sys.argv[1]), Path(sys.argv[2])
output.mkdir(parents=True, exist_ok=True)
cases = (
    ('adictive', 0x10000, 'adictive drums'),
    ('english', 0x10000, 'Hello friend this is a longer sentence.'),
    ('german', 0x40000, 'Guten Tag wie geht es Ihnen heute.'),
    ('german_long', 0x40000, 'Eine Stimme liest diesen Satz bei niedriger Geschwindigkeit.'),
    ('portuguese', 0x70000, 'Bom dia como vai voce.'),
    ('french', 0x30000, 'Bonjour comment allez vous.'),
)
records = []
for label, dialect, text in cases:
    for slider in (0, 10, 20, 30, 40, 50, 60):
        native_speed = round(40 + 110 * slider / 100)
        for gender in (0, 1):
            name = f'{label}-{slider}-{gender}.wav'
            data = capture(library, True, text, native_speed, (output/name).resolve(),
                           buffer_samples=1100, want_phones=False,
                           dialect=dialect, gender=gender)
            data.update(file=name, label=label, dialect=dialect, slider=slider,
                        gender=gender, native_speed=native_speed)
            records.append(data)
            print(f'{name}: {data["samples"]} samples', flush=True)
(output/'references.json').write_text(json.dumps(records,indent=2)+'\n')
