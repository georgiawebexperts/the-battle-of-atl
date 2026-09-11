"""Capture the cooked game's bike/foot HUD for human visual inspection.

A successful capture is not visual acceptance, animation acceptance or FPS proof.
"""
import argparse,json,pathlib,subprocess,struct,hashlib,plistlib,shutil
root=pathlib.Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--width',type=int,choices=[1280,1920],default=1920);args=p.parse_args();height=args.width*9//16
out=root/'work'/f'build027-packaged-{args.width}';out.mkdir(parents=True,exist_ok=True)
bundle=root/'Saved/StagedBuilds/Mac/AuraPlayground.app'
app=bundle/'Contents/MacOS/AuraPlayground'
bundle_id=plistlib.loads((bundle/'Contents/Info.plist').read_bytes())['CFBundleIdentifier']
# Staged builds retain Unreal's app-sandbox entitlement: write inside their own container.
capture=pathlib.Path.home()/'Library/Containers'/bundle_id/'Data/Documents/BattleHUDReview'/str(args.width)
capture.mkdir(parents=True,exist_ok=True)
with (out/'render.log').open('w') as log:
 run=subprocess.run([str(app),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-windowed',f'-ResX={args.width}',f'-ResY={height}','-ForceRes','-NoTextureStreaming','-unattended','-nosound','-BattleHUDReview','-ExecCmds=r.ScreenshotDelegate 0',f'-BattleHUDReviewDir={capture}','-stdout'],stdout=log,stderr=subprocess.STDOUT,timeout=180)
images=[]
for name in ['bike.png','foot.png']:
 path=out/name;shutil.copy2(capture/name,path);data=path.read_bytes();assert data[:8]==b'\x89PNG\r\n\x1a\n';size=struct.unpack('>II',data[16:24]);assert size==(args.width,height),(name,size)
 images.append({'file':str(path.relative_to(root)),'size':size,'sha256':hashlib.sha256(data).hexdigest()})
text=(out/'render.log').read_text();assert 'HUDReview: requested bike and foot captures' in text and run.returncode==0
report={'captures_passed':True,'exit_code':run.returncode,'images':images,'visual_review':'pending','scope':'Native cooked Mac game, controlled bike-to-foot fixture; audio disabled; no physical input or FPS acceptance.'}
(root/'Tests/Results'/f'2026-09-11-build027-presentation-{args.width}.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report))
