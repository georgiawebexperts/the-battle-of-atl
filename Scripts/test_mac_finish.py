"""Cooked finite finish inventory, H input, pickups and remount."""
import json,re,subprocess,argparse
p=argparse.ArgumentParser();p.add_argument("--difficulty",choices=["Easy","Hard"],default="Easy");p.add_argument("--report");args=p.parse_args()
from pathlib import Path
root=Path(__file__).resolve().parents[1];app=root/'Saved/StagedBuilds/Mac/AuraPlayground.app/Contents/MacOS/AuraPlayground';log=root/f'work/mac-finish-{args.difficulty}.log'
with log.open('w') as f:
 r=subprocess.run([str(app),f'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty={args.difficulty}?AutoStart=1','-nullrhi','-unattended','-nosound','-BattleFinishAudit','-stdout'],stdout=f,stderr=subprocess.STDOUT,timeout=80)
m=re.findall(r'BattleFinishAudit: (\{[^\n]+\})',log.read_text());data=json.loads(m[-1]) if m else {'passed':False,'missing_report':True};data['exit_code']=r.returncode;data['passed']=bool(data['passed'] and r.returncode==0)
report=args.report or f'2026-09-11-build042-finish-{args.difficulty}.json'
assert Path(report).name==report
(root/'Tests/Results'/report).write_text(json.dumps(data,indent=2)+'\n');print(json.dumps(data));raise SystemExit(0 if data['passed'] else 1)
