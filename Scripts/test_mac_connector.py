"""Drive the installed connection using real cooked movement and keyboard input."""
import json,re,subprocess
from pathlib import Path
root=Path(__file__).resolve().parents[1]
app=root/'Saved/StagedBuilds/Mac/AuraPlayground.app/Contents/MacOS/AuraPlayground'
log=root/'work/mac-connector-drive.log'
with log.open('w') as stream:
    run=subprocess.run([str(app),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1',
        '-nullrhi','-unattended','-nosound','-BattleConnectorAudit','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=120)
matches=re.findall(r'BattleConnectorAudit: (\{[^\n]+\})',log.read_text())
result=json.loads(matches[-1]) if matches else {'passed':False,'missing_report':True}
result['exit_code']=run.returncode
result['passed']=bool(result.get('passed') and run.returncode==0)
result['scope']='Cooked Mac game, real W/A/D input and CharacterMovement, both directions; display-free, appearance unverified.'
(root/'Tests/Results/2026-09-11-native-connector-drive.json').write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps(result),flush=True)
if not result['passed']:raise SystemExit(1)
