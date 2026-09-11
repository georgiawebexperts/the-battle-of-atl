"""Exercise real moving-disc collision and enemy drops in a cooked Mac game."""
import json,re,subprocess,argparse
from pathlib import Path
root=Path(__file__).resolve().parents[1];p=argparse.ArgumentParser();p.add_argument('--report',default='2026-09-11-native-disc.json');a=p.parse_args();assert Path(a.report).name==a.report
app=root/'Saved/StagedBuilds/Mac/AuraPlayground.app/Contents/MacOS/AuraPlayground';log=root/'work/mac-disc.log'
with log.open('w') as f:
 run=subprocess.run([str(app),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-nullrhi','-unattended','-nosound','-BattleDiscAudit','-stdout'],stdout=f,stderr=subprocess.STDOUT,timeout=90)
m=re.findall(r'BattleDiscAudit: (\{[^\n]+\})',log.read_text());result=json.loads(m[-1]) if m else {'passed':False,'missing_report':True};result['exit_code']=run.returncode;result['passed']=bool(result.get('passed') and run.returncode==0);result['rendered_appearance_verified']=False
(root/'Tests/Results'/a.report).write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result),flush=True)
if not result['passed']:raise SystemExit(1)
