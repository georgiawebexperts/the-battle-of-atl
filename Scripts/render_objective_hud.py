"""Capture objective, danger and drone targeting together."""
import argparse,json,pathlib,plistlib,shutil,subprocess,uuid
root=pathlib.Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--packaged',action='store_true');p.add_argument('--report',required=True);args=p.parse_args();assert pathlib.Path(args.report).name==args.report
out=root/'work'/('objective-review-'+uuid.uuid4().hex);out.mkdir();capture=out
if args.packaged:
 bundle=root/'Saved/StagedBuilds/Mac/AuraPlayground.app';bid=plistlib.loads((bundle/'Contents/Info.plist').read_bytes())['CFBundleIdentifier'];capture=pathlib.Path.home()/'Library/Containers'/bid/'Data/Documents/ObjectiveReview'/uuid.uuid4().hex;capture.mkdir(parents=True);entry=[str(bundle/'Contents/MacOS/AuraPlayground')]
else:entry=['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject')]
with (out/'run.log').open('w') as f:
 r=subprocess.run(entry+['/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-RenderOffscreen','-BattleSkipTutorial','-BattleHUDReview','-BattleObjectiveReview','-BattleDroneReview','-BattleDroneAimReview',f'-BattleHUDReviewDir={capture}','-RCWebControlDisable','-unattended','-nosound','-ResX=1280','-ResY=720','-windowed','-stdout'],stdout=f,stderr=subprocess.STDOUT,timeout=120)
images=[]
for name in ['bike.png','foot.png']:
 if capture!=out and (capture/name).exists():shutil.copy2(capture/name,out/name)
 images.append(str(out/name))
result={'passed':r.returncode==0 and 'HUDReview: requested bike and foot captures' in (out/'run.log').read_text() and all(pathlib.Path(i).exists() for i in images),'packaged':args.packaged,'exit_code':r.returncode,'images':images,'log':str(out/'run.log'),'visual_review':False,'scope':'Forced presentation states only: phone objectives, danger and drone aim. Images require visual review; no combat balance acceptance.'}
(root/'Tests/Results'/args.report).write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result));raise SystemExit(0 if result['passed'] else 1)
