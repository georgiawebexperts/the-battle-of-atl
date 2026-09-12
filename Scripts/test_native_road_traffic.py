"""Exercise road-car braking and restart on the native 10th Street road."""
import json,pathlib,re,subprocess
root=pathlib.Path(__file__).resolve().parents[1];log=root/'work/road-traffic-audit.log'
engine='/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd'
with log.open('w') as stream:
 run=subprocess.run([engine,str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-nullrhi','-BattleSkipTutorial','-BattleRoadTrafficAudit','-unattended','-nosound','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=90)
rows=re.findall(r'RoadTrafficAudit: (\{[^\n]+\})',log.read_text());r={'exit_code':run.returncode,'checks':json.loads(rows[-1]) if rows else None,'scope':'Director API and lifecycle with controlled camera and completed-car fixture. Natural map population and packaged behavior pending.'};r['passed']=run.returncode==0 and r['checks'] is not None and r['checks']['passed']
(root/'Tests/Results/2026-09-12-native-road-traffic-director.json').write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r));raise SystemExit(0 if r['passed'] else 1)
