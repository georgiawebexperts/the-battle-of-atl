"""Real W-driven bike traversal past/into a posed NPC on an isolated native floor."""
import json,pathlib,re,subprocess
root=pathlib.Path(__file__).resolve().parents[1];log=root/'work/incident-bike-audit.log'
with log.open('w') as f:
 r=subprocess.run(['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-nullrhi','-BattleSkipTutorial','-BattleSteeringAudit','-BattleIncidentBikeAudit','-RCWebControlDisable','-unattended','-nosound','-stdout'],stdout=f,stderr=subprocess.STDOUT,timeout=120)
rows=re.findall(r'IncidentBikeAudit: (\{[^\n]+\})',log.read_text());report={'exit_code':r.returncode,'checks':json.loads(rows[-1]) if rows else None,'scope':'Actual W-driven arcade bike, isolated flat floor, one injured pose, 3m lateral near miss followed by head-on contact. Initial positions reset; not actual map placement or all approach directions.'};report['passed']=r.returncode==0 and bool(report['checks']) and report['checks']['passed']
(root/'Tests/Results/2026-09-13-native-incident-bike.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report));raise SystemExit(0 if report['passed'] else 1)
