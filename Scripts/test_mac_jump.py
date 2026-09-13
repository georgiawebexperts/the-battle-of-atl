"""Check actual bike jumps and airtime rewards."""
import argparse,json,re,subprocess,uuid
from pathlib import Path
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser()
p.add_argument('--difficulty',choices=['Easy','Medium','Hard'],default='Easy')
p.add_argument('--editor',action='store_true')
p.add_argument('--report',required=True)
p.add_argument('--low-speed',action='store_true')
p.add_argument('--crest',action='store_true')
a=p.parse_args()
app=root/'Saved/StagedBuilds/Mac/AuraPlayground.app/Contents/MacOS/AuraPlayground'
entry=['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject')] if a.editor else [str(app)]
log=root/f'work/jump-{uuid.uuid4().hex}.log'
with log.open('w') as stream:
 run=subprocess.run(entry+(['-BattleLowSpeedJumpAudit'] if a.low_speed else [])+(['-BattleCrestJumpAudit'] if a.crest else [])+[f'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty={a.difficulty}?AutoStart=1','-game','-RCWebControlDisable','-nullrhi','-unattended','-nosound','-BattleSkipTutorial','-BattleJumpAudit','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=120)
matches=re.findall(r'BattleJumpAudit: (\{[^\n]+\})',log.read_text())
result=json.loads(matches[-1]) if matches else {'passed':False,'missing_report':True}
result.update(exit_code=run.returncode,difficulty=a.difficulty,visual_review=False,low_speed=a.low_speed,crest=a.crest,executable=entry[0],log=str(log))
result['passed']=bool(result.get('passed') and run.returncode==0)
Path(a.report).write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps(result))
if not result['passed']:raise SystemExit(1)
