"""Check actual bike jumps and airtime rewards."""
import argparse,json,re,subprocess,uuid,plistlib
from pathlib import Path
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser()
p.add_argument('--difficulty',choices=['Easy','Medium','Hard'],default='Easy')
p.add_argument('--editor',action='store_true')
p.add_argument('--report',required=True)
p.add_argument('--low-speed',action='store_true')
p.add_argument('--crest',action='store_true')
p.add_argument('--pedal-review',action='store_true')
p.add_argument('--side-review',action='store_true')
p.add_argument('--review',action='store_true')
p.add_argument('--hill',action='store_true')
p.add_argument('--downhill',action='store_true')
p.add_argument('--real',action='store_true')
a=p.parse_args()
if (a.downhill or a.real) and not a.hill:p.error('--downhill/--real require --hill')
app=root/'Saved/StagedBuilds/Mac/AuraPlayground.app/Contents/MacOS/AuraPlayground'
entry=['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject')] if a.editor else [str(app)]
log=root/f'work/jump-{uuid.uuid4().hex}.log'
capture=root/'work'/('hill-jump-review-'+uuid.uuid4().hex)
if a.review:
 if not a.editor:
  bid=plistlib.loads((app.parents[1]/'Info.plist').read_bytes())['CFBundleIdentifier']
  capture=Path.home()/'Library/Containers'/bid/'Data/Documents/BattleJumpReview'/uuid.uuid4().hex
 capture.mkdir(parents=True)
flags=['-RenderOffscreen','-windowed','-ResX=1280','-ResY=720','-ForceRes',f'-BattleJumpReviewDir={capture}'] if a.review else ['-nullrhi']
with log.open('w') as stream:
 run=subprocess.run(entry+(['-BattlePedalReview'] if a.pedal_review else [])+(['-BattleJumpSideReview'] if a.side_review or a.pedal_review else [])+(['-BattleHillJumpAudit'] if a.hill else [])+(['-BattleDownhillJumpAudit'] if a.downhill else [])+(['-BattleRealHillJumpAudit'] if a.real else [])+(['-BattleLowSpeedJumpAudit'] if a.low_speed else [])+(['-BattleCrestJumpAudit'] if a.crest else [])+[f'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty={a.difficulty}?AutoStart=1','-game','-RCWebControlDisable',*flags,'-unattended','-nosound','-BattleSkipTutorial','-BattleJumpAudit','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=120)
matches=re.findall(r'BattleJumpAudit: (\{[^\n]+\})',log.read_text())
result=json.loads(matches[-1]) if matches else {'passed':False,'missing_report':True}
result.update(hill=a.hill,downhill=a.downhill,realistic=a.real,takeoff_diagnostics=re.findall(r'HillJumpTakeoff: ([^\n]+)',log.read_text()),exit_code=run.returncode,difficulty=a.difficulty,visual_review=False,low_speed=a.low_speed,crest=a.crest,executable=entry[0],log=str(log))
result['pedal_contact']=re.findall(r'PedalContactAudit: max_error_cm=([0-9.]+) samples=(\d+) phase_radians=([0-9.]+)',log.read_text())
result['images']=[str(capture/f'{n}.png') for n in ['takeoff','airborne','landing']] if a.review else []
result['captures_complete']=all(Path(x).is_file() for x in result['images'])
result['passed']=bool(result.get('passed') and run.returncode==0 and result['captures_complete'])
Path(a.report).write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps(result))
if not result['passed']:raise SystemExit(1)
