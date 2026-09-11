import json,re,subprocess,argparse
p=argparse.ArgumentParser();p.add_argument("--alternate",action="store_true");args=p.parse_args();suffix="alternate" if args.alternate else "direct"
from pathlib import Path
root=Path(__file__).resolve().parents[1]
with (root/f'work/mac-tutorial-{suffix}.log').open('w') as out:
 r=subprocess.run([str(root/'Saved/StagedBuilds/Mac/AuraPlayground.app/Contents/MacOS/AuraPlayground'),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-ExecCmds=t.IdleWhenNotForeground 0','-nullrhi','-unattended','-nosound','-BattleTutorialAudit',*(['-BattleTutorialAlternate'] if args.alternate else []),'-stdout'],stdout=out,stderr=subprocess.STDOUT,timeout=300)
m=re.findall(r'BattleTutorialAudit: (\{[^\n]+\})',(root/f'work/mac-tutorial-{suffix}.log').read_text());data=json.loads(m[-1]) if m else {'passed':False,'missing_report':True};data['exit_code']=r.returncode;data['passed']=bool(data['passed'] and r.returncode==0)
(root/f'Tests/Results/2026-09-11-build042-tutorial-{suffix}.json').write_text(json.dumps(data,indent=2)+'\n');print(json.dumps(data));raise SystemExit(0 if data['passed'] else 1)
