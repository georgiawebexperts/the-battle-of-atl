"""Exercise a late bike cut-in ahead of moving traffic on native 10th Street."""
import json,pathlib,re,subprocess,argparse
root=pathlib.Path(__file__).resolve().parents[1];log=root/'work/moving-car-audit.log'
parser=argparse.ArgumentParser();parser.add_argument('--app',type=pathlib.Path);parser.add_argument('--report',default='2026-09-12-native-moving-car-impact.json');args=parser.parse_args();assert pathlib.Path(args.report).name==args.report
engine='/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd'
launch=[str(args.app)] if args.app else [engine,str(root/'AuraPlayground.uproject')]
with log.open('w') as stream:
 run=subprocess.run(launch+['/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-nullrhi','-BattleSkipTutorial','-BattleBikeCarAudit','-BattleMovingCarImpact','-unattended','-nosound','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=90)
rows=re.findall(r'BikeCarAudit: (\{[^\n]+\})',log.read_text());r={'packaged':bool(args.app),'exit_code':run.returncode,'checks':json.loads(rows[-1]) if rows else None,'scope':'Late cut-in fixture in front of a moving car on the real road. Recovery classification only; animation and injury remain unverified.'};r['passed']=run.returncode==0 and r['checks'] is not None and r['checks']['passed']
(root/'Tests/Results'/args.report).write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r));raise SystemExit(0 if r['passed'] else 1)
