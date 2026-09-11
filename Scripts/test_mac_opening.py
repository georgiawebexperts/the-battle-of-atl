"""Run the opening through natural completion or an actual Enter key, optionally capture it."""
import argparse,json,re,subprocess,plistlib,uuid,shutil,struct
from pathlib import Path
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--skip',action='store_true');p.add_argument('--review',action='store_true');p.add_argument('--escape',action='store_true');a=p.parse_args();assert sum([a.skip,a.review,a.escape])<=1
mode='escape' if a.escape else 'review' if a.review else 'skip' if a.skip else 'complete'
bundle=root/'Saved/StagedBuilds/Mac/AuraPlayground.app';app=bundle/'Contents/MacOS/AuraPlayground'
bid=plistlib.loads((bundle/'Contents/Info.plist').read_bytes())['CFBundleIdentifier']
capture=Path.home()/'Library/Containers'/bid/'Data/Documents/BattleOpeningReview'/uuid.uuid4().hex;capture.mkdir(parents=True)
out=root/'work'/f'build042-opening-{mode}';out.mkdir(exist_ok=True)
flags=['-windowed','-ResX=1280','-ResY=720','-ForceRes','-BattleOpeningReview',f'-BattleHUDReviewDir={capture}'] if a.review else ['-nullrhi','-BattleOpeningAudit']
if a.skip or a.escape:flags+=['-BattleOpeningSkip']
if a.escape:flags+=['-BattleOpeningEscape']
with (out/'run.log').open('w') as log:
 r=subprocess.run([str(app),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1',*flags,'-unattended','-nosound','-ExecCmds=r.ScreenshotDelegate 0,t.IdleWhenNotForeground 0','-stdout'],stdout=log,stderr=subprocess.STDOUT,timeout=70)
m=re.findall(r'BattleOpeningAudit: (\{[^\n]+\})',(out/'run.log').read_text());d=json.loads(m[-1]) if m else {'passed':False,'missing_report':True};d['exit_code']=r.returncode;d['passed']=bool(d.get('passed') and r.returncode==0)
if a.review and d['passed']:
 d['images']=[]
 for i in range(3):
  dest=out/f'opening{i}.png';shutil.copy2(capture/dest.name,dest);assert struct.unpack('>II',dest.read_bytes()[16:24])==(1280,720);d['images'].append(str(dest.relative_to(root)))
 d['visual_review']='pending'
d['scope']='Opening camera/story UI, unchanged tutorial clock/player location, input/camera return. No audio, final scenery or animation acceptance.'
(root/'Tests/Results'/f'2026-09-11-build042-opening-{mode}.json').write_text(json.dumps(d,indent=2)+'\n');print(json.dumps(d),flush=True);raise SystemExit(0 if d['passed'] else 1)
