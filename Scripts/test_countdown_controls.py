"""Exercise gate countdown keys, timer start and start-banner captures."""
import argparse,json,re,subprocess,uuid,plistlib,shutil
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--app',type=Path);p.add_argument('--report',required=True);p.add_argument('--review',action='store_true');a=p.parse_args();assert Path(a.report).name==a.report
root=Path(__file__).resolve().parents[1];out=root/'work'/('countdown-'+uuid.uuid4().hex);out.mkdir();capture=out
entry=[str(a.app)] if a.app else ['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject')]
if a.app and a.review:
 bid=plistlib.loads((a.app.parent.parent/'Info.plist').read_bytes())['CFBundleIdentifier'];capture=Path.home()/'Library/Containers'/bid/'Data/Documents/CountdownReview'/out.name;capture.mkdir(parents=True)
flags=['-RenderOffscreen','-windowed','-ResX=1280','-ResY=720','-ForceRes',f'-BattleCountdownReviewDir={capture}'] if a.review else ['-nullrhi']
with (out/'run.log').open('w') as f:
 run=subprocess.run(entry+['/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-unattended','-nosound','-RCWebControlDisable','-BattleTutorialAudit','-BattleCountdownControlsAudit','-ExecCmds=r.ScreenshotDelegate 0,t.IdleWhenNotForeground 0','-stdout',*flags],stdout=f,stderr=subprocess.STDOUT,timeout=100)
text=(out/'run.log').read_text();rows=re.findall(r'BattleTutorialAudit: (\{[^\n]+\})',text);r=json.loads(rows[-1]) if rows else {'passed':False,'missing_report':True}
if capture!=out:
 for name in ['countdown.png','go.png']:
  if (capture/name).is_file():shutil.copy2(capture/name,out/name)
r.update(exit_code=run.returncode,packaged=bool(a.app),scope='Gate fixture: real pedal/steer/brake keys during countdown, countdown expiry and no repeat-gate reset. Full approach and visual quality require separate review.',log=str(out/'run.log'),images=[str(out/name) for name in ['countdown.png','go.png']] if a.review else [])
r['passed']=r['passed'] and run.returncode==0 and 'CountdownControls: pedal, steering and brake stay active' in text and all(Path(x).is_file() for x in r['images']);(root/'Tests/Results'/a.report).write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r));raise SystemExit(0 if r['passed'] else 1)
