#!/usr/bin/env python3
"""Original typed Windows feature IDs for newly active Focus/bomb unlocks."""
import importlib.util,sys,tempfile
from pathlib import Path
sys.dont_write_bytecode=True
spec=importlib.util.spec_from_file_location('save_bridge','port/tools/save_bridge.py');bridge=importlib.util.module_from_spec(spec);spec.loader.exec_module(bridge)
level='Game/XmlFiles/levels/grass_exploration_3.xml'
with tempfile.TemporaryDirectory(prefix='deathleak-technique-bridge-') as directory:
 root=Path(directory)
 for features in ({'kSlowMotion':'3'},{'kBombUnlock':'8'},{'kSlowMotion':'3','kBombUnlock':'8'}):
  source=root/'active.dl5';source.write_text('DL5\n'+level+'\n0 0 25 100\nB:0\nC:1\n'+''.join('U:'+name+'\n' for name in features))
  payload,runtime=bridge.dl5_to_windows(source);win=root/'windows.xml';win.write_bytes(payload)
  fields=bridge.windows_fields(win);assert set(fields['gameplay_features_unlocked'][1])=={('int',value) for value in features.values()}
  restored=bridge.windows_to_dl5(fields,level)
  assert all('U:'+name in restored for name in features) and 'C:1' in restored
  assert ('U:kBombUnlock' in restored)==('kBombUnlock' in features)
  assert ('U:kSlowMotion' in restored)==('kSlowMotion' in features)
print('PASS: Focus3/bomb8 original Windows typed feature vectors round-trip independently/together, no fabricated unlocks')
