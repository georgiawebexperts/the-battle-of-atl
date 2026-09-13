"""Check police pursuit, taser recovery, panic, and alert expiry."""
import argparse,json,re,subprocess,uuid,plistlib
from pathlib import Path
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser()
p.add_argument('--difficulty',choices=['Easy','Medium','Hard'],default='Easy')
p.add_argument('--editor',action='store_true')
p.add_argument('--review',action='store_true',help='Render offscreen warning and discharge images (editor only)')
p.add_argument('--report',required=True)
p.add_argument('--closeup',action='store_true',help='Inspect officer without HUD overlays')
a=p.parse_args()
if a.closeup and not a.review:p.error('--closeup requires --review')
app=root/'Saved/StagedBuilds/Mac/AuraPlayground.app/Contents/MacOS/AuraPlayground'
entry=['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject')] if a.editor else [str(app)]
log=root/f'work/trouble-{uuid.uuid4().hex}.log'
capture=root/f'work/police-review-{uuid.uuid4().hex}'
flags=['-nullrhi']
if a.review:
 if not a.editor:
  bid=plistlib.loads((app.parents[1]/'Info.plist').read_bytes())['CFBundleIdentifier']
  capture=Path.home()/'Library/Containers'/bid/'Data/Documents/BattlePoliceReview'/uuid.uuid4().hex
 capture.mkdir(parents=True)
 flags=['-RenderOffscreen','-windowed','-ResX=1280','-ResY=720','-ForceRes',f'-BattlePoliceReviewDir={capture}','-ini:Engine:[ConsoleVariables]:r.ShaderCompiler.JobCacheDDC=0']
if a.closeup:flags+=['-BattlePoliceCloseup']
with log.open('w') as stream:
 run=subprocess.run(entry+[f'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty={a.difficulty}?AutoStart=1','-game','-RCWebControlDisable',*flags,'-unattended','-nosound','-BattleSkipTutorial','-BattleTroubleAudit','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=180 if a.review else 120)
matches=re.findall(r'BattleTroubleAudit: (\{[^\n]+\})',log.read_text())
result=json.loads(matches[-1]) if matches else {'passed':False,'missing_report':True}
result.update(exit_code=run.returncode,difficulty=a.difficulty,visual_review=False,executable=entry[0],log=str(log))
result['images']=[str(x) for x in capture.glob('*.png')] if a.review else []
result['capture_complete']=not a.review or all((capture/name).is_file() for name in ['police-warning.png','police-discharge.png'])
result['passed']=bool(result.get('passed') and run.returncode==0 and result['capture_complete'])
Path(a.report).write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps(result))
if not result['passed']:raise SystemExit(1)
