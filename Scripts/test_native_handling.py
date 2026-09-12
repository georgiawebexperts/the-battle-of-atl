"""Native keyboard steering checks for both handling selections on the main map."""
import json,pathlib,re,subprocess
root=pathlib.Path(__file__).resolve().parents[1]
results=[]
for real in (False,True):
 name='realistic' if real else 'arcade'
 log=root/f'work/handling-{name}.log'
 with log.open('w') as stream:
  run=subprocess.run(['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-nullrhi','-RCWebControlDisable','-BattleSkipTutorial','-BattleSteeringAudit','-unattended','-nosound','-stdout']+(['-BattleRealHandlingAudit'] if real else []),stdout=stream,stderr=subprocess.STDOUT,timeout=90)
 matches=re.findall(r'BattleSteeringAudit: (\{[^\n]+\})',log.read_text())
 result=json.loads(matches[-1]) if matches else {'passed':False,'missing_report':True}
 result.update(mode=name,exit_code=run.returncode);result['passed']=result['passed'] and run.returncode==0;results.append(result)
report={'passed':all(r['passed'] for r in results),'runs':results,'scope':'Native main-world keyboard pedal/brake/steer, lean and remount; realistic P toggle on/off. No hill, traction-limit, high-speed, jump, rendered HUD or packaged acceptance.'}
(root/'Tests/Results/2026-09-12-native-handling.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report));raise SystemExit(0 if report['passed'] else 1)
