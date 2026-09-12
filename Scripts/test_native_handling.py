"""Native keyboard steering checks for both handling selections on the main map."""
import argparse,json,pathlib,re,subprocess
p=argparse.ArgumentParser();p.add_argument("--packaged",action="store_true");p.add_argument("--report",default="2026-09-12-native-handling.json");args=p.parse_args();assert pathlib.Path(args.report).name==args.report
root=pathlib.Path(__file__).resolve().parents[1]
entry=[str(root/"Saved/StagedBuilds/Mac/AuraPlayground.app/Contents/MacOS/AuraPlayground")] if args.packaged else ["/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd",str(root/"AuraPlayground.uproject")]
results=[]
for real in (False,True):
 name='realistic' if real else 'arcade'
 log=root/f'work/handling-{name}.log'
 with log.open('w') as stream:
  run=subprocess.run(entry+['/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-nullrhi','-RCWebControlDisable','-BattleSkipTutorial','-BattleSteeringAudit','-unattended','-nosound','-stdout']+(['-BattleRealHandlingAudit'] if real else []),stdout=stream,stderr=subprocess.STDOUT,timeout=90)
 matches=re.findall(r'BattleSteeringAudit: (\{[^\n]+\})',log.read_text())
 result=json.loads(matches[-1]) if matches else {'passed':False,'missing_report':True}
 result.update(mode=name,exit_code=run.returncode);result['passed']=result['passed'] and run.returncode==0;results.append(result)
report={'packaged':args.packaged,'passed':all(r['passed'] for r in results),'runs':results,'scope':'Native main-world keyboard pedal/brake/steer, lean and remount; realistic P toggle on/off. No hill, traction-limit, high-speed, jump or rendered HUD acceptance.'}
(root/'Tests/Results'/args.report).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report));raise SystemExit(0 if report['passed'] else 1)
