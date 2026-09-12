"""Check native sleeper pursuit movement, timeout and target-loss handling."""
import json,pathlib,re,subprocess
root=pathlib.Path(__file__).resolve().parents[1]
engine='/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd'
log=root/'work/sleeper-chase-native-audit.log'
with log.open('w') as stream:
 result=subprocess.run([engine,str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-nullrhi','-BattleSkipTutorial','-BattleSleeperChaseAudit','-unattended','-nosound','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=90)
rows=re.findall(r'BattleSleeperChaseAudit: (\{[^\n]+\})',log.read_text())
report={'exit_code':result.returncode,'checks':json.loads(rows[-1]) if rows else None,'scope':'Uncooked native chase approach, timeout, speed restoration and target-loss check; no rendered or packaged acceptance'}
report['passed']=result.returncode==0 and report['checks'] is not None and report['checks']['passed']
(root/'Tests/Results/2026-09-12-native-sleep-chase.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report));raise SystemExit(0 if report['passed'] else 1)
