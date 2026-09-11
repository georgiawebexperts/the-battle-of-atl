"""Render the real winning menu after the native finish integration audit."""
import json,plistlib,subprocess,re,uuid,shutil
from pathlib import Path
root=Path(__file__).resolve().parents[1];bundle=root/'Saved/StagedBuilds/Mac/AuraPlayground.app';app=bundle/'Contents/MacOS/AuraPlayground'
bundle_id=plistlib.loads((bundle/'Contents/Info.plist').read_bytes())['CFBundleIdentifier']
capture=Path.home()/'Library/Containers'/bundle_id/'Data/Documents/BattleFinishReview'/uuid.uuid4().hex;capture.mkdir(parents=True)
out=root/'work/build042-win';out.mkdir(exist_ok=True)
with (out/'render.log').open('w') as log:
 r=subprocess.run([str(app),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-windowed','-ResX=1280','-ResY=720','-ForceRes','-unattended','-nosound','-BattleFinishAudit','-BattleFinishReview',f'-BattleHUDReviewDir={capture}','-ExecCmds=r.ScreenshotDelegate 0','-stdout'],stdout=log,stderr=subprocess.STDOUT,timeout=90)
m=re.findall(r'BattleFinishAudit: (\{[^\n]+\})',(out/'render.log').read_text());data=json.loads(m[-1]) if m else {'passed':False};assert data['passed'] and r.returncode==0,data
shutil.copy2(capture/'win.png',out/'win.png');data.update(capture=str((out/'win.png').relative_to(root)),visual_review='pending',scope='Native winning menu after real Artifact/checkpoint triggers and gate guard tests; not full route driving or FPS acceptance.')
(root/'Tests/Results/2026-09-11-build042-win-presentation.json').write_text(json.dumps(data,indent=2)+'\n');print(json.dumps(data))
