"""Native hands-free dismount, deliberate draw and on-foot mobility acceptance."""
import argparse,json,re,subprocess,plistlib,uuid,shutil,struct
from pathlib import Path
root=Path(__file__).resolve().parents[1];p=argparse.ArgumentParser();p.add_argument('--review',action='store_true');a=p.parse_args();mode='review' if a.review else 'audit'
bundle=root/'Saved/StagedBuilds/Mac/AuraPlayground.app';app=bundle/'Contents/MacOS/AuraPlayground';bid=plistlib.loads((bundle/'Contents/Info.plist').read_bytes())['CFBundleIdentifier']
capture=Path.home()/'Library/Containers'/bid/'Data/Documents/BattleFootReview'/uuid.uuid4().hex;capture.mkdir(parents=True)
out=root/'work'/f'build042-foot-{mode}';out.mkdir(exist_ok=True)
flags=['-windowed','-ResX=1280','-ResY=720','-ForceRes','-BattleFootReview',f'-BattleHUDReviewDir={capture}'] if a.review else ['-nullrhi']
with (out/'run.log').open('w') as log:
 r=subprocess.run([str(app),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1',*flags,'-BattleFootAudit','-BattleSkipTutorial','-unattended','-nosound','-ExecCmds=r.ScreenshotDelegate 0,t.IdleWhenNotForeground 0','-stdout'],stdout=log,stderr=subprocess.STDOUT,timeout=90)
m=re.findall(r'BattleFootAudit: (\{[^\n]+\})',(out/'run.log').read_text());d=json.loads(m[-1]) if m else {'passed':False,'missing_report':True};d['exit_code']=r.returncode;d['passed']=bool(d.get('passed') and r.returncode==0)
if a.review and d['passed']:
 d['images']=[]
 for name in ['idle','drawn','walk-a','walk-b','run','jump','crouch']:
  dest=out/f'{name}.png';shutil.copy2(capture/dest.name,dest);assert struct.unpack('>II',dest.read_bytes()[16:24])==(1280,720);d['images'].append(str(dest.relative_to(root)))
 d['visual_review']='pending'
(root/'Tests/Results'/f'2026-09-11-build042-foot-{mode}.json').write_text(json.dumps(d,indent=2)+'\n');print(json.dumps(d),flush=True);raise SystemExit(0 if d['passed'] else 1)
