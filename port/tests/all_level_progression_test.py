#!/usr/bin/env python3
"""Synthetic save-fixture upgrade parity, NOT earned-currency/playthrough evidence."""
import os,re,subprocess,tempfile,xml.etree.ElementTree as ET
from pathlib import Path
bin=os.environ.get('DEATHLEAK_BINARY','port/build/deathleak-linux')
prices={'kDownwardDash':450,'kRoll':25,'kSlowMotion':600,'kDeflection':375,'kSpeedIncrease':200,'kProjectileDamageIncrease':225,'kProjectileSpeedIncrease':200,'kBombUnlock':1000}
seen=set()
with tempfile.TemporaryDirectory(prefix='deathleak-techniques-') as directory:
 root=Path(directory)
 def run(fixture,script,out):
  result=subprocess.run([bin,'--load-save',str(fixture),'--script',script,'--fixed-dt','0.0166667','--simulate-only','--save-file',str(out)],text=True,capture_output=True)
  assert result.returncode==0,result.stdout+result.stderr
  return result.stdout,out.read_text()
 for level in sorted(Path('Game/XmlFiles/levels').glob('*.xml')):
  for scroll in ET.parse(level).getroot().findall('DojoScrollPickup'):
   feature=scroll.find('feature_unlock').get('value');seen.add(feature)
   position=scroll.find('position');x=float(position.get('x'));y=float(position.get('y'))
   floors=[]
   for strip in ET.parse(level).getroot().findall('SolidLineStrip'):
    origin=strip.find('position');points=strip.find('points')
    sx=float(origin.get('x','0'));sy=float(origin.get('y','0'))
    for index in range(1,int(points.get('num_points','0'))):
     ax=sx+float(points.get(f'x_{index-1}'));ay=sy+float(points.get(f'y_{index-1}'))
     bx=sx+float(points.get(f'x_{index}'));by=sy+float(points.get(f'y_{index}'))
     if abs(bx-ax)<1:continue
     t=(x-ax)/(bx-ax)
     if 0<=t<=1:floors.append(ay+t*(by-ay))
   if floors:
    floor=min(floors,key=lambda height:abs(height-y))
    if abs(floor-y)<300:y=floor+214.898743/2

   fixture=root/'start.dl';fixture.write_text(f'DL5\n{level}\n{x} {y} 25 100\nB:2000\nC:1\n')
   before_log,before=run(fixture,'I:32',root/'before.dl')
   log,after=run(fixture,'I:32,P:1',root/'after.dl')
   assert f'Opened {feature} upgrade modal' in log,(feature,log)
   assert f'cost={prices[feature]}' in log,(feature,log)
   assert f'Purchased {feature}' in log and f'U:{feature}\n' in after,(feature,log,after)
   balance=lambda text:int(re.search(r'^B:(\d+)$',text,re.M)[1])
   assert balance(before)-balance(after)==prices[feature],(feature,before,after)
   reload_log,reloaded=run(root/'after.dl','.:1',root/'reloaded.dl')
   assert f'U:{feature}\n' in reloaded,(feature,reloaded)
   if feature=='kBombUnlock':
    fixture.write_text(f'DL5\n{level}\n{x} {y} 25 100\nB:2000\nC:0\n')
    denied_log,denied=run(fixture,'I:32,P:1',root/'denied.dl')
    assert 'Purchased kBombUnlock' not in denied_log and 'U:kBombUnlock' not in denied
   print(f'PASS authored {level.name}#{scroll.get("id")} {feature} cost{prices[feature]} purchase/debit/reload',flush=True)
 assert seen==set(prices),seen
print('PASS all8 source-price/payload upgrades; bomb NG+ gate. Synthetic state, not earned route.')
