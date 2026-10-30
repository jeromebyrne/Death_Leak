#!/usr/bin/env python3
"""Fresh earned vendor purchase, not a full ending or live/physical acceptance."""
from pathlib import Path
import os,re,subprocess,tempfile,xml.etree.ElementTree as ET,json,hashlib
binary=os.environ.get('DEATHLEAK_BINARY','port/build/deathleak-linux')
script=Path('port/tests/routes/current_earned_deflection.script').read_text().strip()
for token in script.split(','):
 command,count=token.split(':');assert len(command)==1 and command in '.RLEQBNYZJIMP' and int(count)>0
with tempfile.TemporaryDirectory(prefix='deathleak-earned-deflection-') as directory:
 save=Path(directory)/'fresh.dl5';assert not save.exists()
 result=subprocess.run([binary,'--play','Game/XmlFiles/levels/forest_exploration_1.xml','--script',script,'--fixed-dt','0.0166667','--simulate-only','--simulate-wall-clock','--combat-state','--save-file',str(save)],stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,timeout=180)
 assert result.returncode==0,result.stdout
 assert 'Player died' not in result.stdout and 'TEST-ONLY encounter clock' in result.stdout,result.stdout
 assert 'Collected key granary at frame 25607' in result.stdout,result.stdout
 assert 'Purchased kDeflection at frame 31001 balance=21' in result.stdout,result.stdout
 assert re.search(r'Simulation ended at .*forest_exploration_1.xml .*orbs=396 keys=1 health=1 .*balance=21',result.stdout),result.stdout
 lines=save.read_text().splitlines();ids=[s for s in lines if s.startswith('Game/') and '#' in s]
 assert len(ids)==len(set(ids))==396 and 'B:21' in lines and 'U:kDeflection' in lines and 'K:granary' in lines
 assert 396==375+21 and float(lines[2].split()[2])==1
 cache={}
 for pickup in ids:
  level,ident=pickup.rsplit('#',1)
  if level not in cache:cache[level]={o.get('id'):o.tag for o in ET.parse(level).getroot()}
  assert cache[level][ident]=='CurrencyOrb',pickup
 if os.environ.get('DEATHLEAK_EVIDENCE_DIR'):
  out=Path(os.environ['DEATHLEAK_EVIDENCE_DIR']);out.mkdir(parents=True,exist_ok=True)
  (out/'earned-purchase.log').write_text(result.stdout);(out/'earned-purchase.dl5').write_bytes(save.read_bytes())
  receipt=dict(binary_sha256=hashlib.sha256(Path(binary).read_bytes()).hexdigest(),script_sha256=hashlib.sha256((script+'\n').encode()).hexdigest(),fresh_process=True,save_load_used=False,unique_authored_pickup_ids=396,source_price_debit=375,remaining_balance=21,conservation='396 = 375 + 21',health=1,keys=['granary'],upgrades=['kDeflection'],purchase_frame=31001,authored_pickup_ids=sorted(ids),test_only_encounter_clock=True,full_ending=False,physical_acceptance=False)
  (out/'earned-purchase-ledger.json').write_text(json.dumps(receipt,indent=2)+'\n')
print('PASS: fresh earned Deflection purchase,396 original pickup IDs =375 debit +21 balance, real granary key, health1; ordinary controls/no loaded state/debug damage; TEST-ONLY encounter clock, NOT full ending/physical acceptance')
