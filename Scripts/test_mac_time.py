"""Verify real pickup/shot/possession events against the requested timer rules."""
import argparse,json,re,subprocess
from pathlib import Path
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--difficulty',choices=['Easy','Medium','Hard'],default='Easy');p.add_argument('--build',default='030');a=p.parse_args();assert a.build.isdigit()
app=root/'Saved/StagedBuilds/Mac/AuraPlayground.app/Contents/MacOS/AuraPlayground'
log=root/f'work/build{a.build}-time-{a.difficulty.lower()}.log'
with log.open('w') as stream:
 run=subprocess.run([str(app),f'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty={a.difficulty}?AutoStart=1','-nullrhi','-unattended','-nosound','-BattleTimeAudit','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=90)
matches=re.findall(r'BattleTimeAudit: (\{[^\n]+\})',log.read_text());result=json.loads(matches[-1]) if matches else {'passed':False,'missing_report':True}
result.update(exit_code=run.returncode,difficulty=a.difficulty,visual_review=False);result['passed']=bool(result.get('passed') and run.returncode==0)
(root/f'Tests/Results/2026-09-11-build{a.build}-time-{a.difficulty.lower()}.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result))
if not result['passed']:raise SystemExit(1)
