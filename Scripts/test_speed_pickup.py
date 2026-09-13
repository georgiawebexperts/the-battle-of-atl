"""Check timed boost collection, acceleration, expiry and mounted-only guards."""
import argparse,json,re,subprocess,uuid,plistlib,shutil
from pathlib import Path
root=Path(__file__).resolve().parents[1];p=argparse.ArgumentParser();p.add_argument('--app',type=Path);p.add_argument('--review',action='store_true');p.add_argument('--report',required=True);a=p.parse_args();assert Path(a.report).name==a.report
out=root/'work'/('speed-pickup-'+uuid.uuid4().hex);out.mkdir();capture=out
if a.app and a.review:
 bid=plistlib.loads((a.app.parent.parent/'Info.plist').read_bytes())['CFBundleIdentifier'];capture=Path.home()/'Library/Containers'/bid/'Data/Documents/SpeedReview'/out.name;capture.mkdir(parents=True)
entry=[str(a.app)] if a.app else ['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject')]
log=out/'run.log'
with log.open('w') as f:
 r=subprocess.run(entry+['/Game/BattleForTheA/Maps/ArcadeBikeLab?game=/Script/AuraPlayground.BattleParkMode?Difficulty=Easy?AutoStart=1','-game','-RenderOffscreen' if a.review else '-nullrhi','-BattleSpeedAudit','-BattleSkipTutorial','-unattended','-nosound','-RCWebControlDisable','-stdout','-ResX=1280','-ResY=720','-windowed']+(['-BattleSpeedReviewDir='+str(capture)] if a.review else []),stdout=f,stderr=subprocess.STDOUT,timeout=100)
if capture!=out:
 for x in capture.glob('*.png'):shutil.copy2(x,out/x.name)
rows=re.findall(r'SpeedPickupAudit: (\{[^\n]+\})',log.read_text());d=json.loads(rows[-1]) if rows else {'passed':False,'missing_report':True};d.update(exit_code=r.returncode,images=[str(x) for x in out.glob('*.png')],log=str(log),packaged=bool(a.app));d['passed']=d['passed'] and r.returncode==0 and (not a.review or len(d['images'])==2);(root/'Tests/Results'/a.report).write_text(json.dumps(d,indent=2)+'\n');print(json.dumps(d));raise SystemExit(0 if d['passed'] else 1)
