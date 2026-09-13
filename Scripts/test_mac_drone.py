"""Native drone warning, collision, recovery and obstruction checks."""
import argparse,json,re,subprocess,uuid
from pathlib import Path
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--difficulty',choices=['Easy','Medium','Hard'],default='Easy');p.add_argument('--editor',action='store_true');p.add_argument('--report');a=p.parse_args()
app=root/'Saved/StagedBuilds/Mac/AuraPlayground.app/Contents/MacOS/AuraPlayground'
log=root/f'work/drone-{uuid.uuid4().hex}.log'
entry=['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject'),'-game','-RCWebControlDisable'] if a.editor else [str(app)]
with log.open('w') as stream:
 run=subprocess.run([*entry,f'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty={a.difficulty}?AutoStart=1','-nullrhi','-unattended','-nosound','-BattleDroneAudit','-BattleSkipTutorial','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=90)
matches=re.findall(r'BattleDroneAudit: (\{[^\n]+\})',log.read_text());result=json.loads(matches[-1]) if matches else {'passed':False,'missing_report':True}
result.update(exit_code=run.returncode,difficulty=a.difficulty,visual_review=False,editor=a.editor,log=str(log));result['passed']=bool(result.get('passed') and run.returncode==0)
report=a.report or f'2026-09-11-build036-drone-{a.difficulty.lower()}.json';assert Path(report).name==report
(root/'Tests/Results'/report).write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result))
if not result['passed']:raise SystemExit(1)
