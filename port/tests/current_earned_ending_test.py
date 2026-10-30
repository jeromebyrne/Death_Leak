#!/usr/bin/env python3
"""Fresh authored ending; explicit test clock; no loaded/injected progression."""
from pathlib import Path
import hashlib,json,os,re,subprocess,tempfile,xml.etree.ElementTree as ET
binary=os.environ.get('DEATHLEAK_BINARY','port/build/deathleak-linux')
script_path=Path('port/tests/routes/current_earned_ending.script')
script=script_path.read_text().strip()
for token in script.split(','):
    c,n=token.split(':');assert len(c)==1 and c in '.RLEQBNYZJIMP' and int(n)>0,token
out=Path(os.environ.get('DEATHLEAK_EVIDENCE_DIR','port/build/current-earned-ending'))
out.mkdir(parents=True,exist_ok=True)
env=os.environ.copy();env.update(SDL_VIDEODRIVER='offscreen',SDL_RENDER_DRIVER='opengl',SDL_AUDIODRIVER='dummy',DEATHLEAK_TEST_ALLOW_SOFTWARE_RENDERER='1',DEATHLEAK_TRACE_CURRENCY='1',DEATHLEAK_TRACE_ENCOUNTERS='1')
cache={};receipt={}
with tempfile.TemporaryDirectory(prefix='deathleak-fresh-ending-') as temporary:
    for stage,frames in [('purchase',31002),('preboat',57470),('completion',61000)]:
        save=Path(temporary)/(stage+'.dl');assert not save.exists()
        args=[binary,'--play','Game/XmlFiles/levels/forest_exploration_1.xml','--script',script,'--frames',str(frames),'--fixed-dt','0.0166667','--simulate-only','--simulate-wall-clock','--combat-state','--save-file',str(save)]
        result=subprocess.run(args,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,env=env,timeout=180)
        log=result.stdout;(out/(stage+'.log')).write_text(log)
        assert result.returncode==0 and 'Player died' not in log,log
        assert 'TEST-ONLY encounter clock follows fixed gameplay dt' in log
        assert 'Purchased kDeflection at frame 31001 balance=21' in log
        assert len(re.findall(r'^Purchased ',log,re.M))==1
        authored=sum(map(int,re.findall(r'^Currency frame=\d+ authored=(\d+)',log,re.M)))
        runtime=sum(map(int,re.findall(r'^Currency frame=\d+ authored=\d+ runtime=(\d+)',log,re.M)))
        match=re.search(r'Simulation ended at (\S+) player=\(([^)]+)\) orbs=(\d+) keys=(\d+) health=([\d.]+) focus=[\d.]+ balance=(\d+).*grounded=(\d+)',log)
        assert match,log
        level,position,count,keys,health,balance,grounded=match.groups()
        assert int(count)==authored and authored+runtime==375+int(balance)
        assert float(health)>0 and grounded=='1'
        lines=save.read_text().splitlines();(out/(stage+'.save')).write_bytes(save.read_bytes())
        if stage!='completion':
            ids=[s for s in lines if s.startswith('Game/') and '#' in s]
            assert len(ids)==len(set(ids))==authored
            assert {s[2:] for s in lines if s.startswith('K:')}==({'granary'} if stage=='purchase' else {'granary','dojo','village_hut','has_repair_tools'})
            assert f'B:{balance}' in lines and 'U:kDeflection' in lines
            for pickup in ids:
                path,ident=pickup.rsplit('#',1)
                if path not in cache:cache[path]={o.get('id'):o.tag for o in ET.parse(path).getroot()}
                assert cache[path][ident]=='CurrencyOrb',pickup
            if stage=='purchase':assert authored==396 and runtime==0 and int(balance)==21
            else:assert level=='Game/XmlFiles/levels/cliff.xml' and int(keys)==4
        else:
            assert lines==['DL6','MENU','C:1']
            assert 'Deflection unlocked=1 count=1' in log and 'Deflected projectile at frame' in log
            assert all(event in log for event in ['Collected key dojo','Collected key village_hut','Collected key has_repair_tools','Boat at frame','Final title DEATH LEAK','EndStory at frame'])
            assert level=='Game/XmlFiles/levels/sea_2.xml' and int(keys)==4
        receipt[stage]=dict(authored_pickups=authored,runtime_reward_pickups=runtime,source_debit=375,balance=int(balance),health=float(health),keys=int(keys),level=level,grounded=True)
receipt['milestones']={name:int(re.search(pattern,log)[1]) for name,pattern in {
 'granary':r'Collected key granary at frame (\d+)', 'dojo':r'Collected key dojo at frame (\d+)',
 'village_hut':r'Collected key village_hut at frame (\d+)', 'repair_tools':r'Collected key has_repair_tools at frame (\d+)',
 'natural_deflection':r'Deflected projectile at frame (\d+)', 'boat':r'Boat at frame (\d+)',
 'title':r'Final title DEATH LEAK at frame (\d+)', 'end_story':r'EndStory at frame (\d+)'
}.items()}
assert receipt['milestones']==dict(granary=25607,dojo=41502,village_hut=43716,repair_tools=49210,natural_deflection=53357,boat=57470,title=58371,end_story=59571)
receipt['natural_deflections']=1
receipt.update(binary_sha256=hashlib.sha256(Path(binary).read_bytes()).hexdigest(),script_sha256=hashlib.sha256(script_path.read_bytes()).hexdigest(),fresh_process_per_snapshot=True,loaded_or_injected_progression=False,test_only_encounter_clock=True,physical_acceptance=False)
(out/'receipt.json').write_text(json.dumps(receipt,indent=2)+'\n')
print('PASS: fresh ordinary-input earned ending;4 keys;independent authored/runtime currency conservation;source debit375;natural earned deflection;alive grounded raft;DL6 MENU C:1;test-only encounter clock, not physical/live pacing')
