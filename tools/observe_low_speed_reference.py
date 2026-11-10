"""Compare voices, source controls and low/high rates with original ECI."""
import json
from pathlib import Path
import sys
from observe_high_speed_reference import capture
library,output=Path(sys.argv[1]),Path(sys.argv[2]);output.mkdir(parents=True,exist_ok=True)
texts=(('en',0x10000,'Papa said better days and quiet voices.'),('de',0x40000,'Guten Tag wie geht es Ihnen heute.'))
settings=(('reed',0,50,65,30,0,0),('flat',0,50,65,0,0,0),('deep',0,70,45,30,0,0),('breathy',0,50,65,30,0,50),('rough',0,50,65,30,50,0),('female',1,50,65,30,0,0),('small',1,20,65,30,0,0))
rows=[]
for label,dialect,text in texts:
 for slider in (20,50,60):
  speed=round(40+1.1*slider)
  for model,gender,head,pitch,inflection,rough,breath in settings:
   voice=(gender,head,pitch,inflection,rough,breath,speed,100)
   for rate in (8000,11025):
    name=f'{label}-{slider}-{model}-{rate}.wav'
    try:
     d=capture(library,True,text,speed,(output/name).resolve(),buffer_samples=1100,want_phones=False,dialect=dialect,gender=gender,sample_rate=rate,voice=voice)
     d.update(file=name,label=label,slider=slider,model=model,dialect=dialect,sample_rate=rate,voice=voice);rows.append(d)
     print(name,d['samples'],flush=True)
    except Exception as e:
     rows.append(dict(file=name,label=label,slider=slider,model=model,dialect=dialect,sample_rate=rate,voice=voice,text=text,error=repr(e)))
     print(name,'ERROR',repr(e),flush=True)
(output/'references.json').write_text(json.dumps(rows,indent=2)+'\n')
