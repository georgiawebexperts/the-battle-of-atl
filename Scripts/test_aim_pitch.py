"""Render level/up/down third-person pistol aim and verify gun/hand response."""
import json,re,subprocess,uuid,argparse,plistlib
from pathlib import Path
root=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser();parser.add_argument('--packaged',action='store_true');parser.add_argument('--close',action='store_true');parser.add_argument('--weapon',choices=['pistol','shotgun','smg','rifle'],default='pistol');parser.add_argument('--report',default='2026-09-13-third-person-aim-pitch.json');args=parser.parse_args();assert Path(args.report).name==args.report
out=root/'work'/('aim-pitch-'+uuid.uuid4().hex);out.mkdir()
bundle=root/'Saved/StagedBuilds/Mac/AuraPlayground.app'
entry=[str(bundle/'Contents/MacOS/AuraPlayground')] if args.packaged else ['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject')]
capture=out
if args.packaged:
 bid=plistlib.loads((bundle/'Contents/Info.plist').read_bytes())['CFBundleIdentifier']
 capture=Path.home()/'Library/Containers'/bid/'Data/Documents/BattleAimReview'/uuid.uuid4().hex;capture.mkdir(parents=True)
with (out/'run.log').open('w') as log:
 run=subprocess.run([*entry,'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-RenderOffscreen','-windowed','-ResX=1280','-ResY=720','-ForceRes','-unattended','-nosound','-BattleSkipTutorial','-BattleFootAudit','-BattleAimPitchAudit',f'-BattleAimWeapon={dict(pistol=0,shotgun=1,smg=2,rifle=4)[args.weapon]}',f'-BattleHUDReviewDir={capture}','-RCWebControlDisable','-stdout']+(['-BattleAimClose'] if args.close else []),stdout=log,stderr=subprocess.STDOUT,timeout=120)
m=re.findall(r'BattleAimPitchAudit: (\{[^\n]+\})',(out/'run.log').read_text())
r=json.loads(m[-1]) if m else {'passed':False,'missing_report':True}
distance=re.findall(r'BattleAimWeaponDistance: ([0-9.]+)',(out/'run.log').read_text())
r.update(weapon=args.weapon,max_weapon_origin_to_hand_cm=float(distance[-1]) if distance else None,exit_code=run.returncode,images=[str(capture/f'aim-{i}.png') for i in [1,2,3]],log=str(out/'run.log'),visual_review=False,packaged=args.packaged)
r['passed']=bool(r['passed'] and run.returncode==0 and all(Path(p).is_file() for p in r['images']))
(root/'Tests/Results'/args.report).write_text(json.dumps(r,indent=2)+'\n')
print(json.dumps(r));raise SystemExit(0 if r['passed'] else 1)
