"""Exercise the saved scene at the actual home start; only its rare selection is forced."""
import json,re,subprocess
from pathlib import Path
root=Path(__file__).resolve().parents[1]
log=root/'work/scooter-startup.log'
with log.open('w') as stream:
    run=subprocess.run(['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-nullrhi','-BattleScooterStartupCheck','-RCWebControlDisable','-unattended','-nosound','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=100)
rows=re.findall(r'ScooterStartupAudit: (\{[^\n]+\})',log.read_text())
report=json.loads(rows[-1]) if rows else {'passed':False,'missing_report':True}
report['exit_code']=run.returncode
report['opening_completed']='BattleOpening: finished duration=' in log.read_text()
report['passed']=bool(report.get('passed') and report['opening_completed'] and run.returncode==0)
report['scope']='Saved main encounter, normal home spawn/opening and real player camera. Selection forced to 100% for this test only; no actor replacement, staging camera or player teleport. Does not test random frequency, visual fidelity or journey to Krog.'
(root/'Tests/Results/2026-09-13-native-scooter-startup.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report));raise SystemExit(0 if report['passed'] else 1)
