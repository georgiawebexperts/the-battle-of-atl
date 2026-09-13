"""Ride beside and into three authored tree types through normal keyboard input."""
import json,pathlib,re,subprocess,argparse,uuid
p=argparse.ArgumentParser();p.add_argument('--report',default='2026-09-13-native-tree-knockoffs.json');args=p.parse_args();assert pathlib.Path(args.report).name==args.report
root=pathlib.Path(__file__).resolve().parents[1];log=root/('work/tree-ride-'+uuid.uuid4().hex+'.log')
engine='/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd'
with log.open('w') as stream:
 run=subprocess.run([engine,str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-nullrhi','-BattleSkipTutorial','-BattleTreeRideAudit','-unattended','-nosound','-stdout'],stdout=stream,stderr=subprocess.STDOUT,timeout=180)
rows=re.findall(r'TreeRideAudit: (\{[^\n]+\})',log.read_text());r={'exit_code':run.returncode,'checks':json.loads(rows[-1]) if rows else None,'scope':'Native keyboard passes beside and into three authored tree instances, one per mesh type. Nearby pedestrians/zombies removed; does not establish full-path clearance, realistic crash animation, on-foot navigation or packaged performance.'};r['passed']=run.returncode==0 and r['checks'] is not None and r['checks']['passed']
(root/'Tests/Results'/args.report).write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r));raise SystemExit(0 if r['passed'] else 1)
