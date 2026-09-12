"""Check the placed ambient sleeper in its actual park review map."""
import json,pathlib,re,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[1]
engine='/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd'
main='--main' in sys.argv
log=root/('work/sleeper-trigger-main-audit.log' if main else 'work/sleeper-trigger-native-audit.log')
with log.open('w') as stream:
 result=subprocess.run([engine,str(root/'AuraPlayground.uproject'),('/Game/PiedmontRide/Maps/PiedmontWorld' if main else '/Game/PiedmontRide/Maps/PiedmontSleeperReview')+'?Difficulty=Easy?AutoStart=1','-game','-nullrhi','-BattleSkipTutorial','-BattleSleeperTriggerAudit','-unattended','-nosound','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=120)
rows=re.findall(r'BattleSleeperTriggerAudit: (\{[^\n]+\})',log.read_text())
report={'exit_code':result.returncode,'checks':json.loads(rows[-1]) if rows else None,'scope':'Uncooked placed sleeper: real player proximity gates, chase and return. Test forces wake probability to 1; normal default remains 0.18. No rendered or packaged acceptance.'}
report['main_map']=main
report['passed']=result.returncode==0 and report['checks'] is not None and report['checks']['passed']
(root/('Tests/Results/2026-09-12-main-sleeper-trigger.json' if main else 'Tests/Results/2026-09-12-native-sleeper-trigger.json')).write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report));raise SystemExit(0 if report['passed'] else 1)
