"""Verify the location-specific guitarist and saxophonist, motion, audio and reactions."""
import argparse,json,plistlib,re,subprocess,uuid,shutil,struct
from pathlib import Path
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--editor',action='store_true');p.add_argument('--review',action='store_true');p.add_argument('--report',required=True);a=p.parse_args()
staged=root/'Saved/StagedBuilds/Mac/AuraPlayground.app';exe=staged/'Contents/MacOS/AuraPlayground'
entry=['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject')] if a.editor else [str(exe)]
capture=root/f'work/musician-review-{uuid.uuid4().hex}'
if a.review and not a.editor:
 bid=plistlib.loads((staged/'Contents/Info.plist').read_bytes())['CFBundleIdentifier'];capture=Path.home()/'Library/Containers'/bid/'Data/Documents/BattleMusicianReview'/uuid.uuid4().hex
capture.mkdir(parents=True)
flags=['-RenderOffscreen','-windowed','-ResX=1280','-ResY=720','-ForceRes',f'-BattleMusicianReviewDir={capture}'] if a.review else ['-nullrhi']
log=root/f'work/musician-{uuid.uuid4().hex}.log'
with log.open('w') as stream:run=subprocess.run(entry+['/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-RCWebControlDisable',*flags,'-unattended','-nosound','-BattleSkipTutorial','-BattleMusicianAudit','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=120)
m=re.findall(r'BattleMusicianAudit: (\{[^\n]+\})',log.read_text());result=json.loads(m[-1]) if m else {'passed':False,'missing_report':True}
result.update(exit_code=run.returncode,editor=a.editor,visual_review=False,log=str(log),images=[])
if a.review:
 out=root/'work/build088-musician-review';out.mkdir(exist_ok=True);src=capture/'park-guitarist.png';dest=out/'park-guitarist.png'
 if src.is_file():shutil.copy2(src,dest);result['images']=[str(dest.relative_to(root))];result['capture_size']=list(struct.unpack('>II',dest.read_bytes()[16:24]))
result['passed']=bool(result.get('passed') and run.returncode==0 and (not a.review or len(result['images'])==1))
Path(a.report).write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result));raise SystemExit(0 if result['passed'] else 1)
