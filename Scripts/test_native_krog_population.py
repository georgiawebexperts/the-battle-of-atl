"""Exercise automatic Krog traffic, queuing and endpoint recycling."""
import json,pathlib,re,subprocess
root=pathlib.Path(__file__).resolve().parents[1];log=root/'work/krog-population-audit.log'
engine='/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd'
with log.open('w') as stream:
 run=subprocess.run([engine,str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontKrogPopulationReview?Difficulty=Easy?AutoStart=1','-game','-RCWebControlDisable','-nullrhi','-BattleSkipTutorial','-BattleTrafficPopulationAudit','-unattended','-nosound','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=200)
rows=re.findall(r'TrafficPopulationAudit: (\{[^\n]+\})',log.read_text());r={'exit_code':run.returncode,'checks':json.loads(rows[-1]) if rows else None,'scope':'Automatic director on the real lanes and occupied-area yield control, with controlled offscreen observation. Yield proof is in the separate occupancy test; no forced wait required here. Rider interaction, visible traffic motion and packaged behavior pending.'};r['passed']=run.returncode==0 and r['checks'] is not None and r['checks']['passed']
(root/'Tests/Results/2026-09-12-native-krog-population.json').write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r));raise SystemExit(0 if r['passed'] else 1)
