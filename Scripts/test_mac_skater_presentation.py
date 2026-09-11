"""Capture a short native skating pose sequence; no gameplay/FPS acceptance."""
import json,re,subprocess,uuid,plistlib,shutil
from pathlib import Path
root=Path(__file__).resolve().parents[1];bundle=root/'Saved/StagedBuilds/Mac/AuraPlayground.app';app=bundle/'Contents/MacOS/AuraPlayground';bid=plistlib.loads((bundle/'Contents/Info.plist').read_bytes())['CFBundleIdentifier'];capture=Path.home()/'Library/Containers'/bid/'Data/Documents/SkaterReview'/uuid.uuid4().hex;capture.mkdir(parents=True)
out=root/'work/build041-skater-poses';out.mkdir(exist_ok=True)
with (out/'render.log').open('w') as f:
 r=subprocess.run([str(app),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-windowed','-ResX=1280','-ResY=720','-ForceRes','-NoTextureStreaming','-unattended','-nosound','-BattleHUDReview','-BattleSkaterReview','-ExecCmds=r.ScreenshotDelegate 0',f'-BattleHUDReviewDir={capture}','-stdout'],stdout=f,stderr=subprocess.STDOUT,timeout=120)
assert r.returncode==0 and 'SkaterReview: frames=24' in (out/'render.log').read_text()
for i in range(24):shutil.copy2(capture/f'skater-{i:03}.png',out/f'skater-{i:03}.png')
print(json.dumps({'frames':24,'directory':str(out),'visual_review':'pending'}))
