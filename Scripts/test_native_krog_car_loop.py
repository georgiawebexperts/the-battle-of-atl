"""Run one native car through the reviewed DeKalb loop, keeping it in view."""
import json,pathlib,re,subprocess
root=pathlib.Path(__file__).resolve().parents[1];log=root/'work/krog-car-loop.log'
with log.open('w') as f:
    run=subprocess.run(['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontKrogTurnaroundReview?Difficulty=Easy?AutoStart=1','-game','-nullrhi','-BattleSkipTutorial','-BattleFurnitureAudit','-BattleCarLoopAudit','-RCWebControlDisable','-unattended','-nosound','-stdout'],stdout=f,stderr=subprocess.STDOUT,timeout=240)
rows=re.findall(r'CarLoopAudit: (\{[^\n]+\})',log.read_text())
j={'exit_code':run.returncode,'checks':json.loads(rows[-1]) if rows else None,'scope':'One native car on real review-map pavement, both turnaround arcs and one complete loop seam; camera follows car. Other cars and pedestrians removed. No multi-car population, rendered motion or main-map acceptance.'}
j['passed']=bool(run.returncode==0 and j['checks'] and j['checks']['passed'])
(root/'Tests/Results/2026-09-13-native-krog-car-loop.json').write_text(json.dumps(j,indent=2)+'\n');print(json.dumps(j));raise SystemExit(0 if j['passed'] else 1)
