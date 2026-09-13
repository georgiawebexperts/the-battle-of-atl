"""Run the opening through natural completion or an actual Enter key, optionally capture it."""
import argparse,json,re,subprocess,plistlib,uuid,shutil,struct
from pathlib import Path
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--skip',action='store_true');p.add_argument('--review',action='store_true');p.add_argument('--escape',action='store_true');p.add_argument('--report');p.add_argument('--detailed-rider',action='store_true');p.add_argument('--grip',action='store_true');p.add_argument('--editor',action='store_true');a=p.parse_args();assert sum([a.skip,a.review,a.escape])<=1
mode='escape' if a.escape else 'review' if a.review else 'skip' if a.skip else 'complete'
bundle=root/'Saved/StagedBuilds/Mac/AuraPlayground.app';app=bundle/'Contents/MacOS/AuraPlayground'
bid=plistlib.loads((bundle/'Contents/Info.plist').read_bytes())['CFBundleIdentifier']
capture=Path.home()/'Library/Containers'/bid/'Data/Documents/BattleOpeningReview'/uuid.uuid4().hex;capture.mkdir(parents=True)
out=root/'work'/f'build042-opening-{mode}';out.mkdir(exist_ok=True)
flags=['-RenderOffscreen','-windowed','-ResX=1280','-ResY=720','-ForceRes','-BattleOpeningReview',f'-BattleHUDReviewDir={capture}'] if a.review else ['-nullrhi','-BattleOpeningAudit']
if a.detailed_rider:
 assert a.editor and a.review
 flags+=['-BattleDetailedRider']
if a.grip:
 assert a.review
 flags+=['-BattleOpeningGripReview']
if a.skip or a.escape:flags+=['-BattleOpeningSkip']
if a.escape:flags+=['-BattleOpeningEscape']
entry=['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject'),'-game','-RCWebControlDisable'] if a.editor else [str(app)]
with (out/'run.log').open('w') as log:
 r=subprocess.run(entry+['/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1',*flags,'-unattended','-nosound','-ExecCmds=r.ScreenshotDelegate 0,t.IdleWhenNotForeground 0','-stdout'],stdout=log,stderr=subprocess.STDOUT,timeout=70)
m=re.findall(r'BattleOpeningAudit: (\{[^\n]+\})',(out/'run.log').read_text());d=json.loads(m[-1]) if m else {'passed':False,'missing_report':True};d['exit_code']=r.returncode;d['passed']=bool(d.get('passed') and r.returncode==0)
if a.review and d['passed']:
 d['images']=[]
 for i in range(3):
  dest=out/f'opening{i}.png';shutil.copy2(capture/dest.name,dest);assert struct.unpack('>II',dest.read_bytes()[16:24])==(1280,720);d['images'].append(str(dest.relative_to(root)))
 d['visual_review']='pending'
d['editor']=a.editor;d['grip_closeup']=a.grip;d['detailed_rider_preview']=a.detailed_rider
if a.detailed_rider:
 d['detailed_assets_loaded']='DetailedRiderPreview: body=m_tal_nrw_body outfit_parts=4' in (out/'run.log').read_text()
 d['passed']=bool(d['passed'] and d['detailed_assets_loaded'])
if a.grip:
 rows=re.findall(r'BattleGripReach: steer=([-0-9.]+) side=(-?1) error_cm=([0-9.]+)',(out/'run.log').read_text())
 d['wrist_reach']=[{'steer':float(t),'side':int(side),'error_cm':float(error)} for t,side,error in rows]
 d['passed']=bool(d['passed'] and len(rows)==6 and {float(t) for t,_,_ in rows}=={-1,0,1} and all(float(error)<1 for _,_,error in rows))

d['scope']='Opening camera/story UI, unchanged tutorial clock/player location, input/camera return. No audio, final scenery or animation acceptance.'
report_name=a.report or f'2026-09-11-build042-opening-{mode}.json'
assert Path(report_name).name==report_name
(root/'Tests/Results'/report_name).write_text(json.dumps(d,indent=2)+'\n');print(json.dumps(d),flush=True);raise SystemExit(0 if d['passed'] else 1)
