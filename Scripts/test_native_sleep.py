"""Run sleeping/wake and interruption checks in an uncooked native game world."""
import json,pathlib,re,subprocess
root=pathlib.Path(__file__).resolve().parents[1]
engine='/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd'
log=root/'work/sleeper-native-audit.log'
with log.open('w') as stream:
 result=subprocess.run([engine,str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-nullrhi','-BattleSkipTutorial','-BattleSleeperAudit','-unattended','-nosound','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=90)
rows=re.findall(r'BattleSleeperAudit: (\{[^\n]+\})',log.read_text())
report={'exit_code':result.returncode,'checks':json.loads(rows[-1]) if rows else None,'scope':'Uncooked native runtime state/pose/interruption check, no rendered presentation or packaged acceptance'}
report['passed']=result.returncode==0 and report['checks'] is not None and report['checks']['passed']
(root/'Tests/Results/2026-09-11-native-sleep-runtime.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report));raise SystemExit(0 if report['passed'] else 1)
