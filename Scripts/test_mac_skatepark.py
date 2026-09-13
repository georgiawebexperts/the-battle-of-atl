"""Ride actual skatepark access, launch bank and bowl; verify time rewards."""
import json,re,subprocess,argparse,uuid,plistlib
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--editor',action='store_true');p.add_argument('--real',action='store_true');p.add_argument('--review',action='store_true');p.add_argument('--report',required=True);a=p.parse_args()
root=Path(__file__).resolve().parents[1];bundle=root/'Saved/StagedBuilds/Mac/AuraPlayground.app'
entry=['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject')] if a.editor else [str(bundle/'Contents/MacOS/AuraPlayground')]
out=root/'work'/('skate-'+uuid.uuid4().hex);out.mkdir();capture=out
if not a.editor:
 bid=plistlib.loads((bundle/'Contents/Info.plist').read_bytes())['CFBundleIdentifier'];capture=Path.home()/'Library/Containers'/bid/'Data/Documents/BattleSkateReview'/uuid.uuid4().hex
extra=['-BattleSkateReal'] if a.real else []
if a.review:
 capture.mkdir(parents=True,exist_ok=True);extra+=['-RenderOffscreen','-windowed','-ResX=1280','-ResY=720','-ForceRes',f'-BattleHUDReviewDir={capture}']
else:extra+=['-nullrhi']
with (out/'run.log').open('w') as f:
 r=subprocess.run(entry+['/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-RCWebControlDisable','-unattended','-nosound','-BattleSkipTutorial','-BattleSkateAudit','-stdout']+extra,stdout=f,stderr=subprocess.STDOUT,timeout=120)
m=re.findall(r'BattleSkateAudit: (\{[^\n]+\})',(out/'run.log').read_text());data=json.loads(m[-1]) if m else {'passed':False,'missing_report':True}
data.update(exit_code=r.returncode,packaged=not a.editor,real=a.real,log=str(out/'run.log'),visual_review=False,images=[str(capture/(name+'.png')) for name in ['overview','airborne']] if a.review else [])
data['observed_handling']='realistic' if 'BattleSkateHandling: realistic' in (out/'run.log').read_text() else 'arcade'
data['passed']=bool(data['passed'] and r.returncode==0 and data['observed_handling']==('realistic' if a.real else 'arcade') and all(Path(i).is_file() for i in data['images']))
Path(a.report).write_text(json.dumps(data,indent=2)+'\n');print(json.dumps(data));raise SystemExit(0 if data['passed'] else 1)
