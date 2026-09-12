"""Exercise road-car braking and restart on the native 10th Street road."""
import json,pathlib,re,subprocess
root=pathlib.Path(__file__).resolve().parents[1];log=root/'work/signal-route-audit.log'
engine='/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd'
with log.open('w') as stream:
 run=subprocess.run([engine,str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontSignalCrossingReview?Difficulty=Easy?AutoStart=1','-game','-nullrhi','-BattleSkipTutorial','-BattleRoadLaneAudit','-BattleRequireSignalWait','-unattended','-nosound','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=180)
rows=re.findall(r'RoadLaneAudit: (\{[^\n]+\})',log.read_text());r={'exit_code':run.returncode,'checks':json.loads(rows[-1]) if rows else None,'scope':'Two opposing 10th Street lanes with mapped cycling signal, requiring an actual stopped queue and full completion. Natural population, driver visibility and packaged acceptance pending.'};r['passed']=run.returncode==0 and r['checks'] is not None and r['checks']['passed']
(root/'Tests/Results/2026-09-12-native-signal-route.json').write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r));raise SystemExit(0 if r['passed'] else 1)
