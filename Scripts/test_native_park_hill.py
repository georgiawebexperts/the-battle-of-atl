"""Bidirectional realistic-handling ride on an existing mapped park hillside path."""
import json,pathlib,re,subprocess
root=pathlib.Path(__file__).resolve().parents[1];log=root/'work/park-hill-realistic.log';osm='1278380268'
with log.open('w') as stream:
 run=subprocess.run(['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-nullrhi','-RCWebControlDisable','-BattleSkipTutorial','-BattleConnectorAudit','-BattleRealHandlingRoute',f'-BattleParkHillPath={osm}','-unattended','-nosound','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=180)
text=log.read_text();matches=re.findall(r'BattleConnectorAudit: (\{[^\n]+\})',text);r=json.loads(matches[-1]) if matches else {'passed':False,'missing_report':True}
matches=re.findall(r'HandlingRouteAudit: (\{[^\n]+\})',text);r['handling']=json.loads(matches[-1]) if matches else None
matches=re.findall(r'TrailGroundAudit: samples=(\d+) paved=(\d+)',text);r['ground_samples']={'total':int(matches[-1][0]),'paved':int(matches[-1][1])} if matches else None
r.update(osm_way_id=osm,exit_code=run.returncode,scope='Current main-world mapped park path, both directions with native realistic handling and W/A/D/Space test driver. Starting teleports; pedestrians disabled. Not whole-park, rendered, subjective balance or full tire simulation acceptance.')
r['passed']=bool(r['passed'] and run.returncode==0 and r['handling'] and r['handling']['realistic'] and r['handling']['elevation_span_cm']>300)
(root/'Tests/Results/2026-09-12-native-park-hill-realistic.json').write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r));raise SystemExit(0 if r['passed'] else 1)
