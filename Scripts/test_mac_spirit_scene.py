"""Cooked memorial screenshot review and real bike traversal of its adjacent trail."""
from pathlib import Path
import argparse,json,re,subprocess,plistlib,uuid,shutil
root=Path(__file__).resolve().parents[1];p=argparse.ArgumentParser();p.add_argument('--review',action='store_true');a=p.parse_args()
bundle=root/'Saved/StagedBuilds/Mac/AuraPlayground.app';bid=plistlib.loads((bundle/'Contents/Info.plist').read_bytes())['CFBundleIdentifier']
capture=Path.home()/'Library/Containers'/bid/'Data/Documents/BattleMemorialReview'/uuid.uuid4().hex;capture.mkdir(parents=True)
mode='memorial-review' if a.review else 'spirit-route';out=root/'work'/('build043-'+mode);out.mkdir(exist_ok=True)
flags=['-windowed','-ResX=1280','-ResY=720','-ForceRes','-BattleMemorialReview',f'-BattleHUDReviewDir={capture}'] if a.review else ['-nullrhi','-BattleSpiritRouteAudit']
with (out/'run.log').open('w') as f:r=subprocess.run([str(bundle/'Contents/MacOS/AuraPlayground'),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1',*flags,'-BattleSkipTutorial','-unattended','-nosound','-ExecCmds=r.ScreenshotDelegate 0,t.IdleWhenNotForeground 0','-stdout'],stdout=f,stderr=subprocess.STDOUT,timeout=120)
raw=(out/'run.log').read_text();name='BattleMemorialReview' if a.review else 'BattleConnectorAudit';matches=re.findall(name+r': (\{[^\n]+\})',raw);d=json.loads(matches[-1]) if matches else {'passed':False,'missing_report':True};d['exit_code']=r.returncode;d['passed']=bool(d.get('passed') and r.returncode==0)
if a.review and d['passed']:
 for n in ['wide','close']:shutil.copy2(capture/(n+'.png'),out/(n+'.png'))
 d['visual_review']='pending'
else:
 g=re.findall(r'TrailGroundAudit: samples=(\d+) paved=(\d+)',raw);d['ground_samples']={'total':int(g[-1][0]),'paved':int(g[-1][1])} if g else {};d['scope']='Both directions past memorial using W/A/D and CharacterMovement; no bear chase or crowds acceptance.'
(root/'Tests/Results'/('2026-09-11-build043-'+mode+'.json')).write_text(json.dumps(d,indent=2)+'\n');print(json.dumps(d));raise SystemExit(0 if d['passed'] else 1)
