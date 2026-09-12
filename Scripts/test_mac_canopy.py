"""Inspect installed cooked canopy at three fixed views; not full-game FPS acceptance."""
import json,pathlib,plistlib,re,shutil,subprocess,uuid
root=pathlib.Path(__file__).resolve().parents[1]
bundle=root/'Saved/StagedBuilds/Mac/AuraPlayground.app'
out=root/'work/canopy-packaged'/uuid.uuid4().hex;out.mkdir(parents=True)
bundle_id=plistlib.loads((bundle/'Contents/Info.plist').read_bytes())['CFBundleIdentifier']
capture=pathlib.Path.home()/'Library/Containers'/bundle_id/'Data/Documents/BattleCanopyReview'/out.name;capture.mkdir(parents=True)
with (out/'run.log').open('w') as log:
 result=subprocess.run([str(bundle/'Contents/MacOS/AuraPlayground'),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-RenderOffscreen','-BattleSkipTutorial','-BattleCanopyReview',f'-BattleHUDReviewDir={capture}','-windowed','-ResX=1920','-ResY=1080','-ForceRes','-unattended','-nosound','-ExecCmds=r.ScreenshotDelegate 0,t.IdleWhenNotForeground 0,t.MaxFPS 0,r.VSync 0','-stdout'],stdout=log,stderr=subprocess.STDOUT,timeout=180)
for picture in capture.glob('*.png'):shutil.copy2(picture,out/picture.name)
text=(out/'run.log').read_text();frames=[json.loads(x) for x in re.findall(r'CanopyFrame: (\{[^\n]+\})',text)];images=[out/f'canopy-{i}.png' for i in (1,2,3)]
r={'passed':result.returncode==0 and len(frames)==3 and all(p.is_file() for p in images) and 'CanopyRuntime: complete' in text,'exit_code':result.returncode,'frames':frames,'images':[str(p) for p in images],'scope':'Cooked main map as installed; no transient asset replacement. Three fixed cameras at1920x1080,5s warmup and4s frame samples; no sustained riding/combat/audio or full-game FPS acceptance.'}
(root/'Tests/Results/2026-09-12-build049-canopy.json').write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r));raise SystemExit(0 if r['passed'] else 1)
