"""Verify the difficulty-scaled park dance circle, animation, speaker and music."""
import argparse,json,plistlib,re,subprocess,uuid,shutil,struct
from pathlib import Path
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--editor',action='store_true');p.add_argument('--review',action='store_true');p.add_argument('--difficulty',choices=['Easy','Medium','Hard'],default='Easy');p.add_argument('--report',required=True);a=p.parse_args()
staged=root/'Saved/StagedBuilds/Mac/AuraPlayground.app';exe=staged/'Contents/MacOS/AuraPlayground'
entry=['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject')] if a.editor else [str(exe)]
capture=root/f'work/dance-review-{uuid.uuid4().hex}'
if a.review and not a.editor:
 bid=plistlib.loads((staged/'Contents/Info.plist').read_bytes())['CFBundleIdentifier'];capture=Path.home()/'Library/Containers'/bid/'Data/Documents/BattleDanceReview'/uuid.uuid4().hex
capture.mkdir(parents=True)
flags=['-RenderOffscreen','-windowed','-ResX=1280','-ResY=720','-ForceRes',f'-BattleDanceReviewDir={capture}'] if a.review else ['-nullrhi']
log=root/f'work/dance-{uuid.uuid4().hex}.log'
with log.open('w') as stream:run=subprocess.run(entry+[f'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty={a.difficulty}?AutoStart=1','-game','-RCWebControlDisable',*flags,'-unattended','-nosound','-BattleSkipTutorial','-BattleDanceAudit','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=120)
m=re.findall(r'BattleDanceAudit: (\{[^\n]+\})',log.read_text());result=json.loads(m[-1]) if m else {'passed':False,'missing_report':True}
result.update(exit_code=run.returncode,difficulty=a.difficulty,editor=a.editor,visual_review=False,log=str(log),images=[])
if a.review:
 out=root/'work/build087-dance-review';out.mkdir(exist_ok=True)
 for phase in ['a','b']:
  src=capture/f'dance-circle-{phase}.png';dest=out/f'{a.difficulty.lower()}-dance-circle-{phase}.png'
  if src.is_file():shutil.copy2(src,dest);result['images'].append(str(dest.relative_to(root)))
 if result['images']:result['capture_size']=list(struct.unpack('>II',(root/result['images'][0]).read_bytes()[16:24]))
result['passed']=bool(result.get('passed') and run.returncode==0 and (not a.review or len(result['images'])==2))
Path(a.report).write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result));raise SystemExit(0 if result['passed'] else 1)
