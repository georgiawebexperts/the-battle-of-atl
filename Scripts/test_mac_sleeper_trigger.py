"""Check the placed ambient sleeper in its actual park review map."""
import json,pathlib,re,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[1]
app=root/'Saved/StagedBuilds/Mac/AuraPlayground.app/Contents/MacOS/AuraPlayground'
if '--app' in sys.argv:app=pathlib.Path(sys.argv[sys.argv.index('--app')+1])
assert app.is_file(),app
main=True
log=root/'work/sleeper-trigger-packaged-audit.log'
with log.open('w') as stream:
 result=subprocess.run([str(app),('/Game/PiedmontRide/Maps/PiedmontWorld' if main else '/Game/PiedmontRide/Maps/PiedmontSleeperReview')+'?Difficulty=Easy?AutoStart=1','-game','-nullrhi','-BattleSkipTutorial','-BattleSleeperTriggerAudit','-unattended','-nosound','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=120)
rows=re.findall(r'BattleSleeperTriggerAudit: (\{[^\n]+\})',log.read_text())
report={'exit_code':result.returncode,'checks':json.loads(rows[-1]) if rows else None,'scope':'Packaged main-map sleeper: real player proximity gates, chase and return. Test forces wake probability to 1; normal default remains 0.18. No rendered appearance or probability-distribution acceptance.'}
report['main_map']=main
report['passed']=result.returncode==0 and report['checks'] is not None and report['checks']['passed']
report_name=sys.argv[sys.argv.index('--report')+1] if '--report' in sys.argv else '2026-09-12-build045-sleeper-trigger.json'
assert pathlib.Path(report_name).name==report_name
(root/'Tests/Results'/report_name).write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report));raise SystemExit(0 if report['passed'] else 1)
