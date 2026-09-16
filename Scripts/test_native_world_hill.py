"""Verify downhill momentum on the installed measured Piedmont/10th hillside."""
import json,pathlib,re,subprocess,uuid
root=pathlib.Path(__file__).resolve().parents[1];log=root/'work'/f'world-hill-{uuid.uuid4().hex}.log'
engine='/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd'
with log.open('w') as stream:
 run=subprocess.run([engine,str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-nullrhi','-RCWebControlDisable','-BattleSkipTutorial','-BattleSteeringAudit','-BattleWorldHillAudit','-unattended','-nosound','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=90)
text=log.read_text();rows=re.findall(r'WorldHillAudit: (\{[^\n]+\})',text);samples=[json.loads(x) for x in re.findall(r'WorldHillSample: (\{[^\n]+\})',text)]
report=json.loads(rows[-1]) if rows else {'passed':False,'missing_report':True};report.update(samples=samples,exit_code=run.returncode,log=str(log),source='Installed mapped hillside beside 10th Street; source route OSM 1035389486 over active USGS terrain.',scope='Native current-world coasting at 900 cm/s for 1.5 seconds in Arcade and Real Bike Physics. Traffic and pedestrians disabled; subjective feel and full downhill route remain playtest checks.')
report['passed']=bool(report.get('passed') and run.returncode==0 and len(samples)==2 and all(x['grounded_samples']==x['samples'] for x in samples))
(root/'Tests/Results/2026-09-16-world-hill-momentum.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report));raise SystemExit(0 if report['passed'] else 1)
