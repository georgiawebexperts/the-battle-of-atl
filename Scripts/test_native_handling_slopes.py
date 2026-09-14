"""Actual bike movement on isolated elevated road/grass planes, not world-route acceptance."""
import json,pathlib,re,subprocess,argparse,uuid
p=argparse.ArgumentParser();p.add_argument("--packaged",action="store_true");p.add_argument("--report",default="2026-09-14-handling-slopes.json");a=p.parse_args()
root=pathlib.Path(__file__).resolve().parents[1];log=root/f'work/handling-slopes-{uuid.uuid4().hex}.log'
with log.open('w') as stream:
 run=subprocess.run(([str(root/'Saved/StagedBuilds/Mac/AuraPlayground.app/Contents/MacOS/AuraPlayground')] if a.packaged else ['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject')])+['/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-nullrhi','-RCWebControlDisable','-BattleSkipTutorial','-BattleSteeringAudit','-BattleHandlingSlopeAudit','-unattended','-nosound','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=90)
text=log.read_text();matches=re.findall(r'HandlingSlopeAudit: (\{[^\n]+\})',text)
report=json.loads(matches[-1]) if matches else {'passed':False,'missing_report':True}
report['samples']=[json.loads(row) for row in re.findall(r'HandlingSlopeSample: (\{[^\n]+\})',text)]
report.update(exit_code=run.returncode,packaged=a.packaged,log=str(log),scope='Native movement on controlled 10-degree incline, flat road/grass and 35-degree grass downhill planes. Initial positions/speeds injected; braking and steering keyboard input. No actual-world hills, visual, tire simulation or whole-route acceptance.')
report['passed']=report['passed'] and run.returncode==0 and len(report['samples'])==9
(root/'Tests/Results'/a.report).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report));raise SystemExit(0 if report['passed'] else 1)
