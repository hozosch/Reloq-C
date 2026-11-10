"""Reference samples across the normal NVDA transition range."""
import json
from pathlib import Path
import sys
from observe_high_speed_reference import capture
library, output = Path(sys.argv[1]), Path(sys.argv[2])
output.mkdir(parents=True, exist_ok=True)
cases=(('english',0x10000,'adictive drums'),
       ('transitions',0x10000,'Papa said better days and quiet voices.'),
       ('german',0x40000,'Guten Tag wie geht es Ihnen heute.'))
records=[]
for label,dialect,text in cases:
    for slider in (20,40,50,52,54,56,58,60):
        speed=round(40+1.1*slider)
        for gender in (0,1):
            name=f'{label}-{slider}-{gender}.wav'
            data=capture(library,True,text,speed,(output/name).resolve(),buffer_samples=1100,
                         want_phones=False,dialect=dialect,gender=gender)
            data.update(file=name,label=label,dialect=dialect,slider=slider,gender=gender,native_speed=speed)
            records.append(data)
            print(name,data['samples'],flush=True)
(output/'references.json').write_text(json.dumps(records,indent=2)+'\n')
