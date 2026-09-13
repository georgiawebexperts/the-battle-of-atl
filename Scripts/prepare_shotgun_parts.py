"""Separate original foregrip/collars from the fixed shotgun, preserving shared coordinates."""
import json,pathlib,copy
root=pathlib.Path(__file__).resolve().parents[1];p=root/'SourceAssets/Weapons/Remington870';g=json.loads((p/'Remington870.gltf').read_text());primitives=g['meshes'][0]['primitives'];assert len(primitives)==19
pump={15,16,17};g['meshes']=[{'name':'ShotgunBody','primitives':[v for i,v in enumerate(primitives) if i not in pump]},{'name':'ShotgunPump','primitives':[v for i,v in enumerate(primitives) if i in pump]}];g['nodes']=[{'name':'ShotgunBody','mesh':0},{'name':'ShotgunPump','mesh':1}];g['scenes']=[{'nodes':[0,1]}];g['scene']=0
(p/'ShotgunParts.gltf').write_text(json.dumps(g));print('Prepared body and moving foregrip in shared source coordinates')
