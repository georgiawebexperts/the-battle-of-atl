"""Verify arcade bike gravity and braking on a controlled native slope."""
import argparse,json,pathlib,re,subprocess,uuid
p=argparse.ArgumentParser();p.add_argument('--packaged',action='store_true');p.add_argument('--report',default='2026-09-16-arcade-downhill.json');a=p.parse_args()
root=pathlib.Path(__file__).resolve().parents[1];log=root/'work'/f'arcade-downhill-{uuid.uuid4().hex}.log'
entry=[str(root/'Saved/StagedBuilds/Mac/AuraPlayground.app/Contents/MacOS/AuraPlayground')] if a.packaged else ['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject')]
with log.open('w') as stream:
 run=subprocess.run([*entry,'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-nullrhi','-RCWebControlDisable','-BattleSkipTutorial','-BattleSteeringAudit','-BattleArcadeDownhillAudit','-unattended','-nosound','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=90)
text=log.read_text();matches=re.findall(r'ArcadeDownhillAudit: (\{[^\n]+\})',text);samples=[json.loads(x) for x in re.findall(r'ArcadeDownhillSample: (\{[^\n]+\})',text)]
report=json.loads(matches[-1]) if matches else {'passed':False,'missing_report':True};report.update(samples=samples,exit_code=run.returncode,packaged=a.packaged,log=str(log),scope='Native arcade movement on a controlled 12-degree plane: downhill coasting, uphill deceleration and downhill braking. World terrain profile and visual feel require separate acceptance.');report['passed']=bool(report.get('passed') and run.returncode==0 and len(samples)==3)
(root/'Tests/Results'/a.report).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report));raise SystemExit(0 if report['passed'] else 1)
