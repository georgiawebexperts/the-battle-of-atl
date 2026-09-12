"""Exercise native hazard obstruction, guard and recovery logic. No visual acceptance."""
import argparse,json,pathlib,re,subprocess,uuid
root=pathlib.Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('hazard',choices=['drone','trouble']);p.add_argument('--app',type=pathlib.Path);p.add_argument('--report');a=p.parse_args()
if a.report and pathlib.Path(a.report).name!=a.report:raise SystemExit('Report must be a filename')
name='BattleDroneAudit' if a.hazard=='drone' else 'BattleTroubleAudit'
out=root/'work/hazard-regressions'/uuid.uuid4().hex;out.mkdir(parents=True)
launcher=[str(a.app)] if a.app else ['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject')]
with (out/'run.log').open('w') as log:
 run=subprocess.run([*launcher,'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-BattleSkipTutorial','-nullrhi','-unattended','-nosound','-RCWebControlDisable',f'-{name}','-stdout'],stdout=log,stderr=subprocess.STDOUT,timeout=180)
text=(out/'run.log').read_text();rows=re.findall(name+r': (\{[^\n]+\})',text)
r={'exit_code':run.returncode,'audit':json.loads(rows[-1]) if rows else None,'log':str(out/'run.log'),'runtime':'packaged' if a.app else 'editor-game','scope':'Native headless hazard logic, physical recovery and obstruction fixtures; no rendered appearance or full game acceptance.'};r['passed']=run.returncode==0 and bool(r['audit']) and r['audit']['passed']
(root/'Tests/Results'/(a.report or f'2026-09-12-native-{a.hazard}-physical-regression.json')).write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r));raise SystemExit(0 if r['passed'] else 1)
