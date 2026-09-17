"""Native two-stab sequence and chase acceptance; animation quality remains separate."""
import argparse,json,re,subprocess,plistlib,uuid,shutil,struct
from pathlib import Path
root=Path(__file__).resolve().parents[1];p=argparse.ArgumentParser();p.add_argument('--review',action='store_true');p.add_argument('--difficulty',choices=['Easy','Hard'],default='Easy');a=p.parse_args()
app=root/'Saved/StagedBuilds/Mac/AuraPlayground.app/Contents/MacOS/AuraPlayground';log=root/'work'/f'build044-knife-{a.difficulty}.log'
bundle=app.parents[2];bid=plistlib.loads((bundle/'Contents/Info.plist').read_bytes())['CFBundleIdentifier']
capture=Path.home()/'Library/Containers'/bid/'Data/Documents/BattleKnifeReview'/uuid.uuid4().hex;capture.mkdir(parents=True)
flags=['-windowed','-ResX=1280','-ResY=720','-ForceRes','-BattleKnifeReview',f'-BattleHUDReviewDir={capture}'] if a.review else ['-nullrhi']
with log.open('w') as f:r=subprocess.run([str(app),f'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty={a.difficulty}?AutoStart=1',*flags,'-BattleKnifeAudit','-BattleSkipTutorial','-unattended','-nosound','-ExecCmds=r.ScreenshotDelegate 0,t.IdleWhenNotForeground 0','-stdout'],stdout=f,stderr=subprocess.STDOUT,timeout=90)
m=re.findall(r'BattleKnifeAudit: (\{[^\n]+\})',log.read_text());d=json.loads(m[-1]) if m else {'passed':False,'missing_report':True};d['exit_code']=r.returncode;d['difficulty']=a.difficulty;d['passed']=bool(d.get('passed') and r.returncode==0)
if a.review and d['passed']:
 out=root/'work/build044-knife-review';out.mkdir(exist_ok=True);d['images']=[]
 for name in ['windup','first-stab','defense','defeated']:
  dest=out/f'{name}.png';shutil.copy2(capture/dest.name,dest);assert struct.unpack('>II',dest.read_bytes()[16:24])==(1280,720);d['images'].append(str(dest.relative_to(root)))
 d['visual_review']='pending'
(root/'Tests/Results'/f'2026-09-11-build044-knife-{a.difficulty}.json').write_text(json.dumps(d,indent=2)+'\n');print(json.dumps(d));raise SystemExit(0 if d['passed'] else 1)
