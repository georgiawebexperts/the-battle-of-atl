"""Verify actual collision shapes and native car movement at road-end closures."""
import json,pathlib,re,subprocess
root=pathlib.Path(__file__).resolve().parents[1];log=root/'work/krog-construction-runtime.log'
with log.open('w') as stream:
 run=subprocess.run(['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontKrogBoundaryReview?Difficulty=Easy?AutoStart=1','-game','-RCWebControlDisable','-nullrhi','-BattleSkipTutorial','-BattleSleeperChaseAudit','-BattleConstructionAudit','-unattended','-nosound','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=120)
rows=re.findall(r'BattleConstructionAudit: (\{[^\n]+\})',log.read_text());report={'exit_code':run.returncode,'checks':json.loads(rows[-1]) if rows else None,'scope':'Native sweeps using actual bike, standing and crouched character capsule dimensions at ground and150/300cm lift, plus two native car routes with crossing control. Not a player jump trajectory, perimeter containment or natural traffic population test.'}
report['passed']=run.returncode==0 and bool(report['checks']) and report['checks']['passed']
(root/'Tests/Results/2026-09-12-native-krog-construction.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report));raise SystemExit(0 if report['passed'] else 1)
