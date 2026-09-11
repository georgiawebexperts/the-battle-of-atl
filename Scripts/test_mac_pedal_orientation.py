"""Render both shoes at four crank positions and measure their forward direction."""
import json, re, subprocess, plistlib, uuid, shutil, argparse
from pathlib import Path
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--walking',action='store_true');args=p.parse_args()
bundle=root/'Saved/StagedBuilds/Mac/AuraPlayground.app'
bid=plistlib.loads((bundle/'Contents/Info.plist').read_bytes())['CFBundleIdentifier']
capture=Path.home()/'Library/Containers'/bid/'Data/Documents/BattlePedalReview'/uuid.uuid4().hex
capture.mkdir(parents=True)
out=root/('work/build044-walking-review' if args.walking else 'work/build044-pedal-review');out.mkdir(exist_ok=True)
mode=[] if args.walking else ['-BattlePedalReview']
with (out/'run.log').open('w') as log:
 result=subprocess.run([str(bundle/'Contents/MacOS/AuraPlayground'),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-BattleSkipTutorial','-BattleLocomotionReview',*mode,f'-BattleHUDReviewDir={capture}','-windowed','-ResX=1280','-ResY=720','-ForceRes','-unattended','-nosound','-ExecCmds=r.ScreenshotDelegate 0,t.IdleWhenNotForeground 0','-stdout'],stdout=log,stderr=subprocess.STDOUT,timeout=90)
if args.walking:
 for name in ['walk','walk-step','run','idle']:shutil.copy2(capture/f'{name}.png',out/f'{name}.png')
 log_text=(out/'run.log').read_text()
 dots={side:float(value) for side,value in re.findall(r'LocoReview: foot_([LR])_forward_dot=([-\d.]+)',log_text)}
 matches=re.findall(r'LocoReview: pass=(\d)',log_text)
 passed=result.returncode==0 and matches==['1'] and len(dots)==2 and min(dots.values())>.5
 report={'passed':passed,'exit_code':result.returncode,'idle_toe_forward_dot':dots,'visual_review':'pending'}
 (root/'Tests/Results/2026-09-11-build044-walking-orientation.json').write_text(json.dumps(report,indent=2)+'\n')
 print(json.dumps(report));raise SystemExit(0 if passed else 1)
reports=re.findall(r'PedalReview: (\{[^\n]+\})',(out/'run.log').read_text())
data=json.loads(reports[-1]) if reports else {'passed':False,'missing_report':True}
data['exit_code']=result.returncode;data['passed']=bool(data['passed'] and result.returncode==0)
if data['passed']:
 for i in range(4):shutil.copy2(capture/f'pedal-{i}.png',out/f'pedal-{i}.png')
 data['visual_review']='pending'
(root/'Tests/Results/2026-09-11-build044-pedal-orientation.json').write_text(json.dumps(data,indent=2)+'\n')
print(json.dumps(data));raise SystemExit(0 if data['passed'] else 1)
