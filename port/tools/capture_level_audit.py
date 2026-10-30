#!/usr/bin/env python3
"""Capture original authored maps on strict hardware; never physical acceptance.
Run from repository root inside the established GPU-enabled build container.
"""
import argparse, hashlib, json, re, subprocess, time
from pathlib import Path
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--binary',default='port/build/deathleak-linux')
p.add_argument('--out',required=True)
p.add_argument('--frames',type=int,default=90)
p.add_argument('--include-auxiliary',action='store_true')
p.add_argument('--only',help='optional exact authored XML stem')
p.add_argument('--script',help='ordinary scripted control diagnostic, not completion proof')
a=p.parse_args();out=Path(a.out);out.mkdir(parents=True,exist_ok=True)
if not 1<=a.frames<=600:raise SystemExit('frames must be within 1..600')
sha=lambda path:hashlib.sha256(Path(path).read_bytes()).hexdigest()
binary_hash=sha(a.binary)
levels=sorted(Path('Game/XmlFiles/levels').glob('*.xml'))
if a.include_auxiliary:
 levels+=sorted(Path('Game/XmlFiles/deprecated_levels').glob('*.xml'))
 levels+=[Path('Game/XmlFiles/animation_preview.xml')]
if a.only:levels=[level for level in levels if level.stem==a.only]
rows=[]
for level in levels:
 name=level.stem if level.parent.name=='levels' else level.parent.name+'--'+level.stem
 cmd=[a.binary,'--play',str(level),'--frames',str(a.frames),'--script',a.script or f'.:{a.frames}',
      '--fixed-dt','0.0166667','--capture',str(out/(name+'.bmp'))]
 started=time.monotonic()
 try:
  result=subprocess.run(cmd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,timeout=120)
  log=result.stdout;code=result.returncode
 except subprocess.TimeoutExpired as e:
  log=(e.stdout or b'').decode(errors='replace') if isinstance(e.stdout,bytes) else (e.stdout or '')
  code=124
 (out/(name+'.log')).write_text(log)
 end=re.search(r'^Simulation ended at (\S+)',log,re.M)
 gpu=re.search(r'^OpenGL device (.+)$',log,re.M)
 strict='accelerated=1 software=0' in log and 'mode=strict-accelerated' in log
 strict=strict and gpu is not None and not any(s in gpu[1].lower() for s in ('llvmpipe','softpipe','swiftshader','software rasterizer'))
 image=out/(name+'.bmp')
 rows.append(dict(level=str(level),name=name,source_xml_sha256=sha(level),binary_sha256=binary_hash,
  exit=code,seconds=round(time.monotonic()-started,3),strict_hardware=bool(strict),
  gpu=gpu[1] if gpu else None,actual_level=end[1] if end else None,
  scene_capture_matches_request=end is not None and end[1]==str(level),
  capture=str(image) if image.exists() else None,capture_sha256=sha(image) if image.exists() else None))
 (out/'manifest.json').write_text(json.dumps(rows,indent=2)+'\n')
 print(json.dumps(rows[-1]),flush=True)
assert sha(a.binary)==binary_hash,'binary changed during capture run'
failed=[r for r in rows if r['exit'] or not r['strict_hardware'] or not r['capture'] or not r['scene_capture_matches_request']]
print(f'Captured {len(rows)} authored maps; {len(failed)} failed/partial diagnostics. Offscreen is NOT physical acceptance.',flush=True)
raise SystemExit(bool(failed))
