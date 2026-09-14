"""Run native music key/persistence or spare-bike exchange checks."""
import argparse,json,re,subprocess,uuid,plistlib,shutil
from pathlib import Path
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('kind',choices=['music','bikes','balance']);p.add_argument('--review',action='store_true');p.add_argument('--saddle-close',action='store_true');p.add_argument('--aerial',action='store_true');p.add_argument('--packaged',action='store_true');p.add_argument('--report',required=True);a=p.parse_args()
out=root/'work'/('feature-'+a.kind+'-'+uuid.uuid4().hex);out.mkdir();log=out/'run.log'
capture=out
if a.packaged and a.review:
 bid=plistlib.loads((root/'Saved/StagedBuilds/Mac/AuraPlayground.app/Contents/Info.plist').read_bytes())['CFBundleIdentifier'];capture=Path.home()/'Library/Containers'/bid/'Data/Documents/SpareBikeReview'/uuid.uuid4().hex;capture.mkdir(parents=True)
entry=[str(root/'Saved/StagedBuilds/Mac/AuraPlayground.app/Contents/MacOS/AuraPlayground')] if a.packaged else ['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject')]
flag={'music':'BattleMusicAudit','bikes':'BattleSpareBikeAudit','balance':'BattleBikeBalanceAudit'}[a.kind];label={'music':'BattleMusicAudit','bikes':'SpareBikeAudit','balance':'BikeBalanceAudit'}[a.kind]
with log.open('w') as f:
 r=subprocess.run(entry+(['-BattleSaddleCloseReview'] if a.saddle_close else [])+(['-BattleAerialWaterReview'] if a.aerial else [])+['/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game',('-RenderOffscreen' if a.review else '-nullrhi'),'-BattleSkipTutorial','-'+flag,'-RCWebControlDisable','-unattended','-stdout']+(['-BattleSpareBikeReviewDir='+str(capture),'-BattleBalanceReviewDir='+str(capture),'-ResX=1280','-ResY=720','-windowed'] if a.review else [])+(['-BattleSteeringAudit'] if a.kind=='balance' else [])+([] if a.kind=='music' else ['-nosound']),stdout=f,stderr=subprocess.STDOUT,timeout=90)
if capture!=out:
 for item in capture.glob('*.png'):shutil.copy2(item,out/item.name)
m=re.findall(label+r': (\{[^\n]+\})',log.read_text());d=json.loads(m[-1]) if m else {'passed':False,'missing_report':True};d.update(images=[str(x) for x in out.glob('*.png')],log=str(log),exit_code=r.returncode,packaged=a.packaged);d['passed']=bool(d['passed'] and r.returncode==0 and (not a.review or bool(d['images'])));(root/'Tests/Results'/a.report).write_text(json.dumps(d,indent=2)+'\n');print(json.dumps(d));raise SystemExit(0 if d['passed'] else 1)
