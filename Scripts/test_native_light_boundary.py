"""Exercise real automatic-light ticks across body/headlamp boundary positions."""
import json,pathlib,re,subprocess
root=pathlib.Path(__file__).resolve().parents[1]
log=root/'work/light-boundary-audit.log'
with log.open('w') as f:
    run=subprocess.run(['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-nullrhi','-BattleSkipTutorial','-BattleFurnitureAudit','-BattleLightBoundaryAudit','-RCWebControlDisable','-unattended','-nosound','-stdout'],stdout=f,stderr=subprocess.STDOUT,timeout=120)
rows=re.findall(r'LightBoundaryAudit: (\{[^\n]+\})',log.read_text())
result={'exit_code':run.returncode,'checks':json.loads(rows[-1]) if rows else None,'scope':'Native automatic-light ticks at a narrow isolated dark volume: bike body only, headlamp only, delayed exit, daylight off. No rendered illumination or complete Krog route acceptance.'}
profile=re.findall(r'ProjectShadowProfile: (\{[^\n]+\})',log.read_text())
result['shadow_profile']=json.loads(profile[-1]) if profile else None
result['project_shadow_profile_passed']=bool(result['shadow_profile'] and result['shadow_profile']['cascades']==2)
result['passed']=result['project_shadow_profile_passed'] and bool(run.returncode==0 and result['checks'] and result['checks']['passed'])
(root/'Tests/Results/2026-09-13-native-light-boundary.json').write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps(result));raise SystemExit(0 if result['passed'] else 1)
