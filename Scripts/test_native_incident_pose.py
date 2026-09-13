"""Native pedestrian encounter-pose lifecycle fixture; not a complete incident playtest."""
import json,pathlib,re,subprocess
root=pathlib.Path(__file__).resolve().parents[1]
log=root/'work/incident-pose-audit.log'
with log.open('w') as stream:
 result=subprocess.run(['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-nullrhi','-BattleSkipTutorial','-BattleFurnitureAudit','-BattleIncidentPoseAudit','-RCWebControlDisable','-unattended','-nosound','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=120)
rows=re.findall(r'BattleIncidentPoseAudit: (\{[^\n]+\})',log.read_text())
report={'exit_code':result.returncode,'checks':json.loads(rows[-1]) if rows else None,'scope':'Native held pose after 0.6s transition settling, horn and timed get-up completion, bike-impact and damage interruption; spawned fixture only, posed head visibility trace, empty standing-height trace and pelvis bike-sized Pawn-object sweep and blocked get-up wait followed by automatic retry after obstruction removal; no whole-body envelope, scene placement, rarity or visual acceptance.'}
report['passed']=result.returncode==0 and bool(report['checks']) and report['checks']['passed']
(root/'Tests/Results/2026-09-13-native-incident-pose-collision.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report));raise SystemExit(0 if report['passed'] else 1)
