"""Actual pedestrian capsule and AI traversal of the short grass navigation link."""
import json,pathlib,re,subprocess
root=pathlib.Path(__file__).resolve().parents[1]
log=root/'work/park-grass-link-runtime.log'
with log.open('w') as stream:
 run=subprocess.run(['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontKrogWorldReview?Difficulty=Easy?AutoStart=1','-game','-RCWebControlDisable','-nullrhi','-BattleSkipTutorial','-BattleSleeperChaseAudit','-BattleGrassLinkAudit','-unattended','-nosound','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=180)
rows=re.findall(r'BattleGrassLinkAudit: (\{[^\n]+\})',log.read_text())
report={'exit_code':run.returncode,'checks':json.loads(rows[-1]) if rows else None,'scope':'Actual pedestrian capsule, CharacterMovement and AI path following across grass in both directions. Autonomous destination changes disabled. Animation and mixed crowd interactions not evaluated.'}
report['passed']=run.returncode==0 and bool(report['checks']) and report['checks']['passed']
(root/'Tests/Results/2026-09-12-native-park-grass-link.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report));raise SystemExit(0 if report['passed'] else 1)
