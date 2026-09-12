"""Run the integrated knife encounter in a chosen Mac app, retaining isolated evidence."""
import argparse,json,plistlib,re,subprocess,uuid
from pathlib import Path
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--app',type=Path,required=True);p.add_argument('--report',required=True);p.add_argument('--render',action='store_true');p.add_argument('--difficulty',choices=['Easy','Hard'],default='Hard');a=p.parse_args()
assert a.app.is_file() and Path(a.report).name==a.report
bid=plistlib.loads((a.app.parents[1]/'Info.plist').read_bytes())['CFBundleIdentifier']
capture=Path.home()/'Library/Containers'/bid/'Data/Documents/BattleKnifeReview'/uuid.uuid4().hex
capture.mkdir(parents=True)
log=root/'work'/('knife-encounter-'+uuid.uuid4().hex+'.log')
flags=['-RenderOffscreen','-ResX=1280','-ResY=720','-ForceRes','-BattleKnifeReview',f'-BattleHUDReviewDir={capture}'] if a.render else ['-nullrhi']
with log.open('w') as f:
 r=subprocess.run([str(a.app),f'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty={a.difficulty}?AutoStart=1','-BattleKnifeAudit','-BattleSkipTutorial','-unattended','-nosound','-ExecCmds=r.ScreenshotDelegate 0,t.IdleWhenNotForeground 0','-stdout',*flags],stdout=f,stderr=subprocess.STDOUT,timeout=120)
rows=re.findall(r'BattleKnifeAudit: (\{[^\n]+\})',log.read_text());d=json.loads(rows[-1]) if rows else {'passed':False,'reason':'Missing audit report'}
d.update(exit_code=r.returncode,app=str(a.app),log=str(log),difficulty=a.difficulty)
d['passed']=bool(d['passed'] and r.returncode==0)
if a.render:
 d['images']=[str(capture/(n+'.png')) for n in ['windup','first-stab','defense','defeated']]
 d['passed']=d['passed'] and all(Path(n).is_file() for n in d['images']);d['visual_review']='pending'
(root/'Tests/Results'/a.report).write_text(json.dumps(d,indent=2)+'\n')
print(json.dumps(d),flush=True);raise SystemExit(0 if d['passed'] else 1)
