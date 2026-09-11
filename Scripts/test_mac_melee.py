"""Exercise F input and chain-lock combat in the cooked native Mac game."""
import json,re,subprocess,argparse
from pathlib import Path
root=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser();parser.add_argument('--report',default='2026-09-11-native-melee.json');args=parser.parse_args();assert Path(args.report).name==args.report
app=root/'Saved/StagedBuilds/Mac/AuraPlayground.app/Contents/MacOS/AuraPlayground'
log=root/'work/mac-melee.log'
with log.open('w') as stream:
 run=subprocess.run([str(app),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-nullrhi','-unattended','-nosound','-BattleMeleeAudit','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=90)
matches=re.findall(r'BattleMeleeAudit: (\{[^\n]+\})',log.read_text());result=json.loads(matches[-1]) if matches else {'passed':False,'missing_report':True}
result['exit_code']=run.returncode;result['passed']=bool(result.get('passed') and run.returncode==0);result['rendered_appearance_verified']=False
(root/'Tests/Results'/args.report).write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result),flush=True)
if not result['passed']:raise SystemExit(1)
