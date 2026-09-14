"""Fire real player shots and observe live crowd flight and recovery."""
import argparse,json,plistlib,re,shutil,subprocess,uuid
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--packaged',action='store_true');p.add_argument('--headless',action='store_true');p.add_argument('--report',required=True);a=p.parse_args()
root=Path(__file__).resolve().parents[1]
bundle=root/'Saved/StagedBuilds/Mac/AuraPlayground.app'
out=root/'work'/('crowd-panic-'+uuid.uuid4().hex);out.mkdir(parents=True)
capture=out
if a.packaged:
 bid=plistlib.loads((bundle/'Contents/Info.plist').read_bytes())['CFBundleIdentifier']
 capture=Path.home()/'Library/Containers'/bid/'Data/Documents/CrowdPanic'/out.name;capture.mkdir(parents=True)
entry=[str(bundle/'Contents/MacOS/AuraPlayground')] if a.packaged else ['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject')]
flags=['-nullrhi'] if a.headless else ['-RenderOffscreen','-windowed','-ResX=1280','-ResY=720',f'-BattlePanicReviewDir={capture}']
with (out/'run.log').open('w') as log:
 try:
  code=subprocess.run(entry+['/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-BattleSkipTutorial','-BattlePanicAudit','-RCWebControlDisable','-unattended','-nosound','-stdout',*flags],stdout=log,stderr=subprocess.STDOUT,timeout=150).returncode
 except subprocess.TimeoutExpired:code='timeout'
markers=re.findall(r'BattlePanicAudit: (\{[^\n]+\})',(out/'run.log').read_text())
r=json.loads(markers[-1]) if markers else {'passed':False,'missing_report':True}
images=[]
for name in ['panic-running.png','panic-away.png','panic-recovered.png']:
 source=capture/name
 if source.exists():
  if source!=out/name:shutil.copy2(source,out/name)
  images.append(str(out/name))
r.update(exit_code=code,packaged=a.packaged,headless=a.headless,log=str(out/'run.log'),images=images,visual_review=False)
r['passed']=bool(r['passed'] and code==0 and (a.headless or len(images)==3))
(root/'Tests/Results'/a.report).write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r))
raise SystemExit(0 if r['passed'] else 1)
