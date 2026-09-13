"""Check pistol ammunition conservation through shots, pickups, and possession."""
import argparse,json,re,subprocess,uuid,plistlib
from pathlib import Path
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser()
p.add_argument('--difficulty',choices=['Easy','Medium','Hard'],default='Easy')
p.add_argument('--editor',action='store_true')
p.add_argument('--bins',action='store_true')
p.add_argument('--review',action='store_true')
p.add_argument('--practice',action='store_true')
p.add_argument('--report',required=True)
a=p.parse_args()
app=root/'Saved/StagedBuilds/Mac/AuraPlayground.app/Contents/MacOS/AuraPlayground'
entry=['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject')] if a.editor else [str(app)]
log=root/f'work/ammo-{uuid.uuid4().hex}.log'
capture=root/'work'/('ammo-bin-'+uuid.uuid4().hex)
if not a.editor:
 bid=plistlib.loads((root/'Saved/StagedBuilds/Mac/AuraPlayground.app/Contents/Info.plist').read_bytes())['CFBundleIdentifier']
 capture=Path.home()/'Library/Containers'/bid/'Data/Documents/BattleAmmoReview'/uuid.uuid4().hex
extra=['-BattleAmmoBinAudit'] if a.bins else []
if not a.practice:extra+=['-BattleSkipTutorial']
if a.review:
 capture.mkdir(parents=True);extra+=['-RenderOffscreen','-windowed','-ResX=1280','-ResY=720','-ForceRes',f'-BattleHUDReviewDir={capture}']
else:extra+=['-nullrhi']
with log.open('w') as stream:
 run=subprocess.run(entry+[f'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty={a.difficulty}?AutoStart=1','-game','-RCWebControlDisable','-unattended','-nosound','-BattleAmmoAudit','-stdout']+extra,stdout=stream,stderr=subprocess.STDOUT,timeout=120)
matches=re.findall(r'BattleAmmoAudit: (\{[^\n]+\})',log.read_text())
result=json.loads(matches[-1]) if matches else {'passed':False,'missing_report':True}
result.update(exit_code=run.returncode,difficulty=a.difficulty,visual_review=False,executable=entry[0],log=str(log))
result.update(bin_layout=a.bins,practice=a.practice,images=[str(capture/'ammo-bin.png')] if a.review and a.bins else [])
result['passed']=bool(result.get('passed') and run.returncode==0)
Path(a.report).write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps(result))
if not result['passed']:raise SystemExit(1)
