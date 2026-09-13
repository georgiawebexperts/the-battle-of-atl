"""Native hands-free dismount, deliberate draw and on-foot mobility acceptance."""
import argparse,json,re,subprocess,plistlib,uuid,shutil,struct
from pathlib import Path
root=Path(__file__).resolve().parents[1];p=argparse.ArgumentParser();p.add_argument('--review',action='store_true');p.add_argument('--editor',action='store_true');p.add_argument('--detailed-rider',action='store_true');p.add_argument('--report');p.add_argument('--body',action='store_true');p.add_argument('--landing-run',action='store_true');a=p.parse_args();mode='review' if a.review else 'audit'
bundle=root/'Saved/StagedBuilds/Mac/AuraPlayground.app';app=bundle/'Contents/MacOS/AuraPlayground';bid=plistlib.loads((bundle/'Contents/Info.plist').read_bytes())['CFBundleIdentifier']
capture=Path.home()/'Library/Containers'/bid/'Data/Documents/BattleFootReview'/uuid.uuid4().hex;capture.mkdir(parents=True)
out=root/'work'/f'build042-foot-{mode}';out.mkdir(exist_ok=True)
flags=['-RenderOffscreen','-windowed','-ResX=1280','-ResY=720','-ForceRes','-BattleFootReview',f'-BattleHUDReviewDir={capture}'] if a.review else ['-nullrhi']
if a.detailed_rider:
 assert a.editor
 flags+=['-BattleDetailedRider']
if a.body:
 assert a.editor and a.detailed_rider and a.review
 flags+=['-BattleFootBodyReview']
if a.landing_run:
 assert a.editor and a.detailed_rider
 flags+=['-BattleLandingRunReview']
entry=['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject'),'-game','-RCWebControlDisable'] if a.editor else [str(app)]
with (out/'run.log').open('w') as log:
 r=subprocess.run([*entry,'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1',*flags,'-BattleFootAudit','-BattleSkipTutorial','-unattended','-nosound','-ExecCmds=r.ScreenshotDelegate 0,t.IdleWhenNotForeground 0','-stdout'],stdout=log,stderr=subprocess.STDOUT,timeout=90)
m=re.findall(r'BattleFootAudit: (\{[^\n]+\})',(out/'run.log').read_text());d=json.loads(m[-1]) if m else {'passed':False,'missing_report':True};d['exit_code']=r.returncode;d['passed']=bool(d.get('passed') and r.returncode==0)
if a.review and d['passed']:
 d['images']=[]
 for name in ['idle','drawn','aimed','reload','walk-a','walk-b','run','jump','land-contact','land-settle','crouch']:
  dest=out/f'{name}.png';shutil.copy2(capture/dest.name,dest);assert struct.unpack('>II',dest.read_bytes()[16:24])==(1280,720);d['images'].append(str(dest.relative_to(root)))
 d['visual_review']='pending'
d['editor']=a.editor;d['detailed_rider_preview']=a.detailed_rider;d['full_body_review']=a.body
if a.detailed_rider:
 d['detailed_arms_loaded']='DetailedArmsPreview: hands=SK_DetailedHands sleeves=SK_DetailedSleeves' in (out/'run.log').read_text()
 d['detailed_body_loaded']='DetailedFootPreview: body=m_tal_nrw_body outfit_parts=4 clips=6' in (out/'run.log').read_text()
 d['detailed_pistol_loaded']='DetailedM1911: loaded' in (out/'run.log').read_text()
 d['landing_clips_loaded']='DetailedLanding: loaded=2' in (out/'run.log').read_text()
 d['landing_selected']=re.findall(r'DetailedLanding: selected=(\S+)',(out/'run.log').read_text())
 d['landing_run_requested']=a.landing_run
 d['landing_matches_request']=any(('Land_Run_' if a.landing_run else 'Land_Stand_') in name for name in d['landing_selected'])
 d['pose_transitions']=re.findall(r'DetailedFootTransition: (begin|complete)=(airborne|landing)',(out/'run.log').read_text())
 d['pose_transitions_completed']=all(['complete',phase] in [list(v) for v in d['pose_transitions']] for phase in ['airborne','landing'])
 d['passed']=bool(d['passed'] and d['detailed_arms_loaded'] and d['detailed_body_loaded'] and d['pose_transitions_completed'] and d['landing_clips_loaded'] and d['landing_matches_request'] and d['detailed_pistol_loaded'])
report=a.report or f'2026-09-11-build042-foot-{mode}.json';assert Path(report).name==report
(root/'Tests/Results'/report).write_text(json.dumps(d,indent=2)+'\n');print(json.dumps(d),flush=True);raise SystemExit(0 if d['passed'] else 1)
