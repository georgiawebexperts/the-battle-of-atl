"""Capture six deterministic crowd outfits through native player cameras."""
import json,pathlib,subprocess,uuid,argparse,plistlib,shutil
root=pathlib.Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--report',default='2026-09-14-crowd-colors.json');p.add_argument('--packaged',action='store_true');a=p.parse_args();assert pathlib.Path(a.report).name==a.report
out=root/'work'/('crowd-outfits-'+uuid.uuid4().hex);out.mkdir();capture=out
if a.packaged:
 bundle=root/'Saved/StagedBuilds/Mac/AuraPlayground.app';bid=plistlib.loads((bundle/'Contents/Info.plist').read_bytes())['CFBundleIdentifier'];capture=pathlib.Path.home()/'Library/Containers'/bid/'Data/Documents/CrowdReview'/uuid.uuid4().hex;capture.mkdir(parents=True);entry=[str(bundle/'Contents/MacOS/AuraPlayground')]
else:entry=['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject')]
command=entry+['/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-RenderOffscreen','-BattleSkipTutorial','-BattleHUDReview','-BattleCrowdVarietyReview',f'-BattleHUDReviewDir={capture}','-RCWebControlDisable','-unattended','-nosound','-ResX=1280','-ResY=720','-windowed','-stdout']
with (out/'run.log').open('w') as f:r=subprocess.run(command,stdout=f,stderr=subprocess.STDOUT,timeout=120)
images=[]
for name in ['bike.png','foot.png']:
 if capture!=out and (capture/name).exists():shutil.copy2(capture/name,out/name)
 images.append(str(out/name))
report={'passed':r.returncode==0 and all(pathlib.Path(i).exists() for i in images),'packaged':a.packaged,'exit_code':r.returncode,'images':images,'log':str(out/'run.log'),'visual_review':False,'scope':'Six deterministic stationary crowd outfits; images require review, no full animation or combat acceptance.'}
(root/'Tests/Results'/a.report).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report));raise SystemExit(0 if report['passed'] else 1)
