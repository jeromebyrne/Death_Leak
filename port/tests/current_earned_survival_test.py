#!/usr/bin/env python3
"""Fresh ordinary-input earned survival after source collision/range repairs.
Fixed/test encounter clock only; not physical display or full earned ending.
"""
import os,re,subprocess,tempfile,xml.etree.ElementTree as ET
from pathlib import Path
binary=os.environ.get('DEATHLEAK_BINARY','port/build/deathleak-linux')
script=Path('port/tests/routes/current_earned_village.script').read_text().strip()
for token in script.split(','):
 command,count=token.split(':');assert command in 'REBY' and int(count)>0
with tempfile.TemporaryDirectory(prefix='deathleak-earned-survival-') as directory:
 save=Path(directory)/'fresh.dl5';assert not save.exists()
 result=subprocess.run([binary,'--play','Game/XmlFiles/levels/forest_exploration_1.xml','--script',script,'--fixed-dt','0.0166667','--simulate-only','--simulate-wall-clock','--save-file',str(save)],stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,timeout=150)
 assert result.returncode==0,result.stdout
 assert 'Player died' not in result.stdout,result.stdout
 for level in ('waterfall_top','forest_exploration_2','forest_exploration_barn','village_forts','village_central'):
  assert 'Entered Game/XmlFiles/levels/'+level+'.xml' in result.stdout,result.stdout
 assert re.search(r'Simulation ended at .*village_central.xml .*orbs=92 keys=0 health=13 .*balance=92 .*grounded=1',result.stdout),result.stdout
 lines=save.read_text().splitlines();ids=[s for s in lines if s.startswith('Game/') and '#' in s]
 assert len(ids)==len(set(ids))==92 and 'B:92' in lines and not any(s.startswith('U:') for s in lines)
 cache={}
 for pickup in ids:
  level,ident=pickup.rsplit('#',1)
  if level not in cache:cache[level]={o.get('id'):o.tag for o in ET.parse(level).getroot()}
  assert cache[level][ident]=='CurrencyOrb',pickup
print('PASS: fresh authored Forest-1 to village-central, five transitions,92 unique original pickups/B92, health13; ordinary jump/fire, no loaded/synthetic state or debug damage; not whole-game/display acceptance')
