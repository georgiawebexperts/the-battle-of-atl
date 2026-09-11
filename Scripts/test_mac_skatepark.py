"""Ride the actual cooked skatepark access, bank and bowl; verify rewards."""
import json,re,subprocess
from pathlib import Path
root=Path(__file__).resolve().parents[1];app=root/'Saved/StagedBuilds/Mac/AuraPlayground.app/Contents/MacOS/AuraPlayground';log=root/'work/mac-skatepark.log'
with log.open('w') as f:
 r=subprocess.run([str(app),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-nullrhi','-unattended','-nosound','-BattleSkateAudit','-stdout'],stdout=f,stderr=subprocess.STDOUT,timeout=70)
m=re.findall(r'BattleSkateAudit: (\{[^\n]+\})',log.read_text());data=json.loads(m[-1]) if m else {'passed':False,'missing_report':True};data['exit_code']=r.returncode;data['passed']=bool(data['passed'] and r.returncode==0)
(root/'Tests/Results/2026-09-11-build040-skatepark.json').write_text(json.dumps(data,indent=2)+'\n');print(json.dumps(data));raise SystemExit(0 if data['passed'] else 1)
