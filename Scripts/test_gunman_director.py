"""Native director placement and encounter lifecycle contract."""
import pathlib,subprocess,re,json
root=pathlib.Path(__file__).resolve().parents[1];log=root/'work/gunman-director-audit.log'
with log.open('w') as f:
 run=subprocess.run(['/Volumes/Adam Assets/Unreal/UE_5.8/Engine/Binaries/Mac/UnrealEditor-Cmd',str(root/'AuraPlayground.uproject'),'/Game/PiedmontRide/Maps/PiedmontWorld?Difficulty=Easy?AutoStart=1','-game','-nullrhi','-BattleSkipTutorial','-BattleGunmanDirectorAudit','-unattended','-nosound','-stdout'],stdout=f,stderr=subprocess.STDOUT,timeout=90)
rows=re.findall(r'GunmanDirectorAudit: (\{[^\n]+\})',log.read_text());r=json.loads(rows[-1]) if rows else {'passed':False,'reason':'Missing audit'}
r.update(exit_code=run.returncode,scope='Native director with real nav placement and accelerated director ticks. Full-route balance, render, sound and statistical encounter distribution not accepted.');r['passed']=r['passed'] and run.returncode==0
(root/'Tests/Results/2026-09-12-gunman-director.json').write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r));raise SystemExit(0 if r['passed'] else 1)
