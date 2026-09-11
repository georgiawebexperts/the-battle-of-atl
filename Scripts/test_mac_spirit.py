"""Native encounter rules only; this does not establish bear art or route acceptance."""
import argparse,json,re,subprocess
from pathlib import Path
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--difficulty',choices=['Easy','Medium','Hard'],default='Easy');a=p.parse_args()
app=root/'Saved/StagedBuilds/Mac/AuraPlayground.app/Contents/MacOS/AuraPlayground'
log=root/'work'/f'build043-spirit-{a.difficulty}.log'
with log.open('w') as f:
 r=subprocess.run([str(app),f'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty={a.difficulty}?AutoStart=1','-nullrhi','-BattleSpiritAudit','-BattleSkipTutorial','-unattended','-nosound','-ExecCmds=t.IdleWhenNotForeground 0','-stdout'],stdout=f,stderr=subprocess.STDOUT,timeout=90)
m=re.findall(r'BattleSpiritAudit: (\{[^\n]+\})',log.read_text());d=json.loads(m[-1]) if m else {'passed':False,'missing_report':True}
d['exit_code']=r.returncode;d['difficulty']=a.difficulty;d['passed']=bool(d.get('passed') and r.returncode==0)
(root/'Tests/Results'/f'2026-09-11-build043-spirit-{a.difficulty}.json').write_text(json.dumps(d,indent=2)+'\n')
print(json.dumps(d));raise SystemExit(0 if d['passed'] else 1)
