"""Exercise keyboard-driven bike impact against a road car on native 10th Street."""
import json,pathlib,re,subprocess
root=pathlib.Path(__file__).resolve().parents[1];log=root/'work/bike-car-audit.log'
engine='/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd'
with log.open('w') as stream:
 run=subprocess.run([engine,str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-nullrhi','-BattleSkipTutorial','-BattleBikeCarAudit','-unattended','-nosound','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=90)
rows=re.findall(r'BikeCarAudit: (\{[^\n]+\})',log.read_text());r={'exit_code':run.returncode,'checks':json.loads(rows[-1]) if rows else None,'scope':'Keyboard-driven bike into a stationary car on the real road. Recovery classification only; crash animation, injury, glancing and moving-car impacts remain unverified.'};r['passed']=run.returncode==0 and r['checks'] is not None and r['checks']['passed']
(root/'Tests/Results/2026-09-12-native-bike-car-impact.json').write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r));raise SystemExit(0 if r['passed'] else 1)
