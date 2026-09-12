"""Verify both Krog/DeKalb approaches hold for a character occupying the crossing."""
import json,pathlib,re,subprocess
root=pathlib.Path(__file__).resolve().parents[1];log=root/'work/krog-rider-audit.log'
engine='/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd'
with log.open('w') as stream:
 run=subprocess.run([engine,str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontKrogCrossingReview?Difficulty=Easy?AutoStart=1','-game','-RCWebControlDisable','-nullrhi','-BattleSkipTutorial','-BattleMonroeOccupancyAudit','-BattleKrogRiderAudit','-unattended','-nosound','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=180)
rows=re.findall(r'KrogRiderAudit: (\{[^\n]+\})',log.read_text());r={'exit_code':run.returncode,'checks':json.loads(rows[-1]) if rows else None,'scope':'Mounted rider starts inside crossing as an occupancy fixture, holds two cars, then uses real W/A/D movement into the tunnel. Cars must finish after rider clears. No full approach, visual or mixed crowd acceptance.'};r['passed']=run.returncode==0 and r['checks'] is not None and r['checks']['passed']
(root/'Tests/Results/2026-09-12-native-krog-rider.json').write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r));raise SystemExit(0 if r['passed'] else 1)
