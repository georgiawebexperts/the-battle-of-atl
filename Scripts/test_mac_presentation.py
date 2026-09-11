"""Capture the cooked game's bike/foot HUD for human visual inspection.

A successful capture is not visual acceptance, animation acceptance or FPS proof.
"""
import argparse,json,pathlib,subprocess,struct,hashlib,plistlib,shutil,uuid,re
root=pathlib.Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--width',type=int,choices=[1280,1920],default=1920);p.add_argument('--build',default='027');p.add_argument('--rig',action='store_true');args=p.parse_args();height=args.width*9//16
assert re.fullmatch(r'[0-9]{3}',args.build)
out=root/'work'/f'build{args.build}-packaged-{args.width}';out.mkdir(parents=True,exist_ok=True)
bundle=root/'Saved/StagedBuilds/Mac/AuraPlayground.app'
app=bundle/'Contents/MacOS/AuraPlayground'
bundle_id=plistlib.loads((bundle/'Contents/Info.plist').read_bytes())['CFBundleIdentifier']
# Staged builds retain Unreal's app-sandbox entitlement: write inside their own container.
capture=pathlib.Path.home()/'Library/Containers'/bundle_id/'Data/Documents/BattleHUDReview'/str(args.width)/uuid.uuid4().hex
capture.mkdir(parents=True,exist_ok=True)
with (out/'render.log').open('w') as log:
 run=subprocess.run([str(app),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-windowed',f'-ResX={args.width}',f'-ResY={height}','-ForceRes','-NoTextureStreaming','-unattended','-nosound','-BattleHUDReview',*(['-BattleRigReview'] if args.rig else []),'-ExecCmds=r.ScreenshotDelegate 0',f'-BattleHUDReviewDir={capture}','-stdout'],stdout=log,stderr=subprocess.STDOUT,timeout=180)
images=[]
for name in ['bike.png','foot.png']+(['aim.png','reload.png'] if args.rig else []):
 path=out/name;shutil.copy2(capture/name,path);data=path.read_bytes();assert data[:8]==b'\x89PNG\r\n\x1a\n';size=struct.unpack('>II',data[16:24]);assert size==(args.width,height),(name,size)
 images.append({'file':str(path.relative_to(root)),'size':size,'sha256':hashlib.sha256(data).hexdigest()})
text=(out/'render.log').read_text();assert 'HUDReview: requested bike and foot captures' in text and run.returncode==0
if args.rig:
 assert 'RigReview: reload_and_remount=1' in text
 assert 'RigReview: grip unreachable' not in text and 'RigReview: fire failed' not in text
 assert len(re.findall(r'RigReview: .* wrist_error',text))==4
report={'captures_passed':True,'exit_code':run.returncode,'images':images,'visual_review':'pending','scope':'Native cooked Mac game, controlled bike-to-foot fixture; audio disabled; no physical input or FPS acceptance.'}
(root/'Tests/Results'/f'2026-09-11-build{args.build}-presentation-{args.width}.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report))
